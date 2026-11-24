//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Datum                                                         //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QVBoxLayout>
#include <QKeyEvent>
#include <QDate>
#include "Global.h"
#include "DateServiceWidget.h"
#include "CalenderPopup.h"
#include "KeyValues.h"
#include "Settings.h"

//////////////////////////////////////////////////////////////////////////////////////////
static UInt08
hiByte(int value)
{
    return UInt08((UInt16(value)>>8)&0x00FF);
}

//////////////////////////////////////////////////////////////////////////////////////////
static UInt08
loByte(int value)
{
    return UInt08((UInt16(value))&0x00FF);
}

//////////////////////////////////////////////////////////////////////////////////////////
static UInt16
toWord(int high,int low)
{
    return UInt16((UInt16(high)<< 8) | (UInt16(low)&0x00FF));
}

//////////////////////////////////////////////////////////////////////////////////////////
static QString
toFormat(void)
{
    QString format;

    if (settings().dateFormatDDMMYYY)
    {   format = QStringLiteral("dd.MM.yyyy");
    }
    else
    {   format = settings().locale.dateFormat(QLocale::ShortFormat);
    }
    return format;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Erzeugt einen Datumstext, bei dem alle Ziffern durch <placeholder> ersetzt sind.
//////////////////////////////////////////////////////////////////////////////////////////
static QString
createDatePattern(QChar digitSubstitution)
{
    QDate date = QDate(2222, 12, 22); // Gültiges Dummy-Datum
    QString format = toFormat();
    if (format.contains("yy") && !format.contains("yyyy")) format.replace("yy", "yyyy");
    QString text = settings().locale.toString(date, format);
    text.replace(QRegularExpression("\\d"), digitSubstitution); // Beispiel: "--.--.----"
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Liest ein Datum aus <text>. Als Trennzeichen werden auch '.' und '/' akzeptiert
//////////////////////////////////////////////////////////////////////////////////////////
static QDate
fromText(const QString& text)
{
    QDate date = QDate::fromString(text, "yyyy-MM-dd");
    if (!date.isValid() ) date = QDate::fromString(text, "yyyy.MM.dd");
    if (!date.isValid() ) date = QDate::fromString(text, "yyyy/MM/dd");
    return date;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Schreibt <date> in loaler Darstellung oder mit '-'s wenn ungültig.
//////////////////////////////////////////////////////////////////////////////////////////
static QString
toText(QDate date)
{
    if (!date.isValid()) return createDatePattern('-');
    QString format = toFormat();
    if (format.contains("yy") && !format.contains("yyyy")) format.replace("yy", "yyyy");
    return settings().locale.toString(date, format);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Erzeugt ein ByteArray aus <date>
//////////////////////////////////////////////////////////////////////////////////////////
ByteArray
static serialize(QDate date)
{
    ByteArray data;
    data = ByteArray(4, 0xFF);
    if (date.isValid())
    {   data[0] = hiByte(date.year());
        data[1] = loByte(date.year());
        data[2] = UInt08(date.month());
        data[3] = UInt08(date.day());
    }
    return data;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Erzeugt ein Datum aus <data>. <data> muss die Größe 4 haben (3 wenn veraltet)
//////////////////////////////////////////////////////////////////////////////////////////
static QDate
deserialize(const ByteArray& data)
{
    switch (data.size())
    {   case 3:  return QDate(toWord(data[0], data[1]), data[2], 1); // Veraltet seit V2.0
        case 4:  return QDate(toWord(data[0], data[1]), data[2], data[3]);
        default: ;
    }
    return QDate();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Holt den Datumsbereich aus <jsonObject>.
//////////////////////////////////////////////////////////////////////////////////////////
DateServiceWidget::Range
DateServiceWidget::toRange(const QJsonObject& jsonObject)
{
    DateServiceWidget::Range range;
    if (jsonObject.contains(KEY::MIN))
    {   QString text = jsonObject.value(KEY::MIN).toString();
        QDate date = fromText(text);
        if (date.isValid())
        {   range.min = date;
        }
        else
        {   mIssue = QObject::tr("\"%1\" must follow the format \"yyyy-mm-dd\"").arg(KEY::MIN);
        }
    }
    if (jsonObject.contains(KEY::MAX))
    {   QString text = jsonObject.value(KEY::MAX).toString();
        QDate date = fromText(text);
        if (date.isValid())
        {   range.max = date;
        }
        else
        {   mIssue = QObject::tr("\"%1\" must follow the format \"yyyy-mm-dd\"").arg(KEY::MAX);
        }
    }
    if (range.min.isValid() && range.max.isValid() && range.min > range.max)
    {   range = DateServiceWidget::Range();
        mIssue = QObject::tr("\"%1\" must be earlier than \"%2\"").arg(KEY::MIN, KEY::MAX);
    }
    return range;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DateServiceWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {   // Ursprünglichen Inhalt wiederherstellen
        populateWidget();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DateServiceWidget::eventFilter(QObject* source, QEvent* event)
{
    if (source == mLineEdit)
    {   if (event->type() == QEvent::MouseButtonPress)
        {   QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {   if (!mIsOperational || mServiceProxy.access() == Service::ReadOnly)
                {   notifyStatus();
                    return true;
                }
                applyChanges();
            }
        }
    }
    return QObject::eventFilter(source, event);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DateServiceWidget::applyChanges(void)
{
    // Popup-Dialog öffnen und Datum holen
    ByteArray serviceData = mServiceProxy.read();
    QDate date = ::deserialize(serviceData);
    CalenderPopup calenderPopupDialog(this);
    date = calenderPopupDialog.popup(date, mRange.min, mRange.max);
    // Änderungen anwenden
    ByteArray widgetData = ::serialize(date);
    if (mServiceProxy.size() == 3)
    {   // Datum-Widgets mit Dienste der Breite 3 (Jahr/Monat) sind seit V2.0 veraltet
        widgetData.pop_back();    // Tag wieder entfernen
    }
    mServiceProxy.write(0, widgetData);
    if (mServiceProxy.access() == Service::WriteOnly) mLineEdit->clear();
    else                                              mLineEdit->setText(toText(date));
    emit dataChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
DateServiceWidget::DateServiceWidget(const ServiceProxy& service,
                                     const QJsonObject& jsonObject,
                                     QWidget* parent)
                 : AbstractServiceWidget(service, parent)
{
    // Dienst
    {   if (jsonObject.contains(KEY::FORMAT))
        {   mIssue = tr("\"%1\" is deprecated for widget \"%2\". Please use number "
                        "widgets for customized date fields instead")
                        .arg(KEY::FORMAT, KEY::DATE);
        }
        else if (mServiceProxy.size() != 4)
        {   mIssue = tr("Service size must be exactly %1").arg(4);
            mIsOperational = false;
        }
        mRange = toRange(jsonObject);
    }
    // Steuerelement
    {   QVBoxLayout* verticalLayout = new QVBoxLayout();
        verticalLayout->setContentsMargins(1, 1, 1, 1);
        mLineEdit = new QLineEdit(this);
        mLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
        mLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mLineEdit->setAlignment(Qt::AlignCenter);
        verticalLayout->addWidget(mLineEdit, 0, Qt::AlignCenter);
        setLayout(verticalLayout);
        QString measurementText = createDatePattern('8');
        int width = Global::calculateWidthFor(measurementText, mLineEdit);  // # Pixels
        mLineEdit->setFixedWidth(width);
        mLineEdit->installEventFilter(this);
        mLineEdit->setReadOnly(true);
        if (mServiceProxy.access() == Service::ReadOnly)
        {   mLineEdit->setFocusPolicy(Qt::NoFocus);
            mLineEdit->setStyleSheet(CFG::BackgroundForReadOnly);
        }
        mLineEdit->setEnabled(mIsOperational);
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
DateServiceWidget::toClipboardText(void)
{
    return mLineEdit->text();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DateServiceWidget::populateWidget(void)
{
    if (mServiceProxy.isUpToDate())
    {   ByteArray data = mServiceProxy.read();
        QDate date = ::deserialize(data);
        mLineEdit->setText(toText(date));
    }
    else
    {   mLineEdit->clear();
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////

