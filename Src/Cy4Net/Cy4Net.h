//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Cy4NET Schnittstelle                                                                //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <QString>
#include <QTimer>
#include "Cy4Types.h"

class QIODevice;

//////////////////////////////////////////////////////////////////////////////////////////
class Cy4Net : public QObject
{
    Q_OBJECT

    // Typen /////////////////////////////////////////////////////////////////////////////
    public:
        typedef void  (*ResponseHandler)(const Cy4Message&, void*);

    private:
        struct  Posting
        {   Cy4Message      message  = Cy4Message();
            ResponseHandler callback = nullptr;
            void*           context  = nullptr;
        };

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        QIODevice*          mIoDevice;
        Cy4Address          mGatewayAddress;
        QTimer              mResponseTimer;
        QTimer              mPauseTimer;
        std::list<Posting>  mPostingQueue;
        bool                mIsTransceiving;

    // Cy4-Antwortbehandler //////////////////////////////////////////////////////////////
    private:
        static void  exitTransceiveEventLoopHandle(const Cy4Message&, void*);

    // Signale ///////////////////////////////////////////////////////////////////////////
    Q_SIGNALS:
        void  activity(void);
        void  transceiveCompleted(const Cy4Message&);

    // Interna ///////////////////////////////////////////////////////////////////////////
    private:
        void      onTextReceived(void);
        void      sendNextPosting(void);
                  Cy4Net(QObject* = nullptr);
        virtual  ~Cy4Net() override;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
        bool            open(const QString&);
        bool            open(const QString&, int);
        void            close(void);
        bool            isOpen(void) const;
        bool            callGateway(void);
        Cy4Address      gateway(void) const;
        Cy4Message      transceive(const Cy4Message&);
        void            post(const Cy4Message&, ResponseHandler = nullptr, void* = nullptr);
        Cy4TypePlate    fetchTypePlate(Cy4Address);
        void            fetchTypePlate(Cy4Address, ResponseHandler, void* = nullptr);
        bool            enterApplication(Cy4Address);
        void            enterApplication(Cy4Address, ResponseHandler, void* = nullptr);
        bool            enterProgramLoader(Cy4Address);
        void            enterProgramLoader(Cy4Address, ResponseHandler, void* = nullptr);
        bool            changeCy4Address(Cy4Address, Cy4Address);
        void            changeCy4Address(Cy4Address, Cy4Address, ResponseHandler, void* = nullptr);
        QString         queryLabel(Cy4Address);
        void            queryLabel(Cy4Address, ResponseHandler, void* = nullptr);
        void            postPause(void);
        bool            isBusy(void) const;
        void            dropPostings(void);
        friend Cy4Net&  cy4Net(void);
};

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Net&  cy4Net(void);
