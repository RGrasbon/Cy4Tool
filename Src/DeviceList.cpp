//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Cy4-Geräteliste                                                                     //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QFont>
#include <QBrush>
#include "Conf.h"
#include "Global.h"
#include "KeyValues.h"
#include "Cy4Net.h"
#include "Settings.h"
#include "DeviceList.h"
#include "MainFrame.h"
#include "DeviceListPage.h"

static const int   COLUMN_COUNT = 5;

//////////////////////////////////////////////////////////////////////////////////////////
static int
gatewayIndex(void)
{
    static int gatewayIndex = INVALID;

    const Cy4Address& gatewayAddress = cy4Net().gateway();
    if (gatewayAddress.isValid())
    {   if (gatewayIndex < 0 || gatewayIndex >= deviceList().size() ||
            deviceList().at(gatewayIndex).address() != gatewayAddress)
        {   // Gateway suchen
            gatewayIndex = INVALID;
            int index = 0;
            while (index < deviceList().size())
            {   if (deviceList().at(index).address() == gatewayAddress)
                {   gatewayIndex = index;
                    break;
                }
                index++;
            }
        }
    }
    else
    {   gatewayIndex = INVALID;
    }
    return gatewayIndex;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::fromJson(const QJsonObject& jsonObject, QList<Item>& list)
{
    list.clear();
    QMap<Cy4Address, Item> map;
    // Dateiidentifikation prüfen
    if (jsonObject.value(KEY::FILE_IDENTIFICATION).toString() != VALUE::DEVICE_LIST_FILE_IDENTIFICATION) return;
    QJsonObject jsonItemList = jsonObject.value(KEY::DEVICES).toObject();
    QStringList keyList = jsonItemList.keys();
    for (int i=0; i<keyList.count(); i++)
    {   QJsonObject jsonItem = jsonItemList.value(keyList[i]).toObject();
        // Adresse
        Cy4Address address = Cy4Address(keyList[i].toStdString());
        if (map.contains(address) || !address.isValid() || address.isBroadcast()) continue;
        // Typenschild
        QString applTypeText    = jsonItem.value(KEY::APPLICATION_TYPE).toString();
        QString applVersionText = jsonItem.value(KEY::APPLICATION_VERSION).toString();
        QString boardTypeText   = jsonItem.value(KEY::BOARD_TYPE).toString();
        Cy4TypePlate typePlate = Cy4TypePlate(Cy4ApplicationType(applTypeText.toStdString()),
                                              Cy4Version(applVersionText.toStdString()),
                                              Cy4BoardType(boardTypeText.toStdString()));
        // Geräteeintrag
        Item item;
        item.device   = Cy4Device(address, typePlate);
        item.label    = jsonItem.value(KEY::APPLICATION_LABEL).toString();
        if (item.label.isEmpty() && jsonItem.contains(KEY::LABEL))
        {    // Veraltet seit V2.0
            item.label    = jsonItem.value(KEY::LABEL).toString();
        }
        item.firmwareFile      = jsonItem.value(KEY::FIRMWARE_FILE).toString();
        item.configurationFile = jsonItem.value(KEY::CONFIGURATION_FILE).toString();
        item.isNew             = jsonItem.value(KEY::NEW_FLAG).toBool(false);
        item.isOnline          = jsonItem.value(KEY::ONLINE_FLAG).toBool(false);
        list.append(item);
    }
    std::sort(list.begin(), list.end());
}

//////////////////////////////////////////////////////////////////////////////////////////
QJsonObject
DeviceList::toJson(const QList<Item>& list) const
{
    QJsonObject jsonObj;
    jsonObj[KEY::FILE_IDENTIFICATION] = VALUE::DEVICE_LIST_FILE_IDENTIFICATION;
    QJsonObject jsonItemList;
    for (int i=0; i<list.count(); i++)
    {   const Item& item = list[i];
        QJsonObject jsonItem;
        jsonItem[KEY::APPLICATION_TYPE]    = item.device.typePlate().applicationType().toText().c_str();
        jsonItem[KEY::APPLICATION_VERSION] = item.device.typePlate().applicationVersion().toText().c_str();
        jsonItem[KEY::BOARD_TYPE]          = item.device.typePlate().boardType().toText().c_str();
        jsonItem[KEY::APPLICATION_LABEL]   = item.label;
        jsonItem[KEY::FIRMWARE_FILE]       = item.firmwareFile;
        jsonItem[KEY::CONFIGURATION_FILE]  = item.configurationFile;
        jsonItem[KEY::NEW_FLAG]            = item.isNew;
        jsonItem[KEY::ONLINE_FLAG]         = item.isOnline;
        QString addressText = item.device.address().toText().c_str();
        jsonItemList[addressText] = jsonItem;
    }
    jsonObj[KEY::DEVICES] = jsonItemList;
    return jsonObj;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
DeviceList::rowCount(const QModelIndex&) const
{
    return mList.count();
}

//////////////////////////////////////////////////////////////////////////////////////////
int
DeviceList::columnCount(const QModelIndex&) const
{
    return COLUMN_COUNT;
}

//////////////////////////////////////////////////////////////////////////////////////////
QVariant
DeviceList::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {   switch (role)
        {   case Qt::DisplayRole:
            {   QString headerText[] = { tr("Address"),
                                         tr("Appl."),
                                         tr("Vers."),
                                         tr("Board"),
                                         tr("Label")
                                       };
                QString text;
                if (section < CARDINALITY(headerText)) text = headerText[section];
                else                                   text = QStringLiteral("???");
                return text;
            }
            case Qt::TextAlignmentRole:
            {   int  alignment;
                if (section < COLUMN_COUNT-1) alignment = Qt::AlignCenter;
                else                          alignment = Qt::AlignVCenter | Qt::AlignLeft;
                return alignment;
            }
            case Qt::FontRole:
            {   QFont font;
                if (section == COLUMN_COUNT-1) font.setItalic(true);
                return font;
            }
            default:
                ;
        }
    }
    return QVariant();
}

//////////////////////////////////////////////////////////////////////////////////////////
Qt::ItemFlags
DeviceList::flags(const QModelIndex& modelIndex) const
{
    Qt::ItemFlags flags;

    flags = QAbstractItemModel::flags(modelIndex);
    int row  = modelIndex.row();
    Cy4Address address = mList.at(row).device.address();
    if (address != cy4Net().gateway())
    {   if (mGatewayIndex != INVALID && mList[mGatewayIndex].device.typePlate().isProgramLoader())
        {   flags &= ~Qt::ItemIsEnabled;
        }
    }
    return flags;
}

//////////////////////////////////////////////////////////////////////////////////////////
QVariant
DeviceList::data(const QModelIndex& modelIndex, int role) const
{
    int column = modelIndex.column();
    int index  = modelIndex.row();
    if (index >= mList.count()) return QVariant();
    const Item& item = mList[index];
    const Cy4Device& device = item.device;
    switch (role)
    {   case Qt::TextAlignmentRole:
        {   int  alignment;
            if (column == 4) alignment = Qt::AlignVCenter | Qt::AlignLeft;
            else             alignment = Qt::AlignCenter;
            return alignment;
        }
        case Qt::DisplayRole:
        {   QString text;
            if (item.isBusy)
            {   text = "× × ×";
            }
            else
            {   switch (column)
                {   case 0:  text = device.address().toText().c_str();                        break;
                    case 1:  text = device.typePlate().applicationType().toText().c_str();    break;
                    case 2:  text = device.typePlate().applicationVersion().toText().c_str(); break;
                    case 3:  text = device.typePlate().boardType().toText().c_str();          break;
                    default: ;
                }
                if (column == COLUMN_COUNT-1) // Letzte Spalte = Gerätebezeichnung
                {   text = item.label;
                }
            }
            return text;
        }
        case Qt::FontRole:
        {   QFont font;
            if (item.isNew) font.setBold(true);
            if (column == COLUMN_COUNT-1) font.setItalic(true);
            return font;
        }
        case Qt::ForegroundRole:
        {   QBrush brush;
            if (modelIndex.flags() & Qt::ItemIsEnabled)
            {   if (!item.isOnline)
                {   const Cy4Device* currentDevice = Global::currentDevice();
                    if (currentDevice != nullptr && item.device == *currentDevice)
                    {   brush = QBrush(QColor(100, 100, 100));
                    }
                    else
                    {   brush = QBrush(QColor(120, 120, 120));
                    }
                }
                else if (item.device.typePlate().isProgramLoader())
                {   brush = QBrush(QColor(222, 27, 27));
                }
                else if (item.device.address() == cy4Net().gateway())
                {   brush = QBrush(QColor(0, 0, 140));
                }
                else
                {   brush = QBrush(QColor(12, 12, 12));
                }
            }
            else
            {   brush = QBrush(QColor(120, 120, 120));
            }
            return brush;
        }
        case Qt::BackgroundRole:
        {   QBrush brush;            
            const Cy4Device* currentDevice = Global::currentDevice();
            if (currentDevice != nullptr && item.device == *currentDevice)
            {   brush = QBrush(QColor(200, 200, 200));
            }
            else
            {   if (device.address().net() == 255) brush = QBrush(QColor(248, 248, 248));
                else                               brush = QBrush(QColor(252, 252, 252));
            }
            return brush;
        }
        case Qt::ToolTipRole:
        {   QString text;
            if (item.device.address() == cy4Net().gateway())
            {   text = tr("Gateway device");
            }
            else if (item.device.address().net() == 255)
            {   text = tr("Device on landing page: 255.*");
            }
            return text;
        }
        default:
            ;
    }
    return QVariant();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::add(Cy4Address address)
{
    int index = indexOf(address);
    if (index == INVALID)
    {   // Einfügeposition bestimmen
        index = 0;
        while (index<mList.count())
        {   const Cy4Address& entryAddress = mList[index].device.address();
            if (address <= entryAddress) break;
            index++;
        }
        // Neuer Eintrag aufnehmen
        Item item;
        Cy4TypePlate typePlate = cy4Net().fetchTypePlate(address);
        item.device   = Cy4Device(address, typePlate);
        item.isOnline = item.device.isValid();
        item.isNew    = true;
        if (item.isOnline)
        {   if (typePlate.isProgramLoader()) item.label = tr("(Programloader)");
            else                             item.label = cy4Net().queryLabel(address);
        }
        beginInsertRows(QModelIndex(), index, index);
        mList.insert(index, item);
        if (mGatewayIndex != INVALID)
        {   if      (address <  cy4Net().gateway()) mGatewayIndex++;
            else if (address == cy4Net().gateway()) mGatewayIndex = index;
        }
        endInsertRows();
        emit deviceInserted(index);
    }
    else
    {   // Bestehenden Eintrag aktualisieren
        fetch(index);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::fetch(int index)
{
    Q_ASSERT(index >= 0 && index < size());
    Item& item = mList[index];
    Cy4Address   address   = item.device.address();
    Cy4TypePlate typePlate = address.isValid() ? cy4Net().fetchTypePlate(address) : Cy4TypePlate();
    if (typePlate.isValid())
    {   QString label;
        if (typePlate.isProgramLoader()) label = tr("(Programloader)");
        else                             label = cy4Net().queryLabel(address);
        Cy4TypePlate listedTypePlate = item.device.typePlate();
        if (listedTypePlate != typePlate || item.label != label || !mList[index].isOnline)
        {   item.device   = Cy4Device(address, typePlate);
            item.label    = label;
            item.isOnline = true;
            if (listedTypePlate.boardType() != typePlate.boardType())
            {   // Boardtyp hat sich geändert
                item.isNew = true;
                item.firmwareFile.clear();
            }
            else if ( (listedTypePlate.applicationType().isProgramLoader()  &&
                       !typePlate.applicationType().isProgramLoader()) ||
                      (!listedTypePlate.applicationType().isProgramLoader() &&
                       typePlate.applicationType().isProgramLoader())
                    )
            {   // Bei Wechsel Programmlader → Applikation oder umgekehrt, 'isNew'
                // unverändert lassen
            }
            else if (listedTypePlate.applicationType() != typePlate.applicationType())
            {   // Applikationstyp hat sich geändert
                item.isNew = true;
            }
            emit dataChanged(createIndex(index, 0), createIndex(index, COLUMN_COUNT-1));
            emit deviceChanged(index);
        }
    }
    else
    {   if (mList[index].isOnline)
        {   mList[index].isOnline = false;
            emit dataChanged(createIndex(index, 0), createIndex(index, COLUMN_COUNT-1));
            emit deviceChanged(index);
        }
    }
    if (address == cy4Net().gateway()) mGatewayIndex = index;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::remove(int index)
{
    Q_ASSERT(index >= 0 && index < size());
    Cy4Address address = mList[index].device.address();
    if (address == cy4Net().gateway()) mGatewayIndex = INVALID;
    beginRemoveRows(QModelIndex(), index, index);
    if (mGatewayIndex != INVALID && index < mGatewayIndex) mGatewayIndex--;
    mList.removeAt(index);
    endRemoveRows();
    emit deviceRemoved(address);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DeviceList::isEmpty(void) const
{
    return mList.isEmpty();
}

//////////////////////////////////////////////////////////////////////////////////////////
int
DeviceList::size(void) const
{
    return mList.count();
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DeviceList::contains(Cy4Address address) const
{
    return indexOf(address) != INVALID;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
DeviceList::indexOf(Cy4Address address) const
{
    if (address.isValid())
    {   if (address == cy4Net().gateway()) return gatewayIndex();
        for (int index = 0; index <mList.count(); index++)
        {   if (mList[index].device.address() == address) return index;
        }
    }
    return INVALID;
}

//////////////////////////////////////////////////////////////////////////////////////////
const Cy4Device&
DeviceList::at(int index) const
{
    Q_ASSERT(index >= 0 && index < size());
    return mList[index].device;
}

//////////////////////////////////////////////////////////////////////////////////////////
const QString&
DeviceList::label(int index) const
{
    Q_ASSERT(index >= 0 && index < size());
    return mList[index].label;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DeviceList::isOnline(int index) const
{
    Q_ASSERT(index >= 0 && index < size());
    int gatewayIndex = ::gatewayIndex();
    if (gatewayIndex == INVALID) return false;
    if (!mList[gatewayIndex].isOnline) return false;
    if (index != gatewayIndex && mList[gatewayIndex].device.typePlate().isProgramLoader())
    {   return false;
    }
    return mList[index].isOnline;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DeviceList::isNew(int index) const
{
    Q_ASSERT(index >= 0 && index < size());
    return mList[index].isNew;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::setNewFlag(int index)
{
    Q_ASSERT(index >= 0 && index < size());
    if (!mList[index].isNew)
    {   mList[index].isNew = true;
        emit dataChanged(createIndex(index, 0), createIndex(index, COLUMN_COUNT-1));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::clearNewFlag(int index)
{
    Q_ASSERT(index >= 0 && index < size());
    if (mList[index].isNew)
    {   mList[index].isNew = false;
        emit dataChanged(createIndex(index, 0), createIndex(index, COLUMN_COUNT-1));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::setBusyFlag(int index)
{
    Q_ASSERT(index >= 0 && index < size());
    if (!mList[index].isBusy)
    {   mList[index].isBusy = true;
        emit dataChanged(createIndex(index, 0), createIndex(index, COLUMN_COUNT-1));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::clearBusyFlag(int index)
{
    Q_ASSERT(index >= 0 && index < size());
    if (mList[index].isBusy)
    {   mList[index].isBusy = false;
        emit dataChanged(createIndex(index, 0), createIndex(index, COLUMN_COUNT-1));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
const QString&
DeviceList::firmwareFilePath(int index) const
{
    Q_ASSERT(index >= 0 && index < size());
    return mList[index].firmwareFile;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::setFirmwareFilePath(int index, const QString& filePath)
{
    Q_ASSERT(index >= 0 && index < size());
    if (mList[index].firmwareFile != filePath)
    {   mList[index].firmwareFile  = filePath;
        emit dataChanged(createIndex(index, 0), createIndex(index, COLUMN_COUNT-1));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
const QString&
DeviceList::configurationFilePath(int index) const
{
    Q_ASSERT(index >= 0 && index < size());
    return mList[index].configurationFile;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::setConfigurationFilePath(int index, const QString& filePath)
{
    Q_ASSERT(index >= 0 && index < size());
    if (mList[index].configurationFile != filePath)
    {   mList[index].configurationFile  = filePath;
        emit dataChanged(createIndex(index, 0), createIndex(index, COLUMN_COUNT-1));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
QModelIndex
DeviceList::modelIndexOf(int index) const
{
    if (index < 0 || index >= size()) return createIndex(-1, 0);
    return createIndex(index, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::load(void)
{
    QString issue;
    load(settings().localCy4ToolDataDirectoryPath+ "/" + CFG::DeviceListFileName, issue);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DeviceList::load(const QString& filePath, QString& issue)
{
    if (!mList.isEmpty())
    {   beginResetModel();
        mList.clear();
        endResetModel();
    }
    if (!Global::pathExists(filePath)) return false;
    QJsonObject jsonObject;
    if (!Global::load(filePath, jsonObject, issue)) return false;
    QList<Item> tmpList;
    fromJson(jsonObject, tmpList);
    if (!tmpList.isEmpty())
    {   beginInsertRows(QModelIndex(), 0, tmpList.count()-1);
        mList = tmpList;
        endInsertRows();
    }
    for (int index=0; index<mList.count(); index++) emit deviceInserted(index);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
DeviceList::save(void) const
{
    QString issue;
    saveAs(settings().localCy4ToolDataDirectoryPath+ "/" + CFG::DeviceListFileName, issue);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
DeviceList::saveAs(const QString& filePath, QString& issue) const
{
    QJsonObject storedJsonObject;
    if (!Global::save(filePath, storedJsonObject, issue)) return false;
    QJsonObject currentJsonObject = toJson(mList);
    if (currentJsonObject != storedJsonObject)
    {   // Speichern nur dann, wenn modifiziert
        return Global::save(filePath, currentJsonObject, issue);
    }
    issue.clear();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
DeviceList&
deviceList(void)
{
    static DeviceList instance;
    return instance;
}

//////////////////////////////////////////////////////////////////////////////////////////
