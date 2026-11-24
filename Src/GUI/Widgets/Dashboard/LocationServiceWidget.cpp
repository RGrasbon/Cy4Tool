//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Dienstort                                                     //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QRegularExpressionValidator>
#include "Conf.h"
#include "LocationServiceWidget.h"
#include "Global.h"
#include "KeyValues.h"

//////////////////////////////////////////////////////////////////////////////////////////
static ByteArray
serialize(Cy4Location location)
{
    UInt32 value = location.isValid() ? UInt32(location.toInteger()) : 0xFFFFFF;
    ByteArray data(3);
    data[0] = UInt08(value >> 16);
    data[1] = UInt08(value >>  8);
    data[2] = UInt08(value >>  0);
    return data;
}

//////////////////////////////////////////////////////////////////////////////////////////
static Cy4Location
deserialize(const ByteArray& data)
{
    if (data.size() != 3) return Cy4Location();
    UInt32 value = 0;
    value += UInt32(data[0]) << 16;
    value += UInt32(data[1]) <<  8;
    value += UInt32(data[2]) <<  0;
    return Cy4Location(int(value));
}

//////////////////////////////////////////////////////////////////////////////////////////
void
LocationServiceWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {   // Ursprünglichen Inhalt wiederherstellen
        populateWidget();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
LocationServiceWidget::onTextChanged(void)
{
    int cursorPosition = mLineEdit->cursorPosition();
    mLineEdit->setText(mLineEdit->text().toUpper());
    mLineEdit->setCursorPosition(cursorPosition);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
LocationServiceWidget::applyChanges(void)
{
    Cy4Location location = Cy4Location(mLineEdit->text().toStdString());
    ByteArray widgetData = ::serialize(location);
    mServiceProxy.write(0, widgetData);
    emit dataChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
LocationServiceWidget::LocationServiceWidget(const ServiceProxy& serviceProxy,
                                             const QJsonObject&,
                                             QWidget*            parent)
                     : AbstractServiceWidget(serviceProxy, parent)
{
    // Dienst
    {   if (serviceProxy.access() == Service::WriteOnly)
        {   mIssue = tr("Write only access not supported for widget \"%1\"").arg(KEY::LOCATION);
        }
        else if (mServiceProxy.size() != 3)
        {   mIssue = tr("Service size must be exactly %1").arg(3);
        }
    }
    // Steuerelement
    {   QVBoxLayout* verticalLayout = new QVBoxLayout();
        verticalLayout->setContentsMargins(1, 1, 1, 1);
        QHBoxLayout* horizontalLayout = new QHBoxLayout();
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setSpacing(0);
        mLineEdit = new QLineEdit(this);
        mLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
        mLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mLineEdit->setAlignment(Qt::AlignRight);
        QRegularExpression regExp = QRegularExpression("[0-9a-fA-F]{,6}");
        mLineEdit->setValidator(new QRegularExpressionValidator(regExp, this));
        connect(mLineEdit, &QLineEdit::editingFinished, this, &LocationServiceWidget::applyChanges);
        connect(mLineEdit, &QLineEdit::textChanged    , this, &LocationServiceWidget::onTextChanged);
        horizontalLayout->addWidget(mLineEdit, 0, Qt::AlignCenter);
        horizontalLayout->addWidget(new QLabel("h"), 0, Qt::AlignCenter);
        verticalLayout->addLayout(horizontalLayout);
        setLayout(verticalLayout);
        int width = Global::calculateWidthFor("888888", mLineEdit);
        mLineEdit->setFixedWidth(width);
        if (mServiceProxy.access() == Service::ReadOnly)
        {   mLineEdit->setReadOnly(true);
            mLineEdit->setAttribute(Qt::WA_TransparentForMouseEvents);
            mLineEdit->setFocusPolicy(Qt::NoFocus);
            mLineEdit->setStyleSheet(CFG::BackgroundForReadOnly);
        }
        mLineEdit->setEnabled(mIsOperational);
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
LocationServiceWidget::toClipboardText(void)
{
    return QString("%1h").arg(mLineEdit->text());
}

//////////////////////////////////////////////////////////////////////////////////////////
void
LocationServiceWidget::populateWidget(void)
{
    if (mServiceProxy.isUpToDate())
    {   ByteArray data = mServiceProxy.read();
        Cy4Location location = ::deserialize(data);
        if (location.isValid()) mLineEdit->setText(location.toText().c_str());
        else                    mLineEdit->setText("------");
    }
    else
    {   mLineEdit->clear();
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////

