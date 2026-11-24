//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Datum                                                         //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QLineEdit>
#include "AbstractServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
namespace Ui
{
    class CalenderPopupDialog;
}

//////////////////////////////////////////////////////////////////////////////////////////
class DateServiceWidget : public AbstractServiceWidget
{
    Q_OBJECT

    // Typen /////////////////////////////////////////////////////////////////////////////
    public:
        struct Range
        {   QDate min;
            QDate max;
        };

    // Daten /////////////////////////////////////////////////////////////////////////////
        QLineEdit* mLineEdit;
        Range      mRange;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        Range      toRange(const QJsonObject&);        
        void       keyPressEvent(QKeyEvent*) override;
        bool       eventFilter(QObject*, QEvent*) override;
		void       applyChanges(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                  DateServiceWidget(const ServiceProxy&,
                                    const QJsonObject&,
                                    QWidget* = nullptr);
        virtual  ~DateServiceWidget() = default;
        QString   toClipboardText(void) override;
        void      populateWidget(void) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
