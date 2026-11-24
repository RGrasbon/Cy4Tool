//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Globale Datenstrukturen                                                             //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QString>
#include <QLineEdit>
#include <QJsonValue>
#include "Cy4Types.h"
#include "Service.h"

//////////////////////////////////////////////////////////////////////////////////////////
namespace Global
{
    void              connectToGateway(void);
    QStringList       translations(void);
    QLocale           toLocal(const QString&, const QLocale& = QLocale(QLocale::English, QLocale::UnitedStates));
    QString           toCode(const QLocale&);
    void              updateLanguage(void);
    bool              load(const QString&, QByteArray&);
    bool              load(const QString&, QByteArray&, QString&);
    bool              load(const QString&, QString&);
    bool              load(const QString&, QString&, QString&);
    bool              load(const QString&, QJsonObject&);
    bool              load(const QString&, QJsonObject&, QString&);
    bool              save(const QString&, const QByteArray&);
    bool              save(const QString&, const QByteArray&, QString&);
    bool              save(const QString&, const QString&);
    bool              save(const QString&, const QString&, QString&);
    bool              save(const QString&, const QJsonObject&);
    bool              save(const QString&, const QJsonObject&, QString&);
    bool              removeFile(const QString&);
    bool              copyFile(const QString&, const QString&);
    bool              pathExists(const QString&);
    QStringList       directoryContent(const QString&, const QString& = QString());
    int               evaluate(const QString&);
    QString           jsonLanguageEntry(const QJsonValue&, bool = false);
    void              doMainLoop(void);
    QString           mapFileNameFor(const Cy4ApplicationType&);
    QString           mapFilePathFor(const Cy4ApplicationType&);
    QString           mapFileUrlFor(const Cy4ApplicationType&);
    bool              openFileInEditor(const QString&, QString*);
    QString           print(Service::Access);
    const Cy4Device*  currentDevice(void);
    const Service*    currentService(void);
    int               calculateWidthFor(const QString&, const QLineEdit*);
    QString           createMeasurementTextFor(double, int);
}

//////////////////////////////////////////////////////////////////////////////////////////



