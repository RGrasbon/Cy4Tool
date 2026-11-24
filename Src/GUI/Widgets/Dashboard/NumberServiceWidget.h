//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Nummer                                                        //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QLineEdit>
#include <QValidator>
#include "AbstractServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
class NumberServiceWidget : public AbstractServiceWidget
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
        QLineEdit*            mLineEdit;
        NumberFormat          mFormat;
        ValueDomain           mValueDomain;
        Precision             mPrecision;
        NumberRange           mRange;
        QMap<double, QString> mSpecialValueMap;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:        
        void  keyPressEvent(QKeyEvent*) override;
		void  applyChanges(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                  NumberServiceWidget(const ServiceProxy&, const QJsonObject&, QWidget* = nullptr);
        virtual  ~NumberServiceWidget() = default;
		QString   toClipboardText(void) override;
        void      populateWidget(void) override;        
};

//////////////////////////////////////////////////////////////////////////////////////////
