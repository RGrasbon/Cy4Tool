//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Dienst-Detailansicht                                                                //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>

//////////////////////////////////////////////////////////////////////////////////////////
namespace Ui
{
    class ServiceDetailsView;
}

//////////////////////////////////////////////////////////////////////////////////////////
class ServiceDetailsView : public QWidget
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::ServiceDetailsView* mUi;

        // Interna ///////////////////////////////////////////////////////////////////////////
        void  onCustomAccess(void);
        void  onCopyToClipboard(void);
        void  changeEvent(QEvent*) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        explicit  ServiceDetailsView(QWidget* = nullptr);
                 ~ServiceDetailsView() override;
        void      show(void);
        void      hide(void);
};
