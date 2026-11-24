//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Verweis                                                       //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include "Conf.h"
#include "LinkServiceWidget.h"
#include "Global.h"
#include "KeyValues.h"

//////////////////////////////////////////////////////////////////////////////////////////
struct Link
{   Cy4Address   address;
    Cy4Location  location;
};

//////////////////////////////////////////////////////////////////////////////////////////
static ByteArray
serialize(const Link& link)
{
    Cy4Address address = link.address;
    ByteArray data(5);
    if (address.isValid())
    {   data[0] = address.net();
        data[1] = address.dev();
    }
    else
    {   data[0] = 0xFF;
        data[1] = 0xFF;
    }
    Cy4Location location = link.location;
    UInt32 value = location.isValid() ? UInt32(location.toInteger()) : 0xFFFFFF;
    data[2] = UInt08(value >> 16);
    data[3] = UInt08(value >>  8);
    data[4] = UInt08(value >>  0);
    return data;
}

//////////////////////////////////////////////////////////////////////////////////////////
static Link
deserialize(const ByteArray& data)
{
    if (data.size() != 5) return Link();
    Link link;
    link.address = Cy4Address(data[0], data[1]);
    UInt32 value = 0;
    value += UInt32(data[2]) << 16;
    value += UInt32(data[3]) <<  8;
    value += UInt32(data[4]) <<  0;
    link.location = Cy4Location(int(value));
    return link;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
LinkServiceWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {   // Ursprünglichen Inhalt wiederherstellen
        populateWidget();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
LinkServiceWidget::onLocationTextChanged(void)
{
    int cursorPosition = mLocationLineEdit->cursorPosition();
    mLocationLineEdit->setText(mLocationLineEdit->text().toUpper());
    mLocationLineEdit->setCursorPosition(cursorPosition);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
LinkServiceWidget::applyChanges(void)
{
    Link link;
    link.address  = Cy4Address(mAddressLineEdit->text().toStdString());
    link.location = Cy4Location(mLocationLineEdit->text().toStdString());
    ByteArray widgetData = ::serialize(link);
    mServiceProxy.write(0, widgetData);
    emit dataChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
LinkServiceWidget::LinkServiceWidget(const ServiceProxy& serviceProxy,
                                     const QJsonObject&,
                                     QWidget*            parent)
                 : AbstractServiceWidget(serviceProxy, parent)
{
    // Dienst
    {   if (serviceProxy.access() == Service::WriteOnly)
        {   mIssue = tr("Write only access not supported for widget \"%1\"")
                        .arg(KEY::LOCATION);
            mIsOperational = false;
        }
        else if (mServiceProxy.size() != 5)
        {   mIssue = tr("Service size must be exactly %1").arg(5);
            mIsOperational = false;
        }
    }
    // Steuerelement
    {   QVBoxLayout* verticalLayout = new QVBoxLayout();
        verticalLayout->setContentsMargins(1, 1, 1, 1);
        QHBoxLayout* horizontalLayout = new QHBoxLayout();
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setSpacing(0);
        // Addressfeld
        {   mAddressLineEdit = new QLineEdit(this);
            mAddressLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
            mAddressLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            mAddressLineEdit->setAlignment(Qt::AlignCenter);
            mAddressLineEdit->setInputMask("000.000;_");
            connect(mAddressLineEdit, &QLineEdit::editingFinished, this, &LinkServiceWidget::applyChanges);
            verticalLayout->addWidget(mAddressLineEdit, 0, Qt::AlignCenter);
            setLayout(verticalLayout);
            int width = Global::calculateWidthFor("888.888", mAddressLineEdit);
            mAddressLineEdit->setFixedWidth(width);
            if (mServiceProxy.access() == Service::ReadOnly)
            {   mAddressLineEdit->setReadOnly(true);
                mAddressLineEdit->setAttribute(Qt::WA_TransparentForMouseEvents);
                mAddressLineEdit->setFocusPolicy(Qt::NoFocus);
                mAddressLineEdit->setStyleSheet(CFG::BackgroundForReadOnly);
            }
            horizontalLayout->addWidget(mAddressLineEdit, 0, Qt::AlignCenter);
            mAddressLineEdit->setEnabled(mIsOperational);
        }
        horizontalLayout->addWidget(new QLabel(":"), 0, Qt::AlignCenter);
        // Dienstort-Feld
        {   mLocationLineEdit = new QLineEdit(this);
            mLocationLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
            mLocationLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            mLocationLineEdit->setAlignment(Qt::AlignRight);
            int width = Global::calculateWidthFor("888888", mLocationLineEdit);
            mLocationLineEdit->setFixedWidth(width);
            if (mServiceProxy.access() == Service::ReadOnly)
            {   mLocationLineEdit->setReadOnly(true);
                mLocationLineEdit->setAttribute(Qt::WA_TransparentForMouseEvents);
                mLocationLineEdit->setFocusPolicy(Qt::NoFocus);
                mLocationLineEdit->setStyleSheet(CFG::BackgroundForReadOnly);
            }
            QRegularExpression regExp = QRegularExpression("[0-9a-fA-F]{,6}");
            mLocationLineEdit->setValidator(new QRegularExpressionValidator(regExp, this));
            connect(mLocationLineEdit, &QLineEdit::editingFinished, this, &LinkServiceWidget::applyChanges);
            connect(mLocationLineEdit, &QLineEdit::textChanged    , this, &LinkServiceWidget::onLocationTextChanged);
            horizontalLayout->addWidget(mLocationLineEdit, 0, Qt::AlignCenter);
            horizontalLayout->addWidget(new QLabel("h"), 0, Qt::AlignCenter);
            mLocationLineEdit->setEnabled(mIsOperational);
        }
        verticalLayout->addLayout(horizontalLayout);
        setLayout(verticalLayout);
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
LinkServiceWidget::toClipboardText(void)
{
    return QString("%1:%2h").arg(mAddressLineEdit->text(),mLocationLineEdit->text());
}

//////////////////////////////////////////////////////////////////////////////////////////
void
LinkServiceWidget::populateWidget(void)
{
    if (mServiceProxy.isUpToDate())
    {   ByteArray data = mServiceProxy.read();
        Link link = ::deserialize(data);
        Cy4Address address = link.address;
        if (address.isValid()) mAddressLineEdit->setText(address.toText().c_str());
        else                   mAddressLineEdit->setText("---.---");
        Cy4Location location = link.location;
        if (location.isValid()) mLocationLineEdit->setText(location.toText().c_str());
        else                    mLocationLineEdit->setText("------");
    }
    else
    {   mAddressLineEdit->clear();
        mLocationLineEdit->clear();
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////

