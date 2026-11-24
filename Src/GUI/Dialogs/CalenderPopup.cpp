//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Popup Dialog Kalender                                                               //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QRegularExpression>
#include "ui_CalenderPopup.h"
#include "CalenderPopup.h"
#include "Settings.h"

static const QDate earliestDate = QDate(1752,  9 ,14);
static const QDate latestDate   = QDate(7999, 12 ,31);

//////////////////////////////////////////////////////////////////////////////////////////
static QString
toText(QDate date)
{
    QString format;
    if (settings().dateFormatDDMMYYY)
    {   format = QStringLiteral("dd.MM.yyyy");
    }
    else
    {   format = settings().locale.dateFormat(QLocale::ShortFormat);
    }
    if (format.contains("yy") && !format.contains("yyyy")) format.replace("yy", "yyyy");
    QString text;
    if (date.isValid())
    {   text = settings().locale.toString(date, format);
    }
    else
    {   QDate tmpDate = QDate(2000, 12, 31);
        text = settings().locale.toString(tmpDate, format);
        text.replace(QRegularExpression("\\d"), QChar('-'));
    }
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CalenderPopup::onPickInvalidButton(void)
{
    done(qMax(QDialog::Accepted, QDialog::Rejected)+1);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CalenderPopup::onPickTodayButton(void)
{
    QDate date = QDate::currentDate();
    mUi->calendar->setSelectedDate(date);
    accept();
}

//////////////////////////////////////////////////////////////////////////////////////////
CalenderPopup::CalenderPopup(QWidget* parent)
             : QDialog(parent, Qt::Popup)

{
    mUi = new Ui::CalenderPopup;
    mUi->setupUi(this);
    connect(mUi->calendar        , &QCalendarWidget::clicked, this, &CalenderPopup::accept);
    connect(mUi->setInvalidButton, &QPushButton::clicked    , this, &CalenderPopup::onPickInvalidButton);
    connect(mUi->setTodayButton  , &QPushButton::clicked    , this, &CalenderPopup::onPickTodayButton);
    if (parent != nullptr) move(parent->mapToGlobal(parent->rect().bottomLeft()));
}

//////////////////////////////////////////////////////////////////////////////////////////
CalenderPopup::~CalenderPopup()
{
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
QDate
CalenderPopup::popup(QDate current, QDate minimum, QDate maximum)
{
    // (1) Insofern Bereich angegeben: Von ... bis angeben
    {   QString minText;
        if (minimum.isValid()) minText = toText(minimum);
        else                   minText = toText(earliestDate);
        mUi->rangeMinLabel->setText(minText);
        QString maxText;
        if (maximum.isValid()) maxText = toText(maximum);
        else                   maxText = toText(latestDate);
        mUi->rangeMaxLabel->setText(maxText);
        mUi->rangeFrame->setVisible(minimum.isValid() || maximum.isValid());
    }
    // (2) Ungültiges Datum, z.B. "--.--.----", als Beschriftung für <setInvalidButton>
    {   QString text = toText(QDate());
        mUi->setInvalidButton->setText(text);
    }
    // (3) Kalendersteuerelement vorbereiten
    if (minimum.isValid()) mUi->calendar->setMinimumDate(minimum);
    else                   mUi->calendar->setMinimumDate(earliestDate);
    if (maximum.isValid()) mUi->calendar->setMaximumDate(maximum);
    else                   mUi->calendar->setMaximumDate(latestDate);
    if (current.isValid()) mUi->calendar->setSelectedDate(current);
    // (4) Popup öffnen
    switch (exec())
    {   case QDialog::Accepted: return mUi->calendar->selectedDate();
        case QDialog::Rejected: return current;
        default:                ;
    }
    return QDate();
}

//////////////////////////////////////////////////////////////////////////////////////////

