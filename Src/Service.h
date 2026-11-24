//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Dienst                                                                              //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <QString>
#include <QJsonObject>
#include "Cy4Types.h"

//////////////////////////////////////////////////////////////////////////////////////////
class Service
{
    // Typen /////////////////////////////////////////////////////////////////////////////
    public:
        enum Access
        {   ReadOnly,
            WriteOnly,
            ReadWrite,
            Unknown
        };

        enum CacheStatus // Zustand der Cache-Daten:
        {   Touched,     // · Cache-Daten wurden beschrieben.
            UptoDate,    // · Cache-Daten stimmen mit den Gerätedaten überein.
            Failed       // · Datenabgleich ist fehlgeschlagen.
        };

    private:
        struct CacheItem
        {   UInt08      data   = 0xFF;
            CacheStatus status = Touched;
        };

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        QString                mName     = QString();
        Cy4Location            mLocation = Cy4Location();
        std::vector<CacheItem> mCache    = std::vector<CacheItem>();
        Access                 mAccess   = Unknown;
        QString                mBrief    = QString();
        QString                mIssue    = QString();

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                               Service(void) = default;
                               Service(const QString&, Cy4Location, int, Access);
                               Service(const QJsonObject&);
        bool                   operator<(const Service&) const;
        bool                   operator>(const Service&) const;
        const QString&         name(void) const;
        bool                   isValid(void) const;
        const QString&         issue(void) const;
        Cy4Location            location(void) const;
        int                    size(void) const;
        Service::Access        access(void) const;
        void                   setBrief(const QString&);
        const QString&         brief(void) const;
        void                   setDataAt(int, UInt08);
        UInt08                 dataAt(int) const;
        CacheStatus            cacheStatusAt(int) const;
        std::list<Cy4Message>  toRequests(Cy4Address, Cy4Message::Access) const;
        std::list<Cy4Message>  toRequests(Cy4Address, Cy4Location, int, Cy4Message::Access) const;
        bool                   fromResponse(const Cy4Message&);
};

//////////////////////////////////////////////////////////////////////////////////////////
