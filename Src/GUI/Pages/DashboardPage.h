//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Bedienfeld-Seite                                                                    //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QBoxLayout>
#include <QMenu>
#include "AbstractPage.h"
#include "AbstractServiceWidget.h"

//////////////////////////////////////////////////////////////////////////////////////////
class EmptyAreaView : public QWidget
{
    Q_OBJECT

    public:
    EmptyAreaView(const QString&, QWidget* = nullptr);
};

//////////////////////////////////////////////////////////////////////////////////////////
namespace Ui
{
    class DashboardPage;
}

//////////////////////////////////////////////////////////////////////////////////////////
class DashboardPage : public AbstractPage
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::DashboardPage*     mUi;
        QMenu                  mMoreMenu;
        QMenu                  mContextMenu;
        AbstractServiceWidget* mCurrentServiceWidget = nullptr;
        volatile bool          mAbortTransfer;
        Cy4Device              mCurrentDevice;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        bool  addArrayOfObjects(const QJsonArray&, QBoxLayout*);
        bool  addObject(const QJsonObject&, QBoxLayout*);
        bool  addGroup(const QJsonObject&, QBoxLayout*&);
        bool  addGrid(const QJsonObject&, QBoxLayout*&);
        bool  addService(const QJsonObject&, QBoxLayout*);
        bool  addSeparator(const QJsonObject&, QBoxLayout*);
        bool  addLabel(const QJsonValue&, QBoxLayout*);
        bool  refreshService(Service&);
        void  populateDashboard(void);
        void  onRefreshAll(void);
        void  onRefreshOne(void);
        void  onReload(void);
        void  onOpenInEditor(void);
        void  onCustomAccess(void);
        void  onCopyToClipboardMenuItem(void);
        void  onDownloadServiceMap(void);
        void  onImportServiceMap(void);
        void  onSaveConfiguration(void);
        void  onLoadConfiguration(void);
        void  onOpenServiceListPageAndSelectService(void);
        void  onContextMenu(QPoint);
        void  onWidgetDataChanged(void);
        void  keyPressEvent(QKeyEvent*) override;
        void  mousePressEvent(QMouseEvent*) override;
        void  retranslateUi(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        explicit                      DashboardPage(QWidget* = nullptr);
                                     ~DashboardPage() override;
        Page                          page(void) const override;
        const AbstractServiceWidget*  currentServiceWidget(void) const;
        void                          onShow(void) override;
        void                          onHide(void) override;
};

