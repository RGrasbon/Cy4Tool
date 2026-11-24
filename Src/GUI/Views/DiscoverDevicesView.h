//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Gerätefindungsansicht                                                               //
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
    class DiscoverDevicesView;
}

//////////////////////////////////////////////////////////////////////////////////////////
class DiscoverDevicesView : public QWidget
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::DiscoverDevicesView* mUi;
        volatile bool            mDiscoveryInProgress = false;

    // Signale ///////////////////////////////////////////////////////////////////////////
    signals:
        void hided(void);

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void  onTextChanged(void);
        void  onSelectLandingPageButton(void);
        void  onDiscover(void);
        void  onClose(void);
        void  keyPressEvent(QKeyEvent*) override;
        void  changeEvent(QEvent*) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        explicit  DiscoverDevicesView(QWidget* = nullptr);
                 ~DiscoverDevicesView() override;
        void      show(void);
        void      hide(void);
};

