//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Dienst-Stellvertreter                                                               //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QString>
#include <QJsonObject>
#include "Service.h"

//////////////////////////////////////////////////////////////////////////////////////////
class ServiceProxy
{
    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Cy4Location      mLocation = Cy4Location();
        int              mSize     = 0;
        Service::Access  mAccess   = Service::Unknown;
        QString          mBrief    = QString();

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                            ServiceProxy(void) = default;
                            ServiceProxy(const QJsonObject&);       
        QString             name(void) const;
        Cy4Location         location(void) const;
        int                 size(void) const;
        Service::Access     access(void) const;
        ByteArray           read(void) const;
        void                write(int, const ByteArray&) const;
        bool                accessFailed(void) const;
        bool                isUpToDate(void) const;
        void                setBrief(const QString&);        
        const QString&      brief(void) const;

};

//////////////////////////////////////////////////////////////////////////////////////////
