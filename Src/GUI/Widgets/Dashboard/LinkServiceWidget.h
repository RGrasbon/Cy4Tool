//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Verweis                                                       //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QLineEdit>
#include <QRegularExpressionValidator>
#include "AbstractServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
class LinkServiceWidget : public AbstractServiceWidget
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        QLineEdit*  mAddressLineEdit;
        QLineEdit*  mLocationLineEdit;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:        
        void  keyPressEvent(QKeyEvent*) override;
        void  onLocationTextChanged(void);
		void  applyChanges(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                  LinkServiceWidget(const ServiceProxy&,
                                    const QJsonObject&,
                                    QWidget* = nullptr);
        virtual  ~LinkServiceWidget() = default;
		QString   toClipboardText(void) override;
        void      populateWidget(void) override;        
};

//////////////////////////////////////////////////////////////////////////////////////////
