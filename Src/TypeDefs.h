//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Universelle Typen & Definitionen                                                    //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <climits>
#include <stdint.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////
//  Definitionen & Konstanten  ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
static  const int     INVALID = INT32_MIN;
#define CARDINALITY(__a__)  (int(sizeof(__a__)/sizeof(__a__[0])))
template <typename T> const T& min(const T& valueA, const T& valueB)                 { return (valueA < valueB) ? valueA : valueB; }
template <typename T> const T& max(const T& valueA, const T& valueB)                 { return (valueA > valueB) ? valueA : valueB; }
template <typename T> const T& bound(const T& lower, const T& value, const T& upper) { return max(lower, min(value, upper));       }

//////////////////////////////////////////////////////////////////////////////////////////
//  Typen  ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
typedef uint8_t      UInt08;     //   8 Bit [0,255]
typedef int8_t       SInt08;     //   8 Bit [-128,+127]
typedef uint16_t     UInt16;     //  16 Bit [0,65535]
typedef int16_t      SInt16;     //  16 Bit [-32768,+32767]
typedef uint32_t     UInt32;     //  32 Bit [0,4294967295]
typedef int32_t      SInt32;     //  32 Bit [-2147483648,+2147483647]
typedef uint64_t     UInt64;     //  64 Bit [0,18446744073709551615]
typedef int64_t      SInt64;     //  64 Bit [-9223372036854775808,+9223372036854775807]
typedef size_t       USize;      //  Typ-alias für Standardgröße

//////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<UInt08> ByteArray;
