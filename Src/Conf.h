//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Konfiguration                                                                       //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QtGlobal>

//////////////////////////////////////////////////////////////////////////////////////////
namespace CFG
{
    constexpr int   StatusBarMessageTime       = 10;    // [s]
    constexpr int   StatusBarProgressTime      = 2;     // [s]
    constexpr char  ToolDataDirectoryName[]    = "Cy4ToolData";
    constexpr char  ServiceMapsDirectoryName[] = "ServiceMaps";
    constexpr char  SettingsFileName[]         = "Cy4ToolSettings.json";
    constexpr char  DeviceListFileName[]       = "DeviceList.json";
    constexpr char  ConfigurationFileName[]    = "Configuration.conf";
    constexpr int   LRUNetAdressCacheCapacity  = 5;     // [#]
    constexpr int   MaximumServiceSize         = 16000000;
    constexpr int   QLineEditHorizontalPading  = 10;
    constexpr char  BackgroundForReadOnly[]    = "background: rgb(245, 245, 245);";
}

//////////////////////////////////////////////////////////////////////////////////////////
