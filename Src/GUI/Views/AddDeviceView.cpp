//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Gerät hinzufügen Ansicht                                                            //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QKeyEvent>
#include "ui_AddDeviceView.h"
#include "AddDeviceView.h"
#include "MainFrame.h"
#include "DeviceList.h"

//////////////////////////////////////////////////////////////////////////////////////////
void
AddDeviceView::onAdd(void)
{
    Cy4Address address = Cy4Address(mUi->addressEdit->text().toStdString());
    if (!address.isValid())
    {   QString text = tr("Invalid address");
        mainFrame().showStatusBarWarning(text);
    }
    if (address.isBroadcast())
    {   QString text = tr("%1 is a broadcast address").arg(address.toText().c_str());
        mainFrame().showStatusBarWarning(text);
    }
    else
    {   int index = deviceList().indexOf(address);
        if (index != INVALID && deviceList().at(index).isValid())
        {   QString text = tr("%1 is occupied by another device")
                              .arg(address.toText().c_str());
            mainFrame().showStatusBarWarning(text);
        }
        else
        {   deviceList().add(address);
            QString text;
            if (index == INVALID) text = tr("New device added at %1").arg(address.toText().c_str());
            else                  text = tr("Device at %1 already exists").arg(address.toText().c_str());
            mainFrame().showStatusBarInfo(text);
            hide();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
AddDeviceView::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    if (key == Qt::Key_Escape) hide();
    if (key == Qt::Key_Return) onAdd();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
AddDeviceView::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) mUi->retranslateUi(this);
    QWidget::changeEvent(event);
}

//////////////////////////////////////////////////////////////////////////////////////////
AddDeviceView::AddDeviceView(QWidget* parent) : QWidget(parent)
{
    mUi = new Ui::AddDeviceView;
    mUi->setupUi(this);
    connect(mUi->addButton  , &QPushButton::clicked, this, &AddDeviceView::onAdd);
    connect(mUi->closeButton, &QPushButton::clicked, this, &AddDeviceView::hide);
}

//////////////////////////////////////////////////////////////////////////////////////////
AddDeviceView::~AddDeviceView()
{
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
AddDeviceView::show(void)
{
    if (!QWidget::isVisible())
    {   mUi->addressEdit->setFocus();
        QWidget::show();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
AddDeviceView::hide(void)
{
    if (QWidget::isVisible())
    {   mUi->addressEdit->clear();
        QWidget::hide();
        emit hided();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
