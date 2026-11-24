//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Nummer                                                        //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QVBoxLayout>
#include <QApplication>
#include <QKeyEvent>
#include "Conf.h"
#include "NumberValidator.h"
#include "NumberServiceWidget.h"
#include "Global.h"
#include "KeyValues.h"

//////////////////////////////////////////////////////////////////////////////////////////
void
NumberServiceWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {   // Ursprünglichen Inhalt wiederherstellen
        populateWidget();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
NumberServiceWidget::applyChanges(void)
{
    bool   ok;
    double value = mLineEdit->text().toDouble(&ok);
    if (ok)
    {   ByteArray widgetData = serialize(value, mFormat);
        mServiceProxy.write(0, widgetData);
        emit dataChanged();
    }
    if (mServiceProxy.access() == Service::WriteOnly) mLineEdit->clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
NumberServiceWidget::NumberServiceWidget(const ServiceProxy& serviceProxy,
                                         const QJsonObject&  jsonObject,
                                         QWidget*            parent)
                   : AbstractServiceWidget(serviceProxy, parent)
{
    // Dienst
    {   if (mServiceProxy.size() > 4)
        {   mIssue = tr("Service size must be 1, 2, 3 or 4");
            mIsOperational = false;
        }
        mFormat = toFormat(jsonObject);
        if (mFormat == NumberFormat::Float && mServiceProxy.size() != 4)
        {   mIssue = tr("Number format \"%1\" requires service size 4").arg(VALUE::FLOAT);
            mIsOperational = false;
        }
        mValueDomain     = toValueDomain(jsonObject);
        mPrecision       = toPrecision(jsonObject, mFormat);
        mRange           = toRange(jsonObject, mFormat, mValueDomain, mServiceProxy.size());
        mSpecialValueMap = toSpecialValues(jsonObject);
    }
    // Steuerelement
    {   QVBoxLayout* verticalLayout = new QVBoxLayout();
        verticalLayout->setContentsMargins(1, 1, 1, 1);
        mLineEdit = new QLineEdit(this);
        mLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
        mLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mLineEdit->setAlignment(Qt::AlignCenter);
        mLineEdit->setValidator(new NumberValidator(mRange.min, mRange.max, int(mPrecision), this));
        connect(mLineEdit, &QLineEdit::editingFinished, this, &NumberServiceWidget::applyChanges);
        verticalLayout->addWidget(mLineEdit, 0, Qt::AlignCenter);
        setLayout(verticalLayout);
        QString measurementTextMin = Global::createMeasurementTextFor(mRange.min, int(mPrecision));
        QString measurementTextMax = Global::createMeasurementTextFor(mRange.max, int(mPrecision));
        int widthMin = Global::calculateWidthFor(measurementTextMin, mLineEdit);
        int widthMax = Global::calculateWidthFor(measurementTextMax, mLineEdit);
        int width = qMax(widthMin, widthMax); // Platzbedarf
        for(const QString& word : std::as_const(mSpecialValueMap))
        {   int wordWidth = Global::calculateWidthFor(word, mLineEdit);
            width = qMax(wordWidth, width);
        }
        width = qMax(Global::calculateWidthFor("---", mLineEdit), width);
        mLineEdit->setFixedWidth(width);
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
NumberServiceWidget::toClipboardText(void)
{
    return mLineEdit->text();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
NumberServiceWidget::populateWidget(void)
{
    if (mServiceProxy.isUpToDate())
    {   ByteArray data = mServiceProxy.read();
        double value = deserialize(data, mValueDomain, mFormat);
        QString text;
        if (mSpecialValueMap.contains(value))
        {   text = mSpecialValueMap[value];
        }
        else
        {   if (value < mRange.min || value > mRange.max)
            {   text = "---";
            }
            else
            {   text = QString::number(value, 'f', int(mPrecision));
            }
        }
        mLineEdit->setText(text);
    }
    else
    {   mLineEdit->clear();
    }
    styleWidget();
}

//////////////////////////////////////////////////////////////////////////////////////////

