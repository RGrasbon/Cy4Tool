//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Abstrakte Basisklasse Seite                                                         //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>

//////////////////////////////////////////////////////////////////////////////////////////
enum class Page
{   DeviceList,
    ServiceList,
    Dashboard,
    Settings,
    Firmware,
    CustomAccess,
    None
};

//////////////////////////////////////////////////////////////////////////////////////////
class AbstractPage : public QWidget
{
    Q_OBJECT

    // Interna ///////////////////////////////////////////////////////////////////////////
    protected:
        void changeEvent(QEvent*) override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        explicit        AbstractPage(QWidget* = nullptr);
        virtual Page    page(void) const = 0;
        virtual void    onShow(void) = 0;
        virtual void    onHide(void) = 0;
        virtual void    retranslateUi(void) = 0;
};
