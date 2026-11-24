//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Geräteadresse                                                 //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QVBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include "Conf.h"
#include "AddressServiceWidget.h"
#include "Global.h"
#include "KeyValues.h"

//////////////////////////////////////////////////////////////////////////////////////////
static ByteArray
serialize(Cy4Address address)
{
    ByteArray data(2);
    if (address.isValid())
    {   data[0] = address.net();
        data[1] = address.dev();
    }
    else
    {   data[0] = 0xFF;
        data[1] = 0xFF;
    }
    return data;
}

//////////////////////////////////////////////////////////////////////////////////////////
static Cy4Address
deserialize(const ByteArray& data)
{
    if (data.size() != 2) return Cy4Address();
    return Cy4Address(data[0], data[1]);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
AddressServiceWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {   // Ursprünglichen Inhalt wiederherstellen
        populateWidget();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
AddressServiceWidget::applyChanges(void)
{
    Cy4Address address = Cy4Address(mLineEdit->text().toStdString());
    ByteArray widgetData = ::serialize(address);
    mServiceProxy.write(0, widgetData);
    emit dataChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
AddressServiceWidget::AddressServiceWidget(const ServiceProxy& serviceProxy,
                                           const QJsonObject&,
                                           QWidget*            parent)
                    : AbstractServiceWidget(serviceProxy, parent)
{
    // Dienst
    {   if (serviceProxy.access() == Service::WriteOnly)
        {   mIssue = tr("Write only access not supported for widget \"%1\"").arg(KEY::ADDRESS);
            mIsOperational = false;
        }
        else if (mServiceProxy.size() != 2)
        {   mIssue = tr("Service size must be exactly %1").arg(2);
            mIsOperational = false;
        }
    }
    // Steuerelement
    {   QVBoxLayout* verticalLayout = new QVBoxLayout();
        verticalLayout->setContentsMargins(1, 1, 1, 1);
        mLineEdit = new QLineEdit(this);
        mLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
        mLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mLineEdit->setAlignment(Qt::AlignCenter);
        mLineEdit->setInputMask("000.000;_");
        connect(mLineEdit, &QLineEdit::editingFinished, this, &AddressServiceWidget::applyChanges);
        verticalLayout->addWidget(mLineEdit, 0, Qt::AlignCenter);
        setLayout(verticalLayout);
        int width = Global::calculateWidthFor("888.888", mLineEdit);
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
AddressServiceWidget::toClipboardText(void)
{
    return mLineEdit->text();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
AddressServiceWidget::populateWidget(void)
{
    if (mServiceProxy.isUpToDate())
    {   ByteArray data = mServiceProxy.read();
        Cy4Address address = ::deserialize(data);
        if (address.isValid()) mLineEdit->setText(address.toText().c_str());
        else                   mLineEdit->setText("---.---");
    }
    else
    {   mLineEdit->clear();
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////

