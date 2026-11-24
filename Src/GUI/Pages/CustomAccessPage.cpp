//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Benutzerdefinierter Zugriff                                                         //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <QClipboard>
#include <ui_CustomAccessPage.h>
#include <QRegularExpressionValidator>
#include "MainFrame.h"
#include "Settings.h"
#include "DeviceList.h"
#include "CustomAccessPage.h"
#include "DashboardPage.h"
#include "Cy4Net.h"

using namespace ::std;

const int MAX_FAILS = 3;

//////////////////////////////////////////////////////////////////////////////////////////
static void
setupLocationOffset(QHexEdit* hexEdit, Cy4Location location)
{
    if (!location.isValid()) return;
    if (settings().locationOutputRelative)
    {   hexEdit->setAddressOffset(0);
    }
    else
    {   hexEdit->setAddressOffset(location.toInteger());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
static void
fillHexEdit(QHexEdit* hexEdit, UInt08 data)
{
    for (int i=0; i<hexEdit->data().length(); i++) hexEdit->replace(i, data);
    hexEdit->clearHighlighted();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onGoBackButton(void)
{
    mainFrame().showPage(mainFrame().previousPage()->page());
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onClearAll(void)
{
    mUi->addressEdit->clear();
    mUi->locationEdit->clear();
    mUi->countEdit->clear();
    mUi->hexEdit->setData(QByteArray());
    validateParameter();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onParameterChanged(void)
{
    QObject* source = sender();
    if (source == mUi->addressEdit)
    {   Cy4Address  address(mUi->addressEdit->text().toStdString());
        if (address.isValid())
        {   mUi->addressEdit->setText(address.toText().c_str());
        }
    }
    else if (source == mUi->locationEdit)
    {   Cy4Location location(mUi->locationEdit->text().toStdString());
        if (location.isValid())
        {   mUi->locationEdit->setText(location.toText().c_str());
        }
    }
    else if (source == mUi->countEdit)
    {   int count = mUi->countEdit->text().toInt();
        if (count > 0 && count <= 0xFFFFFF)
        {   mUi->countEdit->setText(QString::number(count));
        }
    }
    validateParameter();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onLocationTextChanged(void)
{
    int cursorPosition = mUi->locationEdit->cursorPosition();
    mUi->locationEdit->setText(mUi->locationEdit->text().toUpper());
    mUi->locationEdit->setCursorPosition(cursorPosition);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onLocationBaseChanged(void)
{
    mUi->absoluteLocationButton->setChecked(!settings().locationOutputRelative);
    mUi->relativeLocationButton->setChecked( settings().locationOutputRelative);
    mUi->absoluteLocationMenuItem->setChecked(!settings().locationOutputRelative);
    mUi->relativeLocationMenuItem->setChecked( settings().locationOutputRelative);
    Cy4Location location(mUi->locationEdit->text().toStdString());
    setupLocationOffset(mUi->hexEdit, location);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onAccess(void)
{
    QObject* source = sender();
    if      (source == mUi->readButton)  transceive(Cy4Message::Read);
    else if (source == mUi->writeButton) transceive(Cy4Message::Write);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onCopyToClipboard(void)
{
    QString text = mUi->hexEdit->toReadableString();
    QApplication::clipboard()->setText(text);
    mainFrame().showStatusBarInfo(tr("Hex dump data copied to clipboard"));
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onOverwriteData(void)
{
    QObject* source = sender();
    if      (source == mUi->overwriteWith00hMenuItem) fillHexEdit(mUi->hexEdit, 0x00);
    else if (source == mUi->overwriteWithFFhMenuItem) fillHexEdit(mUi->hexEdit, 0xFF);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onLocationBase(void)
{
    QObject* source = sender();
    if (source == mUi->absoluteLocationButton || source == mUi->absoluteLocationMenuItem)
    {   settings().locationOutputRelative = false;
    }
    if (source == mUi->relativeLocationButton || source == mUi->relativeLocationMenuItem)
    {   settings().locationOutputRelative = true;
    }
    onLocationBaseChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onContextMenu(QPoint position)
{
    mUi->copyToClipboardMenuItem->setEnabled(!mUi->hexEdit->data().isEmpty());
    mContextMenu.exec(mUi->hexEdit->viewport()->mapToGlobal(position));
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onDataChanged(void)
{
    mUi->accessResultStack->setCurrentIndex(0);
    bool hasData = !mUi->hexEdit->data().isEmpty();
    mUi->copyToClipboardMenuItem->setEnabled(hasData);
    mUi->overwriteWith00hMenuItem->setEnabled(hasData);
    mUi->overwriteWithFFhMenuItem->setEnabled(hasData);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onCurrentLocationChanged(qint64 position)
{
    QString text;
    if (!mUi->hexEdit->data().isEmpty())
    {   UInt08 value = mUi->hexEdit->dataAt(position, 1)[0];
        QString valueText = QString("%1").arg(value, 2, 16, QChar('0')).toUpper();
        Cy4Location location(mUi->locationEdit->text().toStdString());
        if (location.isValid())
        {   if (mUi->absoluteLocationButton->isChecked())
            {   QString absoluteLocationText = Cy4Location(location + position).toText().c_str();
                text = QString("%1h : %2h").arg(absoluteLocationText)
                                           .arg(valueText);
            }
            else
            {   QString relativeLocationText = Cy4Location(location).toText().c_str();
                text = QString("%1h + %2 : %3h").arg(relativeLocationText)
                                                .arg(position)
                                                .arg(valueText);
            }
        }
    }
    mUi->currentLocationText->setText(text);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::validateParameter(void)
{
    // Textfelder auswerten
    bool validInput   = true;
    bool isBroadcast  = false;
    bool isPgmLdrArea = false;
    bool hasData      = false;
    if (mUi->addressEdit->text() == ".")
    {   validInput = false;
    }
    else
    {   Cy4Address address = Cy4Address(mUi->addressEdit->text().toStdString());
        if (!address.isValid())
        {   QString text = tr("Invalid address");
            mainFrame().showStatusBarWarning(text);
            validInput = false;
        }
        else if (address.isBroadcast())
        {   QString text = tr("Broadcast is write-only");
            mainFrame().showStatusBarInfo(text);
            isBroadcast = true;
        }
    }
    Cy4Location location;
    if (mUi->locationEdit->text().isEmpty())
    {   validInput = false;
    }
    else
    {   location = Cy4Location(mUi->locationEdit->text().toStdString());
        isPgmLdrArea = location.isValid() && location < 0x100000;
        if (!location.isValid())
        {   mainFrame().showStatusBarWarning(tr("Invalid location"));
            validInput = false;
        }
        else if (isBroadcast && (location < 0xE00000 || location > 0xEFFFFF))
        {   mainFrame().showStatusBarWarning(tr("Broadcast area: E00000h .. EFFFFFh"));
            validInput = false;
        }
        else if (isPgmLdrArea)
        {   mainFrame().showStatusBarInfo(tr("Program loader area: 000000h .. 0FFFFFh"));
        }
        setupLocationOffset(mUi->hexEdit, location);
    }
    if (mUi->countEdit->text().isEmpty())
    {   validInput = false;
    }
    else
    {   int count = mUi->countEdit->text().toInt();
        if (count < 1)
        {   mainFrame().showStatusBarWarning(tr("Invalid count: %1").arg(count));
            validInput = false;
        }
        else if (location + count > 0xFFFFFF)
        {   mainFrame().showStatusBarWarning(tr("Request exceeds location FFFFFFh"));
            validInput = false;
        }
        else
        {   hasData = true;
            if (count != mUi->hexEdit->data().size())
            {   if (USize(count) == mUi->hexEdit->data()) return;
                QByteArray data = mUi->hexEdit->data();
                data.resize(count);
                mUi->hexEdit->setData(data);
                fillHexEdit(mUi->hexEdit, 0xFF);
                setupLocationOffset(mUi->hexEdit, location);
            }
        }
    }
    // Umsetzung der Auswertung
    mUi->readButton->setEnabled(validInput && !isBroadcast);
    mUi->readLabel->setEnabled(validInput && !isBroadcast);
    mUi->writeButton->setEnabled(validInput);
    mUi->writeLabel->setEnabled(validInput);
    mUi->copyToClipboardMenuItem->setEnabled(hasData);
    mUi->overwriteWith00hMenuItem->setEnabled(hasData);
    mUi->overwriteWithFFhMenuItem->setEnabled(hasData);
    if (validInput && !isBroadcast && !isPgmLdrArea) mainFrame().clearStatusBarMessage();
    onDataChanged();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::transceive(Cy4Message::Access access)
{
    Cy4Address  address  = Cy4Address(mUi->addressEdit->text().toStdString());
    Cy4Location location = Cy4Location(mUi->locationEdit->text().toStdString());
    int         count    = mUi->countEdit->text().toInt();
    QString     accessText = access == Cy4Message::Read ? tr("Read") : tr("Write");
    assert(mUi->hexEdit->data().size() == count);
    mUi->frame->setEnabled(false);
    Service service("", location, count, Service::ReadWrite);
    if (access == Cy4Message::Write)
    {   // Daten aus <hexEdit> in <service> umkopieren
        QByteArray data = mUi->hexEdit->data();
        for (int i=0; i<count; i++) service.setDataAt(i, data[i]);
    }
    list<Cy4Message> list = service.toRequests(address, access);
    int messageCounter = 0;
    int failCounter    = 0;
    ::list<Cy4Message>::const_iterator it = list.cbegin();
    while (it != list.cend())
    {   mainFrame().showStatusBarInfo(QString("%1 ...").arg(accessText));
        if (list.size() > 1) mainFrame().showStatusBarProgress(0, messageCounter, list.size()-1);
        const Cy4Message& request = *it;
        Cy4Message response = cy4Net().transceive(request);
        if (response.isValid())
        {   messageCounter++;
            failCounter = 0;
            if (access == Cy4Message::Read)
            {   // Antwortdaten von <response> ins <hexEdit> übertragen
                int position = Cy4Location(response.location() - location).toInteger();
                for (int i=0; i<response.count(); i++)
                {   mUi->hexEdit->replace(position+i, char(response[i]));
                }
            }
            it++;
        }
        else
        {   failCounter++;
            if (failCounter == MAX_FAILS)
            {   if (access == Cy4Message::Read)
                {   // Übertragung fehlgeschlagen, Daten in <hexEdit> überschreiben
                    fillHexEdit(mUi->hexEdit, 0xFF);
                }
                break;
            }
        }
    }
    mUi->hexEdit->clearHighlighted();
    if (it == list.cend())
    {   mainFrame().showStatusBarInfo(QString("%1 ... %2").arg(accessText, tr("completed")));
        if (access == Cy4Message::Write)
        {   mUi->accessResultStack->setCurrentIndex(1);
            // Bei Schreibzugriffen im Bereich FFFF00 .. FFFFFF, Gerät in der Geräteliste
            // aktualisierenGeräteliste aktualisieren
            if (location >= 0xFFFF00)
            {   int index = deviceList().indexOf(address);
                cy4Net().postPause();
                deviceList().fetch(index);
            }
        }
        mUi->accessResultStack->setCurrentIndex(1);
    }
    else
    {   mainFrame().showStatusBarInfo(QString("%1 ... %2").arg(accessText, tr("failed")));
        mUi->accessResultStack->setCurrentIndex(2);
    }
    mUi->frame->setEnabled(true);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::retranslateUi(void)
{
    mUi->retranslateUi(this);
}

//////////////////////////////////////////////////////////////////////////////////////////
CustomAccessPage::CustomAccessPage(QWidget* parent) : AbstractPage(parent)
{
    // GUI
    {   mUi = new Ui::CustomAccessPage;
        mUi->setupUi(this);
    }
    // (...)-Menü
    {   mMoreMenu.addAction(mUi->copyToClipboardMenuItem);
        mMoreMenu.addAction(mUi->absoluteLocationMenuItem);
        mMoreMenu.addAction(mUi->relativeLocationMenuItem);
        mMoreMenu.addSeparator();
        mMoreMenu.addAction(mUi->overwriteWith00hMenuItem);
        mMoreMenu.addAction(mUi->overwriteWithFFhMenuItem);
        mUi->moreButton->setMenu(&mMoreMenu);

    }
    // Kontextmenü
    {   mContextMenu.addAction(mUi->copyToClipboardMenuItem);
    }
    // Ereignisverknüpfungen
    {   connect(mUi->goBackButton            , &QPushButton::clicked                , this, &CustomAccessPage::onGoBackButton);
        connect(mUi->clearAllButton          , &QPushButton::clicked                , this, &CustomAccessPage::onClearAll);
        connect(mUi->addressEdit             , &QLineEdit::editingFinished          , this, &CustomAccessPage::onParameterChanged);
        connect(mUi->locationEdit            , &QLineEdit::editingFinished          , this, &CustomAccessPage::onParameterChanged);
        connect(mUi->countEdit               , &QLineEdit::editingFinished          , this, &CustomAccessPage::onParameterChanged);
        connect(mUi->locationEdit            , &QLineEdit::textChanged              , this, &CustomAccessPage::onLocationTextChanged);
        connect(mUi->readButton              , &QPushButton::clicked                , this, &CustomAccessPage::onAccess);
        connect(mUi->writeButton             , &QPushButton::clicked                , this, &CustomAccessPage::onAccess);
        connect(mUi->copyToClipboardMenuItem , &QAction::triggered                  , this, &CustomAccessPage::onCopyToClipboard);
        connect(mUi->overwriteWith00hMenuItem, &QAction::triggered                  , this, &CustomAccessPage::onOverwriteData);
        connect(mUi->overwriteWithFFhMenuItem, &QAction::triggered                  , this, &CustomAccessPage::onOverwriteData);
        connect(mUi->absoluteLocationMenuItem, &QAction::triggered                  , this, &CustomAccessPage::onLocationBase);
        connect(mUi->relativeLocationMenuItem, &QAction::triggered                  , this, &CustomAccessPage::onLocationBase);
        connect(mUi->absoluteLocationButton  , &QPushButton::clicked                , this, &CustomAccessPage::onLocationBase);
        connect(mUi->relativeLocationButton  , &QPushButton::clicked                , this, &CustomAccessPage::onLocationBase);
        connect(mUi->hexEdit                 , &QHexEdit::customContextMenuRequested, this, &CustomAccessPage::onContextMenu);
        connect(mUi->hexEdit                 , &QHexEdit::dataChanged               , this, &CustomAccessPage::onDataChanged);
        connect(mUi->hexEdit                 , &QHexEdit::currentAddressChanged     , this, &CustomAccessPage::onCurrentLocationChanged);
    }
    // Sonstige Initialisierungen
    {   QRegularExpression regExp1 = QRegularExpression("[0-9a-fA-F]{,6}");
        mUi->locationEdit->setValidator(new QRegularExpressionValidator(regExp1, this));
        QRegularExpression regExp2 = QRegularExpression("[0-9]{,4}");
        mUi->countEdit->setValidator(new QRegularExpressionValidator(regExp2, this));
        mUi->hexEdit->setDynamicBytesPerLine(true);
        mUi->hexEdit->setAddressWidth(6);
        mUi->hexEdit->setHexCaps(true);
        mUi->hexEdit->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        onLocationBaseChanged();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
CustomAccessPage::~CustomAccessPage()
{
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
Page
CustomAccessPage::page(void) const
{
    return Page::CustomAccess;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onShow(void)
{
    Cy4Address  address(mUi->addressEdit->text().toStdString());
    Cy4Location location(mUi->locationEdit->text().toStdString());
    int         count(mUi->countEdit->text().toInt());
    Cy4Address  presetAddress  = address;
    Cy4Location presetLocation = location;
    int         presetCount    = count;
    const Cy4Device* device = Global::currentDevice();
    // Wenn diese Seite von ...
    if (mainFrame().previousPage()->page() == Page::ServiceList)
    {   // ... der Diensteliste-Seite aus geöffnet wurde, Voreinstellungen wenn möglich
        // vom aktuellen Dienst übernehmen
        const Service* currentService = Global::currentService();
        if (currentService != nullptr && currentService->isValid())
        {   if (device != nullptr) presetAddress = device->address();
            presetLocation = currentService->location();
            presetCount    = currentService->size();
        }
    }
    else if (mainFrame().previousPage()->page() == Page::Dashboard)
    {   // ... von der Bedienfeld-Seite aus geöffnet wurde, Voreinstellungen wenn
        // möglichst vom aktuellen Dienst-Steuerelement übernehmen
        DashboardPage* dashboardPage = qobject_cast<DashboardPage*>(mainFrame().previousPage());
        const AbstractServiceWidget* currentWidget = dashboardPage->currentServiceWidget();
        if (currentWidget != nullptr)
        {   if (device != nullptr) presetAddress  = device->address();
            presetLocation = currentWidget->service().location();
            presetCount    = currentWidget->service().size();
        }
    }
    if (address != presetAddress || location != presetLocation || count != presetCount)
    {   fillHexEdit(mUi->hexEdit, 0xFF);
    }
    else
    {   if (device != nullptr) presetAddress  = device->address();
        presetLocation = Cy4Location();
        presetCount    = INVALID;
        mUi->hexEdit->setData(QByteArray());
    }
    mUi->addressEdit->setText(presetAddress.toText().c_str());
    if (presetLocation.isValid()) mUi->locationEdit->setText(presetLocation.toText().c_str());
    else                          mUi->locationEdit->setText("");
    if (presetCount != INVALID) mUi->countEdit->setText(QString::number(presetCount));
    else                        mUi->countEdit->setText("");
    validateParameter();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
CustomAccessPage::onHide(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
