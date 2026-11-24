//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Popup Dialog Uhr                                                                    //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QDialog>
#include <QTime>
#include <QValidator>

namespace Ui { class ClockPopup; }

//////////////////////////////////////////////////////////////////////////////////////////
class ClockPopup : public QDialog
{
    Q_OBJECT

    friend class TimeRangeValidator;

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::ClockPopup* mUi;
        bool            mSecondsEnabled;
        QTime           mTime;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        bool     is12hFormat(void) const;
        bool     secondsEnabled(void) const;
        bool     isPM(void) const;
        void     onPickInvalidButton(void);
        void     onPickNowButton(void);
        void     onSet(void);

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                ClockPopup(QWidget* = nullptr);
               ~ClockPopup() override;
         QTime  popup(QTime, bool, QTime = QTime(), QTime = QTime());
};

//////////////////////////////////////////////////////////////////////////////////////////
class TimeRangeValidator : public QValidator
{
    Q_OBJECT

    private:
        QTime  mMin;
        QTime  mMax;

    public:
                TimeRangeValidator(QTime, QTime, QObject* = nullptr);
        State   validate(QString& input, int& pos) const override;
};

//////////////////////////////////////////////////////////////////////////////////////////

