//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Gerät hinzufügen Ansicht                                                            //
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
    class AddDeviceView;
}

//////////////////////////////////////////////////////////////////////////////////////////
class AddDeviceView : public QWidget
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::AddDeviceView*  mUi;
        bool                mDiscoveryInProgress = false;

    // Signale ///////////////////////////////////////////////////////////////////////////
    signals:
        void  hided(void);

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void  onTextChanged(void);
        void  onAdd(void);
        void  onClose(void);
        void  keyPressEvent(QKeyEvent*) override;
        void  changeEvent(QEvent*) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        explicit  AddDeviceView(QWidget* = nullptr);
                 ~AddDeviceView() override;
        void      show(void);
        void      hide(void);
};

