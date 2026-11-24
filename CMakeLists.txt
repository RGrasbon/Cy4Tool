##################################################################

set(APPL_NAME     "Cy4Tool")
set(APPL_PRODUCT  "Cy4NET Components Administrataion and Maintenance Tool")

##################################################################

cmake_minimum_required(VERSION 3.19)

project(Cy4Tool
        VERSION 2.0.2
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets SerialPort Network LinguistTools)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)

configure_file(
    "Res/Version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Version.h"
)

set(HEADERS_DIRECTORY Src)
set(HEADERS
    ${HEADERS_DIRECTORY}/Cy4Net/Cy4Firmware.h
    ${HEADERS_DIRECTORY}/Cy4Net/Cy4Net.h
    ${HEADERS_DIRECTORY}/Cy4Net/Cy4Types.h
    ${HEADERS_DIRECTORY}/GUI/Dialogs/CalenderPopup.h
    ${HEADERS_DIRECTORY}/GUI/Dialogs/ClockPopup.h
    ${HEADERS_DIRECTORY}/GUI/Pages/AbstractPage.h
    ${HEADERS_DIRECTORY}/GUI/Pages/DashboardPage.h
    ${HEADERS_DIRECTORY}/GUI/Pages/DeviceListPage.h
    ${HEADERS_DIRECTORY}/GUI/Pages/FirmwarePage.h
    ${HEADERS_DIRECTORY}/GUI/Pages/CustomAccessPage.h
    ${HEADERS_DIRECTORY}/GUI/Pages/ServiceListPage.h
    ${HEADERS_DIRECTORY}/GUI/Pages/SettingsPage.h
    ${HEADERS_DIRECTORY}/GUI/Views/AddDeviceView.h
    ${HEADERS_DIRECTORY}/GUI/Views/ChangeAddressView.h
    ${HEADERS_DIRECTORY}/GUI/Views/DiscoverDevicesView.h
    ${HEADERS_DIRECTORY}/GUI/Views/ServiceDetailsView.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/AbstractServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/AddressServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/DateServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/DummyServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/EnumerationServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/LinkServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/LocationServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/NumberServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/TableServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/TextServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/TimeServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/ToggleServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/Dashboard/TriggerServiceWidget.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/QHexEdit/chunks.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/QHexEdit/color_manager.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/QHexEdit/commands.h
    ${HEADERS_DIRECTORY}/GUI/Widgets/QHexEdit/qhexedit.h
    ${HEADERS_DIRECTORY}/GUI/MainFrame.h
    ${HEADERS_DIRECTORY}/Service.h
    ${HEADERS_DIRECTORY}/ServiceMap.h
    ${HEADERS_DIRECTORY}/ServiceProxy.h
    ${HEADERS_DIRECTORY}/Utilities/Downloader.h
    ${HEADERS_DIRECTORY}/Utilities/LRUCache.h
    ${HEADERS_DIRECTORY}/Utilities/NumberValidator.h
    ${HEADERS_DIRECTORY}/Conf.h
    ${HEADERS_DIRECTORY}/DeviceList.h
    ${HEADERS_DIRECTORY}/Global.h
    ${HEADERS_DIRECTORY}/KeyValues.h
    ${HEADERS_DIRECTORY}/Settings.h
    ${HEADERS_DIRECTORY}/TypeDefs.h
)

set(SOURCES_DIRECTORY Src)
set(SOURCES
    ${SOURCES_DIRECTORY}/Cy4Net/Cy4Firmware.cpp
    ${SOURCES_DIRECTORY}/Cy4Net/Cy4Net.cpp
    ${SOURCES_DIRECTORY}/Cy4Net/Cy4Types.cpp
    ${SOURCES_DIRECTORY}/GUI/Dialogs/CalenderPopup.cpp
    ${SOURCES_DIRECTORY}/GUI/Dialogs/ClockPopup.cpp
    ${SOURCES_DIRECTORY}/GUI/Pages/AbstractPage.cpp
    ${SOURCES_DIRECTORY}/GUI/Pages/DashboardPage.cpp
    ${SOURCES_DIRECTORY}/GUI/Pages/DeviceListPage.cpp
    ${SOURCES_DIRECTORY}/GUI/Pages/FirmwarePage.cpp
    ${SOURCES_DIRECTORY}/GUI/Pages/CustomAccessPage.cpp
    ${SOURCES_DIRECTORY}/GUI/Pages/ServiceListPage.cpp
    ${SOURCES_DIRECTORY}/GUI/Pages/SettingsPage.cpp
    ${SOURCES_DIRECTORY}/GUI/Views/AddDeviceView.cpp
    ${SOURCES_DIRECTORY}/GUI/Views/ChangeAddressView.cpp
    ${SOURCES_DIRECTORY}/GUI/Views/DiscoverDevicesView.cpp
    ${SOURCES_DIRECTORY}/GUI/Views/ServiceDetailsView.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/AbstractServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/AddressServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/DateServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/DummyServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/EnumerationServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/LinkServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/LocationServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/NumberServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/TableServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/TextServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/TimeServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/ToggleServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard/TriggerServiceWidget.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/QHexEdit/chunks.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/QHexEdit/color_manager.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/QHexEdit/commands.cpp
    ${SOURCES_DIRECTORY}/GUI/Widgets/QHexEdit/qhexedit.cpp
    ${SOURCES_DIRECTORY}/GUI/MainFrame.cpp
    ${SOURCES_DIRECTORY}/Service.cpp
    ${SOURCES_DIRECTORY}/ServiceMap.cpp
    ${SOURCES_DIRECTORY}/ServiceProxy.cpp
    ${SOURCES_DIRECTORY}/Utilities/Downloader.cpp
    ${SOURCES_DIRECTORY}/Utilities/NumberValidator.cpp
    ${SOURCES_DIRECTORY}/DeviceList.cpp
    ${SOURCES_DIRECTORY}/Global.cpp
    ${SOURCES_DIRECTORY}/Main.cpp
    ${SOURCES_DIRECTORY}/Settings.cpp
)

set(GUI_DIRECTORY GUI)
set(GUI_FORMS
    ${GUI_DIRECTORY}/MainFrame.ui
    ${GUI_DIRECTORY}/Dialogs/CalenderPopup.ui
    ${GUI_DIRECTORY}/Dialogs/ClockPopup.ui
    ${GUI_DIRECTORY}/Pages/DashboardPage.ui
    ${GUI_DIRECTORY}/Pages/DeviceListPage.ui
    ${GUI_DIRECTORY}/Pages/FirmwarePage.ui
    ${GUI_DIRECTORY}/Pages/CustomAccessPage.ui
    ${GUI_DIRECTORY}/Pages/ServiceListPage.ui
    ${GUI_DIRECTORY}/Pages/SettingsPage.ui
    ${GUI_DIRECTORY}/Views/AddDeviceView.ui
    ${GUI_DIRECTORY}/Views/ChangeAddressView.ui
    ${GUI_DIRECTORY}/Views/DiscoverDevicesView.ui
    ${GUI_DIRECTORY}/Views/ServiceDetailsView.ui
)
qt_wrap_ui(UI_HEADERS ${GUI_FORMS})

set(RESOURCES_DIRECTORY Res)
set(RESOURCES
    ${RESOURCES_DIRECTORY}/Resources.qrc
)

qt_add_executable(${PROJECT_NAME}
    WIN32
    ${HEADERS}
    ${SOURCES}
    ${UI_HEADERS}
    ${RESOURCES}
    ${RESOURCES_DIRECTORY}/ApplIcon/ApplIcon.rc
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
target_include_directories(${PROJECT_NAME} PRIVATE
#    ${CMAKE_CURRENT_BINARY_DIR}
    ${HEADERS_DIRECTORY}
    ${HEADERS_DIRECTORY}/GUI
    ${HEADERS_DIRECTORY}/GUI/Dialogs
    ${HEADERS_DIRECTORY}/GUI/Pages
    ${HEADERS_DIRECTORY}/GUI/Views
    ${HEADERS_DIRECTORY}/GUI/Widgets
    ${HEADERS_DIRECTORY}/GUI/Widgets/QHexEdit
    ${SOURCES_DIRECTORY}/GUI/Widgets/Dashboard
    ${HEADERS_DIRECTORY}/Cy4Net
    ${HEADERS_DIRECTORY}/Utilities
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::SerialPort
    Qt::Network
)

target_compile_definitions(${PROJECT_NAME} PRIVATE APPL_NAME="${APPL_NAME}"
                                                   APPL_PRODUCT="${APPL_PRODUCT}"
                                                   __STRICT_ANSI__
                                                   #CY4_MESSAGE_MONITOR
                                                   #CY4_MESSAGE_SHORT_PAYLOAD
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra -Wunused-function>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ${PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

source_group("Resources" FILES ${RESOURCES})
source_group("UI Files" FILES ${UIS})

