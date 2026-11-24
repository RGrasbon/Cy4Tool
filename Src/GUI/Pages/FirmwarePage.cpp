//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Firmware-Seite                                                                      //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QPushButton>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include "ui_FirmwarePage.h"
#include "Settings.h"
#include "MainFrame.h"
#include "FirmwarePage.h"
#include "DeviceList.h"
#include "Cy4Firmware.h"
#include "Cy4Net.h"

//////////////////////////////////////////////////////////////////////////////////////////
enum class Symbol
{   Blank,
    Info,
    Warning
};

//////////////////////////////////////////////////////////////////////////////////////////
static QString
printHex(int value)
{
    if (value == INVALID) return "---";
    return QString("%1h").arg(QString("%1").arg(value, 4, 16, QChar('0')).toUpper());
}

//////////////////////////////////////////////////////////////////////////////////////////
static void
showMessage(Ui::FirmwarePage* ui, Symbol symbol, const QString& text = QString())
{
    switch (symbol)
    {   case Symbol::Info:    ui->infoStack->setCurrentIndex(1); break;
        case Symbol::Warning: ui->infoStack->setCurrentIndex(2); break;
        default:              ui->infoStack->setCurrentIndex(0);
    }
    ui->infoText->setText(text);
}

//////////////////////////////////////////////////////////////////////////////////////////
static void
printDeviceInTitle(Ui::FirmwarePage* ui, const Cy4Device& device)
{
    const Cy4TypePlate& typePlate = device.typePlate();
    ui->titleBoardTypeText->setText(typePlate.boardType().toText().c_str());
    ui->titleApplicationTypeText->setText(typePlate.applicationType().toText().c_str());
    ui->titleApplicationVersionText->setText(typePlate.applicationVersion().toText().c_str());
    int index = deviceList().indexOf(device.address());
    if (index == INVALID) ui->titleLabelText->clear();
    else                  ui->titleLabelText->setText(deviceList().label(index));
}

//////////////////////////////////////////////////////////////////////////////////////////
static void
deplayFirmware(Ui::FirmwarePage* ui, const Cy4Device& device)
{
    if (!device.isValid()) return;
    Cy4Address address = device.address();
    // (1) Gerät in den Programmlader-Modus versetzen
    {   if (!cy4Net().enterProgramLoader(address))
        {   showMessage(ui, Symbol::Warning, QObject::tr("Failed to enter program loader"));
            ui->progressBar->setValue(100);
            return;
        }
        Cy4TypePlate typePlate = cy4Net().fetchTypePlate(address);
        if (!typePlate.isValid())
        {   showMessage(ui, Symbol::Warning, QObject::tr("Could not retrieve type plate"));
            ui->progressBar->setValue(100);
            return;
        }
        if (!typePlate.isProgramLoader())
        {   showMessage(ui, Symbol::Warning, QObject::tr("No program loader found on device"));
            ui->progressBar->setValue(100);
            return;
        }
    }
    // (2) Firmware hochladen
    {   showMessage(ui, Symbol::Info, QObject::tr("Uploading firmware ..."));
        Cy4Firmware firmware(ui->filePathText->text());
        std::list<Cy4Message> list = firmware.toMessages(address);
        int repeatCounter = 0;
        int sendCounter   = 0;
        std::list<Cy4Message>::const_iterator it = list.cbegin();
        while (it != list.cend())
        {   const Cy4Message& request = *it;
            Cy4Message response = cy4Net().transceive(request);
            int percent;
            percent = qRound(double(sendCounter*100.0)/double(list.size()));
            ui->progressBar->setValue(percent);
            if (response.isValid())
            {   it++;
                sendCounter++;
                repeatCounter = 0;
            }
            else
            {   if (repeatCounter == 3)
                {   showMessage(ui, Symbol::Warning, QObject::tr("Upload failed"));
                    ui->progressBar->setValue(100);
                    return;
                }
                repeatCounter++;
            }
        }
        ui->progressBar->setValue(100);
    }
    // (3) Applikation starten und damit den Hochladevorgang abschliessen
    {   if (!cy4Net().enterApplication(address))
        {   showMessage(ui, Symbol::Warning, QObject::tr("Failed to enter application"));
            return;
        }
        Cy4TypePlate typePlate = cy4Net().fetchTypePlate(address);
        if (!typePlate.isValid())
        {   showMessage(ui, Symbol::Warning, QObject::tr("Could not retrieve type plate"));
            return;
        }
        if (typePlate.isProgramLoader())
        {   showMessage(ui, Symbol::Warning, QObject::tr("No application found on device"));
            return;
        }
        showMessage(ui, Symbol::Info, QObject::tr("Upload completed successfully"));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
FirmwarePage::onSelectFile(void)
{
    QString preset = mUi->filePathText->text();
    if (preset.isEmpty()) preset = settings().localCy4ToolDataDirectoryPath;
    QString filter = "*.hex\n*.*";
    QString title    = tr("Load firmware HEX  binary file");
    QString filePath = QFileDialog::getOpenFileName(this, title, preset, filter);
    if (!filePath.isEmpty())
    {   mUi->filePathText->setText(filePath);
        onFirmwareChanged();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
FirmwarePage::onFirmwareChanged(void)
{
    QString currentFilePath = mUi->filePathText->property("CurrentFilePath").toString();
    QString editedFilePath  = mUi->filePathText->text();
    mUi->filePathText->setProperty("CurrentFilePath", editedFilePath);
    Cy4Firmware firmware;
    QFileInfo fileInfo = QFileInfo(editedFilePath);
    if (!fileInfo.isDir() && fileInfo.isFile())
    {   firmware.load(editedFilePath);
        mainFrame().clearStatusBarMessage();
    }
    Cy4Firmware::BinaryInfo binaryInfo        = firmware.binaryInfo();
    Cy4TypePlate            firmwareTypePlate = firmware.typePlate();
    mUi->startText->setText(printHex(binaryInfo.first));
    mUi->lastText->setText(printHex(binaryInfo.last));
    if (binaryInfo.size == INVALID) mUi->sizeText->setText("--- bytes");
    else                            mUi->sizeText->setText(QString("%1 bytes").arg(binaryInfo.size));
    mUi->firmwareBoardTypeText->setText(firmwareTypePlate.boardType().toText().c_str());
    mUi->firmwareApplicationTypeText->setText(firmwareTypePlate.applicationType().toText().c_str());
    mUi->firmwareApplicationVersionText->setText(firmwareTypePlate.applicationVersion().toText().c_str());
    bool allowDepolyment = false;
    if (fileInfo.isFile())
    {   const Cy4TypePlate& currentTypePlate = mDevice.typePlate();
        if (!firmwareTypePlate.isValid())
        {   showMessage(mUi, Symbol::Blank);
            allowDepolyment = false;
        }
        else if (!binaryInfo.isValid())
        {   showMessage(mUi, Symbol::Warning, tr("HEX binary file failure"));
        }
        else if (!firmwareTypePlate.isValid())
        {   showMessage(mUi, Symbol::Warning, tr("No type plate found in binary"));
            allowDepolyment = true;
        }
        else if (firmwareTypePlate.isProgramLoader())
        {   showMessage(mUi, Symbol::Info, tr("Binary is a program loader module"));
        }
        else if (firmwareTypePlate.boardType() != currentTypePlate.boardType())
        {   showMessage(mUi, Symbol::Warning, tr("Firmware intended for different board"));
            allowDepolyment = true;
        }
        else if (!currentTypePlate.isProgramLoader() && firmwareTypePlate.applicationType() != currentTypePlate.applicationType())
        {   showMessage(mUi, Symbol::Info, tr("Application will be altered"));
            allowDepolyment = true;
        }
        else if( firmwareTypePlate.applicationVersion() == currentTypePlate.applicationVersion())
        {   showMessage(mUi, Symbol::Info, tr("Firmware is up to date"));
            allowDepolyment = true;
        }
        else
        {   showMessage(mUi, Symbol::Blank);
            allowDepolyment = true;
        }
    }
    else
    {   mainFrame().showStatusBarInfo(tr("No such file: \"%1\"").arg(fileInfo.fileName()));
        showMessage(mUi, Symbol::Blank);
        allowDepolyment = false;
    }
    mUi->deployButton->setEnabled(allowDepolyment);
    mUi->deployLabel->setEnabled(allowDepolyment);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
FirmwarePage::onDeploy(void)
{
    if (!mDevice.isValid()) return;
    if (QMessageBox::question(this,
                              APPL_NAME,
                              tr("Start deployment?"),
                              QMessageBox::Yes,
                              QMessageBox::No) != QMessageBox::Yes) return;
    mUi->deployButton->setEnabled(false);
    mUi->deployLabel->setEnabled(false);
    mUi->filePathText->setEnabled(false);
    mUi->selectFileButton->setEnabled(false);
    printDeviceInTitle(mUi, Cy4Device());
    emit uploadChanged();
    mUploadInProgress = true;
    deplayFirmware(mUi, mDevice);
    mUploadInProgress = false;
    if (!cy4Net().gateway().isValid()) cy4Net().callGateway();
    int index = deviceList().indexOf(mDevice.address());
    if (index != INVALID) deviceList().fetch(index);
    printDeviceInTitle(mUi, mDevice);
    mUi->deployButton->setEnabled(true);
    mUi->deployLabel->setEnabled(true);
    mUi->filePathText->setEnabled(true);
    mUi->selectFileButton->setEnabled(true);
    emit uploadChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
FirmwarePage::retranslateUi(void)
{
    mUi->retranslateUi(this);
}

//////////////////////////////////////////////////////////////////////////////////////////
FirmwarePage::FirmwarePage(QWidget* parent) : AbstractPage(parent)
{
    // GUI
    {   mUi = new Ui::FirmwarePage;
        mUi->setupUi(this);
    }
    // Ereignisverknüpfungen
    {   connect(mUi->filePathText    , &QLineEdit::editingFinished, this, &FirmwarePage::onFirmwareChanged);
        connect(mUi->selectFileButton, &QToolButton::clicked      , this, &FirmwarePage::onSelectFile);
        connect(mUi->deployButton    , &QPushButton::clicked      , this, &FirmwarePage::onDeploy);
    }
    // Sonstige Initialisierungen
    mUploadInProgress = false;
}

//////////////////////////////////////////////////////////////////////////////////////////
FirmwarePage::~FirmwarePage()
{
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
Page
FirmwarePage::page(void) const
{
    return Page::Firmware;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
FirmwarePage::onShow(void)
{
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) mDevice = Cy4Device();
    else                   mDevice = *device;
    mUi->firmwareFrame->setEnabled(mDevice.isValid());
    printDeviceInTitle(mUi, mDevice);
    int index = deviceList().indexOf(mDevice.address());
    mUi->filePathText->setEnabled(index != INVALID);
    mUi->selectFileButton->setEnabled(index != INVALID);
    if (index == INVALID) mUi->filePathText->clear();
    else                  mUi->filePathText->setText(deviceList().firmwareFilePath(index));
    mUi->progressBar->setValue(0);
    mUi->filePathText->setProperty("CurrentFilePath", QString());
    if (mUi->filePathText->text().isEmpty()) showMessage(mUi, Symbol::Blank);
    onFirmwareChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
FirmwarePage::onHide(void)
{
    if (uploadInProgress()) cy4Net().enterProgramLoader(mDevice.address());
    int index = deviceList().indexOf(mDevice.address());
    if (index != INVALID)
    {   deviceList().setFirmwareFilePath(index, mUi->filePathText->text());
    }
    mDevice = Cy4Device();
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
FirmwarePage::uploadInProgress(void) const
{
    return mUploadInProgress;
}

//////////////////////////////////////////////////////////////////////////////////////////
