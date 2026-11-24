//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Cy4-Geräteliste                                                                     //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <QList>
#include <QByteArray>
#include <QTimer>
#include <QJsonObject>
#include <QItemSelection>
#include "Cy4Types.h"

//////////////////////////////////////////////////////////////////////////////////////////
class DeviceList : public QAbstractTableModel
{
    Q_OBJECT

    // Typen /////////////////////////////////////////////////////////////////////////////
    private:
        struct Item
        {   Cy4Device  device            = Cy4Device();
            bool       isOnline          = false;
            bool       isNew             = false;
            bool       isBusy            = false;
            QString    label             = QString();
            QString    firmwareFile      = QString();
            QString    configurationFile = QString();
            bool       operator<(const Item& other) const
                       {   return device.address() <  other.device.address();
                       }
            bool       operator>(const Item& other) const
                       {   return device.address() >  other.device.address();
                       }
        };

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        QList<Item>  mList;
        int          mGatewayIndex = INVALID;

    // Signale ///////////////////////////////////////////////////////////////////////////
    signals:
        void  deviceInserted(int);
        void  deviceChanged(int);
        void  deviceRemoved(Cy4Address);

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void           fromJson(const QJsonObject&, QList<Item>&);
        QJsonObject    toJson(const QList<Item>&) const;
        int            rowCount(const QModelIndex&) const override;
        int            columnCount(const QModelIndex&) const override;
        QVariant       headerData(int, Qt::Orientation, int = Qt::DisplayRole ) const override;
        Qt::ItemFlags  flags(const QModelIndex&) const override;
        QVariant       data(const QModelIndex&, int = Qt::DisplayRole) const override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        void                add(Cy4Address);
        void                fetch(int);
        void                remove(int);
        bool                isEmpty(void) const;
        int                 size(void) const;
        bool                contains(Cy4Address) const;
        int                 indexOf(Cy4Address) const;
        const Cy4Device&    at(int) const;
        const QString&      label(int) const;
        bool                isOnline(int) const;
        bool                isNew(int) const;
        void                setNewFlag(int);
        void                clearNewFlag(int);
        void                setBusyFlag(int);
        void                clearBusyFlag(int);
        const QString&      firmwareFilePath(int) const;
        void                setFirmwareFilePath(int, const QString&);
        const QString&      configurationFilePath(int) const;
        void                setConfigurationFilePath(int, const QString&);        
        QModelIndex         modelIndexOf(int) const;
        void                load(void);
        bool                load(const QString&, QString&);
        void                save(void) const;
        bool                saveAs(const QString&, QString&) const;
        friend DeviceList&  deviceList(void);
};

//////////////////////////////////////////////////////////////////////////////////////////
DeviceList&  deviceList(void);
