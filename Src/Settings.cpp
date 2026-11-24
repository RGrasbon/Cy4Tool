//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Cy4Tool Einstellungen                                                               //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>
#include "Conf.h"
#include "KeyValues.h"
#include "Global.h"
#include "Settings.h"

//////////////////////////////////////////////////////////////////////////////////////////
static QPoint
toQPoint(const QJsonValue& jsonValue, const QPoint deflt)
{
    if (jsonValue.isUndefined() || jsonValue.isNull()) return deflt;
    QPoint point;
    QJsonObject object = jsonValue.toObject();
    point.setX(object.value(KEY::POINT_X).toInt());
    point.setY(object.value(KEY::POINT_Y).toInt());
    return point;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QJsonValue
toJson(const QPoint point)
{
    QJsonObject object;
    object[KEY::POINT_X] = point.x();
    object[KEY::POINT_Y] = point.y();
    return object;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QSize
toQSize(const QJsonValue& jsonValue, QSize deflt)
{
    if (jsonValue.isUndefined() || jsonValue.isNull()) return deflt;
    QSize size;
    QJsonObject object = jsonValue.toObject();
    size.setWidth(object.value(KEY::SIZE_WIDTH).toInt());
    size.setHeight(object.value(KEY::SIZE_HEIGHT).toInt());
    return size;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QJsonValue
toJson(QSize size)
{
    QJsonObject object;
    object[KEY::SIZE_WIDTH]  = size.width();
    object[KEY::SIZE_HEIGHT] = size.height();
    return object;
}

//////////////////////////////////////////////////////////////////////////////////////////
static ConnectionType
toConnectionType(const QJsonValue& jsonValue, ConnectionType deflt)
{
    if (jsonValue.isUndefined() || jsonValue.isNull()) return deflt;
    if (jsonValue.toString() == KEY::CONNECTION_TYPE_SOCKET) return ConnectionType::Socket;
    if (jsonValue.toString() == KEY::CONNECTION_TYPE_SERIAL) return ConnectionType::Serial;
    return ConnectionType::None;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QJsonValue
toJson(ConnectionType connectionType)
{
    switch (connectionType)
    {   case ConnectionType::Socket: return KEY::CONNECTION_TYPE_SOCKET;
        case ConnectionType::Serial: return KEY::CONNECTION_TYPE_SERIAL;
        default:                     return "none";
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
static Socket
toSocket(const QJsonValue& jsonValue, const Socket& deflt)
{
    if (jsonValue.isUndefined() || jsonValue.isNull()) return deflt;
    Socket socket;
    QJsonObject object = jsonValue.toObject();
    socket.ip   = object.value(KEY::SOCKET_IP).toString();
    socket.port = object.value(KEY::SOCKET_PORT).toInt();
    return socket;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QJsonValue
toJson(const Socket& socket)
{
    QJsonObject object;
    object[KEY::SOCKET_IP]   = socket.ip;
    object[KEY::SOCKET_PORT] = socket.port;
    return object;
}

//////////////////////////////////////////////////////////////////////////////////////////
static LRUCache<int>
toLRUIntCache(const QJsonValue& jsonValue, const LRUCache<int>& deflt)
{
    if (jsonValue.isUndefined() || jsonValue.isNull()) return deflt;
    LRUCache<int> lruIntCache(CFG::LRUNetAdressCacheCapacity);
    QJsonArray array = jsonValue.toArray();
    for (int i=0; i<array.count(); i++)
    {   int position = array.count()-i-1;   // Rückwärts
        int value = array.at(position).toInt();
        if (value > 0 && value < 256) lruIntCache.use(value);
    }
    return lruIntCache;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QJsonValue
toJson(const LRUCache<int>& lruIntCache)
{
    QJsonArray array;
    for (int i=0; i<lruIntCache.count(); i++)
    {   array.append(lruIntCache.at(i));
    }
    return array;
}

//////////////////////////////////////////////////////////////////////////////////////////
static bool
fromJson(const QJsonObject& object, Settings& settings)
{
    if (object.value(KEY::FILE_IDENTIFICATION).toString() != VALUE::SETTINGS_FILE_IDENTIFICATION) return false;
    settings.windowPosition                      = toQPoint(object.value(KEY::WINDOW_POSITION),                         defaults().windowPosition);
    settings.windowSize                          = toQSize(object.value(KEY::WINDOW_SIZE),                              defaults().windowSize);
    settings.locale                              = Global::toLocal(object[KEY::LANGUAGE].toString(),                    defaults().locale);
    settings.autoSerialPortDetection             = object.value(KEY::AUTO_SERIAL_PORT_DETECTION).toBool(                defaults().autoSerialPortDetection);
    settings.gatewaySerialPort                   = object.value(KEY::GATEWAY_SERIAL_PORT).toString(                     defaults().gatewaySerialPort);
    settings.gatewaySocket                       = toSocket(object.value(KEY::GATEWAY_SOCKET),                          defaults().gatewaySocket);
    settings.currentGatewayConnection            = toConnectionType(object.value(KEY::CURRENT_GATEWAY_CONNECTION),      defaults().currentGatewayConnection);
    settings.onlineResourcesUrl                  = object.value(KEY::ONLINE_RESOURCES_URL).toString(                    defaults().onlineResourcesUrl);
    settings.localCy4ToolDataDirectoryPath       = object.value(KEY::LOCAL_CY4TOOL_DATA_DIRECTORY_PATH).toString(       defaults().localCy4ToolDataDirectoryPath);
    settings.localServiceMapsDirectoryPath       = object.value(KEY::LOCAL_SERVICE_MAPS_DIRECTORY_PATH).toString(       defaults().localServiceMapsDirectoryPath);
    settings.lruDiscoveryNetAddressCache         = toLRUIntCache(object.value(KEY::LRU_DISCOVERY_NET_ADDRESS_CACHE),    defaults().lruDiscoveryNetAddressCache);
    settings.deviceListImportExportDirectoryPath = object.value(KEY::DEVICE_LIST_IMPORT_EXPORT_DIRECTORY_PATH).toString(defaults().deviceListImportExportDirectoryPath);
    settings.serviceMapsImportExportFilePath     = object.value(KEY::SERVICE_MAPS_IMPORT_EXPORT_FILE_PATH).toString(    defaults().serviceMapsImportExportFilePath);
    settings.locationOutputRelative              = object.value(KEY::LOCATION_OUTPUT_RELATIVE).toBool(                  defaults().locationOutputRelative);
    settings.reloadOnOpenDashboardPage           = object.value(KEY::RELOAD_ON_OPEN_DASHBOARD_PAGE).toBool(             defaults().reloadOnOpenDashboardPage);
    settings.allowLanguageFallback               = object.value(KEY::ALLOW_LANGUAGE_FALLBACK).toBool(                   defaults().allowLanguageFallback);
    settings.timeFormat24h                       = object.value(KEY::TIME_FORMAT_24H).toBool(                           defaults().timeFormat24h);
    settings.dateFormatDDMMYYY                   = object.value(KEY::DATE_FORMAT_DDMMYYYY).toBool(                      defaults().dateFormatDDMMYYY);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
static QJsonObject
toJson(const Settings& settings)
{
    QJsonObject jsonObject;
    jsonObject[KEY::FILE_IDENTIFICATION]                      = VALUE::SETTINGS_FILE_IDENTIFICATION;
    jsonObject[KEY::WINDOW_POSITION]                          = toJson(settings.windowPosition);
    jsonObject[KEY::WINDOW_SIZE]                              = toJson(settings.windowSize);
    jsonObject[KEY::LANGUAGE]                                 = Global::toCode(settings.locale);
    jsonObject[KEY::AUTO_SERIAL_PORT_DETECTION]               = settings.autoSerialPortDetection;
    jsonObject[KEY::GATEWAY_SERIAL_PORT]                      = settings.gatewaySerialPort;
    jsonObject[KEY::GATEWAY_SOCKET]                           = toJson(settings.gatewaySocket);
    jsonObject[KEY::CURRENT_GATEWAY_CONNECTION]               = toJson(settings.currentGatewayConnection);
    jsonObject[KEY::ONLINE_RESOURCES_URL]                     = settings.onlineResourcesUrl;
    jsonObject[KEY::LOCAL_CY4TOOL_DATA_DIRECTORY_PATH]        = settings.localCy4ToolDataDirectoryPath;
    jsonObject[KEY::LOCAL_SERVICE_MAPS_DIRECTORY_PATH]        = settings.localServiceMapsDirectoryPath;
    jsonObject[KEY::LRU_DISCOVERY_NET_ADDRESS_CACHE]          = toJson(settings.lruDiscoveryNetAddressCache);
    jsonObject[KEY::DEVICE_LIST_IMPORT_EXPORT_DIRECTORY_PATH] = settings.deviceListImportExportDirectoryPath;
    jsonObject[KEY::SERVICE_MAPS_IMPORT_EXPORT_FILE_PATH]     = settings.serviceMapsImportExportFilePath;
    jsonObject[KEY::LOCATION_OUTPUT_RELATIVE]                 = settings.locationOutputRelative;
    jsonObject[KEY::RELOAD_ON_OPEN_DASHBOARD_PAGE]            = settings.reloadOnOpenDashboardPage;
    jsonObject[KEY::ALLOW_LANGUAGE_FALLBACK]                  = settings.allowLanguageFallback;
    jsonObject[KEY::TIME_FORMAT_24H]                          = settings.timeFormat24h;
    jsonObject[KEY::DATE_FORMAT_DDMMYYYY]                     = settings.dateFormatDDMMYYY;
    return jsonObject;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Funktionen  //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void
Settings::load(void)
{
    *this = defaults();
    QString filePath = QDir::homePath() + "/.Cy4NET/" + CFG::SettingsFileName;
    if (!Global::pathExists(filePath))
    {   std::cout << "No settings file found: Applying factory defaults" << std::endl;
        return;
    }
    QJsonObject jsonObject;
    if (!Global::load(filePath, jsonObject)) return;
    if (!fromJson(jsonObject, *this))
    {   std::cout << "Settings file ID failure: Applying factory defaults" << std::endl;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void
Settings::save(void) const
{
    QString filePath = QDir::homePath() + "/.Cy4NET/" + CFG::SettingsFileName;
    QJsonObject storedJsonObject;
    Global::load(filePath, storedJsonObject);
    QJsonObject currentJsonObject = toJson(*this);
    if (currentJsonObject != storedJsonObject)
    {   // Nur wenn modifiziert
        Global::save(filePath, currentJsonObject);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Singleton ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Settings&
settings(void)
{
    static Settings instance;
    return instance;
}

//////////////////////////////////////////////////////////////////////////////////////////
const Settings&
defaults(void)
{
    static const Settings instance;
    return instance;
}

//////////////////////////////////////////////////////////////////////////////////////////
