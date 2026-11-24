//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Hauptrahmen                                                                         //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QMenu>
#include <QTranslator>
#include "AbstractPage.h"
#include "Global.h"

//////////////////////////////////////////////////////////////////////////////////////////
namespace Ui
{
    class MainFrame;
}

//////////////////////////////////////////////////////////////////////////////////////////
class MainFrame : public QMainWindow
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Ui::MainFrame*  mUi;
        QTranslator     mTranslator;
        QMenu           mMenu;
        QTimer          mStatusBarMessageTimer;
        QTimer          mStatusBarProgressTimer;
        QTimer          mCy4ActivityTimer;
        AbstractPage*   mPreviousPage;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
    // Interna ///////////////////////////////////////////////////////////////////////////
    protected:
        void  onChanged(void);
        void  onCy4Activity(void);
        void  onPageButton(void);
        void  onMenuActionAbout(void);
        void  onStatusBarTimeout(void);
        void  updateLanguage(void);
        void  connectToGateway(void);
        void  closeEvent(QCloseEvent*) override;
        void  keyPressEvent(QKeyEvent*) override;
        void  changeEvent(QEvent*) override;
              MainFrame(QWidget* = nullptr);
             ~MainFrame() override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        void                 showPage(Page);
        AbstractPage*        page(Page) const;
        AbstractPage*        currentPage(void) const;
        AbstractPage*        previousPage(void) const;
        void                 showStatusBarInfo(const QString&, bool = false);
        void                 showStatusBarWarning(const QString&, bool = false);
        void                 clearStatusBarMessage(void);
        void                 showStatusBarProgress(int, int, int);
        void                 setEnabled(bool);
        friend void          Global::updateLanguage(void);
        friend void          Global::connectToGateway(void);
        friend MainFrame&    mainFrame(void);
};

//////////////////////////////////////////////////////////////////////////////////////////
MainFrame& mainFrame(void);

