//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Diensteliste-Seite                                                                  //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QMenu>
#include "AbstractPage.h"
#include "Service.h"

//////////////////////////////////////////////////////////////////////////////////////////
namespace Ui
{
    class ServiceListPage;
}

//////////////////////////////////////////////////////////////////////////////////////////
class ServiceListPage : public AbstractPage
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::ServiceListPage*  mUi;
        QMenu                 mMoreMenu;
        QMenu                 mContextMenu;
        Cy4Device             mCurrentDevice;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void  populateList(void);
        void  onOpenInfoView(void);
        void  onReload(void);
        void  onOpenInEditor(void);
        void  onCustomAccess(void);
        void  onSearchTextChanged(void);
        void  onFindNext(void);
        void  onDownload(void);
        void  onImport(void);
        void  onCopyToClipboard(void);
        void  onContextMenu(QPoint);
        void  keyPressEvent(QKeyEvent*) override;
        void  retranslateUi(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        explicit        ServiceListPage(QWidget* = nullptr);
                       ~ServiceListPage() override;
        Page            page(void) const override;
        const Service*  currentService(void) const;
        void            onShow(void) override;
        void            onHide(void) override;
};

