//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Cy4Tool Einstellungen                                                               //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QStandardPaths>
#include <QString>
#include <QPoint>
#include <QSize>
#include <QDir>
#include "Conf.h"
#include "LRUCache.h"

#ifdef Q_OS_WIN
    static const char* serialPortName = "COM1";
#else
    static const char* serialPortName = "/dev/ttyS0";
#endif

enum class ConnectionType
{   Socket,
    Serial,
    None
};

struct Socket
{   QString ip   = QString();
    int     port = 0;
    Socket(void) = default;
    Socket(const QString& ipAdr, int portNo) { ip = ipAdr; port = portNo; }
};

//////////////////////////////////////////////////////////////////////////////////////////
class Settings
{
    // Daten /////////////////////////////////////////////////////////////////////////////
    public:
        QPoint          windowPosition                      = QPoint(100, 100);
        QSize           windowSize                          = QSize(400, 400);
        QLocale         locale                              = QLocale(QLocale::English, QLocale::UnitedStates);
        bool            autoSerialPortDetection             = true;
        QString         gatewaySerialPort                   = QString::fromLatin1(serialPortName);
        Socket          gatewaySocket                       = Socket(QStringLiteral("192.0.2.1"), 4001);
        ConnectionType  currentGatewayConnection            = ConnectionType::Serial;
        QString         onlineResourcesUrl                  = QStringLiteral("http://www.cy4net.org");
        QString         localCy4ToolDataDirectoryPath       = QDir::toNativeSeparators(QDir::homePath() + "/.Cy4NET");
        QString         localServiceMapsDirectoryPath       = QDir::toNativeSeparators(localCy4ToolDataDirectoryPath + "/ServiceMaps");
        LRUCache<int>   lruDiscoveryNetAddressCache         = LRUCache<int>(CFG::LRUNetAdressCacheCapacity);
        QString         deviceListImportExportDirectoryPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
        QString         serviceMapsImportExportFilePath     = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
        bool            locationOutputRelative              = false;
        bool            reloadOnOpenDashboardPage           = true;
        bool            allowLanguageFallback               = false;
        bool            timeFormat24h                       = false;
        bool            dateFormatDDMMYYY                   = false;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
                                Settings(void) = default;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        void                    load(void);
        void                    save(void) const;
        friend Settings&        settings(void);
        friend const Settings&  defaults(void);
};

//////////////////////////////////////////////////////////////////////////////////////////
Settings&        settings(void);
const Settings&  defaults(void);



