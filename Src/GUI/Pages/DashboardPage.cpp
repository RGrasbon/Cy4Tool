//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Bedienfeld-Seite                                                                    //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QKeyEvent>
#include <QJsonObject>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include "ui_DashboardPage.h"
#include "MainFrame.h"
#include "DashboardPage.h"
#include "MainFrame.h"
#include "Global.h"
#include "KeyValues.h"
#include "Cy4Net.h"
#include "DeviceList.h"
#include "ServiceMap.h"
#include "Settings.h"
#include "Downloader.h"
#include "AddressServiceWidget.h"
#include "DateServiceWidget.h"
#include "DummyServiceWidget.h"
#include "EnumerationServiceWidget.h"
#include "LinkServiceWidget.h"
#include "LocationServiceWidget.h"
#include "NumberServiceWidget.h"
#include "TableServiceWidget.h"
#include "TextServiceWidget.h"
#include "TimeServiceWidget.h"
#include "ToggleServiceWidget.h"
#include "TriggerServiceWidget.h"

using namespace std;

enum class FileAccess
{   Load,
    Save
};

//////////////////////////////////////////////////////////////////////////////////////////
// transceive(): Überträgt die Anfrage <request> ans Gerät und liefert die Antwort      //
// ------------  zurück. Sollte die erste Übertragung fehlschlagen unternimmt die       //
//  Funktion einen zweiten Versuch.                                                     //
//                                                                                      //
//  Eingabe: · request: Zu sendende Anfrage.                                            //
//  Ausgabe: Erhaltene Antwort. Schlägt die Übertragung fehl, ist die zurückgelieferte  //
//           Antwort ungültig.                                                          //
//////////////////////////////////////////////////////////////////////////////////////////
static Cy4Message
transceive(const Cy4Message& request)
{
    Cy4Message response = cy4Net().transceive(request);
    if (!response.isValid()) response = cy4Net().transceive(request); // 2. Versuch
    return response;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  transceive(): Je <access> werden Daten vom Gerät in den Cache von <service> gelesen //
//  ------------  ('Cy4Message::Read') oder Daten aus dem Cache in Gerät geschrieben    //
//  ('Cy4Message::Write'). Der zu übertragende Inhalt kann mit <location> und <size>    //
//  festgelegt werden.                                                                  //
//                                                                                      //
//  Eingabe: · address:       Adresse des Zielgerätes.                                  //
//           · service:       Der lokale Dienst.                                        //
//           · location:      Dienstort in <Service>, ab der <size> Datenbytes gelesen  //
//                            bzw. geschrieben werden.                                  //
//           · size:          Anzahl der zu lesenden Datenbytes.                        //
//           · access:        'Cy4Message::Read' Daten vom Gerät lesen, 'Cy4Message::   //
//                            Write' ins Gerät schreiben.                               //
//           · abortTransfer: (Optional) Das Setzen dieser externen bool-Referenz       //
//                            beendet beendet unmittelbar die laufende Übertragung.     //
//  Ausgabe: 'true', wenn der Austausch erfolgreich war.                                //
//////////////////////////////////////////////////////////////////////////////////////////
static bool
transceive(Cy4Address          address,
           Service&            service,
           Cy4Location         location,
           int                 size,
           Cy4Message::Access  access,
           volatile bool*      abortTransfer = nullptr)
{
    // Liste benötigter Nachrichten zusammenstellen
    list<Cy4Message> list = service.toRequests(address, location, size, access);
    // Sukzessiv alle Anfragen senden und den Service-Cache aktualisieren
    ::list<Cy4Message>::const_iterator it = list.cbegin();
    while (it != list.cend())
    {   const Cy4Message& request = *it;
        Cy4Message response = transceive(request);
        service.fromResponse(response);
        if (!response.isValid()) return false;
        if (abortTransfer != nullptr && *abortTransfer) break;
        it++;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
static bool
transceive(Cy4Address          address,
           Service&            service,
           Cy4Message::Access  access,
           volatile bool*      abortTransfer = nullptr)
{
    return transceive(address,
                      service,
                      service.location(),
                      service.size(),
                      access,
                      abortTransfer);
}

//////////////////////////////////////////////////////////////////////////////////////////
static void
updateServiceWidgets(const Ui::DashboardPage* ui)
{
    QList<QWidget*> list = ui->scrollArea->findChildren<QWidget*>();
    for(QWidget* widget : std::as_const(list))
    {   AbstractServiceWidget* serviceWidget = qobject_cast<AbstractServiceWidget*>(widget);
        if (serviceWidget != nullptr) serviceWidget->populateWidget();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
//  isDashboardEmpty(): Prüft, ob sich auf dem Bedienfeld Dienste-Widgets befinden.     //
//  ------------------                                                                  //
//                                                                                      //
//  Eingabe: · ui: GUI der Bedienfeld-Seite.                                            //
//  Ausgabe: 'true' wenn das Bedienfeld leer ist.                                       //
//////////////////////////////////////////////////////////////////////////////////////////
static bool
isDashboardEmpty(const Ui::DashboardPage* ui)
{
    const QList<QWidget*> list = ui->scrollArea->findChildren<QWidget*>();
    for(QWidget* widget : std::as_const(list))
    {   AbstractServiceWidget* serviceWidget = qobject_cast<AbstractServiceWidget*>(widget);
        if (serviceWidget != nullptr) return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  hasWidget(): Die Dienstekarte erlaubt die Definition von Diensten auch ohne Widget. //
//  -----------  Diese Funktion prüft, ob <service> in der Bedienfeld-Seite mit         //
//  (mindestens) einem Widget vertreten ist.                                            //
//                                                                                      //
//  Eingabe: · ui:      GUI der Bedienfeld-Seite.                                       //
//           · service: Der betroffene Dienst.                                          //
//  Ausgabe: 'true' wenn zu <service> ein Widget existiert.                             //
//////////////////////////////////////////////////////////////////////////////////////////
static bool
hasWidget(const Ui::DashboardPage* ui, const Service& service)
{
    QList<QWidget*> list = ui->scrollArea->findChildren<QWidget*>();
    for(QWidget* widget : std::as_const(list))
    {   AbstractServiceWidget* serviceWidget = qobject_cast<AbstractServiceWidget*>(widget);
        if (serviceWidget != nullptr)
        {   const ServiceProxy& serviceProxy = serviceWidget->service();
            if ((service.location() < serviceProxy.location() + serviceProxy.size()) &&
                (serviceProxy.location() < service.location() + service.size()))
            {   return true;
            }
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QWidget*
createNoDashboardSymbol(void)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    {   horizontalLayout->addStretch();
        {   QVBoxLayout* verticalLayout = new QVBoxLayout();
            verticalLayout->addStretch();
            {   QLabel* label = new QLabel();
                label->setPixmap(QPixmap(QString::fromUtf8(":/Symbols/Dashboard_P.png")));
                label->setAlignment(Qt::AlignmentFlag::AlignCenter);
                verticalLayout->addWidget(label);
            }
            {   QLabel* label = new QLabel();
                QFont font;
                font.setPointSize(12);
                font.setItalic(true);
                label->setFont(font);
                label->setStyleSheet(QString::fromUtf8("color: rgba(25, 25, 25, 90);"));
                label->setAlignment(Qt::AlignmentFlag::AlignCenter);
                label->setText(QObject::tr("No service map"));
                verticalLayout->addWidget(label);
            }
            verticalLayout->addStretch();
            verticalLayout->setStretch(0, 1);
            verticalLayout->setStretch(1, 0);
            verticalLayout->setStretch(2, 0);
            verticalLayout->setStretch(3, 2);
            horizontalLayout->addLayout(verticalLayout);
        }
        horizontalLayout->addStretch();
    }
    widget->setLayout(horizontalLayout);
    return widget;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QString
configurationFilePath(FileAccess fileAccess, const Cy4Device& device)
{
    QString filePath;
    int index = deviceList().indexOf(device.address());
    if (index != INVALID && !deviceList().configurationFilePath(index).isEmpty())
    {   filePath = deviceList().configurationFilePath(index);
    }
    else
    {   filePath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    }
    QString preset = filePath;
    QString filter = QString("%1 (*.config);;%2 (*.*)")
                             .arg(QObject::tr("Configuration files"),
                                  QObject::tr("All files"));
    switch (fileAccess)
    {   case FileAccess::Load:
        {   QString title  = QObject::tr("Load application configuration from ...");
            preset = QFileInfo(preset).absolutePath();
            filePath = QFileDialog::getOpenFileName(NULL, title, preset, filter);
            break;
        }
        case FileAccess::Save:
        {   QString title  = QObject::tr("Save application configuration to ...");
            filePath = QFileDialog::getSaveFileName(NULL, title, preset, filter);
            if (!filePath.isEmpty())
            {   QFileInfo fileInfo(filePath);
                if (fileInfo.suffix().isEmpty()) filePath += ".config";
            }
            break;
        }
        default:
            return QString();

    }
    if( filePath.isEmpty() ) return QString();
    if (index != INVALID) deviceList().setConfigurationFilePath(index, filePath);
    return filePath;
}

//////////////////////////////////////////////////////////////////////////////////////////
static Cy4Message
toWriteRequest(Cy4Address address, const QJsonObject& jsonObject)
{
    Cy4Location location = Cy4Location(jsonObject.value(KEY::LOCATION).toString().toStdString());
    QJsonArray payloadArray = jsonObject.value(KEY::PAYLOAD).toArray();
    ByteArray payload(payloadArray.size());
    for (int i=0; i<payloadArray.size(); i++)
    {   int data = payloadArray[i].toInt(INT_MIN);
        if( data < 0 || data > 255 ) return Cy4Message();
        payload[i] = char(data);
    }
    return Cy4Message(address, location, payload);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DashboardPage::addArrayOfObjects(const QJsonArray& jsonArray, QBoxLayout* layout)
{
    for (int i=0; i<jsonArray.size(); i++)
    {   QJsonObject jsonObject = jsonArray[i].toObject();
        if( !addObject(jsonObject, layout) ) return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DashboardPage::addGroup(const QJsonObject& jsonObject, QBoxLayout*& layout)
{
    // Hinweismeldung: "enabled-if" wird nicht mehr unterstützt
    if (jsonObject.contains("enabled-if"))
    {   mainFrame().showStatusBarWarning(tr("Property \"%1\" is no longer supported")
                                            .arg("enabled-if"));
    }
    // Container
    QWidget* container;
    {   if (jsonObject.contains(KEY::CAPTION))
        {   container = new QGroupBox();
            QString caption = Global::jsonLanguageEntry(jsonObject.value(KEY::CAPTION));
            caption.replace("&", "&&");
            static_cast<QGroupBox*>(container)->setTitle(caption);
        }
        else
        {   container = new QFrame();
        }
        container->setToolTip(Global::jsonLanguageEntry(jsonObject.value(KEY::BRIEF)));
    }
    // Ausrichtung
    Qt::Orientation orientation = Qt::Vertical;
    {   if (jsonObject.contains(KEY::ORIENTATION))
        {   if (jsonObject.value(KEY::ORIENTATION).toString() == VALUE::HORIZONTAL)
            {   orientation = Qt::Horizontal;
            }
            else if (jsonObject.value(KEY::ORIENTATION).toString() != VALUE::VERTICAL)
            {   QString text = tr("Value for \"%1\" must be \"%2\" or \"%3\"")
                                  .arg(KEY::ORIENTATION, VALUE::HORIZONTAL, VALUE::VERTICAL);
                mainFrame().showStatusBarWarning(text);
                return false;
            }
        }
    }
    // Unterlayout
    QBoxLayout* subLayout;
    {   QSizePolicy policy = container->sizePolicy();
        switch (orientation)
        {   case Qt::Horizontal:
                subLayout = new QHBoxLayout;
                subLayout->setDirection(QBoxLayout::LeftToRight);
                container->setSizePolicy(policy.horizontalPolicy(),QSizePolicy::Maximum);
                break;
            case Qt::Vertical:
                subLayout = new QVBoxLayout;
                subLayout->setDirection(QBoxLayout::TopToBottom);
                container->setSizePolicy(QSizePolicy::Maximum, policy.verticalPolicy());
                break;
            default:
                return false;
        }
        int margin = qobject_cast<QFrame*>(container) == nullptr ? 10 : 0;
        subLayout->setContentsMargins(margin, margin, margin, margin);
        subLayout->setSpacing(12);
    }
    // Gruppenmitglieder
    if (jsonObject.contains(KEY::ITEMS))
    {   if (!jsonObject.value(KEY::ITEMS).isArray())
        {   QString text = tr("Service Map File: JSON array expected in section \"%1\"")
                              .arg(KEY::ITEMS);
            mainFrame().showStatusBarWarning(text);
            return false;
        }
        const QJsonArray& array = jsonObject.value(KEY::ITEMS).toArray();
        if (!addArrayOfObjects(array, subLayout))
        {   return false;
        }
        subLayout->addStretch();
        container->setLayout(subLayout);
        layout->addWidget(container);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DashboardPage::addGrid(const QJsonObject& jsonObject, QBoxLayout*& layout)
{
    // Hinweismeldung: "enabled-if" wird nicht mehr unterstützt
    if (jsonObject.contains("enabled-if"))
    {   mainFrame().showStatusBarWarning(tr("Property \"%1\" is no longer supported")
                                            .arg("enabled-if"));
    }
    // Container anlegen
    QWidget* container;
    {   if (jsonObject.contains(KEY::CAPTION))
        {   container = new QGroupBox();
            QString caption = Global::jsonLanguageEntry(jsonObject.value(KEY::CAPTION));
            static_cast<QGroupBox*>(container)->setTitle(caption);
        }
        else
        {   container = new QFrame();
        }
        container->setToolTip(Global::jsonLanguageEntry(jsonObject.value(KEY::BRIEF)));
        container->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    }
    // Unterlayout anlegen
    QGridLayout* subLayout = new QGridLayout;
    int margin = qobject_cast<QFrame*>(container) == nullptr ? 10 : 0;
    subLayout->setContentsMargins(margin, margin, margin, margin);
    subLayout->setSpacing(12);
    // Gruppenmitglieder einlesen
    if (jsonObject.contains(KEY::ITEMS))
    {   QJsonObject jsonGridObject = jsonObject.value(KEY::ITEMS).toObject();
        QStringList keyList = jsonGridObject.keys();
        for(const QString& key : std::as_const(keyList))
        {   QStringList coordinate = key.split(",");
            if( coordinate.count() != 2 )
            {   QString text = tr("Grid: Malformed item coodinate: \"%1\" (must be "
                                  "\"<col>, <row>\")").arg(key);
                mainFrame().showStatusBarWarning(text);
                return false;
            }
            int column;
            int row;
            {   bool ok;
                column = coordinate[0].toInt(&ok);
                if( column < 0 || column >= 1000 || !ok)
                {   QString text = tr("Grid: Corrupt column value in coodinate: \"%1\"")
                                      .arg(key);
                    mainFrame().showStatusBarWarning(text);
                    return false;
                }
                row = coordinate[1].toInt(&ok);
                if( row < 0 || row >= 1000 || !ok)
                {   QString text = tr("Grid: Corrupt row value in coodinate: \"%1\"")
                                      .arg(key);
                    mainFrame().showStatusBarWarning(text);
                    return false;
                }
            }
            QVBoxLayout* itemLayout = new QVBoxLayout();
            if (!addObject(jsonGridObject.value(key).toObject(), itemLayout))
            {   delete itemLayout;
                return false;
            }
            subLayout->addLayout(itemLayout, row, column);
        }
        container->setLayout(subLayout);
        layout->addWidget(container);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DashboardPage::addService(const QJsonObject& jsonObject, QBoxLayout* layout)
{
    // Dienst-Stellvertreter
    ServiceProxy serviceProxy = ServiceProxy(jsonObject);
    // Hinweismeldung: "enabled-if" wird nicht mehr unterstützt
    if (jsonObject.contains("enabled-if"))
    {   mainFrame().showStatusBarWarning(tr("Property \"%1\" is no longer supported")
                                            .arg("enabled-if"));
    }
    // JSON-Sektion des Steuerelementes
    if (!jsonObject.contains(KEY::WIDGET)) return true;
    const QJsonObject& jsonWidget = jsonObject.value(KEY::WIDGET).toObject();
    // Aufstellung
    Qt::Orientation lineup = Qt::Horizontal;
    if (jsonWidget.contains(KEY::LINE_UP))
    {   QString value = jsonWidget.value(KEY::LINE_UP).toString();
        if (value == VALUE::VERTICAL)
        {   lineup = Qt::Vertical;
        }
        else if (value != VALUE::HORIZONTAL)
        {   QString text = tr("Service at location %1h: Unknown value for property "
                              "\"%2\": \"%3\"").arg(serviceProxy.location().toText(),
                                                    KEY::LINE_UP,
                                                    value);
            mainFrame().showStatusBarWarning(text);
            return false;
        }
    }
    // Unter-Layout
    QBoxLayout* subLayout;
    {   switch(lineup)
        {   case Qt::Horizontal:
                subLayout = new QHBoxLayout();
                subLayout->setDirection(QBoxLayout::LeftToRight);
                break;
            case Qt::Vertical:
                subLayout = new QVBoxLayout();
                subLayout->setDirection(QBoxLayout::TopToBottom);
                break;
            default:
                return false;
        }
        layout->addLayout(subLayout);
        subLayout->setSpacing(3);
    }
    // Bezeichnungen
    if (jsonWidget.contains(KEY::LABEL))
    {   QLabel* label = new QLabel();
        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QString text = Global::jsonLanguageEntry(jsonWidget.value(KEY::LABEL));
        label->setText(text);
        subLayout->addWidget(label);
    }
    // Service-Widget
    {   if (!jsonWidget.contains(KEY::TYPE))
        {   QString text = tr("Service at location %1h: Entry \"%2\" missing")
                              .arg(serviceProxy.location().toText())
                              .arg(KEY::TYPE);
            mainFrame().showStatusBarWarning(text);
            return false;
        }
        QString value = jsonWidget.value(KEY::TYPE).toString();
        AbstractServiceWidget* serviceWidget;
        if      (value == KEY::ADDRESS)     serviceWidget = new AddressServiceWidget(serviceProxy, jsonWidget);
        else if (value == KEY::DATE)        serviceWidget = new DateServiceWidget(serviceProxy, jsonWidget);
        else if (value == KEY::ENUMERATION) serviceWidget = new EnumerationServiceWidget(serviceProxy, jsonWidget);
        else if (value == KEY::LINK)        serviceWidget = new LinkServiceWidget(serviceProxy, jsonWidget);
        else if (value == KEY::LOCATION)    serviceWidget = new LocationServiceWidget(serviceProxy, jsonWidget);
        else if (value == KEY::NUMBER)      serviceWidget = new NumberServiceWidget(serviceProxy, jsonWidget);
        else if (value == KEY::TABLE)       serviceWidget = new TableServiceWidget(serviceProxy, jsonWidget);
        else if (value == KEY::TEXT)        serviceWidget = new TextServiceWidget(serviceProxy, jsonWidget);
        else if (value == KEY::TIME)        serviceWidget = new TimeServiceWidget(serviceProxy, jsonWidget);
        else if (value == KEY::TOGGLE)      serviceWidget = new ToggleServiceWidget(serviceProxy, jsonWidget);
        else if (value == KEY::TRIGGER)     serviceWidget = new TriggerServiceWidget(serviceProxy, jsonWidget);
        else                                serviceWidget = new DummyServiceWidget(serviceProxy, jsonWidget);
        connect(serviceWidget, &QWidget::customContextMenuRequested, this, &DashboardPage::onContextMenu);
        connect(serviceWidget, &AbstractServiceWidget::dataChanged , this, &DashboardPage::onWidgetDataChanged);
        //subLayout->addWidget(serviceWidget);
        //int stretch = 0 ;
        //value == KEY::TABLE
        if (value == KEY::TABLE)
            subLayout->addWidget(serviceWidget, 1);
        else                     subLayout->addWidget(serviceWidget);

    }
    // Zusatz (z.B. Einheit)
    if (jsonWidget.contains(KEY::APPENDIX))
    {   QLabel* label = new QLabel();
        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QString text = Global::jsonLanguageEntry(jsonWidget.value(KEY::APPENDIX));
        label->setText(text);
        subLayout->addWidget(label);
    }
    if ((lineup == Qt::Horizontal && layout->direction() == QBoxLayout::TopToBottom) ||
        (lineup == Qt::Vertical   && layout->direction() == QBoxLayout::LeftToRight))
    {   subLayout->addStretch();
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DashboardPage::addSeparator(const QJsonObject& jsonObject, QBoxLayout* layout)
{
     // Name des Dienstes
    if (!jsonObject.isEmpty())
    {   QString text = tr("Section \"%1\": Value \"%2\" expected").arg(KEY::SEPARATOR,
                                                                       KEY::LINE);
        mainFrame().showStatusBarWarning(text);
        return false;
    }
    QFrame::Shape shape = QFrame::NoFrame;
    switch (layout->direction())
    {   case QBoxLayout::TopToBottom: shape = QFrame::HLine; break;
        case QBoxLayout::LeftToRight: shape = QFrame::VLine; break;
        default:                      return false;
    }
    QFrame* line = new QFrame();
    line->setFrameShape(shape);
    line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DashboardPage::addLabel(const QJsonValue& jsonValue, QBoxLayout* layout)
{
    const QJsonObject& jsonObject = jsonValue.toObject();
    QString text;
    Qt::Alignment alignment;
    if (jsonObject.contains(KEY::TEXT) || jsonObject.contains(KEY::ALIGN))
    {   // "Label" als Widget
        if (!jsonObject.contains(KEY::TEXT))
        {   QString text = tr("In Widget \"%1\": Entry \"%2\" missing").arg(KEY::LABEL,
                                                                            KEY::TEXT);
            mainFrame().showStatusBarWarning(text);
            return false;
        }
        text = Global::jsonLanguageEntry(jsonObject.value(KEY::TEXT));
        alignment = Qt::AlignHCenter;
        if (jsonObject.contains(KEY::ALIGN))
        {   QJsonValue alignValue = jsonObject.value(KEY::ALIGN);
            if      (alignValue == VALUE::LEFT)   alignment = Qt::AlignLeft;
            else if (alignValue == VALUE::RIGHT)  alignment = Qt::AlignRight;
            else if (alignValue != VALUE::CENTER)
            {   QString text = tr("Value for property \"%1\" must be \"%2\", \"%3\" or "
                                  "\"%4\"").arg(KEY::ALIGN,
                                                VALUE::LEFT,
                                                VALUE::RIGHT,
                                                VALUE::CENTER);
                mainFrame().showStatusBarWarning(text);
                return false;
            }
        }
    }
    else
    {   text = Global::jsonLanguageEntry(jsonValue);
        alignment = Qt::AlignLeft;
    }
    QLabel* label = new QLabel(text);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    label->setAlignment(alignment);
    layout->addWidget(label);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::populateDashboard(void)
{
    QWidget* widget = nullptr;
    const QJsonArray& jsonDashboard = serviceMap().dashboard();
    if (!jsonDashboard.isEmpty())
    {   QBoxLayout* layout = new QVBoxLayout();
        if (addArrayOfObjects(jsonDashboard, layout))
        {   layout->addStretch();
            layout->setSizeConstraint(QLayout::SetFixedSize);
            widget = new QWidget();
            widget->setLayout(layout);
        }
        else
        {   delete layout;
        }
    }
    if (widget == nullptr) widget = createNoDashboardSymbol();
    mUi->scrollArea->setWidget(widget);
    mCurrentServiceWidget = nullptr;
    // Steuerelemente freigeben
    bool isDeviceAssigned = mCurrentDevice.isValid();
    bool emptyDashboard = isDashboardEmpty(mUi);
    mUi->reloadMenuItem->setEnabled(isDeviceAssigned);
    mUi->refreshAllButton->setEnabled(isDeviceAssigned && !emptyDashboard);
    mUi->refreshAllMenuItem->setEnabled(isDeviceAssigned && !emptyDashboard);
    mUi->refreshOneMenuItem->setEnabled(isDeviceAssigned && !emptyDashboard);
    mUi->saveConfigurationMenuItem->setEnabled(isDeviceAssigned && !emptyDashboard);
    mUi->loadConfigurationMenuItem->setEnabled(isDeviceAssigned && !emptyDashboard);
    mUi->jumpToRelatedServiceMenuItem->setEnabled(isDeviceAssigned && !emptyDashboard);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DashboardPage::addObject(const QJsonObject& jsonObject, QBoxLayout* layout)
{
    if (jsonObject.contains(KEY::GROUP))
    {   if (!addGroup(jsonObject.value(KEY::GROUP).toObject(), layout)) return false;
    }
    else if (jsonObject.contains(KEY::GRID))
    {   if(!addGrid(jsonObject.value(KEY::GRID).toObject(), layout)) return false;
    }
    else if (jsonObject.contains(KEY::SERVICE))
    {   if(!addService(jsonObject.value(KEY::SERVICE).toObject(), layout)) return false;
    }
    else if (jsonObject.contains(KEY::SEPARATOR))
    {   if(!addSeparator(jsonObject.value(KEY::SEPARATOR).toObject(), layout)) return false;
    }
    else if (jsonObject.contains(KEY::LABEL))
    {   if(!addLabel(jsonObject.value(KEY::LABEL), layout)) return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onRefreshAll(void)
{
    if (cy4Net().isBusy()) return;
    if (isDashboardEmpty(mUi)) return;
    mAbortTransfer = false;
    mainFrame().setEnabled(false);
    mUi->toolBarFrame->setEnabled(false);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    // Cache neu füllen
    bool success = true;
    for (int i=0; i<serviceMap().size(); i++)
    {   mainFrame().showStatusBarProgress(0, i, serviceMap().size()-1);
        Service& service = serviceMap().at(i);
        if (!service.isValid())
        {   success = false;
            continue;
        }        
        if (!hasWidget(mUi, service)) continue; // Kein Widget, kein Refresh
        if (success)
        {   QString text = QObject::tr("Read ... service %1h:%2")
                                       .arg(service.location().toText())
                                       .arg(service.size());
            mainFrame().showStatusBarInfo(text);
        }
        Cy4Address address = mCurrentDevice.address();
        if (!transceive(address, service, Cy4Message::Read, &mAbortTransfer))
        {   success = false;
            QString text = tr("Read ... service %1h:%2 failed")
                          .arg(service.location().toText())
                          .arg(service.size());
            mainFrame().showStatusBarWarning(text);
        }
        if (mAbortTransfer) break;
    }
    if (success)
    {   if (mAbortTransfer)
        {   mainFrame().showStatusBarInfo(QObject::tr("Refresh ... stopped"));
        }
        else
        {   mainFrame().showStatusBarInfo(QObject::tr("Refresh ... completed"));
        }
    }
    updateServiceWidgets(mUi);
    QGuiApplication::restoreOverrideCursor();
    mainFrame().setEnabled(true);
    mUi->toolBarFrame->setEnabled(true);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onRefreshOne(void)
{
    if (cy4Net().isBusy()) return;
    if (mCurrentServiceWidget == nullptr) return;
    if (!mCurrentDevice.isValid()) return;
    // (1) Daten für <mCurrentServiceWidget> vom Gerät abrufen
    const ServiceProxy& serviceProxy = mCurrentServiceWidget->service();
    int index = serviceMap().indexOf(serviceProxy.location(), serviceProxy.size(), serviceProxy.access());
    if (index == INVALID) return;
    Service& service = serviceMap().at(index);
    if (!service.isValid() || service.access() == Service::WriteOnly) return;
    mAbortTransfer = false;
    mainFrame().setEnabled(false);
    mUi->scrollArea->setEnabled(false);
    mUi->toolBarFrame->setEnabled(false);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QString text = tr("Read ... service %1h:%2").arg(service.location().toText())
                                                .arg(service.size());
    Cy4Address address = mCurrentDevice.address();
    mainFrame().showStatusBarInfo(text);
    bool success = transceive(address,
                              service,
                              serviceProxy.location(),
                              serviceProxy.size(),
                              Cy4Message::Read,
                              &mAbortTransfer);
    if (!success)
    {   QString text = tr("Read ... service %1h:%2 failed")
                          .arg(service.location().toText())
                          .arg(service.size());
        mainFrame().showStatusBarWarning(text);
    }
    else if (mAbortTransfer)
    {   mainFrame().showStatusBarInfo(tr("Refresh ... stopped"));
    }
    else
    {   mainFrame().showStatusBarInfo(tr("Refresh ... complete"));
    }
    // (2) Steuerelemente mit den erhaltenen Dienst-Daten ausstatten
    mCurrentServiceWidget->populateWidget();
    QGuiApplication::restoreOverrideCursor();
    mainFrame().setEnabled(true);
    mUi->scrollArea->setEnabled(true);
    mUi->toolBarFrame->setEnabled(true);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onReload(void)
{
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) mCurrentDevice = Cy4Device();
    else                   mCurrentDevice = *device;
    QString issue;
    if (serviceMap().load(mCurrentDevice, issue))
    {   mainFrame().showStatusBarInfo(tr("Service map reloaded"));
    }
    else
    {   mainFrame().showStatusBarWarning(issue);
    }
    populateDashboard();
    onRefreshAll();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onOpenInEditor(void)
{
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) return;
    Cy4ApplicationType type = device->typePlate().applicationType();
    QString filePath = Global::mapFilePathFor(type);
    QString issue;
    if (!Global::openFileInEditor(filePath, &issue))
    {   mainFrame().showStatusBarWarning(issue);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onCustomAccess(void)
{
    mainFrame().showPage(Page::CustomAccess);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onCopyToClipboardMenuItem(void)
{
    if (mCurrentServiceWidget == nullptr) return;
    QString text = mCurrentServiceWidget->toClipboardText();
    if (!text.isEmpty())
    {   QApplication::clipboard()->setText(text);
        mainFrame().showStatusBarInfo(tr("Widget content copied to clipboard"));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onDownloadServiceMap(void)
{
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) return;
    Cy4ApplicationType type = device->typePlate().applicationType();
    QString tempFilePath = QString("%1/tmp_%2").arg(QDir::tempPath(), type.toText());
    QString url = Global::mapFileUrlFor(type);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString issue;
    bool success = downloader().get(url, tempFilePath, issue);
    if (!success)
    {   mainFrame().showStatusBarInfo(QObject::tr("Download failed: %1").arg(issue));
        QGuiApplication::restoreOverrideCursor();
        return;
    }
    if (serviceMap().import(*device, tempFilePath, issue))
    {   mainFrame().showStatusBarInfo(tr("Service map downloaded"));
    }
    else
    {   mainFrame().showStatusBarWarning(issue);
    }
    Global::removeFile(tempFilePath);
    onReload();
    QGuiApplication::restoreOverrideCursor();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onImportServiceMap(void)
{
    QString preset = settings().serviceMapsImportExportFilePath;
    if (!Global::pathExists(preset)) preset = settings().localCy4ToolDataDirectoryPath;
    QString filter   = "*.json\n*.*";
    QString title    = tr("Import service map");
    QString filePath = QFileDialog::getOpenFileName(NULL, title, preset, filter);
    if (filePath.isEmpty()) return;
    settings().serviceMapsImportExportFilePath = filePath;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) return;
    QString issue;
    if (serviceMap().import(*device, filePath, issue))
    {   mainFrame().showStatusBarInfo(tr("Service map imported"));
    }
    else
    {   mainFrame().showStatusBarWarning(issue);
    }
    mCurrentDevice = serviceMap().device();
    populateDashboard();
    onRefreshAll();
    QGuiApplication::restoreOverrideCursor();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onSaveConfiguration(void)
{
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) return;
    if (*device != serviceMap().device()) return;
    QString filePath = configurationFilePath(FileAccess::Save, *device);
    if (filePath.isEmpty()) return;
    mainFrame().setEnabled(false);
    QJsonObject jsonObject;
    Cy4Address address = device->address();
    // Kopfinformationen einfügen
    {   const Cy4TypePlate& typePlate = device->typePlate();
        jsonObject[KEY::FILE_IDENTIFICATION] = VALUE::CONFIGURATION_FILE_IDENTIFICATION;
        jsonObject[KEY::APPLICATION_LABEL]   = cy4Net().queryLabel(address);
        jsonObject[KEY::APPLICATION_TYPE]    = typePlate.applicationType().toText().c_str();
        jsonObject[KEY::APPLICATION_VERSION] = typePlate.applicationVersion().toText().c_str();
        jsonObject[KEY::CREATION_DATE]       = QDateTime::currentDateTime().toString("MMM dd yyyy, hh:mm:ss");
    }
    // Alle schreib-/lesbaren Dienste auslesen und als <jsonMessageObject> umsetzen
    {   QJsonArray jsonMessageArray;
        for (int i=0; i<serviceMap().size(); i++)
        {   mainFrame().showStatusBarProgress(0, i, serviceMap().size()-1);
            Service& service = serviceMap().at(i);
            if (!service.isValid() ||service.access() != Service::ReadWrite) continue;
            QString text = tr("Save service content %1h:%2")
                              .arg(service.location().toText())
                              .arg(service.size());
            mainFrame().showStatusBarInfo(text);
            Cy4Location location = service.location();
            int size = service.size();
            list<Cy4Message> list = service.toRequests(address, location, size, Cy4Message::Read);
            ::list<Cy4Message>::const_iterator it = list.cbegin();
            QJsonObject jsonMessageObject;
            int count = 0;
            while (it != list.cend())
            {   const Cy4Message& request = *it;
                Cy4Message response = transceive(request);
                service.fromResponse(response);
                if (response.isValid())
                {   QString name = service.name();
                    if (list.size() > 1)
                    {   // Wenn der Dienst mehrere Nachrichten umfasst, Nummer zufügen
                        count++;
                        name.append(QString(" (#%1/%2)").arg(count).arg(list.size()));
                    }
                    jsonMessageObject[QString("#%1").arg(KEY::NAME)] = name;
                    jsonMessageObject[KEY::LOCATION] = QString("%1h").arg(response.location().toText().c_str());
                    QJsonArray jsonPayloadArray;
                    for (int i=0; i<response.count(); i++) jsonPayloadArray.append(response[i]);
                    jsonMessageObject[KEY::PAYLOAD] = jsonPayloadArray;
                    jsonMessageArray.append(jsonMessageObject);
                }
                it++;
            }
        }
        jsonObject[KEY::MESSAGES] = jsonMessageArray;
    }
    // JSON-Konfigurationsdatei schreiben
    {   QJsonDocument jsonDocument(jsonObject);
        QFile file(filePath);
        if (Global::save(filePath, jsonDocument.toJson()))
        {   QString text = tr("Service content written to %1")
                              .arg(QFileInfo(filePath).fileName());
            mainFrame().showStatusBarInfo(text);
        }
        else
        {   QString text = tr("Write service content to %1 failed")
                              .arg(QFileInfo(filePath).fileName());
            mainFrame().showStatusBarWarning(text);
        }
    }
    mainFrame().setEnabled(true);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onLoadConfiguration(void)
{
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) return;
    if (*device != serviceMap().device()) return;
    Cy4Address address = device->address();
    // JSON-Konfigurationsdatei lesen
    QJsonObject jsonObject;
    {   QString filePath = configurationFilePath(FileAccess::Load, *device);
        if (filePath.isEmpty()) return;
        mainFrame().showStatusBarInfo(tr("Load configuration ..."));
        QString issue;
        if (!Global::load(filePath, jsonObject, issue))
        {   mainFrame().showStatusBarWarning(issue);
            return;
        }
    }
    // Dateiidentifikation prüfen
    {   QString fileId = jsonObject.value(KEY::FILE_IDENTIFICATION).toString();
        if (fileId != VALUE::CONFIGURATION_FILE_IDENTIFICATION &&
            fileId != DEPRECATED::CONFIGURATION_FILE_IDENTIFICATION)
        {   QString text = tr("Configuration file format id \"%1\" expected")
                              .arg(VALUE::CONFIGURATION_FILE_IDENTIFICATION);
            mainFrame().showStatusBarWarning(text);
            return;
        }
    }
    // Applikationstypennummer prüfen
    {   QString jsonText;
        if (jsonObject.contains(KEY::APPLICATION_TYPE))
        {   jsonText = jsonObject.value(KEY::APPLICATION_TYPE).toString();
        }
        else if (jsonObject.contains(DEPRECATED::APPLICATION_TYPE))
        {   jsonText = jsonObject.value(DEPRECATED::APPLICATION_TYPE).toString();
        }
        Cy4ApplicationType fileApplicationType    = Cy4ApplicationType(jsonText.toStdString());
        Cy4ApplicationType currentApplicationType = device->typePlate().applicationType();
        bool mismatch = fileApplicationType.isValid() &&
                        fileApplicationType != currentApplicationType;
        if (mismatch)
        {   QString text = tr("<b>Application type mismatch</b><p>"
                              "Origin of configuration was application type <b>%1</b>,<br>"
                              "current target application type is <b>%2</b>.<p>"
                              "Continue anyway?").arg(fileApplicationType.toText(),
                                                      currentApplicationType.toText());
            if (QMessageBox::warning(NULL,
                                     APPL_NAME,
                                     text,
                                     QMessageBox::Yes,
                                     QMessageBox::No) == QMessageBox::No) return;
        }
    }
    // Applikationsversion prüfen
    {   QString jsonText;
        if (jsonObject.contains(KEY::APPLICATION_VERSION))
        {   jsonText = jsonObject.value(KEY::APPLICATION_VERSION).toString();
        }
        else if (jsonObject.contains(DEPRECATED::APPLICATION_VERSION))
        {   jsonText = jsonObject.value(DEPRECATED::APPLICATION_VERSION).toString();
        }
        Cy4Version fileApplicationVersion    = Cy4Version(jsonText.toStdString());
        Cy4Version currentApplicationVersion = device->typePlate().applicationVersion();
        bool mismatch = fileApplicationVersion.isValid() &&
                        fileApplicationVersion != currentApplicationVersion;
        if (mismatch)
        {   QString text = tr("<b>Version mismatch</b><p>"
                              "Origin of configuration was application version <b>%1</b>,<br>"
                              "current target application version is <b>%2</b>.<p>"
                              "Continue anyway?").arg(fileApplicationVersion.toText(),
                                                      currentApplicationVersion.toText());
            if (QMessageBox::warning(NULL,
                                     APPL_NAME,
                                     text,
                                     QMessageBox::Yes,
                                     QMessageBox::No) == QMessageBox::No) return;
        }
    }
    mainFrame().setEnabled(false);
    // Nachrichten aus Datei extrahieren und Dienste beschreiben
    int failedCounter = 0;
    {   QJsonArray jsonMessageArray = jsonObject.value(KEY::MESSAGES).toArray();
        for (int i=0; i<jsonMessageArray.count(); i++)
        {   mainFrame().showStatusBarProgress(0, i, jsonMessageArray.count()-1);
            QJsonObject jsonMessageObject = jsonMessageArray[i].toObject();
            // Schreibaufforderung holen
            Cy4Message writeRequest = toWriteRequest(address, jsonMessageObject);
            if (!writeRequest.isValid()) continue;   // Sollte eigentlich nicht vorkommen
            // Dienst im Gerät beschreiben
            if (!transceive(writeRequest).isValid())
            {   failedCounter++;
                continue;
            }
            // Dienst-Cache beschreiben
            // Hinweis: Der Dienst-Cache wird mittels "fromResponse()" mit Daten befüllt.
            //          Da die Methode jedoch eine Leseantwort als Parameter erwartet,
            //          muss die aus der JSON-Datei hervorgegangenen Schreibaufforderungen
            //          vor Übergabe an "fromResponse()" zunächst in eine Leseantwort
            //          umgewandelt werden.
            Cy4Message readResponse = Cy4Message(writeRequest.address(),
                                                 writeRequest.location(),
                                                 writeRequest.count());
            for (int i=0; i<writeRequest.count(); i++) readResponse[i] = writeRequest[i];
            int index = serviceMap().indexOf(readResponse.location(),
                                             readResponse.count(),
                                             Service::ReadWrite);
            if (index != INVALID)
            {   Service& service = serviceMap().at(index);
                service.fromResponse(readResponse);
            }
        }
        updateServiceWidgets(mUi);
    }
    // Applikationsbezeichnung und Geräteliste aktualisieren
    {   int index = deviceList().indexOf(address);
        if (index == INVALID)
        {   mUi->titleLabelText->clear();
        }
        else
        {   deviceList().fetch(index);
            mUi->titleLabelText->setText(deviceList().label(index));
        }
    }
    QString text = tr("Load configuration ... completed");
    if (failedCounter > 0)
    {   text.append("  ");
        text.append(tr("(%1 messages failed)").arg(failedCounter));
    }
    mainFrame().showStatusBarInfo(text);
    mainFrame().setEnabled(true);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onOpenServiceListPageAndSelectService(void)
{
    mainFrame().showPage(Page::ServiceList);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onContextMenu(QPoint)
{
    AbstractServiceWidget* source = qobject_cast<AbstractServiceWidget*>(sender());
    if (source != nullptr)
    {   mCurrentServiceWidget = source;
        bool isReadable = mCurrentServiceWidget->service().access() != Service::WriteOnly;
        mUi->refreshOneMenuItem->setEnabled(isReadable);
        mContextMenu.exec(QCursor::pos());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onWidgetDataChanged(void)
{
    AbstractServiceWidget* widget = qobject_cast<AbstractServiceWidget*>(sender());
    if (widget == nullptr) return;
    const ServiceProxy& serviceProxy = widget->service();
    int index = serviceMap().indexOf(serviceProxy.location(),
                                     serviceProxy.size(),
                                     serviceProxy.access());
    if (index == INVALID) return;
    Service& service = serviceMap().at(index);
    // Nur Daten mit Cache-Zustand 'Touched' schicken (bzw. die Sequenz)
    Cy4Location location = service.location();
    int         size     = service.size();
    {   int startIndex = INVALID;
        int lastIndex  = 0;
        for (int i=0; i<size; i++)
        {   if (service.cacheStatusAt(i) != Service::UptoDate)
            {   if (startIndex == INVALID) startIndex = i;
                lastIndex = i;
            }
        }
        if (startIndex == INVALID) return; // Nichts zu schreiben
        location += startIndex;
        size = lastIndex - startIndex + 1;
    }
    QString text = tr("Write ... service %1h:%2").arg(location.toText()).arg(size);
    mainFrame().showStatusBarInfo(text);
    Cy4Address address = mCurrentDevice.address();
    mAbortTransfer = false;
    if (transceive(address,
                   service,
                   location,
                   size,
                   Cy4Message::Write,
                   &mAbortTransfer))
    {   QString text = tr("Write ... service %1h:%2 done").arg(location.toText()).arg(size);
        mainFrame().showStatusBarInfo(text);
    }
    else
    {   // Übertragung fehlgeschlagen
        QString text = tr("Write ... service %1h:%2 failed").arg(location.toText()).arg(size);
        mainFrame().showStatusBarWarning(text);
    }
    widget->styleWidget();
    // Bei Zugriff im Bereich FFFF00 .. FFFFFF, Gerät in der Geräteliste aktualisieren
    if (location >= 0xFFFF00)
    {   int deviceListIndex = deviceList().indexOf(address);
        if (deviceListIndex == INVALID)
        {   mUi->titleLabelText->clear();
        }
        else
        {   cy4Net().postPause();
            deviceList().fetch(deviceListIndex);
            mUi->titleLabelText->setText(deviceList().label(deviceListIndex));
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    if (key == Qt::Key_Escape)
    {   mAbortTransfer = true;
        event->accept();
    }
    else if (key == Qt::Key_F5)
    {   onRefreshAll();
        event->accept();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {   mAbortTransfer = true;
        event->accept();
    }
    else
    {   QWidget::mousePressEvent(event);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::retranslateUi(void)
{
    mUi->retranslateUi(this);
    QString dummy;
    serviceMap().load(mCurrentDevice, dummy);
    populateDashboard();
    onRefreshAll();
    updateServiceWidgets(mUi);
}

//////////////////////////////////////////////////////////////////////////////////////////
DashboardPage::DashboardPage(QWidget* parent) : AbstractPage(parent)
{
    // GUI
    {   mUi = new Ui::DashboardPage;
        mUi->setupUi(this);
    }
    // (...)-Menü
    {   mMoreMenu.addAction(mUi->refreshAllMenuItem);
        mMoreMenu.addSeparator();
        mMoreMenu.addAction(mUi->reloadMenuItem);
        mMoreMenu.addAction(mUi->downloadMeniItem);
        mMoreMenu.addAction(mUi->importMenuItem);
        mMoreMenu.addAction(mUi->openInEditorMenuItem);
        mMoreMenu.addSeparator();
        mMoreMenu.addAction(mUi->customAccessMenuItem);
        mMoreMenu.addSeparator();
        mMoreMenu.addAction(mUi->saveConfigurationMenuItem);
        mMoreMenu.addAction(mUi->loadConfigurationMenuItem);
        mUi->moreButton->setMenu(&mMoreMenu);
    }
    // Kontextmenü
    {   mContextMenu.addAction(mUi->refreshOneMenuItem);
        mContextMenu.addAction(mUi->jumpToRelatedServiceMenuItem);
        mContextMenu.addAction(mUi->customAccessMenuItem);
        mContextMenu.addAction(mUi->copyToClipboardMenuItem);
    }
    // Ereignisverknüpfungen
    {   connect(mUi->openInEditorButton          , &QPushButton::clicked, this, &DashboardPage::onOpenInEditor);
        connect(mUi->reloadServiceMapButton      , &QPushButton::clicked, this, &DashboardPage::onReload);
        connect(mUi->refreshAllButton            , &QPushButton::clicked, this, &DashboardPage::onRefreshAll);
        connect(mUi->refreshAllMenuItem          , &QAction::triggered  , this, &DashboardPage::onRefreshAll);
        connect(mUi->refreshOneMenuItem          , &QAction::triggered  , this, &DashboardPage::onRefreshOne);
        connect(mUi->downloadMeniItem            , &QAction::triggered  , this, &DashboardPage::onDownloadServiceMap);
        connect(mUi->importMenuItem              , &QAction::triggered  , this, &DashboardPage::onImportServiceMap);
        connect(mUi->openInEditorMenuItem        , &QAction::triggered  , this, &DashboardPage::onOpenInEditor);
        connect(mUi->customAccessMenuItem        , &QAction::triggered  , this, &DashboardPage::onCustomAccess);
        connect(mUi->copyToClipboardMenuItem     , &QAction::triggered  , this, &DashboardPage::onCopyToClipboardMenuItem);
        connect(mUi->saveConfigurationMenuItem   , &QAction::triggered  , this, &DashboardPage::onSaveConfiguration);
        connect(mUi->loadConfigurationMenuItem   , &QAction::triggered  , this, &DashboardPage::onLoadConfiguration);
        connect(mUi->jumpToRelatedServiceMenuItem, &QAction::triggered  , this, &DashboardPage::onOpenServiceListPageAndSelectService);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
DashboardPage::~DashboardPage()
{
    delete mUi;
}

//////////////////////////////////////////////////////////////////////////////////////////
Page
DashboardPage::page(void) const
{
    return Page::Dashboard;
}

//////////////////////////////////////////////////////////////////////////////////////////
const AbstractServiceWidget*
DashboardPage::currentServiceWidget(void) const
{
    return mCurrentServiceWidget;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onShow(void)
{
    const Cy4Device* device = Global::currentDevice();
    if (device == nullptr) return;
    if (mCurrentDevice != *device || isDashboardEmpty(mUi))
    {   // Kopfzeile bestücken und Dienstekarte laden
        const Cy4TypePlate& typePlate = device->typePlate();
        mUi->titleApplicationTypeText->setText(typePlate.applicationType().toText().c_str());
        mUi->titleApplicationVersionText->setText(typePlate.applicationVersion().toText().c_str());
        int index = deviceList().indexOf(device->address());
        if (index == INVALID)
        {   mUi->titleLabelText->clear();
        }
        else
        {   mUi->titleLabelText->setText(deviceList().label(index));
            deviceList().clearNewFlag(index);
        }
        onReload();
    }
    else
    {   if (settings().reloadOnOpenDashboardPage) onRefreshAll();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DashboardPage::onHide(void)
{
    if (mCurrentDevice != serviceMap().device())
    {   // Tritt nur ein, wenn die Anwendung geschlossen wird
        mCurrentServiceWidget = nullptr;
        delete mUi->scrollArea->takeWidget();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
