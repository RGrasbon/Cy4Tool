//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Geräteliste-Seite                                                                   //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QMenu>
#include <QItemSelection>
#include <QStyledItemDelegate>
#include <QPainter>
#include "AbstractPage.h"
#include "Cy4Types.h"

//////////////////////////////////////////////////////////////////////////////////////////
namespace Ui
{
    class DeviceListPage;
}

//////////////////////////////////////////////////////////////////////////////////////////
class DeviceListPage : public AbstractPage
{
    Q_OBJECT

    // Typen /////////////////////////////////////////////////////////////////////////////
    private:
        class RowDelegate : public QStyledItemDelegate
        {   public:
                RowDelegate(QObject* = nullptr);
                void paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;
        };

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::DeviceListPage*  mUi;
        RowDelegate*         mRowDelegate;
        QMenu                mMoreMenu;
        QMenu                mContextMenu;
        volatile bool        mRefreshingInProgress = false;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void  updateSeparatorLines(void);
        void  onChanged(void);
        void  onAdd(void);
        void  onRemove(void);
        void  onDiscover(void);
        void  onUploadFirmware(void);
        void  onRefresh(void);
        void  onRefreshAll(void);
        void  onRestart(void);
        void  onEnterPgmLdr(void);
        void  onChangeAddress(void);
        void  onMarkAsNew(void);
        void  onImportExportDeviceList(void);
        void  onCustomAccess(void);
        void  onRowClicked(const QModelIndex&);
        void  onRowDoubleClicked(const QModelIndex&);
        void  onContextMenu(QPoint);
        void  onDeviceInserted(int);
        void  onDeviceRemoved(Cy4Address);
        void  scrollToRow(int);
        void  keyPressEvent(QKeyEvent*) override;
        void  retranslateUi(void) override;

    // Signale ///////////////////////////////////////////////////////////////////////////
    Q_SIGNALS:
        void  selectedChanged(void);

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        explicit          DeviceListPage(QWidget* = nullptr);
                         ~DeviceListPage() override;
        Page              page(void) const override;
        const Cy4Device*  currentDevice(void) const;
        void              onShow(void) override;
        void              onHide(void) override;
};
