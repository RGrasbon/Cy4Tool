//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Aufzählung                                                    //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QVBoxLayout>
#include <QApplication>
#include <QKeyEvent>
#include "Conf.h"
#include "EnumerationServiceWidget.h"
#include "Keys.h"

//////////////////////////////////////////////////////////////////////////////////////////
static QString
fromByteArray(const ByteArray& data)
{
    if (data.empty()) return QString();
    int length = 0;
    while (length < data.size() && data[length] != 0) length++;
    QByteArray tmpdata(length, 0);
    for (int i=0; i<tmpdata.length(); i++) tmpdata[i] = data[i];
    return QString::fromUtf8(tmpdata);
}

//////////////////////////////////////////////////////////////////////////////////////////
static ByteArray
toByteArray(const QString& text)
{
    QString    tmpText = text;
    QByteArray tmpData;
    while (true)
    {   tmpData = tmpText.toUtf8();
        if (tmpData.size() <= CFG::MaximumServiceSize) break;
        tmpText.chop(1);
    }
    tmpData.append('\0');
    ByteArray data(tmpData.size());
    for (int i=0; i<data.size(); i++) data[i] = tmpData[i];
    return data;
}

//////////////////////////////////////////////////////////////////////////////////////////
/*void
EnumerationServiceWidget::fwdResponse(const Cy4Message& msg, void* object)
{
    EnumerationServiceWidget* widget = static_cast<EnumerationServiceWidget*>(object);
    Cy4Message::Access  acc;
    bool accepted;
    if( widget->take(msg, acc, accepted) ) widget->cbResponse(acc, accepted);
}
*/
//////////////////////////////////////////////////////////////////////////////////////////
/*void
EnumerationServiceWidget::cbResponse(Cy4Message::Access acc , bool accepted)
{
    setEnabled(true);
    styleWidget(accepted);
    switch( acc )
    {   case Cy4Message::READ:
        {   QString  textLine;
            QByteArray tmpArray = srv.data();
            tmpArray.append('\0');
            if( accepted ) textLine = QString::fromUtf8(tmpArray);
            setText(textLine);
            home(false);
            break;
        }
        case Cy4Message::WRITE:
            if( accepted ) setModified(false);
            break;
        default:
            Q_ASSERT(false);
    }
    emit updated(this, acc, accepted);
}
*/
//////////////////////////////////////////////////////////////////////////////////////////
/*void
EnumerationServiceWidget::onActivity(void)
{
    if( srv.access() == Cy4Service::READ_ONLY || !isModified() ) return;
    QByteArray data = text().toUtf8();
    data = data.left(srv.size());
    for( int i=0; i<srv.size();  i++ ) srv[i] = '\0';
    for( int i=0; i<data.size(); i++ ) srv[i] = UInt08(data[i]);
    if( post(Cy4Message::WRITE, this, fwdResponse) ) setEnabled(false);
    emit changed(this);
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
void
EnumerationServiceWidget::onTextChanged(void)
{
qDebug("onTextChanged");
    widget->setModified(false);

    ByteArray data = toByteArray(widget->text());
    mService.write(data);
    emit dataChanged();



   /* QString textLine = text();
    bool modified = false;
    int maxLen = srv.size();
    if( maxLen < 1 ) maxLen = 1;
    while (true)
    {   QByteArray data = textLine.toUtf8();
        if( data.size() <= maxLen )
        {   if (modified) setText(data);
            break;
        }
        textLine.chop(1);
        modified = true;
    }*/
}

//////////////////////////////////////////////////////////////////////////////////////////
void
EnumerationServiceWidget::keyPressEvent(QKeyEvent* event)
{
    QKeyEvent* tabEvent;
    if (event->matches(QKeySequence::InsertParagraphSeparator))
    {   event->ignore();
        if (widget->isModified())
        {   onTextChanged();
        }
        tabEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
        QApplication::postEvent(this, tabEvent);
    }
    else if (event->key() == Qt::Key_Escape)
    {   // Ursprüngliche Zeichenkette wiederherstellen
        equip();
    }
    else
    {   //QLineEdit::keyPressEvent(event);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
EnumerationServiceWidget::EnumerationServiceWidget(const ServiceProxy& service,
                                                   const QJsonObject& jsonObject,
                                                   QWidget* parent)
                        : AbstractServiceWidget(service, parent)
{
    QVBoxLayout* verticalLayout = new QVBoxLayout();
    widget = new QLineEdit(this);
    verticalLayout->addWidget(widget);
    setLayout(verticalLayout);
    int maxLength = qMax(1, service.size());
    widget->setMaxLength(maxLength);
    widget->setReadOnly(service.access() == Service::ReadOnly);
    QString  measureText;
    measureText.fill('A', qMin(30, maxLength+2));
    int width = fontMetrics().horizontalAdvance(measureText);
    setMinimumWidth(width);
    setupStyle(this);
    connect(widget, &QLineEdit::editingFinished, this, &EnumerationServiceWidget::onTextChanged);
}

//////////////////////////////////////////////////////////////////////////////////////////
const char*
EnumerationServiceWidget::widgetKey(void) const
{
    return KEY::TEXT;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
EnumerationServiceWidget::equip(void)
{
    if (mService.isUpToDate())
    {   ByteArray data = mService.read();
        widget->setText(fromByteArray(data));
    }
    else
    {   widget->clear();
    }
    setupStyle(this);
}

//////////////////////////////////////////////////////////////////////////////////////////

