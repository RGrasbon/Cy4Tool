//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Geräteadresse ändern Ansicht                                                        //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QKeyEvent>
#include "ui_ChangeAddressView.h"
#include "ChangeAddressView.h"
#include "MainFrame.h"
#include "Cy4Net.h"
#include "DeviceList.h"

//////////////////////////////////////////////////////////////////////////////////////////
static Cy4Address
defaultLandingPageAddress(const Cy4Device& device)
{
    int net = 255;
    const Cy4BoardType& boardType = device.typePlate().boardType();
    int dev = 100 + UInt32(boardType.toInteger()) % 100;
    return Cy4Address(net, dev);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ChangeAddressView::validateAddress(void)
{
    bool accepted = true;
    QString addressText = mUi->addressEdit->text();
    if (addressText == ".")
    {   mUi->changeButton->setEnabled(false);
    }
    else
    {   Cy4Address address = Cy4Address(addressText.toStdString());
        if (!address.isValid())
        {   QString text = tr("Invalid address");
            mainFrame().showStatusBarWarning(text);
            accepted = false;
        }
        else if (address.isBroadcast())
        {   QString text = tr("%1 is a broadcast address").arg(address.toText().c_str());
            mainFrame().showStatusBarWarning(text);
            accepted = false;
        }
        else
        {   int newIndex = deviceList().indexOf(address);
            if (newIndex != INVALID && deviceList().at(newIndex).isValid())
            {   QString text = tr("%1 is occupied by another device").arg(address.toText().c_str());
                mainFrame().showStatusBarWarning(text);
                accepted = false;
            }
            mUi->addressEdit->setText(address.toText().c_str());
        }
        mUi->changeButton->setEnabled(accepted);
    }
    if (accepted) mainFrame().clearStatusBarMessage();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ChangeAddressView::onPresetDefault(void)
{
    Cy4Address address = defaultLandingPageAddress(mDevice);
    if (address.isValid()) mUi->addressEdit->setText(address.toText().c_str());
    validateAddress();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ChangeAddressView::onTextChanged(void)
{
    Cy4Address newAddress = Cy4Address(mUi->addressEdit->text().toStdString());
    mUi->presetDefaultButton->setEnabled(newAddress != defaultLandingPageAddress(mDevice));
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ChangeAddressView::onEditingFinished(void)
{
    validateAddress();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ChangeAddressView::onChange(void)
{
    Cy4Address currentAddress = mDevice.address();
    Cy4Address newAddress = Cy4Address(mUi->addressEdit->text().toStdString());
    if (newAddress == currentAddress)
    {   hide();
    }
    else
    {   int currentIndex = deviceList().indexOf(currentAddress);
        bool isNew = currentIndex != INVALID ? deviceList().isNew(currentIndex) : false;
        QString text;
        if (currentIndex != INVALID && cy4Net().changeCy4Address(currentAddress, newAddress))
        {   deviceList().remove(currentIndex);
            int newIndex = deviceList().indexOf(newAddress);
            if (newIndex != INVALID) deviceList().remove(newIndex);
            deviceList().add(newAddress);
            if (!isNew)
            {   int index = deviceList().indexOf(newAddress);
                deviceList().clearNewFlag(index);
            }
            text = tr("Device address now %1").arg(newAddress.toText().c_str());
            if (currentAddress == cy4Net().gateway()) cy4Net().callGateway();
        }
        else
        {   text = tr("Change address failed").arg(newAddress.toText().c_str());
        }
        mainFrame().showStatusBarInfo(text);
        hide();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ChangeAddressView::onClose(void)
{
    hide();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ChangeAddressView::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    if (key == Qt::Key_Escape)
    {   hide();
    }
    if (key == Qt::Key_Return)
    {   validateAddress();
        if (mUi->changeButton->isEnabled()) onChange();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ChangeAddressView::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) mUi->retranslateUi(this);
    QWidget::changeEvent(event);
}

//////////////////////////////////////////////////////////////////////////////////////////
ChangeAddressView::ChangeAddressView(QWidget* parent) : QWidget(parent)
{
    mUi = new Ui::ChangeAddressView;
    mUi->setupUi(this);
    connect(mUi->presetDefaultButton, &QPushButton::clicked      , this, &ChangeAddressView::onPresetDefault);
    connect(mUi->addressEdit        , &QLineEdit::textChanged    , this, &ChangeAddressView::onTextChanged);
    connect(mUi->addressEdit        , &QLineEdit::editingFinished, this, &ChangeAddressView::onEditingFinished);
    connect(mUi->changeButton       , &QPushButton::clicked      , this, &ChangeAddressView::onChange);
    connect(mUi->closeButton        , &QPushButton::clicked      , this, &ChangeAddressView::onClose);
}

//////////////////////////////////////////////////////////////////////////////////////////
ChangeAddressView::~ChangeAddressView()
{
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ChangeAddressView::show(const Cy4Device& device)
{
    mDevice = device;
    if (!QWidget::isVisible())
    {   mUi->addressEdit->setText(mDevice.address().toText().c_str());
        mUi->addressEdit->setFocus();
        QWidget::show();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ChangeAddressView::hide(void)
{
    if (QWidget::isVisible())
    {   mUi->addressEdit->clear();
        QWidget::hide();
        emit hided();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
