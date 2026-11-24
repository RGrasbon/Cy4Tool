//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Anwendungs-Dienstekarte                                                             //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QObject>
#include <QList>
#include <QJsonArray>
#include "Service.h"

//////////////////////////////////////////////////////////////////////////////////////////
class ServiceMap
{
    // Typen /////////////////////////////////////////////////////////////////////////////
    public:
        enum ItemTag    // Kennzeichnung eines Listeneintrages:
        {   Valid,      // · Eintrag der Dienstekarte ist gültig
            Invalid,    // · Der zum Eintrag gehörende Dienst ist ungültig
            PgmLdrArea, // · Dienst ist gültig, liegt aber im Dienstbereich der Programmlader
            Overlap,    // · Zwei Dienste überlappen sich
            Adjacent,   // · Dienst liegt angrenzend zum Nachbardienst
            Mandatory,  // · Kollidierende Dienste im Bereich der Pflichtdienste
            Unknown     // · Unbekannt
        };

    private:
        struct Item
        {   Service  service = Service();
            ItemTag  tag     = Unknown;
            bool     operator<(const Item& other) const { return service < other.service; };
            bool     operator>(const Item& other) const { return service > other.service; };
        };

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Cy4Device    mDevice;
        QList<Item>  mList;
        QJsonArray   mJsonDashboard;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        ServiceMap(void);
       ~ServiceMap();

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        void               clear(void);
        int                size(void) const;
        bool               isValid(void) const;
        bool               isEmpty(void) const;
        const Service&     at(int) const;
        Service&           at(int);
        int                indexOf(Cy4Location, int, Service::Access) const;
        ItemTag            tag(int) const;
        bool               load(const Cy4Device&, QString&);
        bool               import(const Cy4Device&, const QString&, QString&);
        const QJsonArray&  dashboard(void) const;
        const Cy4Device&   device(void) const;
    friend ServiceMap&     serviceMap(void);
};

//////////////////////////////////////////////////////////////////////////////////////////
ServiceMap&  serviceMap(void);

//////////////////////////////////////////////////////////////////////////////////////////
