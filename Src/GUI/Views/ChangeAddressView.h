//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Geräteadresse ändern Ansicht                                                        //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
#include "Cy4Types.h"

//////////////////////////////////////////////////////////////////////////////////////////
namespace Ui
{
    class ChangeAddressView;
}

//////////////////////////////////////////////////////////////////////////////////////////
class ChangeAddressView : public QWidget
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::ChangeAddressView*  mUi;
        Cy4Device               mDevice;

    // Signale ///////////////////////////////////////////////////////////////////////////
    signals:
        void  hided(void);

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void  validateAddress(void);
        void  onPresetDefault(void);
        void  onTextChanged(void);
        void  onEditingFinished(void);
        void  onCheckAddress(void);
        void  onChange(void);
        void  onClose(void);
        void  keyPressEvent(QKeyEvent*) override;
        void  changeEvent(QEvent*) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        explicit  ChangeAddressView(QWidget* = nullptr);
                 ~ChangeAddressView() override;
        void      show(const Cy4Device&);
        void      hide(void);
};

