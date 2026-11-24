//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Web-Dateilader                                                                      //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <QEventLoop>
#include <QFile>
#include "Downloader.h"

//////////////////////////////////////////////////////////////////////////////////////////
Downloader::Downloader(void) : QObject(NULL)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
Downloader::~Downloader()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Downloader::get(const QString& url, QString& destinationFilePath, QString& issue)
{
    if (destinationFilePath.isEmpty())
    {   issue = tr("File write failure: Empty file name given");
        return false;
    }
    if( !QUrl(url).isValid() )
    {   issue = tr("Download failed: Invalid URL \"%1\"").arg(url);
        return false;
    }
    QEventLoop  eventLoop;
    connect(&manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
    QNetworkReply* reply = manager.get(QNetworkRequest(url));
    eventLoop.exec();
    QByteArray fileData;
    if (reply->error() == QNetworkReply::NoError)
    {   fileData = reply->readAll();
    }
    else
    {   issue = QString("%1.").arg(reply->errorString());
        return false;
    }
    reply->deleteLater();
    QFile file(destinationFilePath);
    if (!file.open(QIODevice::WriteOnly))
    {   issue = tr("File open failure: \"%1\"").arg(destinationFilePath);
        return false;
    }
    if (file.write(fileData) != fileData.length())
    {   QFile::remove(destinationFilePath);
        issue = tr("File write failure: \"%1\"").arg(destinationFilePath);
        return false;
    }
    issue.clear();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
Downloader&
downloader(void)
{
    static Downloader instance;

    return instance;
}

//////////////////////////////////////////////////////////////////////////////////////////
