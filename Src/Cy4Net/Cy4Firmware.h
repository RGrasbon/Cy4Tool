//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Cy4NET Firmware                                                                     //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <QString>
#include "Cy4Types.h"

//////////////////////////////////////////////////////////////////////////////////////////
class Cy4Firmware
{
    // Typen /////////////////////////////////////////////////////////////////////////////
    public:
        struct BinaryInfo
        {   int first    = INVALID;
            int last     = INVALID;
            int size     = INVALID;
            int coverage = INVALID;
            bool isValid(void) { return first != INVALID; }
        };

    // Daten /////////////////////////////////////////////////////////////////////////////
    private:
        std::string    mHexText;
        BinaryInfo     mBinaryInfo;
        Cy4TypePlate   mTypePlate;

    // Funktionen ////////////////////////////////////////////////////////////////////////
    public:
                               Cy4Firmware(void) = default;
                               Cy4Firmware(const QString&);
        bool                   load(const QString&);
        bool                   load(const QString&, QString&);
        const BinaryInfo&      binaryInfo(void) const;
        const Cy4TypePlate&    typePlate(void) const;
        std::list<Cy4Message>  toMessages(const Cy4Address&) const;
};

//////////////////////////////////////////////////////////////////////////////////////////




