//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Hauptrahmen                                                                         //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QString>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSerialPortInfo>
#include "ui_MainFrame.h"
#include "Version.h"
#include "Conf.h"
#include "TypeDefs.h"
#include "MainFrame.h"
#include "Settings.h"
#include "DeviceList.h"
#include "Cy4Net.h"
#include "ServiceMap.h"
#include "Global.h"

//////////////////////////////////////////////////////////////////////////////////////////
static void
ensureVisible(QMainWindow* window)
{
    QScreen* screen = window->screen();
    if (screen != nullptr)
    {   QRect screenGeometry = screen->availableGeometry();
        QRect windowGeometry = window->geometry();
        if (!screenGeometry.contains(windowGeometry))
        {   window->move(screenGeometry.center() - window->rect().center());
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
static QPushButton*
buttonFor(Page page, const Ui::MainFrame* ui)
{
    switch (page)
        {   case Page::DeviceList:  return ui->deviceListButton;
            case Page::ServiceList: return ui->serviceListButton;
            case Page::Dashboard:   return ui->dashboardButton;
            case Page::Settings:    return ui->settingsButton;
            default:                return nullptr;
        }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::onChanged(void)
{
    QObject* source = sender();
    if (source == mUi->deviceListPage)
    {   const Cy4Device* device = Global::currentDevice();
        if (device == nullptr) return;
        int index = deviceList().indexOf(device->address());
        bool isOnline            = index == INVALID ? false : deviceList().isOnline(index);
        bool isCurrentAvailable  = index != INVALID;
        bool isInApplicationMode = isOnline && !deviceList().at(index).typePlate().isProgramLoader();
        bool allowPageChange = isOnline && isInApplicationMode && isCurrentAvailable;
        mUi->serviceListButton->setEnabled(allowPageChange);
        mUi->dashboardButton->setEnabled(allowPageChange);
        mUi->serviceListMenuItem->setEnabled(mUi->serviceListButton->isEnabled());
        mUi->dashboardMenuItem->setEnabled(mUi->dashboardButton->isEnabled());
    }
    else if (source == mUi->firmwarePage)
    {   mUi->pageButtonArea->setEnabled(!mUi->firmwarePage->uploadInProgress());
    }
    if (!cy4Net().gateway().isValid()) mUi->gatewayInterfaceText->clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::onCy4Activity(void)
{
    QObject* source = sender();
    if (source == &cy4Net())
    {   mCy4ActivityTimer.stop();
        mCy4ActivityTimer.start(100);
        mUi->statusBarActivity->setCurrentIndex(1);
    }
    else
    {   mUi->statusBarActivity->setCurrentIndex(0);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::onPageButton(void)
{
    const struct
    {   QObject*  object;
        Page      page;
    } map[] =
    {   { mUi->deviceListButton      , Page::DeviceList  },
        { mUi->deviceListMenuItem    , Page::DeviceList  },
        { mUi->serviceListButton     , Page::ServiceList },
        { mUi->serviceListMenuItem   , Page::ServiceList },
        { mUi->dashboardButton       , Page::Dashboard   },
        { mUi->dashboardMenuItem     , Page::Dashboard   },
        { mUi->settingsButton        , Page::Settings    },
        { mUi->settingsMenuItem      , Page::Settings    }
    };
    QObject* source = sender();
    for (int row = 0; row < CARDINALITY(map); row++)
    {   if (map[row].object == source)
        {   showPage(map[row].page);
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::onMenuActionAbout(void)
{
    QString  infoText =
    QStringLiteral("<html><body>"
                   "<span style=\"font-size:24pt;font-weight:600;\">%1</span>"
                   "<p><b>Cy4NET Components Administration and Maintenance Tool</b></p>"
                   "<hr/>"
                   "<p>Cy4NET client V%2 (build %3 %4)</p>"
                   "<p>Copyright &copy; 2018 - 2026 Reimar Grasbon</p>"
                   "<p><a href=\"http://cy4net.org\" style=\"font-style:italic;\">"
                   "www.cy4net.org</a></p>"
                   "<hr/>"
                   "<h3>Licence info</h3>"
                   "<p>This software is subjected to the FreeBSD License. "
                   "Redistribution and use in source and binary forms, with or without "
                   "modification, are permitted provided that the following conditions "
                   "are met:</p>"
                   "<p>(1) Redistributions of source code must retain the above "
                   "copyright notice, this list of conditions and the following "
                   "disclaimer.</p>"
                   "<p>(2) Redistributions in binary form must reproduce the above "
                   "copyright notice, this list of conditions and the following "
                   "disclaimer in the documentation and/or other materials provided "
                   "with the distribution.</p>"
                   "<p style=\"font-variant: small-caps;\"><small>"
                   "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "
                   "\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT "
                   "LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS "
                   "FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE "
                   "COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, "
                   "INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL "
                   "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE "
                   "GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS "
                   "INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, "
                   "WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING "
                   "NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF "
                   "THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
                   "</small></p>"
                   "<hr/>"
                   "<p>This application is based on the Community Edition of Qt6, ref. "
                   "<a href=\"https://www.qt.io\" style=\"font-style:italic;\">"
                   "www.qt.io</a>.</p>"
                   "<p>QHexEdit is a hex editor widget written by Winfried Simon in C++ "
                   "for the Qt (Qt5, Qt6) framework Copyright &copy; 2010-2025. "
                   "Sources and licence information can be found on GitHub: "
                   "<a href=\"https://github.com\" "
                   "style=\"font-style:italic;\">://github.com</a>.</p>"
                   "</body></html>"
                   ).arg(QStringLiteral(APPL_NAME),
                         QStringLiteral(VERSION_STRING),
                         QStringLiteral(__DATE__),
                         QStringLiteral(__TIME__));
    QMessageBox aboutBox(this);
    aboutBox.setWindowTitle(tr("About Cy4Tool"));
    aboutBox.setIconPixmap(QStringLiteral(":/ApplIcon/Cy4NET_Icon_064x064.png"));
    aboutBox.setText(infoText);
    aboutBox.addButton(QMessageBox::Close);
    QPushButton* qtButton    = aboutBox.addButton("About Qt...", QMessageBox::ActionRole);
    aboutBox.exec();
    if (aboutBox.clickedButton() == qtButton) QMessageBox::aboutQt(this, "About Qt");
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::onStatusBarTimeout(void)
{
    QObject* source = sender();
    if (source == &mStatusBarMessageTimer)
    {   clearStatusBarMessage();
    }
    else if (source == &mStatusBarProgressTimer)
    {   mUi->statusBarProgressBar->hide();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::connectToGateway(void)
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    cy4Net().close();
    mUi->gatewayInterfaceText->clear();
    switch (settings().currentGatewayConnection)
    {   case ConnectionType::Socket:
        {   // TCP-Verbindung
            const Socket& socket = settings().gatewaySocket;
            showStatusBarInfo(tr("Connect socket %1 : %2 ...").arg(socket.ip).arg(socket.port));
            Global::doMainLoop();
            if (cy4Net().open(socket.ip, socket.port))
            {   if (cy4Net().callGateway())
                {   mUi->gatewayInterfaceText->setText(QStringLiteral("Gateway @ %1")
                                                                     .arg(settings().gatewaySocket.ip));
                }
                else
                {   showStatusBarWarning(tr("No gateway connected"), true);
                }
            }
            else
            {   showStatusBarWarning(tr("Connection to socket %1 failed").arg(socket.ip));
            }
            break;
        }
        case ConnectionType::Serial:
        {   // Serielle Verbindung
            QList<QString> listOfPorts;
            // (1) Verfügbare serielle Ports auflisten, voreingestellten als erstes
            {   QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
                for (int i=0; i<list.length(); i++)
                {   QString portName = list[i].systemLocation();
                    if (portName.startsWith(QStringLiteral("\\\\.\\")))
                    {   portName = portName.right(portName.length()-4);
                    }
                    if (portName.contains(QStringLiteral("COM"), Qt::CaseInsensitive))
                    {   portName = portName.toUpper();
                    }
                    if (portName == settings().gatewaySerialPort)
                    {   listOfPorts.prepend(portName);
                    }
                    else if (settings().autoSerialPortDetection)
                    {   listOfPorts.append(portName);
                    }
                }
                if (listOfPorts.isEmpty())
                {   QString text;
                    if (settings().autoSerialPortDetection)
                    {   text = tr("No serial ports available");
                    }
                    else if (settings().gatewaySerialPort.isEmpty())
                    {   text = tr("Port not specified");
                    }
                    else
                    {   text = tr("Port not available: %1").arg(settings().gatewaySerialPort);
                    }
                    showStatusBarWarning(text);
                    break;
                }
            }
            // (2) Alle Ports der Liste durchgehen und versuchen ein Gateway to kontaktieren
            for (int i=0; i<listOfPorts.length(); i++)
            {   QString portName = listOfPorts[i];
                if (cy4Net().open(portName))
                {   showStatusBarInfo(tr("Probe %1 ...").arg(portName));
                    if (cy4Net().callGateway())
                    {   mUi->gatewayInterfaceText->setText(QStringLiteral("Gateway @ %1").arg(portName));
                        settings().gatewaySerialPort = portName;
                        break;
                    }
                }
            }
            if (!cy4Net().gateway().isValid())
            {   QString text;
                if (settings().autoSerialPortDetection)
                {   text = tr("No response from gateway");
                }
                else
                {   text = tr("No response from gateway @ %1").arg(listOfPorts[0]);
                }
                showStatusBarInfo(text, true);
            }
            break;
        }
        default:
        {   // Bislang keine Verbingsart ausgewählt
        }
    }
    bool connectionEstablished = cy4Net().gateway().isValid();
    if (connectionEstablished)
    {   deviceList().add(cy4Net().gateway());
        showStatusBarInfo(tr("Connection to gateway at %1 established").arg(cy4Net().gateway().toText().c_str()));
        if (currentPage() == mUi->deviceListPage) showPage(Page::Settings);
        showPage(Page::DeviceList);
    }
    QGuiApplication::restoreOverrideCursor();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::closeEvent(QCloseEvent* event)
{
    if (mUi->firmwarePage->uploadInProgress())
    {   if (QMessageBox::question(this,
                                  APPL_NAME,
                                  tr("Firmware upload in progress.<br>"
                                     "Exit application anyway?"),
                                  QMessageBox::Yes,
                                  QMessageBox::No) == QMessageBox::No)
        {   event->ignore();
            return;
        }
    }
    serviceMap().clear();
    cy4Net().close();
    AbstractPage* instance = static_cast<AbstractPage*>(mUi->pageArea->currentWidget());
    instance->onHide();
    settings().windowPosition = pos();
    settings().windowSize     = size();
    event->accept();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::keyPressEvent(QKeyEvent* event)
{
    // 'KeyPress'-Ereignisse an die aktuelle Seite weiterleiten
    AbstractPage* page = currentPage();
    if (page->page() == Page::DeviceList ||
        page->page() == Page::Dashboard)
    {   QApplication::sendEvent(page, event);
        if (event->isAccepted()) return;
    }
    QMainWindow::keyPressEvent(event);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) mUi->retranslateUi(this);
    QWidget::changeEvent(event);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::updateLanguage(void)
{
    QString translationFile;
    if (settings().locale != QLocale(QLocale::English, QLocale::UnitedStates))
    {   QString code = Global::toCode(settings().locale);
        translationFile = QString(":/Trans/trans_%1.qm").arg(code);
    }
    if (mTranslator.load(translationFile))
    {   QCoreApplication* app = QCoreApplication::instance();
        if (app != NULL) app->installTranslator(&mTranslator);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
MainFrame::MainFrame(QWidget* parent) : QMainWindow(parent)
{
    // Fenster: GUI, Titel, Position und Größe
    {   mUi = new Ui::MainFrame;
        mUi->setupUi(this);
        setWindowTitle(QStringLiteral("%1 V%2").arg(QStringLiteral(APPL_NAME),
                                                    QStringLiteral(VERSION_STRING)));
        move(settings().windowPosition);
        ensureVisible(this);
        resize(settings().windowSize);
    }
    // Menü/Kontextmenü
    {   mMenu.addAction(mUi->deviceListMenuItem);
        mMenu.addAction(mUi->serviceListMenuItem);
        mMenu.addAction(mUi->dashboardMenuItem);
        mMenu.addAction(mUi->settingsMenuItem);
        mMenu.addSeparator();
        mMenu.addAction(mUi->aboutMenuItem);
        mMenu.addAction(mUi->closeApplicationMenuItem);
        mUi->menuButton->setMenu(&mMenu);
    }
    // Ereignisverknüpfungen
    {   connect(mUi->deviceListPage          , &DeviceListPage::selectedChanged, this, &MainFrame::onChanged);
        connect(mUi->firmwarePage            , &FirmwarePage::uploadChanged    , this, &MainFrame::onChanged);
        connect(&cy4Net()                    , &Cy4Net::activity               , this, &MainFrame::onCy4Activity, Qt::QueuedConnection);
        connect(&mCy4ActivityTimer           , &QTimer::timeout                , this, &MainFrame::onCy4Activity);
        connect(&mStatusBarMessageTimer      , &QTimer::timeout                , this, &MainFrame::onStatusBarTimeout);
        connect(&mStatusBarProgressTimer     , &QTimer::timeout                , this, &MainFrame::onStatusBarTimeout);
        connect(mUi->deviceListButton        , &QPushButton::clicked           , this, &MainFrame::onPageButton);
        connect(mUi->serviceListButton       , &QPushButton::clicked           , this, &MainFrame::onPageButton);
        connect(mUi->dashboardButton         , &QPushButton::clicked           , this, &MainFrame::onPageButton);
        connect(mUi->settingsButton          , &QPushButton::clicked           , this, &MainFrame::onPageButton);
        connect(mUi->deviceListMenuItem      , &QAction::triggered             , this, &MainFrame::onPageButton);
        connect(mUi->serviceListMenuItem     , &QAction::triggered             , this, &MainFrame::onPageButton);
        connect(mUi->dashboardMenuItem       , &QAction::triggered             , this, &MainFrame::onPageButton);
        connect(mUi->settingsMenuItem        , &QAction::triggered             , this, &MainFrame::onPageButton);
        connect(mUi->aboutMenuItem           , &QAction::triggered             , this, &MainFrame::onMenuActionAbout);
        connect(mUi->closeApplicationMenuItem, &QAction::triggered             , this, &MainFrame::close);
    }
    // Entsprechende Schaltflächen vorerst deaktivieren
    {   mUi->serviceListButton->setEnabled(false);
        mUi->dashboardButton->setEnabled(false);
        mUi->serviceListMenuItem->setEnabled(false);
        mUi->dashboardMenuItem->setEnabled(false);
    }
    // Sonstige Initialisierungen
    {   mStatusBarMessageTimer.setSingleShot(true);
        mUi->statusBarMessageStack->setCurrentIndex(0);
        mCy4ActivityTimer.setSingleShot(true);
        mUi->statusBarActivity->setCurrentIndex(0);
        mUi->gatewayInterfaceText->clear();
        mUi->pageArea->setCurrentIndex(0);
        mUi->deviceListButton->setChecked(true);
        mUi->statusBarProgressBar->hide();
        mPreviousPage = nullptr;
        updateLanguage();
    }
    // Öffnen der Startseite und Verbindung zum Gateway initiieren
    {   QTimer::singleShot(100, mUi->deviceListPage, &DeviceListPage::onShow);
        QTimer::singleShot(1000, this, &MainFrame::connectToGateway);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
MainFrame::~MainFrame()
{
    mStatusBarMessageTimer.stop();
    mCy4ActivityTimer.stop();
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::showPage(Page page)
{
    if (page == Page::None) return;    
    clearStatusBarMessage();
    // (1) Die zu <page> gehörende Seitenschaltfläche auswählen
    for (int i=0; i<mUi->pageArea->count(); i++)
    {   AbstractPage* pageWidget = static_cast<AbstractPage*>(mUi->pageArea->widget(i));
        QPushButton* button = buttonFor(pageWidget->page(), mUi);
        if (button != nullptr) button->setChecked(page == pageWidget->page());
    }
    // (2) Index der gewünschten Seite ermitteln
    int index = INVALID;
    for (int i=0; i<mUi->pageArea->count(); i++)
    {   AbstractPage* pageWidget = static_cast<AbstractPage*>(mUi->pageArea->widget(i));
        if (page == pageWidget->page()) index = i;
    }
    // (3) Bestehende Seite schliessen, gewünschte Seite öffnen
    AbstractPage* currentPage = MainFrame::currentPage();
    if (index != INVALID && page != currentPage->page())
    {   currentPage->onHide();
        mPreviousPage = currentPage;
        mUi->pageArea->setCurrentIndex(index);
        QPushButton* button = buttonFor(page, mUi);
        if (button != nullptr && !button->isDown() && !button->isChecked())
        {   button->setChecked(true);
        }
        for (int i=0; i<mUi->pageArea->count(); i++)
        {   AbstractPage* pageWidget = static_cast<AbstractPage*>(mUi->pageArea->widget(i));
            if (pageWidget->page() == page)
            pageWidget->onShow();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
AbstractPage*
MainFrame::page(Page page) const
{
    switch (page)
    {   case Page::DeviceList:   return mUi->deviceListPage;
        case Page::ServiceList:  return mUi->serviceListPage;
        case Page::Dashboard:    return mUi->dashboardPage;
        case Page::Settings:     return mUi->settingsPage;
        case Page::Firmware:     return mUi->firmwarePage;
        case Page::CustomAccess: return mUi->customAccessPage;
        default:                 return nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
AbstractPage*
MainFrame::currentPage(void) const
{
    return static_cast<AbstractPage*>(mUi->pageArea->currentWidget());
}

//////////////////////////////////////////////////////////////////////////////////////////
AbstractPage*
MainFrame::previousPage(void) const
{
    return mPreviousPage;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::showStatusBarInfo(const QString& text, bool permanent)
{
    mUi->statusBarInfoText->setText(text);
    mUi->statusBarMessageStack->setCurrentIndex(1);
    if (permanent) mStatusBarMessageTimer.stop();
    else           mStatusBarMessageTimer.start(CFG::StatusBarMessageTime*1000);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::showStatusBarWarning(const QString& text, bool permanent)
{
    mUi->statusBarWarningText->setText(text);
    mUi->statusBarMessageStack->setCurrentIndex(2);
    if (permanent) mStatusBarMessageTimer.stop();
    else           mStatusBarMessageTimer.start(CFG::StatusBarMessageTime*1000);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::clearStatusBarMessage(void)
{
    mUi->statusBarMessageStack->setCurrentIndex(0);
    mUi->statusBarInfoText->clear();
    mUi->statusBarWarningText->clear();
    mStatusBarMessageTimer.stop();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::showStatusBarProgress(int start, int progress, int end)
{
    if (!mUi->statusBarProgressBar->isVisible()) mUi->statusBarProgressBar->show();
    mUi->statusBarProgressBar->setMinimum(start);
    mUi->statusBarProgressBar->setMaximum(end);
    mUi->statusBarProgressBar->setValue(progress);
    mStatusBarProgressTimer.start(CFG::StatusBarProgressTime*1000);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
MainFrame::setEnabled(bool enable)
{
    mUi->pageButtonArea->setEnabled(enable);
}

//////////////////////////////////////////////////////////////////////////////////////////
MainFrame&
mainFrame(void)
{
    static MainFrame instance;
    return instance;
}

//////////////////////////////////////////////////////////////////////////////////////////
