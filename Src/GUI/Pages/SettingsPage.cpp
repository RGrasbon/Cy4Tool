//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Einstellungsseite                                                                   //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QFileDialog>
#include "ui_SettingsPage.h"
#include "Global.h"
#include "SettingsPage.h"
#include "Settings.h"
#include "Cy4Net.h"

//////////////////////////////////////////////////////////////////////////////////////////
static QString
pathNameForFlag(const QString& code)
{
    QString tmpCode = code;
    if (tmpCode == "en-US") tmpCode = "en-GB"; // UK- anstelle US-Flagge
    QString pathFileName = QString(":/Symbols/Flag_%1.png").arg(tmpCode);
    if (!QFile::exists(pathFileName)) return QString();
    return pathFileName;
}

//////////////////////////////////////////////////////////////////////////////////////////
static void
setFlagOnLanguageButton(QPushButton* button)
{
    QString code = Global::toCode(settings().locale);
    QString filePathName = pathNameForFlag(code);
    if (filePathName.isEmpty())
    {   // Kein Flagge, dann Code-Text
        button->setText(code);
        button->setIcon(QIcon());
    }
    else
    {   button->setText("");
        button->setIcon(QIcon((filePathName)));
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::onEditingFinished(void)
{
    qDebug("onEditingFinished(void)");

    settings().gatewaySocket                 = Socket(mUi->socketIpText->text(), mUi->socketPortText->text().toInt());
    settings().gatewaySerialPort             = mUi->serialPortText->text();
    settings().localCy4ToolDataDirectoryPath = mUi->localCy4ToolDataDirectoryText->text();
    settings().localServiceMapsDirectoryPath = mUi->localServiceMapsDirectoryText->text();
    settings().onlineResourcesUrl            = mUi->onlineResourcesUrlText->text();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::populateUi(void)
{
    mUi->socketIpText->setText(settings().gatewaySocket.ip);
    mUi->socketPortText->setText(QString::number(settings().gatewaySocket.port));
    mUi->serialPortText->setText(settings().gatewaySerialPort);
    mUi->autoSerialPortDetectionCheck->setChecked(settings().autoSerialPortDetection);
    mUi->localCy4ToolDataDirectoryText->setText(settings().localCy4ToolDataDirectoryPath);
    mUi->localServiceMapsDirectoryText->setText(settings().localServiceMapsDirectoryPath);
    mUi->onlineResourcesUrlText->setText(settings().onlineResourcesUrl);
    {   QSignalBlocker blocker(mUi->reloadOnOpenDashboardPageCheckBox);
        mUi->reloadOnOpenDashboardPageCheckBox->setChecked(settings().reloadOnOpenDashboardPage);
    }
    setFlagOnLanguageButton(mUi->languageButton);
    {   QSignalBlocker blocker(mUi->timeFormat24hCheckBox);
        mUi->timeFormat24hCheckBox->setChecked(settings().timeFormat24h);
    }
    {   QSignalBlocker blocker(mUi->dateFormatDDMMYYYCheckBox);
        mUi->dateFormatDDMMYYYCheckBox->setChecked(settings().dateFormatDDMMYYY);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::onConnect(void)
{
    QObject* source = sender();
    if (source == mUi->connectViaSocketButton)
    {   settings().currentGatewayConnection = ConnectionType::Socket;
    }
    else if (source == mUi->connectViaSerialButton)
    {   settings().currentGatewayConnection = ConnectionType::Serial;
    }
    cy4Net().close();
    Global::connectToGateway();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::onAutoSerialPortDetection(int newState)
{
    switch (Qt::CheckState(newState))
    {   case Qt::Unchecked: settings().autoSerialPortDetection = false; break;
        case Qt::Checked:   settings().autoSerialPortDetection = true;  break;
        default:            ;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::onSelectDirectory(void)
{
    QObject* source = sender();
    if (source == mUi->selectLocalCy4ToolDataDirectoryButton)
    {   QString title = tr("Select local Cy4Tool data directory");
        QString directoryPath = mUi->localCy4ToolDataDirectoryText->text();
        if (!QFileInfo::exists(directoryPath)) directoryPath = defaults().localCy4ToolDataDirectoryPath;
        directoryPath = QFileDialog::getExistingDirectory(NULL, title, directoryPath);
        if( !directoryPath.isEmpty() )
        {   QString hhh = QDir::toNativeSeparators(directoryPath);
            settings().localCy4ToolDataDirectoryPath = QDir::toNativeSeparators(directoryPath);
            populateUi();
        }
    }
    else if (source == mUi->selectLocalServiceMapsDirectoryButton)
    {   QString title = tr("Select local service maps directory");
        QString directoryPath = mUi->localServiceMapsDirectoryText->text();
        if (!QFileInfo::exists(directoryPath)) directoryPath = defaults().localServiceMapsDirectoryPath;
        directoryPath = QFileDialog::getExistingDirectory(NULL, title, directoryPath);
        if( !directoryPath.isEmpty() )
        {   settings().localServiceMapsDirectoryPath = QDir::toNativeSeparators(directoryPath);
            populateUi();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::onReset(void)
{
    QObject* source = sender();
    if (source == mUi->resetSocketButton)
    {   settings().gatewaySocket = defaults().gatewaySocket;
    }
    else if (source == mUi->resetSerialPortButton)
    {   settings().gatewaySerialPort = defaults().gatewaySerialPort;
        settings().autoSerialPortDetection = defaults().autoSerialPortDetection;
    }
    else if (source == mUi->resetLocalCy4ToolDataDirectoryButton)
    {   settings().localCy4ToolDataDirectoryPath = defaults().localCy4ToolDataDirectoryPath;
    }
    else if (source == mUi->resetLocalServiceMapsDirectoryButton)
    {   settings().localServiceMapsDirectoryPath = defaults().localServiceMapsDirectoryPath;
    }
    else if (source == mUi->resetOnlineResourcesUrlButton)
    {   settings().onlineResourcesUrl = defaults().onlineResourcesUrl;
    }
    populateUi();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::onReloadOnOpenDashboardPage(bool checked)
{
    settings().reloadOnOpenDashboardPage = checked;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::onAllowLanguageFallback(bool checked)
{
    settings().allowLanguageFallback = checked;
    QEvent languageChangeEvent(QEvent::LanguageChange);
    QCoreApplication::sendEvent(qApp, &languageChangeEvent);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::onFlipLanguage(void)
{
    QLocale locale = settings().locale;
    QString code = Global::toCode(locale);
    QStringList list = Global::translations();
    int index = qMax(0, list.indexOf(code));
    index = (index+1)%list.size();
    code = list[index];
    settings().locale = Global::toLocal(code);
    setFlagOnLanguageButton(mUi->languageButton);
    Global::updateLanguage();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::onDateTimeFormat(bool checked)
{
    QObject* source = sender();
    if (source == mUi->timeFormat24hCheckBox)
    {   settings().timeFormat24h = checked;
    }
    else if (source == mUi->dateFormatDDMMYYYCheckBox)
    {   settings().dateFormatDDMMYYY = checked;
    }
    QEvent languageChangeEvent(QEvent::LanguageChange);
    QCoreApplication::sendEvent(qApp, &languageChangeEvent);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::retranslateUi(void)
{
    mUi->retranslateUi(this);
}

//////////////////////////////////////////////////////////////////////////////////////////
SettingsPage::SettingsPage(QWidget* parent) : AbstractPage(parent)
{
    // GUI
    {   mUi = new Ui::SettingsPage;
        mUi->setupUi(this);
    }
    // Ereignisverknüpfungen
    {
        connect(mUi->socketIpText                         , &QLineEdit::editingFinished  , this, &SettingsPage::onEditingFinished);
        connect(mUi->socketPortText                       , &QLineEdit::editingFinished  , this, &SettingsPage::onEditingFinished);
        connect(mUi->serialPortText                       , &QLineEdit::editingFinished  , this, &SettingsPage::onEditingFinished);
        connect(mUi->localCy4ToolDataDirectoryText        , &QLineEdit::editingFinished  , this, &SettingsPage::onEditingFinished);
        connect(mUi->localServiceMapsDirectoryText        , &QLineEdit::editingFinished  , this, &SettingsPage::onEditingFinished);
        connect(mUi->onlineResourcesUrlText               , &QLineEdit::editingFinished  , this, &SettingsPage::onEditingFinished);
        connect(mUi->connectViaSocketButton               , &QPushButton::clicked        , this, &SettingsPage::onConnect);
        connect(mUi->connectViaSerialButton               , &QPushButton::clicked        , this, &SettingsPage::onConnect);
        connect(mUi->autoSerialPortDetectionCheck         , &QCheckBox::checkStateChanged, this, &SettingsPage::onAutoSerialPortDetection);
        connect(mUi->selectLocalCy4ToolDataDirectoryButton, &QToolButton::clicked        , this, &SettingsPage::onSelectDirectory);
        connect(mUi->selectLocalServiceMapsDirectoryButton, &QToolButton::clicked        , this, &SettingsPage::onSelectDirectory);
        connect(mUi->resetSocketButton                    , &QToolButton::clicked        , this, &SettingsPage::onReset);
        connect(mUi->resetSerialPortButton                , &QToolButton::clicked        , this, &SettingsPage::onReset);
        connect(mUi->resetLocalCy4ToolDataDirectoryButton , &QToolButton::clicked        , this, &SettingsPage::onReset);
        connect(mUi->resetLocalServiceMapsDirectoryButton , &QToolButton::clicked        , this, &SettingsPage::onReset);
        connect(mUi->resetOnlineResourcesUrlButton        , &QToolButton::clicked        , this, &SettingsPage::onReset);
        connect(mUi->reloadOnOpenDashboardPageCheckBox    , &QCheckBox::toggled          , this, &SettingsPage::onReloadOnOpenDashboardPage);
        connect(mUi->allowLanguageFallbackCheckBox        , &QCheckBox::toggled          , this, &SettingsPage::onAllowLanguageFallback);
        connect(mUi->languageButton                       , &QToolButton::clicked        , this, &SettingsPage::onFlipLanguage);
        connect(mUi->timeFormat24hCheckBox                , &QCheckBox::toggled          , this, &SettingsPage::onDateTimeFormat);
        connect(mUi->dateFormatDDMMYYYCheckBox            , &QCheckBox::toggled          , this, &SettingsPage::onDateTimeFormat);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
SettingsPage::~SettingsPage()
{
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
Page
SettingsPage::page(void) const
{
    return Page::Settings;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::onShow(void)
{
    populateUi();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
SettingsPage::onHide(void)
{    
}

//////////////////////////////////////////////////////////////////////////////////////////
