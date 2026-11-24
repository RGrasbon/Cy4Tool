//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Wechselschalter                                               //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QKeyEvent>
#include <QAbstractitemView>
#include "Global.h"
#include "ToggleServiceWidget.h"
#include "KeyValues.h"

//////////////////////////////////////////////////////////////////////////////////////////
void
ToggleServiceWidget::applyChanges(void)
{
    if (!mConfirm.isEmpty())
    {   if (QMessageBox::question(this, APPL_NAME, mConfirm) != QMessageBox::Yes) return;
    }
    ByteArray widgetData;
    if (mCheckBox->isChecked()) widgetData = serialize(mOnValue);
    else                        widgetData = serialize(mOffValue);
    mServiceProxy.write(0, widgetData);
    emit dataChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
ToggleServiceWidget::ToggleServiceWidget(const ServiceProxy& serviceProxy,
                                         const QJsonObject&  jsonObject,
                                         QWidget*            parent)
                   : AbstractServiceWidget(serviceProxy, parent)
{
    // Dienst
    {   if (serviceProxy.access() == Service::WriteOnly)
        {   mIssue = tr("Write only access not supported for widget \"%1\"").arg(KEY::TOGGLE);
            mIsOperational = false;
        }
        else if (mServiceProxy.size() > 4)
        {   mIssue = tr("Service size must be 1, 2, 3 or 4");
            mIsOperational = false;
        }
        mValueDomain = toValueDomain(jsonObject);
        mOffValue = toNumber(jsonObject.value(KEY::OFF_VALUE), mValueDomain, 0);
        mOnValue  = toNumber(jsonObject.value(KEY::ON_VALUE), mValueDomain, 1);
        mConfirm = Global::jsonLanguageEntry(jsonObject.value(KEY::CONFIRM));
    }
    // Steuerelement
    {   QVBoxLayout* verticalLayout = new QVBoxLayout();
        verticalLayout->setContentsMargins(1, 1, 1, 1);
        mCheckBox = new QCheckBox(this);
        connect(mCheckBox, &QCheckBox::toggled, this, &ToggleServiceWidget::applyChanges);
        verticalLayout->addWidget(mCheckBox, 0, Qt::AlignCenter);
        setLayout(verticalLayout);
        if (serviceProxy.access() == Service::ReadOnly || !mIsOperational)
        {   mCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents);
            mCheckBox->setFocusPolicy(Qt::NoFocus);
        }
        mCheckBox->setEnabled(mIsOperational);
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
ToggleServiceWidget::toClipboardText(void)
{
    SInt32 value;
    if (mCheckBox->isChecked()) value = mOnValue;
    else                        value = mOffValue;
    switch (mValueDomain)
    {   case ValueDomain::Signed:   return QString::number(value);
        case ValueDomain::Unsigned: return QString::number(static_cast<UInt32>(value));
        default:                    ;
    }
    return QString();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ToggleServiceWidget::populateWidget(void)
{
    if (mServiceProxy.isUpToDate())
    {   ByteArray data = mServiceProxy.read();
        SInt32 value = deserialize(data, mValueDomain);
        bool blocked = mCheckBox->signalsBlocked();
        mCheckBox->blockSignals(true);
        if      (value == mOnValue)  mCheckBox->setChecked(true);
        else if (value == mOffValue) mCheckBox->setChecked(false);
        else
        {   mIssue = tr("Unexpected toggle value: %1, must be %2 or %3").arg(value)
                                                                        .arg(mOnValue)
                                                                        .arg(mOffValue);
        }
        mCheckBox->blockSignals(blocked);
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////

