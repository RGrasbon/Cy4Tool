//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Zeit                                                          //
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
class TimeServiceWidget : public AbstractServiceWidget
{
    Q_OBJECT

    // Typen /////////////////////////////////////////////////////////////////////////////
    public:
        struct Range
        {   QTime min;
            QTime max;
        };

    // Daten /////////////////////////////////////////////////////////////////////////////
        QLineEdit* mLineEdit;
        bool       mSecondsEnabled;
        Range      mRange;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        Range      toRange(const QJsonObject&);
        bool       toShowSeconds(const QJsonObject&);        
        void       keyPressEvent(QKeyEvent*) override;
        bool       eventFilter(QObject*, QEvent*) override;
		void       applyChanges(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                  TimeServiceWidget(const ServiceProxy&, const QJsonObject&, QWidget* = nullptr);
        virtual  ~TimeServiceWidget() = default;
		QString   toClipboardText(void) override;
        void      populateWidget(void) override;        
};

//////////////////////////////////////////////////////////////////////////////////////////
