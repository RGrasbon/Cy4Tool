//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Firmware-Seite                                                                      //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AbstractPage.h"
#include "Cy4Types.h"

//////////////////////////////////////////////////////////////////////////////////////////
namespace Ui
{
    class FirmwarePage;
}

//////////////////////////////////////////////////////////////////////////////////////////
class FirmwarePage : public AbstractPage
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::FirmwarePage*  mUi;
        Cy4Device          mDevice;
        volatile bool      mUploadInProgress;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void  onSelectFile(void);
        void  onFirmwareChanged(void);
        void  onDeploy(void);
        void  retranslateUi(void) override;

    // Signale ///////////////////////////////////////////////////////////////////////////
    Q_SIGNALS:
        void  uploadChanged(void);

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        explicit  FirmwarePage(QWidget* = nullptr);
                 ~FirmwarePage() override;
        Page      page(void) const override;
        void      onShow(void) override;
        void      onHide(void) override;
        bool      uploadInProgress(void) const;
};

