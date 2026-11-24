//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Text                                                          //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QLineEdit>
#include "AbstractServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
class TextServiceWidget : public AbstractServiceWidget
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
        QLineEdit* mLineEdit;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:            
        void  keyPressEvent(QKeyEvent*) override;
		void  applyChanges(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                  TextServiceWidget(const ServiceProxy&, const QJsonObject&, QWidget* = nullptr);
        virtual  ~TextServiceWidget() = default;
		QString   toClipboardText(void) override;
        void      populateWidget(void) override;        
};

//////////////////////////////////////////////////////////////////////////////////////////
