//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Cy4NET Typen                                                                        //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include "TypeDefs.h"

static const int CY4_PAYLOAD_CAPACITY = 64;

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Adresse  //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class Cy4Address
{
    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        int  mNet = INVALID;
        int  mDev = INVALID;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                     Cy4Address(void) = default;
                     Cy4Address(int, int);
        explicit     Cy4Address(const char*);
        explicit     Cy4Address(const std::string&);
        bool         operator==(Cy4Address) const;
        bool         operator!=(Cy4Address) const;
        bool         operator>(Cy4Address) const;
        bool         operator>=(Cy4Address) const;
        bool         operator<(Cy4Address) const;
        bool         operator<=(Cy4Address) const;
        bool         isValid(void) const;
        bool         isBroadcast(void) const;
        int          net(void) const;
        int          dev(void) const;
        std::string  toText(void) const;
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Dienstort  ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class Cy4Location
{
    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        int  mLocation = INVALID;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                          Cy4Location(void) = default;
        explicit          Cy4Location(int);
        explicit          Cy4Location(const char*);
        explicit          Cy4Location(const std::string&);
            bool          operator==(Cy4Location) const;
            bool          operator==(int) const;
            bool          operator!=(Cy4Location) const;
            bool          operator!=(int) const;
            bool          operator> (Cy4Location) const;
            bool          operator> (int) const;
            bool          operator>=(Cy4Location) const;
            bool          operator>=(int) const;
            bool          operator< (Cy4Location) const;
            bool          operator< (int) const;
            bool          operator<=(Cy4Location) const;
            bool          operator<=(int) const;
            Cy4Location   operator+ (Cy4Location) const;
            Cy4Location   operator+ (int) const;
    friend  Cy4Location   operator+ (int, Cy4Location);
            Cy4Location   operator- (Cy4Location) const;
            Cy4Location   operator- (int) const;
    friend  Cy4Location   operator- (int, Cy4Location);
            Cy4Location&  operator+=(Cy4Location);
            Cy4Location&  operator+=(int);
            Cy4Location&  operator-=(Cy4Location);
            Cy4Location&  operator-=(int);
            bool          isValid(void) const;
            int           toInteger(void) const;
            std::string   toText(void) const;
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Nachricht  ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class Cy4Message
{
    // Typen /////////////////////////////////////////////////////////////////////////////
    public:
        enum Access
        {   Read,
            Write
        };

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        bool         mValid    = false;
        Access       mAccess   = Read;
        Cy4Address   mAddress  = Cy4Address();
        Cy4Location  mLocation = Cy4Location();
        ByteArray    mData     = ByteArray();

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                          Cy4Message(void) = default;
                          Cy4Message(Cy4Address, Cy4Location, int);
                          Cy4Message(Cy4Address, Cy4Location, const ByteArray&);
        bool              isValid(void) const;
        bool              isBroadcast(void) const;
        void              invalidate(void);
        Access            access(void) const;
        Cy4Address        address(void) const;
        Cy4Location       location(void) const;
        int               count(void) const;
        const ByteArray&  data(void) const;
        UInt08            operator[](int) const;
        UInt08&           operator[](int);
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Typennummer  //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class Cy4TypeNumber
{
    // Daten /////////////////////////////////////////////////////////////////////////////
    protected:
        int mNumber = INVALID;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    protected:
                                Cy4TypeNumber(void) = default;
                                Cy4TypeNumber(int);
                                Cy4TypeNumber(const char*);
                                Cy4TypeNumber(const Cy4TypeNumber&);
        virtual                ~Cy4TypeNumber() = default;

    public:
        virtual Cy4TypeNumber&  operator=(const Cy4TypeNumber&);
        bool                    operator==(const Cy4TypeNumber&) const;
        bool                    operator!=(const Cy4TypeNumber&) const;
        bool                    isValid(void) const;
        virtual bool            isPrivate(void) const;
        int                     toInteger(void) const;
        virtual std::string     toText(void) const = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Anwendungstypennummer  ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class Cy4ApplicationType : public Cy4TypeNumber
{
    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                      Cy4ApplicationType(void) = default;
        explicit      Cy4ApplicationType(int);
        explicit      Cy4ApplicationType(const char*);
        explicit      Cy4ApplicationType(const std::string&);
        virtual bool  isProgramLoader(void) const;
        std::string   toText(void) const override;
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Boardtypennummer  /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class Cy4BoardType : public Cy4TypeNumber
{
    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                     Cy4BoardType(void) = default;
        explicit     Cy4BoardType(int);
        explicit     Cy4BoardType(const char*);
        explicit     Cy4BoardType(const std::string&);
        std::string  toText(void) const override;
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Version  //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class Cy4Version
{
    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        int mVersion = INVALID;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                     Cy4Version(void) = default;
                     Cy4Version(int, int);
        explicit     Cy4Version(const char*);
        explicit     Cy4Version(const std::string&);
        bool         operator==(Cy4Version) const;
        bool         operator!=(Cy4Version) const;
        bool         operator< (Cy4Version) const;
        bool         operator<=(Cy4Version) const;
        bool         operator> (Cy4Version) const;
        bool         operator>=(Cy4Version) const;
        bool         isValid(void) const;
        int          toInteger(void) const;
        std::string  toText(void) const;
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Typenschild  //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class Cy4TypePlate
{
    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        Cy4ApplicationType  mApplicationType;
        Cy4Version          mApplicationVersion;
        Cy4BoardType        mBoardType;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                            Cy4TypePlate(void) = default;
                            Cy4TypePlate(const Cy4ApplicationType&, Cy4Version, const Cy4BoardType&);
        explicit            Cy4TypePlate(const Cy4Message&);
        explicit            Cy4TypePlate(const UInt08*);
        explicit            Cy4TypePlate(const ByteArray&);
        bool                operator==(const Cy4TypePlate&) const;
        bool                operator!=(const Cy4TypePlate&) const;
        bool                isValid(void) const;
        bool                isProgramLoader(void) const;
        Cy4ApplicationType  applicationType(void) const;
        Cy4Version          applicationVersion(void) const;
        Cy4BoardType        boardType(void) const;
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Gerät  ////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
class Cy4Device
{
    // Daten /////////////////////////////////////////////////////////////////////////////
    protected:
        Cy4Address    mAddress   = Cy4Address();
        Cy4TypePlate  mTypePlate = Cy4TypePlate();

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                             Cy4Device(void) = default;
                             Cy4Device(Cy4Address, const Cy4TypePlate&);
        bool                 operator==(const Cy4Device&) const;
        bool                 operator!=(const Cy4Device&) const;
        Cy4Address           address(void) const;
        const Cy4TypePlate&  typePlate(void) const;
        bool                 isValid(void) const;
};

//////////////////////////////////////////////////////////////////////////////////////////
