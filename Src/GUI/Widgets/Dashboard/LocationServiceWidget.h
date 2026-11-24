//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Dienstort                                                     //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QLineEdit>
#include "AbstractServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
class LocationServiceWidget : public AbstractServiceWidget
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
        QLineEdit*  mLineEdit;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:        
        void  keyPressEvent(QKeyEvent*) override;
        void  onTextChanged(void);
		void  applyChanges(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                  LocationServiceWidget(const ServiceProxy&,
                                        const QJsonObject&,
                                        QWidget* = nullptr);
        virtual  ~LocationServiceWidget() = default;
		QString   toClipboardText(void) override;
        void      populateWidget(void) override;        
};

//////////////////////////////////////////////////////////////////////////////////////////
