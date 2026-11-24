//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Text                                                          //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QVBoxLayout>
#include <QApplication>
#include <QKeyEvent>
#include <QStyleOption>
#include "Conf.h"
#include "Global.h"
#include "TextServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Erzeugt eine Zeichenkette, die der mittleren Breite von Texten der Länge <length> in
// dem <lineEdit> zugeordneten Zeichensatz entspricht.
//////////////////////////////////////////////////////////////////////////////////////////
static QString
createMeasurementTextFor(int length, const QLineEdit* lineEdit)
{
    QFontMetrics metric(lineEdit->font());
    int targetPixelWidth = qBound(20, length * metric.averageCharWidth(), 1000);
    QString text;
    while (metric.horizontalAdvance(text) < targetPixelWidth) text.append('x');
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
static ByteArray
serialize(const QString& text, int maxSize)
{
    QString    tmpText = text;
    QByteArray tmpData;
    while (true)
    {   tmpData = tmpText.toUtf8();
        if (tmpData.size() <= maxSize) break;
        tmpText.chop(1);
    }
    tmpData.append('\0');
    ByteArray data(tmpData.size());
    for (USize i=0; i<data.size(); i++) data[i] = tmpData[i];
    return data;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QString
deserialize(const ByteArray& data)
{
    USize length = 0;
    while (length < data.size() && data[length] != 0) length++;
    QByteArray tmpdata(length, 0);
    for (int i=0; i<tmpdata.length(); i++) tmpdata[i] = data[i];
    return QString::fromUtf8(tmpdata);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TextServiceWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {   // Ursprünglichen Inhalt wiederherstellen
        populateWidget();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TextServiceWidget::applyChanges(void)
{
    ByteArray serviceData = mServiceProxy.read();
    ByteArray widgetData  = ::serialize(mLineEdit->text(), mServiceProxy.size());
    int       startIndex;
    ByteArray updateData;
    if (mServiceProxy.access() == Service::WriteOnly)
    {   mLineEdit->clear();
        startIndex = 0;
        updateData = widgetData;
    }
    else
    {   // <updateData> erhält die veränderte Bytesequenz von <widgetData> gegenüber <serviceData>
        startIndex = INVALID;
        int lastIndex  = 0;
        int size = int(std::min(serviceData.size(), widgetData.size()));
        for (int i=0; i<size; i++)
        {   if (serviceData[i] != widgetData[i])
            {   if (startIndex == INVALID) startIndex = i;
                lastIndex = i;
            }
        }
        if (startIndex == INVALID)
        {   // Keine Änderungen
            startIndex = 0;
        }
        else
        {   updateData = ByteArray(widgetData.begin()+startIndex,
                                   widgetData.begin()+lastIndex+1);
        }
    }
    mServiceProxy.write(startIndex, updateData);
    emit dataChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
TextServiceWidget::TextServiceWidget(const ServiceProxy& serviceProxy,
                                     const QJsonObject&,
                                     QWidget*            parent)
                 : AbstractServiceWidget(serviceProxy, parent)
{
    // Steuerelement
    {   QVBoxLayout* verticalLayout = new QVBoxLayout();
        verticalLayout->setContentsMargins(1, 1, 1, 1);
        mLineEdit = new QLineEdit(this);
        mLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
        mLineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        connect(mLineEdit, &QLineEdit::editingFinished, this, &TextServiceWidget::applyChanges);
        verticalLayout->addWidget(mLineEdit, 0, Qt::AlignCenter);
        setLayout(verticalLayout);
        int maxLength = qMax(1, serviceProxy.size()-1); // # Zeichen
        mLineEdit->setMaxLength(maxLength);
        QString measurementText = createMeasurementTextFor(maxLength, mLineEdit);
        int width = Global::calculateWidthFor(measurementText, mLineEdit);  // # Pixels
        mLineEdit->setMinimumWidth(width);
        if (mServiceProxy.access() == Service::ReadOnly)
        {   mLineEdit->setReadOnly(true);
            mLineEdit->setAttribute(Qt::WA_TransparentForMouseEvents);
            mLineEdit->setFocusPolicy(Qt::NoFocus);
            mLineEdit->setStyleSheet(CFG::BackgroundForReadOnly);
        }
        mLineEdit->setEnabled(mIsOperational);
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
TextServiceWidget::toClipboardText(void)
{
    return mLineEdit->text();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
TextServiceWidget::populateWidget(void)
{
    if (mServiceProxy.isUpToDate())
    {   ByteArray data = mServiceProxy.read();
        QString text = ::deserialize(data);
        mLineEdit->setText(text);
    }
    else
    {   mLineEdit->clear();
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////

