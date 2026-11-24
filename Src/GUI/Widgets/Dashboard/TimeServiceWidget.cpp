//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Zeit                                                          //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QVBoxLayout>
#include <QKeyEvent>
#include <QDate>
#include "Global.h"
#include "TimeServiceWidget.h"
#include "ClockPopup.h"
#include "KeyValues.h"
#include "Settings.h"

//////////////////////////////////////////////////////////////////////////////////////////
static QString
toFormat(bool showSeconds)
{
    QString format;

    if (settings().timeFormat24h)
    {   if (showSeconds) format = QStringLiteral("HH:mm:ss");
        else             format = QStringLiteral("HH:mm");
    }
    else
    {   if (showSeconds) format = settings().locale.timeFormat(QLocale::LongFormat);
        else             format = settings().locale.timeFormat(QLocale::ShortFormat);
        format.remove('t'); // ohne Zeitzone
        format = format.trimmed();
    }
    return format;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Erzeugt einen Zeit-Text, bei dem alle Ziffern durch <placeholder> ersetzt sind.
//////////////////////////////////////////////////////////////////////////////////////////
static QString
createTimePattern(QChar digitSubstitution, bool showSeconds)
{
    QString format = toFormat(showSeconds);
    QTime time = QTime(22, 22, 22); // Dummy-Zeit
    QString text = settings().locale.toString(time, format);
    text.replace(QRegularExpression("\\d"), digitSubstitution); // Beispiel: "--:--:--"
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Schreibt <time> oder mit '-'s wenn ungültig.
//////////////////////////////////////////////////////////////////////////////////////////
static QString
toText(QTime time, bool showSeconds)
{
    QString format = toFormat(showSeconds);
    if (!time.isValid()) return createTimePattern('-', showSeconds);
    return settings().locale.toString(time, format);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Erzeugt ein ByteArray der Länge 3 aus <time>
//////////////////////////////////////////////////////////////////////////////////////////
static ByteArray
serialize(QTime time)
{
    ByteArray data;
    data = ByteArray(3, 0xFF);
    if (time.isValid())
    {   data[0] = UInt08(time.hour());
        data[1] = UInt08(time.minute());
        data[2] = UInt08(time.second());
    }
    return data;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Erzeugt eine Zeit aus <data>. <data> muss Größe 2 oder 3 haben
//////////////////////////////////////////////////////////////////////////////////////////
static QTime
deserialize(const ByteArray& data)
{
    switch (data.size())
    {   case 2:  return QTime(data[0], data[1],    0   );
        case 3:  return QTime(data[0], data[1], data[2]);
        default: ;
    }
    return QTime();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Holt den Datumsbereich aus <jsonObject>
//////////////////////////////////////////////////////////////////////////////////////////
TimeServiceWidget::Range
TimeServiceWidget::toRange(const QJsonObject& jsonObject)
{
    TimeServiceWidget::Range range;
    if (jsonObject.contains(KEY::MIN))
    {   QString text = jsonObject.value(KEY::MIN).toString();
        QTime time = QTime::fromString(text, "HH:mm:ss");
        if (!time.isValid()) time = QTime::fromString(text, "HH:mm");
        if (time.isValid())
        {   range.min = time;
        }
        else
        {   mIssue = QObject::tr("\"%1\" must follow the format \"hh:mm\" or "
                                 "\"hh:mm:ss\"").arg(KEY::MIN);
        }
    }
    if (jsonObject.contains(KEY::MAX))
    {   QString text = jsonObject.value(KEY::MAX).toString();
        QTime time = QTime::fromString(text, "HH:mm:ss");
        if (!time.isValid()) time = QTime::fromString(text, "HH:mm");
        if (time.isValid())
        {   range.max = time;
        }
        else
        {   mIssue = QObject::tr("\"%1\" must follow the format \"hh:mm\" or "
                                 "\"hh:mm:ss\"").arg(KEY::MAX);
        }
    }
    if (range.min.isValid() && range.max.isValid() && range.min > range.max)
    {   range = TimeServiceWidget::Range();
        mIssue = QObject::tr("\"%1\" must be earlier than \"%2\"").arg(KEY::MIN, KEY::MAX);
    }
    return range;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Holt das Sekunden-Anzeige-Eigenschaft aus <jsonObject>
//////////////////////////////////////////////////////////////////////////////////////////
bool
TimeServiceWidget::toShowSeconds(const QJsonObject& jsonObject)
{
    bool show = false;
    if (jsonObject.contains(KEY::SECONDS_ENABLED))
    {   if (!jsonObject.value(KEY::SECONDS_ENABLED).isBool())
        {   mIssue = QObject::tr("\"%1\" must be true or false")
                                 .arg(KEY::SECONDS_ENABLED);
        }
        else
        {   show = jsonObject.value(KEY::SECONDS_ENABLED).toBool();
            if (show && mServiceProxy.size() != 3)
            {   mIssue = QObject::tr("Time widget width \"%1\" = true requires service "
                                     "width 3").arg(KEY::SECONDS_ENABLED);
            }
        }
    }
    return show;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TimeServiceWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {   // Ursprünglichen Inhalt wiederherstellen
        populateWidget();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
TimeServiceWidget::eventFilter(QObject* source, QEvent* event)
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
TimeServiceWidget::applyChanges(void)
{
    // Popup-Dialog öffnen und Zeit holen
    ByteArray serviceData = mServiceProxy.read();
    QTime time = ::deserialize(serviceData);
    ClockPopup clockPopupDialog(this);
    time = clockPopupDialog.popup(time, mSecondsEnabled, mRange.min, mRange.max);
    // Änderungen anwenden
    ByteArray widgetData = ::serialize(time);
    if (mServiceProxy.size() == 2) widgetData.pop_back();
    mServiceProxy.write(0, widgetData);
    if (mServiceProxy.access() == Service::WriteOnly)
    {   mLineEdit->clear();
    }
    else
    {   mLineEdit->setText(toText(time, mSecondsEnabled));
    }
    emit dataChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
TimeServiceWidget::TimeServiceWidget(const ServiceProxy& service,
                                     const QJsonObject& jsonObject,
                                     QWidget* parent)
                 : AbstractServiceWidget(service, parent)
{
    // Dienst
    {   if (mServiceProxy.size() < 2 || mServiceProxy.size() > 3)
        {   mIssue = tr("Service size must be 2 (hours, minutes) or 3 (*, seconds)");
            mIsOperational = false;
        }
        if (jsonObject.contains(KEY::FORMAT))
        {   mIssue = tr("\"%1\" is deprecated for widget \"%2\". Please use number "
                        "widgets for customized time fields instead")
                        .arg(KEY::FORMAT, KEY::TIME);
        }
        mSecondsEnabled = toShowSeconds(jsonObject);
        mRange          = toRange(jsonObject);
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
        QString measurementText = createTimePattern('8', mSecondsEnabled);
        int width = Global::calculateWidthFor(measurementText, mLineEdit);  // # Pixels
        mLineEdit->setFixedWidth(width);
        if (mIsOperational) mLineEdit->installEventFilter(this);
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
TimeServiceWidget::toClipboardText(void)
{
    return mLineEdit->text();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TimeServiceWidget::populateWidget(void)
{
    if (mServiceProxy.isUpToDate())
    {   ByteArray data = mServiceProxy.read();
        QTime time = ::deserialize(data);
        mLineEdit->setText(toText(time, mSecondsEnabled));
    }
    else
    {   mLineEdit->clear();
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////

