//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Auslöser                                                      //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QPushButton>
#include "AbstractServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
class TriggerServiceWidget : public AbstractServiceWidget
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
        QPushButton* mPushButton;
        ValueDomain  mValueDomain;
        SInt32       mTriggerValue;
        QString      mConfirm;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void  applyChanges(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                  TriggerServiceWidget(const ServiceProxy&,
                                       const QJsonObject&,
                                       QWidget* = nullptr);
        virtual  ~TriggerServiceWidget() = default;
		QString   toClipboardText(void) override;		
        void      populateWidget(void) override;        
};

//////////////////////////////////////////////////////////////////////////////////////////


