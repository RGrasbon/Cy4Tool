//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Benutzerdefinierter Zugriff                                                         //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QMenu>
#include "AbstractPage.h"
#include "Cy4Types.h"

//////////////////////////////////////////////////////////////////////////////////////////
namespace Ui
{
    class CustomAccessPage;
}

//////////////////////////////////////////////////////////////////////////////////////////
class CustomAccessPage : public AbstractPage
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::CustomAccessPage* mUi;
        QMenu                 mMoreMenu;
        QMenu                 mContextMenu;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void  onGoBackButton(void);
        void  onClearAll(void);
        void  onParameterChanged(void);
        void  onLocationTextChanged(void);
        void  onLocationBaseChanged(void);
        void  onAccess(void);
        void  onCopyToClipboard(void);
        void  onOverwriteData(void);
        void  onLocationBase(void);
        void  onContextMenu(QPoint position);
        void  onDataChanged(void);
        void  onCurrentLocationChanged(qint64);
        void  validateParameter(void);
        void  transceive(Cy4Message::Access);
        void  retranslateUi(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        explicit  CustomAccessPage(QWidget* = nullptr);
                 ~CustomAccessPage() override;
        Page      page(void) const override;
        void      onShow(void) override;
        void      onHide(void) override;
};

