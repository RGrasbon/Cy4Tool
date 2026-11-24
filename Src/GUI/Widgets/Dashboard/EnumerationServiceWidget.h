//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Aufzählung                                                    //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QComboBox>
#include "AbstractServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
class EnumerationServiceWidget : public AbstractServiceWidget
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
        QComboBox*   mComboBox;
        ValueDomain  mValueDomain;
        QString      mConfirm;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:        
        bool  eventFilter(QObject*, QEvent*) override;
		void  applyChanges(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                  EnumerationServiceWidget(const ServiceProxy&,
                                           const QJsonObject&,
                                           QWidget* = nullptr);
        virtual  ~EnumerationServiceWidget() = default;
		QString   toClipboardText(void) override;
        void      populateWidget(void) override;        
};

//////////////////////////////////////////////////////////////////////////////////////////
