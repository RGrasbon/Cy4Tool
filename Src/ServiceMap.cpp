//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Anwendungs-Dienstekarte                                                             //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include "Global.h"
#include "KeyValues.h"
#include "Cy4Types.h"
#include "ServiceMap.h"
#include "Settings.h"

//////////////////////////////////////////////////////////////////////////////////////////
static bool
matchesVersion(const QString& text, Cy4Version version)
{
    QStringList list = text.split(',');     // "V1.0, V1.2, ... "
    for (const QString& text: static_cast<const QStringList&>(list))
    {   Cy4Version tmpVersion = Cy4Version(text.toStdString());
        if (tmpVersion == version) return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QJsonObject
loadMapFileFor(const Cy4ApplicationType& type, const QString& filePath, QString& issue)
{
    QJsonObject jsonServiceMap;
    if (!Global::load(filePath, jsonServiceMap, issue)) return QJsonObject();
    if (jsonServiceMap.value(KEY::FILE_IDENTIFICATION).toString() != VALUE::SERVICE_MAP_FILE_IDENTIFICATION)
    {   issue = QObject::tr("File is no service map");
        return QJsonObject();
    }
    if (!jsonServiceMap.contains(KEY::APPLICATION_TYPE) &&
        jsonServiceMap.contains(DEPRECATED::APPLICATION_TYPE))
    {   issue = QObject::tr("JSON key \"%1\" is deprecated. Use \"%2\" instead")
                            .arg(DEPRECATED::APPLICATION_TYPE, KEY::APPLICATION_TYPE);
    }
    if (!jsonServiceMap.contains(KEY::APPLICATION_BRIEF) &&
        jsonServiceMap.contains(DEPRECATED::APPLICATION_LABEL))
    {   issue = QObject::tr("JSON key \"%1\" is deprecated. Use \"%2\" instead")
                            .arg(DEPRECATED::APPLICATION_LABEL, KEY::APPLICATION_BRIEF);
    }
    if (!jsonServiceMap.contains(KEY::APPLICATION_TYPE) &&
        !jsonServiceMap.contains(DEPRECATED::APPLICATION_TYPE))
    {   issue = QObject::tr("JSON section \"%1\" missing").arg(KEY::APPLICATION_TYPE);
        return QJsonObject();
    }
    QString jsonTypeText = jsonServiceMap.value(KEY::APPLICATION_TYPE).toString();
    if (jsonTypeText.isEmpty()) jsonTypeText = jsonServiceMap.value(DEPRECATED::APPLICATION_TYPE).toString();
    Cy4ApplicationType jsonType = Cy4ApplicationType(jsonTypeText.toStdString());
    if (!jsonType.isValid())
    {   issue = QObject::tr("Invalid application type in section \"%1\": %2")
                            .arg(KEY::APPLICATION_TYPE, jsonTypeText);
        return QJsonObject();
    }
    if (jsonType != type)
    {   issue = QObject::tr("Service map intended for different application: %1")
                            .arg(jsonType.toText());
        return QJsonObject();
    }
    return jsonServiceMap;
}

//////////////////////////////////////////////////////////////////////////////////////////
static bool
preorder(const QJsonObject& jsonObject, QList<Service>& list)
{
    if (jsonObject.contains(KEY::GROUP))
    {   const QJsonObject& jsonGroup = jsonObject.value(KEY::GROUP).toObject();
        if (!jsonGroup.contains(KEY::ITEMS)) return true;
        if (!jsonGroup.value(KEY::ITEMS).isArray()) return false;
        const QJsonArray& jsonGroupArray = jsonGroup.value(KEY::ITEMS).toArray();
        for(const QJsonValue& jsonValue : jsonGroupArray)
        {   if( !preorder(jsonValue.toObject(), list) ) return false;
        }
    }
    else if (jsonObject.contains(KEY::GRID))
    {   const QJsonObject& jsonGrid = jsonObject.value(KEY::GRID).toObject();
        if (!jsonGrid.contains(KEY::ITEMS)) return true;
        const QJsonObject& jsonGridItems = jsonGrid.value(KEY::ITEMS).toObject();
        QStringList keyList = jsonGridItems.keys();
        for(const QString& key: static_cast<const QStringList>(keyList))
        {   if( !preorder(jsonGridItems[key].toObject(), list) ) return false;
        }
    }
    else if (jsonObject.contains(KEY::SERVICE))
    {   const QJsonObject& jsonService = jsonObject.value(KEY::SERVICE).toObject();
        Service service = Service(jsonService);
        list.append(service);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  subset(): Prüft, ob einer der angegebenen Dienste Teilmenge des anderen ist. Mög-   //
//  --------  liche Konstellationen sind:                                               //
//                                                                                      //
//           location                                                                   //
//              :                                                                       //
//              :····size····                                                           //
//              :           :                                                           //
//        A ----|+++++++++++|-------------        A --------------|++++++++|---         //
//        B -------|++++++++|-------------        B -----|+++++++++++++++++|---         //
//                                                                                      //
//        A -----|+++++++++++++|----------        A -------|+++++++++|---------         //
//        B -------|+++++++++|------------        B -----|+++++++++++++|-------         //
//                                                                                      //
//  Sind die Dienstvertreter deckungsgleich, ist der Rückkabewert 'true'. Ist           //
//  <serviceA> oder <serviceB> ungültig, ist das Ergebnis undefiniert.                  //
//                                                                                      //
//  Eingabe: · serviceA: Erster zu prüfender Dienst.                                    //
//           · serviceB: Zweiter zu prüfender Dienst.                                   //
//  Ausgabe: 'true', wenn <serviceA> Teilmenge von <serviceB> oder umgekehrt ist.       //
//////////////////////////////////////////////////////////////////////////////////////////
static bool
subset(const Service& serviceA, const Service& serviceB)
{
    Cy4Location locationAEnd = serviceA.location() + serviceA.size();
    Cy4Location locationBEnd = serviceB.location() + serviceB.size();
    return (serviceA.location() <= serviceB.location() && locationAEnd >= locationBEnd) ||
           (serviceA.location() >= serviceB.location() && locationAEnd <= locationBEnd);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  adjacent(): Prüft, ob die angegebenen Dienste unmittelbar angrenzend liegen, sich   //
//  ----------  aber nicht überscheiden (die Schnittmenge ist leer).                    //
//  Mögliche Konstellationen sind:                                                      //
//                                                                                      //
//           location                                                                   //
//              :                                                                       //
//              :····size····                                                           //
//              :           :                                                           //
//        A ----|+++++++++++|-------------        A --------------|++++++++|---         //
//        B -----------------|++++++++|---        B ---|+++++++++|-------------         //
//                                                                                      //
//  Ist <serviceA> oder <serviceB> ungültig, ist das Ergebnis undefiniert.              //
//                                                                                      //
//  Eingabe: · serviceA: Erster zu prüfender Dienst.                                    //
//           · serviceB: Zweiter zu prüfender Dienst.                                   //
//  Ausgabe: 'true', wenn <serviceA> zu <serviceB> oder <serviceB> zu <serviceA>        //
//           adjazent liegt.                                                            //
//////////////////////////////////////////////////////////////////////////////////////////
static bool
adjacent(const Service& serviceA, const Service& serviceB)
{
    return serviceA.location() + serviceA.size() == serviceB.location() ||
           serviceB.location() + serviceB.size() == serviceA.location();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  intersecting(): Prüft, ob sich die angegebenen Dienste überscheiden. Das ist dann   //
//  --------------  der Fall, wenn <serviceA> und <serviceB> eine nicht leere Schnitt-  //
//  menge haben. Mögliche Konstellationen sind:                                         //
//                                                                                      //
//           location                                                                   //
//              :                                                                       //
//              :····size····                                                           //
//              :           :                                                           //
//        A ----|+++++++++++|-------------        A --------------|++++++++|---         //
//        B -------------|++++++++|-------        B ----|++++++++++++|---------         //
//                                                                                      //
//        A --------|+++++++|-------------        A ---|+++++++++++++++++|-----         //
//        B ----|+++++++++++++++++|-------        B -----|+++++++++++|---------         //
//                                                                                      //
//  Ist <serviceA> oder <serviceB> ungültig, ist das Ergebnis undefiniert.              //
//                                                                                      //
//  Eingabe: · serviceA: Erster zu prüfender Dienst.                                    //
//           · serviceB: Zweiter zu prüfender Dienst.                                   //
//  Ausgabe: 'true', wenn <serviceA> und <serviceB> sich überscheiden.                  //
//////////////////////////////////////////////////////////////////////////////////////////
static bool
intersecting(const Service& serviceA, const Service& serviceB)
{
    return serviceA.location() + serviceA.size() > serviceB.location() &&
           serviceB.location() + serviceB.size() > serviceA.location();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  merge(): Fusioniert die angegebenen Dienste. Haben <serviceA> und <serviceB> die    //
//  -------  gleiche Zugriffsart, erhält der fusionierte Dienste ebenfalls diese        //
//  Zugriffsart. Andernfalls ist seine Zugriffsart schreib/lesbar (R/W). Der Name wird  //
//  von <serviceA> übernommen.                                                          //
//  Ist <serviceA> oder <serviceB> ungültig, ist das Ergebnis undefiniert.              //
//                                                                                      //
//  Eingabe: · serviceA: Erster zu fusionierende Dienst.                                //
//           · serviceB: Zweiter zu fusionierende Dienst.                               //
//  Ausgabe: Der aus der Fusion von <serviceA> und <serviceB> hervorgegangene Dienst.   //
//////////////////////////////////////////////////////////////////////////////////////////
static Service
merge(const Service& serviceA, const Service& serviceB)
{
    Cy4Location locationBegin = min(serviceA.location(), serviceB.location());
    Cy4Location locationEnd   = max(serviceA.location() + serviceA.size(),
                                    serviceB.location() + serviceB.size());
    int size = (locationEnd - locationBegin).toInteger();
    Service::Access access = Service::ReadWrite;
    if (serviceA.access() == serviceB.access()) access = serviceA.access();
    QString name;
    if(!serviceA.name().isEmpty() && serviceB.name().isEmpty())
    {   name = serviceA.name();
    }
    if(serviceA.name().isEmpty() && !serviceB.name().isEmpty())
    {   name = serviceB.name();
    }
    else if(!serviceB.name().isEmpty() && !serviceB.name().isEmpty())
    {   if (serviceA.name() == serviceB.name())
        {   name = serviceA.name();
        }
        else if (locationBegin < 0xFFFFF0)
        {   name = QString("%1 / %2").arg(serviceA.name(), serviceB.name());
        }
        else
        {   // <serviceA> überschreibt den Pflichtdienst in <serviceB>
            name = QString("%1 (%2)").arg(serviceA.name(), serviceB.name());
        }
    }
    return Service(name, locationBegin, size, access);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  checkMandatoryServiceConflict(): Pflichtdienste dürfen überschrieben aber nicht zu- //
//  -------------------------------  sammengeführt werden. Die Funktion prüft, ob       //
//  <service> im Bereich der Pflichtdienste liegt aber von keinem der Pflichtdienste    //
//  überdeckt wird.                                                                     //
//                                                                                      //
//  Eingabe: · service: Zu prüfender Dienst.                                            //
//  Ausgabe: 'true', wenn <service> Pflichtdienstbedingung verletzt.                    //
//////////////////////////////////////////////////////////////////////////////////////////
static bool
checkMandatoryServiceConflict(const Service& service)
{
    Cy4Location locationServiceEnd = service.location() + service.size();
    if (locationServiceEnd >= 0xFFFFF0)
    {   struct MandatoryService
        {   Cy4Location locationBegin;
            Cy4Location locationEnd;
        } const static mandatory[] = { { Cy4Location(0xFFFFF0), Cy4Location(0xFFFFF0 + 7) },
                                       { Cy4Location(0xFFFFF8), Cy4Location(0xFFFFF8 + 1) },
                                       { Cy4Location(0xFFFFFA), Cy4Location(0xFFFFFA + 1) },
                                       { Cy4Location(0xFFFFFC), Cy4Location(0xFFFFFC + 2) }
                                     };
        for (int i=0; i<CARDINALITY(mandatory); i++)
        {   if (service.location() >= mandatory[i].locationBegin &&
                locationServiceEnd <= mandatory[i].locationEnd)
            {   return false;
            }
        }
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
ServiceMap::ServiceMap(void)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
ServiceMap::~ServiceMap()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceMap::clear(void)
{
    *this = ServiceMap();
}

//////////////////////////////////////////////////////////////////////////////////////////
int
ServiceMap::size(void) const
{
    return mList.length();
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
ServiceMap::isValid(void) const
{
    for(const Item& item : mList)
    {   if (item.tag != Valid) return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
ServiceMap::isEmpty(void) const
{
    return mList.isEmpty();
}

//////////////////////////////////////////////////////////////////////////////////////////
const Service&
ServiceMap::at(int index) const
{
    return const_cast<ServiceMap&>(*this).at(index);
}

//////////////////////////////////////////////////////////////////////////////////////////
Service&
ServiceMap::at(int index)
{
    assert(index >= 0 && index < int(mList.size()));
    return mList[USize(index)].service;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
ServiceMap::indexOf(Cy4Location location, int size, Service::Access access) const
{
    // (1) Den zu <location> passenden Dienst aus der Dienstekarte suchen
    int index = INVALID;
    {   if (mList.isEmpty()) return INVALID;
        int a = 0;
        int b = mList.length()-1;
        if (location < mList[a].service.location())
        {   // <location> liegt vor den Diensten der Dienstekarte
            return INVALID;
        }
        else if (location < mList[b].service.location())
        {   int m;
            do
            {   m = a + ((b-a)/2);
                Cy4Location actual = mList[m].service.location();
                if (location > actual)
                {   a = m+1;
                }
                else if (location < actual)
                {   b = m-1;
                }
                else
                {   b = m;  // <location> startet genau auf dem Dienst mit Index 'm'
                    break;
                }
            } while (a <= b);
        }
        index = b;
    }
    const Service* mapService = &mList[index].service;
    // (2) Prüft, ob <location:size> Teilmenge des gefundenen Dienstes ist
    {   Cy4Location endOfService = location + size;
        Cy4Location endOfMapService = mapService->location() + mapService->size();
        if (endOfService > endOfMapService) return INVALID;
    }
    // (3) Prüfen, ob Zugriffsarten passen
    {   if (mapService->access() == Service::ReadOnly && access != Service::ReadOnly)
        {   return INVALID;
        }
        if (mapService->access() == Service::WriteOnly && access != Service::WriteOnly)
        {   return INVALID;
        }
    }
    return index;
}

//////////////////////////////////////////////////////////////////////////////////////////
ServiceMap::ItemTag
ServiceMap::tag(int index) const
{
    assert(index >= 0 && index < int(mList.size()));
    return mList[USize(index)].tag;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
ServiceMap::load(const Cy4Device& device, QString& issue)
{
    Cy4Device tmpDevice = device;
    clear();
    if (!tmpDevice.isValid())
    {   issue = QObject::tr("Load service map failed: Invalid device");
        return false;
    }
    // (1) Dienstekarte-Datei laden und prüfen
    QJsonObject jsonServiceMap;
    {   Cy4ApplicationType type = tmpDevice.typePlate().applicationType();
        QString filePath = Global::mapFilePathFor(type);
        if (!Global::pathExists(filePath))
        {   issue = QObject::tr("No service map file found for %1")
                                .arg(type.toText().c_str());
            return false;
        }
        jsonServiceMap = loadMapFileFor(type, filePath, issue);
        if (jsonServiceMap.isEmpty()) return false;
    }
    // (2) Die zu <version> passende JSON-Sektion in <jsonDashboard> extrahieren
    QJsonArray jsonDashboard;
    {   Cy4Version version = tmpDevice.typePlate().applicationVersion();
        QStringList keyList = jsonServiceMap.keys();
        QString key;
        while (!keyList.isEmpty())
        {   key = keyList.first();
            if (matchesVersion(key, version)) break;
            keyList.removeFirst();
        }
        if (keyList.isEmpty())
        {   issue = QObject::tr("No section found for \"%1\"")
                                .arg(version.toText());
            return false;
        }
        if (!jsonServiceMap.value(key).isArray())
        {   issue = QObject::tr("JSON array expected in section \"%1\"")
                                .arg(version.toText());
            return false;
        }
        jsonDashboard = jsonServiceMap.value(key).toArray();
    }
    // (3) Dienste aus <jsonDashboard> auslesen und <itemList> bestücken
    QList<Item> itemList;
    {   QList<Service> serviceList;
        for (const QJsonValue& jsonValue: static_cast<const QJsonArray&>(jsonDashboard))
        {   preorder(jsonValue.toObject(), serviceList);
        }
        for (int i=0; i<serviceList.length(); i++)
        {   Item item;
            item.service = serviceList[i];
            itemList.append(item);
        }
    }
    // (4) Pflichtdienste hinzufügen
    {   QJsonObject jsonMandatories;
        Global::load("://MandatoryServices.json", jsonMandatories);
        QJsonArray jsonArray = jsonMandatories.value(KEY::MANDATORY_SERVICES).toArray();
        for (const QJsonValue& value: static_cast<const QJsonArray&>(jsonArray))
        {   const QJsonObject& jsonService = value.toObject().value(KEY::SERVICE).toObject();
            Service service = Service(jsonService);
            QString briefText = Global::jsonLanguageEntry(jsonService.value(KEY::BRIEF));
            service.setBrief(briefText);
            Item item;
            item.service = service;
            itemList.append(item);
        }
    }
    // (5) Liste (nach Dienstorten) aufsteigend sortieren (stabil)
    std::stable_sort(itemList.begin(), itemList.end());
    // (6) Adjazente Dienste fusionieren
    if (!itemList.isEmpty())
    {   QList<Item>::iterator itA = itemList.begin();
        QList<Item>::iterator itB = itemList.begin();
        itB++;
        while (itB != itemList.end())
        {   const Service& serviceA = itA->service;
            const Service& serviceB = itB->service;
            bool merge;
            if (!serviceA.isValid() || !serviceB.isValid())
            {   merge = false;
            }
            else if (checkMandatoryServiceConflict(serviceA) ||
                     checkMandatoryServiceConflict(serviceB))
            {   // Pflichtdienste können überschrieben nicht aber zusammengeführt werden
                merge = false;
            }
            else if (subset(serviceA, serviceB))
            {   // Wird einer der Dienste durch den anderen komplett überdeckt, geht
                // bei einer Fusion der kleinere in den größeren auf. Das geht nur,
                // wenn beide Dienste entweder die gleiche Zugriffsart haben oder der
                // größere Dienst schreib/lesbar (R/W) ist.
                merge = (serviceA.access() == serviceB.access()) ||
                        (serviceA.size() >= serviceB.size() && serviceA.access() == Service::ReadWrite) ||
                        (serviceB.size() >= serviceA.size() && serviceB.access() == Service::ReadWrite);
            }
            else if (intersecting(serviceA, serviceB))
            {   // Überlappen sich die Dienste, überdecken sich aber nicht,
                // (¬subset()) kann eine Fusion nur stattfinden, wenn beide die
                // gleiche Zugriffsart haben.
                merge = serviceA.access() == serviceB.access();
            }
            else if (adjacent(serviceA, serviceB))
            {   // Wie bei intersecting() können adjazent liegenden Dienste nur
                // fusioniert werden, wenn sie die gleiche Zugriffsart haben.
                merge = serviceA.access() == serviceB.access();
            }
            else
            {   merge = false;
            }
            if (merge)
            {   QString brief;
                Service mergedService = ::merge(serviceA, serviceB);
                if (serviceA.brief() != serviceB.brief() &&
                    !serviceB.brief().isEmpty() &&
                    serviceB.location() < 0xFFFFF0)
                {   brief = QString("%1\n%2").arg(serviceA.brief(), serviceB.brief());
                }
                else
                {   brief = serviceA.brief();
                }
                mergedService.setBrief(brief);
                itA->service = mergedService;
                itB = itemList.erase(itB);
            }
            else
            {   itA++;
                itB++;
            }
        }
    }
    // (7) Status der Listeneinträge bestimmen
    if (!itemList.isEmpty())
    {   // 'Valid', 'Invalid', 'PgmLdrArea'?
        for (QList<Item>::iterator it = itemList.begin(); it < itemList.end(); it++)
        {   Item& item = *it;
            if (item.service.isValid())
            {   if      (item.service.location() < 0x100000) item.tag = PgmLdrArea;
                else                                         item.tag = Valid;
            }
            else
            {   item.tag = Invalid;
            }
        }
        // 'Overlap', 'Adjacent' ?
        {   QList<Item>::iterator itA = itemList.begin();
            QList<Item>::iterator itB = itemList.begin();
            itB++;
            while (itB != itemList.end())
            {   Item& itemA = *itA;
                Item& itemB = *itB;
                if (itemA.tag == Valid && itemB.tag == Valid)
                {   if (subset(itemA.service, itemB.service) ||
                        intersecting(itemA.service, itemB.service))
                    {   itemA.tag = Overlap;
                        itemB.tag = Overlap;
                    }
                    else if (adjacent(itemA.service, itemB.service))
                    {   itemA.tag = Adjacent;
                        itemB.tag = Adjacent;
                    }
                    if (itemA.tag == Overlap || itemA.tag == Adjacent)
                    {   if (checkMandatoryServiceConflict(itemA.service) ||
                            checkMandatoryServiceConflict(itemB.service))
                        {   itemA.tag = Mandatory;
                            itemB.tag = Mandatory;
                        }
                    }
                }
                itA++;
                itB++;
            }
        }
    }
    // (8) Abschluss
    mDevice        = tmpDevice;
    mList          = itemList;
    mJsonDashboard = jsonDashboard;
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
ServiceMap::import(const Cy4Device& device, const QString& filePath, QString& issue)
{
    Cy4ApplicationType type = device.typePlate().applicationType();
    if (!device.isValid()) return false;
    // (1) Datei in <filePath> prüfen
    {   QJsonObject jsonServiceMap = loadMapFileFor(type, filePath, issue);
        if (jsonServiceMap.isEmpty()) return false;
    }
    // (2) Bestehende Dienstekarten-Datei durch die Datei in <filePath> ersetzen
    {   QString applFileName = Global::mapFileNameFor(type);
        QString applFilePath = QString("%1/%2").arg(settings().localServiceMapsDirectoryPath,
                                                               applFileName);
        if (Global::pathExists(applFilePath))
        {
            QString text = QObject::tr("Replace existing service map file \"%1\"?")
                                       .arg(applFileName);
            if (QMessageBox::question(nullptr,
                                      APPL_NAME,
                                      text,
                                      QMessageBox::Yes,
                                      QMessageBox::No) == QMessageBox::No) return false;
        }
        if (!Global::copyFile(filePath, applFilePath))
        {   issue = QObject::tr("Saving service map file failed");
            return false;
        }
    }
    // (3) Dienstekarte aktualisieren
    return load(device, issue);
}

//////////////////////////////////////////////////////////////////////////////////////////
const QJsonArray&
ServiceMap::dashboard(void) const
{
    return mJsonDashboard;
}

//////////////////////////////////////////////////////////////////////////////////////////
const Cy4Device&
ServiceMap::device(void) const
{
    return mDevice;
}

//////////////////////////////////////////////////////////////////////////////////////////
ServiceMap&
serviceMap(void)
{
    static ServiceMap instance;
    return instance;
}

//////////////////////////////////////////////////////////////////////////////////////////


