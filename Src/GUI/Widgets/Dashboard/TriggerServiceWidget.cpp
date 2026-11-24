//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Auslöser                                                      //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QVBoxLayout>
#include <QMessageBox>
#include "TriggerServiceWidget.h"
#include "Global.h"
#include "KeyValues.h"

//////////////////////////////////////////////////////////////////////////////////////////
void
TriggerServiceWidget::applyChanges(void)
{
    if (!mConfirm.isEmpty())
    {   if (QMessageBox::question(this, APPL_NAME, mConfirm) != QMessageBox::Yes) return;
    }
    ByteArray widgetData = serialize(mTriggerValue);
    mServiceProxy.write(0, widgetData);
    emit dataChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
TriggerServiceWidget::TriggerServiceWidget(const ServiceProxy& serviceProxy,
                                           const QJsonObject& jsonObject,
                                           QWidget* parent)
                    : AbstractServiceWidget(serviceProxy, parent)
{
    // Dienst
    {   if (serviceProxy.access() != Service::WriteOnly)
        {   mIssue = tr("Widget \"%1\" must be write only").arg(KEY::TRIGGER);
            mIsOperational = false;
        }
        if (mServiceProxy.size() > 4)
        {   mIssue = tr("Service size must be 1, 2, 3 or 4");
            mIsOperational = false;
        }
        mValueDomain  = toValueDomain(jsonObject);
        mTriggerValue = toNumber(jsonObject.value(KEY::TRIGGER_VALUE), mValueDomain, 0x5E);
        mConfirm = Global::jsonLanguageEntry(jsonObject.value(KEY::CONFIRM));
    }
    // Steuerelement
    {   QVBoxLayout* verticalLayout = new QVBoxLayout();
        verticalLayout->setContentsMargins(1, 1, 1, 1);
        mPushButton = new QPushButton(this);
        verticalLayout->addWidget(mPushButton, 0, Qt::AlignCenter);
        setLayout(verticalLayout);
        QIcon icon;
        icon.addFile(QStringLiteral(":/Symbols/Trigger_A.png"), QSize(), QIcon::Active, QIcon::On);
        icon.addFile(QStringLiteral(":/Symbols/Trigger_A.png"), QSize(), QIcon::Disabled, QIcon::Off);
        mPushButton->setIcon(icon);
        connect(mPushButton, &QPushButton::pressed, this, &TriggerServiceWidget::applyChanges);
        if (!mIsOperational)
        {   mPushButton->setAttribute(Qt::WA_TransparentForMouseEvents);
            mPushButton->setFocusPolicy(Qt::NoFocus);
        }
        mPushButton->setEnabled(mIsOperational);
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
TriggerServiceWidget::toClipboardText(void)
{
    switch (mValueDomain)
    {   case ValueDomain::Signed:   return QString::number(mTriggerValue);
        case ValueDomain::Unsigned: return QString::number(static_cast<UInt32>(mTriggerValue));
        default:                    ;
    }
    return QString();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TriggerServiceWidget::populateWidget(void)
{
    // ... "Write only"
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////

