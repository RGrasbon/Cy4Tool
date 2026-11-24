//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Cy4Tool Anwendung                                                                   //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <QApplication>
#include <QStyleHints>
#include <QDir>
#include <QLockFile>
#include <QMessageBox>
#include "Version.h"
#include "MainFrame.h"
#include "Settings.h"
#include "DeviceList.h"

//////////////////////////////////////////////////////////////////////////////////////////
int
main(int argc, char* argv[])
{
    std::cout << std::endl
         << "--------------------------------------------------------" << std::endl
         << " " << APPL_NAME << std::endl
         << " " << VERSION_STRING << std::endl
         << " Version " << VERSION_STRING << std::endl
         << " Build: " << __DATE__ << " : " << __TIME__ << std::endl
         << std::endl
         << " Copyright (c) 2018 - 2026 Reimar Grasbon" << std::endl
         << "--------------------------------------------------------" << std::endl
         << std::endl;
    QApplication application(argc, argv);
    application.styleHints()->setColorScheme(Qt::ColorScheme::Light);
    QLockFile lockFile(QDir::toNativeSeparators(QDir::tempPath() + "/Cy4Tool.lock"));
    if (!lockFile.tryLock(100))
    {   QMessageBox::warning(NULL, APPL_NAME,
                     QObject::tr("Another instance of Cy4Tool application is already "
                                 "running!"));
        exit(0);
    }
    QDir directoryPath = QDir::toNativeSeparators(QDir::homePath() + "/.Cy4NET");
    if (!directoryPath.exists()) directoryPath.mkpath(".");
    settings().load();
    directoryPath.setPath(settings().localCy4ToolDataDirectoryPath);
    if (!directoryPath.exists())
    {   settings().localCy4ToolDataDirectoryPath = defaults().localCy4ToolDataDirectoryPath;
    }
    directoryPath.setPath(settings().localServiceMapsDirectoryPath);
    if (!directoryPath.exists())
    {   settings().localServiceMapsDirectoryPath = defaults().localServiceMapsDirectoryPath;
    }
    deviceList().load();
    mainFrame().show();
    application.exec();
    deviceList().save();
    settings().save();
    return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////

