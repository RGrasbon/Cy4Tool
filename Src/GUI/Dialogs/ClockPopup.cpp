//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Popup Dialog Uhr                                                                    //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QValidator>
#include "ui_ClockPopup.h"
#include "ClockPopup.h"
#include "Settings.h"

//////////////////////////////////////////////////////////////////////////////////////////
static QTime
convert12to24(QTime time, bool pm)
{
    if (!time.isValid()) return QTime();
    int hour = time.hour();
    if (pm)
    {   if (hour != 12) hour += 12; // 1 PM → 13, 11 PM → 23
    }
    else
    {   if (hour == 12) hour = 0;   // 12 AM → 00
    }
    return QTime(hour, time.minute(), time.second());
}

//////////////////////////////////////////////////////////////////////////////////////////
static QTime
convert24to12(QTime time, bool& pm)
{
    if (!time.isValid()) return QTime();
    int hour = time.hour();
    pm = (hour >= 12);
    hour = hour % 12;
    if (hour == 0) hour = 12; // 00:xx und 12:xx werden im 12h-System zu 12
    return QTime(hour, time.minute(), time.second());
}

//////////////////////////////////////////////////////////////////////////////////////////
static QTime
fromText(const QString& text, bool is12hFormat, bool isPM, bool secondsEnabled)
{
    if (text.isEmpty() || text.contains('_')) return QTime();
    QString format;
    if (is12hFormat) format = "h:mm";
    else             format = "H:mm";
    if (secondsEnabled) format.append(":ss");
    QTime time = QTime::fromString(text, format);
    if (is12hFormat) time = convert12to24(time, isPM);
    return time;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QString
toText(QTime time, bool is12hFormat, bool secondsEnabled)
{
    QString text;
    if (time.isValid())
    {   QTime tmpTime = time;
        QString format;
        if (is12hFormat)
        {   bool pm = false;
            tmpTime = convert24to12(time, pm);
            format = "hh:mm";
        }
        else
        {   format = "HH:mm";
        }
        if (secondsEnabled) format.append(":ss");
        text = tmpTime.toString(format);
    }
    else
    {   text = "-:--";
        if (!is12hFormat) text.prepend("-");
        if (secondsEnabled) text.append(":--");
    }
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
ClockPopup::is12hFormat(void) const
{
    return mUi->amPmStack->currentIndex() == 1;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
ClockPopup::secondsEnabled(void) const
{
    return mSecondsEnabled;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
ClockPopup::isPM(void) const
{
    return mUi->amPmComboBox->currentIndex() == 1;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ClockPopup::onPickInvalidButton(void)
{
    mTime = QTime();
    accept();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ClockPopup::onPickNowButton(void)
{
    mTime = QTime::currentTime();
    accept();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ClockPopup::onSet(void)
{
    QString text = mUi->timeEdit->text();
    const TimeRangeValidator* validator = qobject_cast<const TimeRangeValidator*>(mUi->timeEdit->validator());
    int pos = 0;
    if (validator->validate(text, pos) == QValidator::Invalid) return;
    mTime = fromText(text, is12hFormat(), isPM(), mSecondsEnabled);
    accept();
}

//////////////////////////////////////////////////////////////////////////////////////////
ClockPopup::ClockPopup(QWidget* parent) : QDialog(parent, Qt::Popup)
{
    mUi = new Ui::ClockPopup;
    mUi->setupUi(this);
    connect(mUi->pickInvalidButton, &QPushButton::clicked      , this, &ClockPopup::onPickInvalidButton);
    connect(mUi->pickNowButton    , &QPushButton::clicked      , this, &ClockPopup::onPickNowButton);
    connect(mUi->timeEdit         , &QLineEdit::editingFinished, this, &ClockPopup::onSet);
    connect(mUi->amPmComboBox     , &QComboBox::activated      , this, &ClockPopup::onSet);
    if (parent != nullptr) move(parent->mapToGlobal(parent->rect().bottomLeft()));
}

//////////////////////////////////////////////////////////////////////////////////////////
ClockPopup::~ClockPopup()
{
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
QTime
ClockPopup::popup(QTime current, bool secondsEnabled, QTime minimum, QTime maximum)
{
    if (current.isValid()) mTime = current;
    else                   mTime = QTime(12,0); // Bei ungültigem <current>: Vorgabe 12:00
    mSecondsEnabled = secondsEnabled;
    bool is12hFormat;
    // (1) 12h/24h Zeitformat bestimmen
    {   if (settings().timeFormat24h)
        {   is12hFormat = false;
        }
        else
        {   const QLocale& locale = settings().locale;
            QString format = locale.timeFormat(QLocale::ShortFormat);
            is12hFormat = format.contains("AP", Qt::CaseInsensitive);
        }
    }
    // (2) Steuerelemente initialisieren
    {   const QLocale& locale = settings().locale;
        mUi->amPmComboBox->setItemText(0, locale.amText());
        mUi->amPmComboBox->setItemText(1, locale.pmText());
        mUi->amPmStack->setCurrentIndex(is12hFormat ? 1 : 0);
        if (mSecondsEnabled) mUi->timeEdit->setInputMask("00:00:00;_");
        else                 mUi->timeEdit->setInputMask("00:00;_");
        mUi->timeEdit->setText(toText(mTime, is12hFormat, mSecondsEnabled));
        mUi->timeEdit->setValidator(new TimeRangeValidator(minimum, maximum, this));
        QString minText;
        if (minimum.isValid()) minText = toText(minimum, is12hFormat, mSecondsEnabled);
        else                   minText = toText(QTime(0,0,0), is12hFormat, mSecondsEnabled);
        mUi->rangeMinLabel->setText(minText);
        QString maxText;
        if (maximum.isValid()) maxText = toText(maximum, is12hFormat, mSecondsEnabled);
        else                   maxText = toText(QTime(23,59,59), is12hFormat, mSecondsEnabled);
        mUi->rangeMaxLabel->setText(maxText);
        mUi->rangeFrame->setVisible(minimum.isValid() || maximum.isValid());
    }
    // (3) Ungültiges Zeit, z.B. "--:--:--", als Beschriftung für <setInvalidButton>
    {   QString text = toText(QTime(), is12hFormat, mSecondsEnabled);
        mUi->pickInvalidButton->setText(text);
    }
    // (4) Popup öffnen
    switch (exec())
    {   case QDialog::Accepted: return mTime;
        case QDialog::Rejected: return current;
        default:                ;
    }
    return QTime();
}

//////////////////////////////////////////////////////////////////////////////////////////
TimeRangeValidator::TimeRangeValidator(QTime min, QTime max, QObject* parent)
                  : QValidator(parent)
{
    mMin = min;
    mMax = max;
}

//////////////////////////////////////////////////////////////////////////////////////////
QValidator::State
TimeRangeValidator::validate(QString& text, int&) const
{
    if (text.isEmpty() || text.contains('_')) return Intermediate;
    const ClockPopup* popup = dynamic_cast<const ClockPopup*>(parent());
    QTime time = fromText(text, popup->is12hFormat(), popup->isPM(), popup->secondsEnabled());
    if (!time.isValid()) return Invalid;
    if (mMin.isValid() && time < mMin) return Invalid;
    if (mMax.isValid() && time > mMax) return Invalid;
    return Acceptable;
}

//////////////////////////////////////////////////////////////////////////////////////////

