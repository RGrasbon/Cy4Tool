//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  JSON-Schlüssel                                        								//
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////
namespace KEY
{   constexpr char ACCESS[]                                   = "access";
    constexpr char ADDRESS[]                                  = "address";
    constexpr char ALIGN[]                                    = "align";
    constexpr char ALLOW_LANGUAGE_FALLBACK[]                  = "allow-language-fallback";
    constexpr char APPENDIX[]                                 = "appendix";
    constexpr char APPLICATION_BRIEF[]                        = "application-brief";
    constexpr char APPLICATION_LABEL[]                        = "application-label";
    constexpr char APPLICATION_TYPE[]                         = "application-type";
    constexpr char APPLICATION_VERSION[]                      = "application-version";
    constexpr char ASSIGNMENTS[]                              = "assignments";
    constexpr char AUTO_SERIAL_PORT_DETECTION[]               = "auto-serial-port-detection";
    constexpr char BOARD_TYPE[]                               = "board-type";
    constexpr char BRIEF[]                                    = "brief";
    constexpr char CAPTION[]                                  = "caption";
    constexpr char COLUMNS[]                                  = "columns";
    constexpr char CONFIGURATION_FILE[]                       = "configuration-file";
    constexpr char CONFIGURATION_IMPORT_EXPORT_FILE_PATH[]    = "configuration-import-export-file-path";
    constexpr char CONFIRM[]                                  = "confirm";
    constexpr char CONNECTION_TYPE_SERIAL[]                   = "serial";
    constexpr char CONNECTION_TYPE_SOCKET[]                   = "socket";
    constexpr char CREATION_DATE[]                            = "created";
    constexpr char CURRENT_GATEWAY_CONNECTION[]               = "current-gateway-connection";
    constexpr char DATE[]                                     = "date";
    constexpr char DATE_FORMAT_DDMMYYYY[]                     = "date-format-ddmmyyyy";
    constexpr char DEVICES[]                                  = "devices";
    constexpr char DEVICE_LIST_IMPORT_EXPORT_DIRECTORY_PATH[] = "device-list-import-export-directory-path";
    constexpr char DOMAIN[]                                   = "domain";
    constexpr char ENUMERATION[]                              = "enumeration";
    constexpr char FILE_IDENTIFICATION[]                      = "#";
    constexpr char FIRMWARE_FILE[]                            = "firmware-file";
    constexpr char FORMAT[]                                   = "format";
    constexpr char GATEWAY_SERIAL_PORT[]                      = "gateway-serial-port";
    constexpr char GATEWAY_SOCKET[]                           = "gateway-socket";
    constexpr char GRID[]                                     = "grid";
    constexpr char GROUP[]                                    = "group";
    constexpr char ITEMS[]                                    = "items";
    constexpr char LABEL[]                                    = "label";
    constexpr char LANGUAGE[]                                 = "language";
    constexpr char LINE[]                                     = "line";
    constexpr char LINE_UP[]                                  = "line-up";
    constexpr char LINK[]                                     = "link";
    constexpr char LOCAL_CY4TOOL_DATA_DIRECTORY_PATH[]        = "local-cy4tool-data-directory-path";
    constexpr char LOCAL_SERVICE_MAPS_DIRECTORY_PATH[]        = "local-service-maps-directory-path";
    constexpr char LOCATION[]                                 = "location";
    constexpr char LOCATION_OUTPUT_RELATIVE[]                 = "location-output-relative";
    constexpr char LRU_DISCOVERY_NET_ADDRESS_CACHE[]          = "lru-discovery-net-address-cache";
    constexpr char MANDATORY_SERVICES[]                       = "mandatory-services";
    constexpr char MAX[]                                      = "max";
    constexpr char MESSAGES[]                                 = "messages";
    constexpr char MIN[]                                      = "min";
    constexpr char NAME[]                                     = "name";
    constexpr char NEW_FLAG[]                                 = "is-new";
    constexpr char NUMBER[]                                   = "number";
    constexpr char OFF_VALUE[]                                = "off-value";
    constexpr char ONLINE_FLAG[]                              = "is-online";
    constexpr char ONLINE_RESOURCES_URL[]                     = "online-resources-url";
    constexpr char ON_VALUE[]                                 = "on-value";
    constexpr char ORIENTATION[]                              = "orientation";
    constexpr char PAYLOAD[]                                  = "payload";
    constexpr char POINT_X[]                                  = "point-x";
    constexpr char POINT_Y[]                                  = "point-y";
    constexpr char PRECISION[]                                = "precision";
    constexpr char READ_ONLY[]                                = "read-only";
    constexpr char READ_WRITE[]                               = "read-write";
    constexpr char RELOAD_ON_OPEN_DASHBOARD_PAGE[]            = "reload-on-open-dashboard-page";
    constexpr char SECONDS_ENABLED[]                          = "seconds-enabled";
    constexpr char SEPARATOR[]                                = "separator";
    constexpr char SERVICE[]                                  = "service";
    constexpr char SERVICE_MAPS_IMPORT_EXPORT_FILE_PATH[]     = "service-maps-import-export-file-path";
    constexpr char SIZE[]                                     = "size";
    constexpr char SIZE_HEIGHT[]                              = "size-height";
    constexpr char SIZE_WIDTH[]                               = "size-width";
    constexpr char SOCKET_IP[]                                = "ip";
    constexpr char SOCKET_PORT[]                              = "port";
    constexpr char SPECIALS[]                                 = "specials";
    constexpr char TABLE[]                                    = "table";
    constexpr char TEXT[]                                     = "text";
    constexpr char TIME[]                                     = "time";
    constexpr char TIME_FORMAT_24H[]                          = "time-format-24h";
    constexpr char TOGGLE[]                                   = "toggle";
    constexpr char TRIGGER[]                                  = "trigger";
    constexpr char TRIGGER_VALUE[]                            = "trigger-value";
    constexpr char TYPE[]                                     = "type";
    constexpr char WIDGET[]                                   = "widget";
    constexpr char WIDTH[]                                    = "width";
    constexpr char WINDOW_POSITION[]                          = "window-position";
    constexpr char WINDOW_SIZE[]                              = "window-size";
    constexpr char WRITE_ONLY[]                               = "write-only";
};

namespace VALUE
{   constexpr char CENTER[]                            = "center";
    constexpr char CONFIGURATION_FILE_IDENTIFICATION[] = "Cy4NET Application Data";
    constexpr char DEVICE_LIST_FILE_IDENTIFICATION[]   = "Cy4Tool Local Device List";
    constexpr char FIXPOINT1[]                         = "fixpoint1";
    constexpr char FIXPOINT2[]                         = "fixpoint2";
    constexpr char FIXPOINT3[]                         = "fixpoint3";
    constexpr char FIXPOINT4[]                         = "fixpoint4";
    constexpr char FIXPOINT5[]                         = "fixpoint5";
    constexpr char FIXPOINT6[]                         = "fixpoint6";
    constexpr char FLOAT[]                             = "float";
    constexpr char HORIZONTAL[]                        = "horizontal";
    constexpr char INTEGER[]                           = "integer";
    constexpr char LEFT[]                              = "left";
    constexpr char RIGHT[]                             = "right";
    constexpr char SERVICE_MAP_FILE_IDENTIFICATION[]   = "Cy4Tool Application Service Map";
    constexpr char SETTINGS_FILE_IDENTIFICATION[]      = APPL_NAME" settings";
    constexpr char SIGNED[]                            = "signed";
    constexpr char UNSIGNED[]                          = "unsigned";
    constexpr char VERTICAL[]                          = "vertical";
};

namespace DEPRECATED    // Veraltet seit V2.00
{   constexpr char APPLICATION_LABEL[]                 = "appl-label";
    constexpr char APPLICATION_TYPE[]                  = "appl-type";
    constexpr char APPLICATION_VERSION[]               = "appl-vers";
    constexpr char CONFIGURATION_FILE_IDENTIFICATION[] = "Cy4NET application data";
    constexpr char SEPARATOR[]                         = "separator";
    constexpr char TITLE[]                             = "title";
};

//////////////////////////////////////////////////////////////////////////////////////////

