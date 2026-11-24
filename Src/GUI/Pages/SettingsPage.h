//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Einstellungsseite                                                                   //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AbstractPage.h"

//////////////////////////////////////////////////////////////////////////////////////////
namespace Ui
{
    class SettingsPage;
}

//////////////////////////////////////////////////////////////////////////////////////////
class SettingsPage : public AbstractPage
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::SettingsPage*  mUi;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void  populateUi(void);
        void  onEditingFinished(void);
        void  onConnect(void);
        void  onAutoSerialPortDetection(int);
        void  onSelectDirectory(void);
        void  onReset(void);
        void  onReloadOnOpenDashboardPage(bool);
        void  onAllowLanguageFallback(bool);
        void  onFlipLanguage(void);
        void  onDateTimeFormat(bool);
        void  retranslateUi(void) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        explicit  SettingsPage(QWidget* = nullptr);
                 ~SettingsPage() override;
        Page      page(void) const override;
        void      onShow(void) override;
        void      onHide(void) override;
};

