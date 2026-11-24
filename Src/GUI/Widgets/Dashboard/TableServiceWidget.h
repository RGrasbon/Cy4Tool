//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Service-Steuerelement Tabelle                                                       //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QTableWidget>
#include <QStyledItemDelegate>
#include "AbstractServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
class TableServiceItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

	// Interna ///////////////////////////////////////////////////////////////////////////
    protected:
        void     paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;
        QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;        

	// Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        TableServiceItemDelegate(QWidget *parent = nullptr);
};

//////////////////////////////////////////////////////////////////////////////////////////
class TableServiceWidget : public AbstractServiceWidget
{
    Q_OBJECT

    friend class TableServiceItemDelegate;

    // Daten /////////////////////////////////////////////////////////////////////////////
        QTableWidget*          mTableWidget;
        NumberFormat           mFormat;
        int                    mWidth;
        ValueDomain            mValueDomain;
        Precision              mPrecision;
        NumberRange            mRange;
        QMap<double, QString>  mSpecialValueMap;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        int     toWidth(const QJsonObject&);
        int     toColumnCount(const QJsonObject&);
        void    setItemValue(QTableWidgetItem*, double);
        void    setItemText(QTableWidgetItem*, const QString&);
        double  fromItemText(QTableWidgetItem*);        
        void    onItemChanged(QTableWidgetItem*);
        bool    eventFilter(QObject*, QEvent*) override;
		void    applyChanges(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                  TableServiceWidget(const ServiceProxy&,
                                     const QJsonObject&,
                                     QWidget* = nullptr);
        virtual  ~TableServiceWidget() = default;
		QString   toClipboardText(void) override;
        void      populateWidget(void) override;        
};

//////////////////////////////////////////////////////////////////////////////////////////
