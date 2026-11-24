//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Popup Dialog Kalender                                                               //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QDialog>
#include <QDate>

namespace Ui { class CalenderPopup; }

//////////////////////////////////////////////////////////////////////////////////////////
class CalenderPopup : public QDialog
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::CalenderPopup* mUi;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void    onPickInvalidButton(void);
        void    onPickTodayButton(void);

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                CalenderPopup(QWidget* = nullptr);
               ~CalenderPopup() override;
         QDate  popup(QDate, QDate = QDate(), QDate = QDate());
};

//////////////////////////////////////////////////////////////////////////////////////////

