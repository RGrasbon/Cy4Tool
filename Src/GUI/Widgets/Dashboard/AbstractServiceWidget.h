//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Basisklasse                                                   //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
#include <QMouseEvent>
#include "ServiceProxy.h"

enum class NumberFormat     // Zahlenformat
{   Integer,                // · Ganzzahl
    Fixpoint1 = 1,          // · Festkommazahl mit 1 Nachkommastelle
    Fixpoint2 = 2,          // · Festkommazahl mit 2 Nachkommastellen
    Fixpoint3 = 3,          // · Festkommazahl mit 3 Nachkommastellen
    Fixpoint4 = 4,          // · Festkommazahl mit 4 Nachkommastellen
    Fixpoint5 = 5,          // · Festkommazahl mit 5 Nachkommastellen
    Fixpoint6 = 6,          // · Festkommazahl mit 6 Nachkommastellen
    Float                   // · 32-Bit Fliesskommazahl nach IEEE 754
};

enum class Precision        // Genauigkeit (durch Anzahl Nachkommastellen):
{   Digits0 = 0,            // · x
    Digits1 = 1,            // · x.x
    Digits2 = 2,            // · x.xx
    Digits3 = 3,            // · x.xxx
    Digits4 = 4,            // · x.xxxx
    Digits5 = 5,            // · x.xxxxx
    Digits6 = 6             // · x.xxxxxx
};

enum class ValueDomain      // Vorzeichenbehaftung:
{   Signed,                 // · Vorzeichenbehaftete Zahl
    Unsigned                // · Vorzeichenlose Zahl
};

//////////////////////////////////////////////////////////////////////////////////////////
class AbstractServiceWidget : public QWidget
{
    Q_OBJECT

    // Typen /////////////////////////////////////////////////////////////////////////////
    protected:
        struct NumberRange
        {   double min = -qInf();
            double max = +qInf();
        };

    // Daten /////////////////////////////////////////////////////////////////////////////
    protected:
        ServiceProxy  mServiceProxy;
        QString       mIssue;
        QColor        mOriginalBackgroundColor;
        bool          mIsOperational;

    // Signale ///////////////////////////////////////////////////////////////////////////
    signals:
        void  dataChanged(void);

    // Interna ///////////////////////////////////////////////////////////////////////////
    protected:
        ValueDomain            toValueDomain(const QJsonObject&);
        NumberFormat           toFormat(const QJsonObject&);
        Precision              toPrecision(const QJsonObject& jsonObject, NumberFormat format);
        NumberRange            toRange(const QJsonObject&, NumberFormat, ValueDomain, int);
        QMap<double, QString>  toSpecialValues(const QJsonObject&);
        SInt32                 toNumber(const QJsonValue&, ValueDomain, SInt32 = INVALID);
        ByteArray              serialize(double, NumberFormat = NumberFormat::Integer) const;
        ByteArray              serialize(double, NumberFormat, int) const;
        double                 deserialize(const ByteArray&, ValueDomain, NumberFormat = NumberFormat::Integer) const;
        void                   notifyStatus(void) const;
        void                   mousePressEvent(QMouseEvent*) override;
		virtual void           applyChanges(void) = 0;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                             AbstractServiceWidget(const ServiceProxy&, QWidget* = nullptr);
        virtual             ~AbstractServiceWidget() = default;
        const QString&       issue(void) const;
        const ServiceProxy&  service(void) const;
        void                 styleWidget(void);
        virtual QString      toClipboardText(void) = 0;
		virtual void         populateWidget(void) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
