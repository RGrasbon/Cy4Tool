//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Wechselschalter                                               //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QCheckBox>
#include "AbstractServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
class ToggleServiceWidget : public AbstractServiceWidget
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
        QCheckBox*  mCheckBox;
        ValueDomain mValueDomain;
        SInt32      mOffValue;
        SInt32      mOnValue;
        QString     mConfirm;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void  applyChanges(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                  ToggleServiceWidget(const ServiceProxy&,
                                      const QJsonObject&,
                                      QWidget* = nullptr);
        virtual  ~ToggleServiceWidget() = default;
		QString   toClipboardText(void) override;
        void      populateWidget(void) override;       
};

//////////////////////////////////////////////////////////////////////////////////////////
