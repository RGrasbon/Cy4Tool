//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Web-Dateilader                                                                      //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>

//////////////////////////////////////////////////////////////////////////////////////////
class Downloader : public QObject
{
    Q_OBJECT

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        QNetworkAccessManager  manager;

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        Downloader(void);
       ~Downloader();

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        bool                get(const QString&, QString&, QString&);
        friend Downloader&  downloader(void);
};

//////////////////////////////////////////////////////////////////////////////////////////
extern Downloader&          downloader(void);
