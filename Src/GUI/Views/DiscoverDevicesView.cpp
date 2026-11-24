//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Gerätefindungsansicht                                                               //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QIntValidator>
#include <QKeyEvent>
#include "ui_DiscoverDevicesView.h"
#include "MainFrame.h"
#include "DiscoverDevicesView.h"
#include "Cy4Net.h"
#include "DeviceList.h"
#include "Settings.h"

//////////////////////////////////////////////////////////////////////////////////////////
static int
currentNetAddress(const QComboBox* comboBox)
{
    bool ok;
    int net = comboBox->currentText().toInt(&ok);
    if (!ok || net < 1 || net > 255) net = INVALID;
    return net;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DiscoverDevicesView::onTextChanged(void)
{
    int net = currentNetAddress(mUi->netAddressCombo);
    mUi->startStopButton->setEnabled(net != INVALID);
    mUi->selectLandingPageButton->setEnabled(currentNetAddress(mUi->netAddressCombo) != 255);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DiscoverDevicesView::onSelectLandingPageButton(void)
{
    mUi->netAddressCombo->setCurrentText("255");
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DiscoverDevicesView::onDiscover(void)
{
    if (mDiscoveryInProgress)
    {   mDiscoveryInProgress = false;
    }
    else
    {   int netAddress = currentNetAddress(mUi->netAddressCombo);
        if (netAddress == INVALID) return;
        mDiscoveryInProgress = true;
        settings().lruDiscoveryNetAddressCache.use(netAddress);
        mUi->startStopButton->setText(tr("Stop"));
        // Gerätefindung durchführen
        int discovered = 0;
        mUi->netAddressCombo->setEnabled(false);
        mUi->selectLandingPageButton->setEnabled(false);
        for (int devAddress = 1; devAddress<255; devAddress++)
        {   mUi->progressBar->setValue(devAddress);
            Cy4Address address = Cy4Address(netAddress, devAddress);
            QString text = tr("Probe %1, discovered: %2").arg(address.toText().c_str()).arg(discovered);
            mainFrame().showStatusBarInfo(text);
            int index = deviceList().indexOf(address);
            if (index != INVALID) deviceList().setBusyFlag(index);
            Cy4TypePlate typePlate = cy4Net().fetchTypePlate(address);
            if (typePlate.isValid())
            {   Cy4Device device = Cy4Device(address, typePlate);
                if (index == INVALID)
                {   deviceList().add(device.address());
                    discovered++;
                }
                else
                {   deviceList().fetch(index);
                }
            }
            else if (!cy4Net().gateway().isValid())
            {   mDiscoveryInProgress = false;
            }
            if (index != INVALID) deviceList().clearBusyFlag(index);
            if (!mDiscoveryInProgress)
            {   if (cy4Net().gateway().isValid())
                {   mainFrame().showStatusBarInfo(tr("Discovery stopped"));
                }
                else
                {   mainFrame().showStatusBarWarning(tr("Gateway lost"));
                }
                break;
            }
        }
        if (mDiscoveryInProgress)
        {   mDiscoveryInProgress = false;
            QString text = tr("Discovery completed, discovered: %1").arg(discovered);
            mainFrame().showStatusBarInfo(text);
        }
        mUi->netAddressCombo->setEnabled(true);
        mUi->selectLandingPageButton->setEnabled(true);
        mUi->startStopButton->setText(tr("Start"));
        mUi->progressBar->setValue(0);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DiscoverDevicesView::onClose(void)
{
    hide();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DiscoverDevicesView::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    if (key == Qt::Key_Escape) hide();
    if (key == Qt::Key_Return) onDiscover();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DiscoverDevicesView::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) mUi->retranslateUi(this);
    QWidget::changeEvent(event);
}

//////////////////////////////////////////////////////////////////////////////////////////
DiscoverDevicesView::DiscoverDevicesView(QWidget* parent) : QWidget(parent)
{
    // GUI
    {   mUi = new Ui::DiscoverDevicesView;
        mUi->setupUi(this);
    }
    // Ereignisverknüpfungen
    {   connect(mUi->netAddressCombo        , &QComboBox::editTextChanged, this, &DiscoverDevicesView::onTextChanged);
        connect(mUi->selectLandingPageButton, &QPushButton::clicked      , this, &DiscoverDevicesView::onSelectLandingPageButton);
        connect(mUi->startStopButton        , &QPushButton::clicked      , this, &DiscoverDevicesView::onDiscover);
        connect(mUi->closeButton            , &QPushButton::clicked      , this, &DiscoverDevicesView::onClose);
    }
    mUi->netAddressCombo->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9][0-9][0-9]")));
}

//////////////////////////////////////////////////////////////////////////////////////////
DiscoverDevicesView::~DiscoverDevicesView()
{
    delete mUi->netAddressCombo->validator();
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DiscoverDevicesView::show(void)
{
    if (!QWidget::isVisible())
    {   for (int i=0; i<settings().lruDiscoveryNetAddressCache.count(); i++)
        {   QString text = QString("%1").arg(settings().lruDiscoveryNetAddressCache.at(i));
            mUi->netAddressCombo->addItem(text);
        }
        mUi->progressBar->setValue(0);
        mUi->startStopButton->setText(QObject::tr("Start"));
        mUi->netAddressCombo->setFocus();
        int net = currentNetAddress(mUi->netAddressCombo);
        mUi->selectLandingPageButton->setEnabled(net != 255);
        QWidget::show();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DiscoverDevicesView::hide(void)
{
    if (QWidget::isVisible())
    {   mDiscoveryInProgress = false;
        int net = currentNetAddress(mUi->netAddressCombo);
        if (net != INVALID) settings().lruDiscoveryNetAddressCache.use(net);
        mUi->netAddressCombo->clear();
        QWidget::hide();
        emit hided();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
