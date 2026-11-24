//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Cy4NET Typen                                                                        //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include "Cy4Types.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////
static int
fromBCD(int hex)   // Bsp: 12345h -> 12345
{
    int    dez  = 0;
    UInt32 mult = 1;
    while (hex != 0)
    {   UInt08 digit = UInt08(hex & 0x0F);
        if (digit > 9) return INVALID;
        dez += digit * mult;
        mult *= 10;
        hex >>= 4;
    }
    return dez;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
parseDecimal(const char* text, SInt32& value)
{
    value = INVALID;
    while (isblank(*text)) text++;
    bool isNegative = *text == '-';
    if (isNegative || *text == '+') text++;
    if (!isdigit(*text)) return false;  // Keine Ziffern
    SInt64 tmpValue = 0;
    if (isNegative)
    {   do
        {   int digit = int(*text) - int('0');
            tmpValue = tmpValue * 10 - digit;
            if (tmpValue < -SInt64(0x80000000)) return false; // Unterlauf
            text++;
        } while (isdigit(*text));
    }
    else
    {   do
        {   int n = int(*text) - int('0');
            tmpValue = tmpValue * 10 + n;
            if (tmpValue > SInt64(0x7FFFFFFF)) return false; // Überlauf
            text++;
        } while (isdigit(*text));
    }
    while (isblank(*text)) text++;
    if (*text != '\0') return false;  // Ungültiges Zeichen am Ende der Zahl
    value = SInt32(tmpValue);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
static string
printDecimal(SInt32 value, int minDigits = 1)
{
    const int maxDigits = 10; // Maximal 10 Ziffern: "2147483648"
    minDigits = bound(1, minDigits, maxDigits);
    char text[maxDigits+2]; // Inklusiv '-' und Abschlussnullbyte
    USize index = 0;
    if (value < 0)
    {   text[0] = '-';
        if (value == -2147483648)
        {   // Spezialfall
            for (USize i=1; i<sizeof("-2147483648"); i++) text[i] = "-2147483648"[i];
            return text;
        }
        value = -value;
        index = 1;
    }
    // Vorzeichenlose 31 bit Ganzzahl ausgeben
    UInt32 div = 1000000000;
    int c = maxDigits;
    do
    {   int digit = int((UInt32(value)/div)%10);
        if (digit != 0 || c<=minDigits)
        {   text[index] = char(int('0')+digit);
            c = 0 ;
            index++;
        }
        c--;
        div /= 10;
    } while (div != 0);
    text[index] = '\0';
    return text;
}

//////////////////////////////////////////////////////////////////////////////////////////
static bool
parseHexadecimal(const char* text, UInt32& value)
{
    value = 0;
    // Leerzeichen am Anfang überlesen
    while (isblank(*text)) text++;
    // Gegebenenfalls "0x"-Präfix überspringen
    bool has0xPrefix = text[0] == '0' && (text[1] == 'x' || text[1] == 'X');
    if (has0xPrefix) text += 2;
    if (*text == '\0' || isblank(*text)) return false;
    int relevantDigits = 0;
    UInt32 tmpValue = 0;
    do
    {   UInt08 c = UInt08(toupper(*text));
        UInt32 n;
        if      (c >= '0' && c <= '9') n =  0 + c - '0';
        else if (c >= 'A' && c <= 'F') n = 10 + c - 'A';
        else                           break;
        if (n != 0 || relevantDigits != 0) relevantDigits++; // relevante Ziffern zählen
        if (relevantDigits > int(sizeof(UInt32)*2)) return false; // Überlauf
        tmpValue = (tmpValue<<4) + n;
        text++;
    } while (*text != '\0');
    if (tolower(*text) == 'h')
    {   if (has0xPrefix) return false; // Entweder "0x..." oder "...h"
        text++;  // 'h'-Postfix überspringen
    }
    // Leerzeichen am Ende überlesen
    while (isblank(*text)) text++;
    if (*text != '\0') return false;  // Ungültiges Zeichen am Ende der Zahl
    value = tmpValue;
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
static string
printHexadecimal(UInt32 value, int minDigits = 1)
{
    const int maxDigits = 8; // Maximal 8 Ziffern: "FFFFFFFF"
    minDigits = bound(1, minDigits, maxDigits);
    int position = maxDigits;
    char text[8+1]; // Zusätzlich Abschlussnullbyte
    text[position] = '\0';
    do
    {   position--;
        int nibble = value & 0x0F;
        if (nibble < 10) text[position] = char('0'    + nibble);
        else             text[position] = char('A'-10 + nibble);
        minDigits--;
        value >>= 4;
    } while (value > 0 || minDigits > 0);
    return string(&text[position]);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Address::Cy4Address(int otherNet, int otherDev)
{
    mNet = otherNet;
    mDev = otherDev;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Address::Cy4Address(const char* text)
{
    *this = Cy4Address(string(text));
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Address::Cy4Address(const string& text)
{
    mNet = INVALID;
    mDev = INVALID;
    USize pos = text.find('.');
    if (pos == string::npos) return;
    string netText = text.substr(0, pos);
    string devText = text.substr(pos+1);
    int tempNet;
    if (!parseDecimal(netText.c_str(), tempNet) || tempNet < 0 || tempNet > 255) return;
    int tempDev;
    if (!parseDecimal(devText.c_str(), tempDev) || tempDev < 0 || tempDev > 255) return;
    mNet = tempNet;
    mDev = tempDev;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Address::operator==(Cy4Address other) const
{
    return mNet == other.mNet && mDev == other.mDev;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Address::operator!=(Cy4Address other) const
{
    return !operator==(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Address::operator>(Cy4Address other) const
{
    bool valid1 = isValid();
    bool valid2 = other.isValid();                 //  this   |  other
    switch (valid1 + 2*valid2)                     // --------+---------
    {   case 0:  return false;                     // invalid > invalid -> false
        case 1:  return false;                     //  valid  > invalid -> false
        case 2:  return true;                      // invalid >  valid  -> true
        default: ;
    }
    if (mNet == other.mNet) return mDev > other.mDev;
    return mNet > other.mNet;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Address::operator>=(Cy4Address other) const
{
    return operator>(other) || operator==(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Address::operator<(Cy4Address other) const
{
    return !operator>=(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Address::operator<=(Cy4Address other) const
{
    return !operator>(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Address::isValid(void) const
{
    return mNet >= 0 && mNet <= 255 &&
           mDev >= 0 && mDev <= 255 &&
           (mNet != 255 || mDev != 255);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Address::isBroadcast(void) const
{
    return mNet == 0 || mDev == 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
Cy4Address::net(void) const
{
    return mNet;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
Cy4Address::dev(void) const
{
    return mDev;
}

//////////////////////////////////////////////////////////////////////////////////////////
string
Cy4Address::toText(void) const
{
    if (!isValid()) return "---.---";
    return printDecimal(mNet, 3) + "." + printDecimal(mDev, 3);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Dienstort  ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location::Cy4Location(int value)
{
    mLocation = value;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location::Cy4Location(const char* text)
{
    UInt32 value;
    if (parseHexadecimal(text, value)) mLocation = int(value);
    else                               mLocation = INVALID;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location::Cy4Location(const string& text)
{
    *this = Cy4Location(text.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator==(Cy4Location other) const
{
    return operator==(other.mLocation);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator==(int value) const
{
    return mLocation == value;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator!=(Cy4Location other) const
{
    return !operator==(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator!=(int value) const
{
    return !operator==(value);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator>(Cy4Location other) const
{
    return operator>(other.mLocation);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator>(int value) const
{
    return mLocation > value;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator>=(Cy4Location other) const
{
    return operator>=(other.mLocation);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator>=(int value) const
{
    return operator>(value) || operator==(value);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator<(Cy4Location other) const
{
    return operator<(other.mLocation);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator<(int value) const
{
    return mLocation < value;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator<=(Cy4Location other) const
{
    return operator<=(other.mLocation);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::operator<=(int value) const
{
    return operator<(value) || operator==(value);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location
Cy4Location::operator+(Cy4Location location) const
{
    return operator+(location.mLocation);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location
Cy4Location::operator+(int value) const
{
    return Cy4Location(mLocation + value);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location
operator+(int value, Cy4Location location)
{
    return Cy4Location(value + location.mLocation);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location
Cy4Location::operator-(Cy4Location location) const
{
    return operator-(location.mLocation);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location
Cy4Location::operator-(int value) const
{
    return Cy4Location(mLocation - value);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location
operator-(int value, Cy4Location location)
{
    return Cy4Location(value - location.mLocation);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location&
Cy4Location::operator+=(Cy4Location location)
{
    return operator+=(location.mLocation);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location&
Cy4Location::operator+=(int value)
{
    mLocation += value;
    return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location&
Cy4Location::operator-=(Cy4Location location)
{
    return operator-=(location.mLocation);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location&
Cy4Location::operator-=(int value)
{
    mLocation -= value;
    return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Location::isValid(void) const
{
    return mLocation >= 0x000000 && mLocation < 0xFFFFFF;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
Cy4Location::toInteger(void) const
{
    return mLocation;
}

//////////////////////////////////////////////////////////////////////////////////////////
string
Cy4Location::toText(void) const
{
    if (!isValid()) return "------";
    return printHexadecimal(static_cast<UInt32>(mLocation), 6);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Nachricht  ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Cy4Message::Cy4Message(Cy4Address address, Cy4Location location, int size)
{
    assert (size >= 0 && size <= CY4_PAYLOAD_CAPACITY);
    // Konstruktor für eine Lese-Nachricht
    mValid    = address.isValid()  && location != Cy4Location() && size > 0;
    mAccess   = Read;
    mAddress  = address;
    mLocation = location;
    mData.resize(USize(size));
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Message::Cy4Message(Cy4Address address, Cy4Location location, const ByteArray& data)
{
    assert (data.size() <= CY4_PAYLOAD_CAPACITY);
    // Konstruktor für eine Schreib-Nachricht
    mValid    = address.isValid()  && location.isValid() && data.size() > 0;
    mAccess   = Write;
    mAddress  = address;
    mLocation = location;
    mData     = data;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Message::isValid(void) const
{
    return mValid;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Message::isBroadcast(void) const
{
    return mAddress.isBroadcast();
}

//////////////////////////////////////////////////////////////////////////////////////////
void
Cy4Message::invalidate(void)
{
    mValid = false;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Message::Access
Cy4Message::access(void) const
{
    return mAccess;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Address
Cy4Message::address(void) const
{
    return mAddress;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location
Cy4Message::location(void) const
{
    return mLocation;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
Cy4Message::count(void) const
{
    return int(mData.size());
}

//////////////////////////////////////////////////////////////////////////////////////////
const ByteArray&
Cy4Message::data(void) const
{
    return mData;
}

//////////////////////////////////////////////////////////////////////////////////////////
UInt08
Cy4Message::operator[](int index) const
{
    return const_cast<Cy4Message&>(*this).operator[](index);
}

//////////////////////////////////////////////////////////////////////////////////////////
UInt08&
Cy4Message::operator[](int index)
{
    if (index < 0 || index >= int(mData.size()))
    {   assert(index >= 0 && index < int(mData.size()));
    }
    assert(index >= 0 && index < int(mData.size()));
    return mData[USize(index)];
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Typennummer  //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Cy4TypeNumber::Cy4TypeNumber(int value)
{
    mNumber = value;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4TypeNumber::Cy4TypeNumber(const Cy4TypeNumber& other)
{
    mNumber = other.mNumber;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4TypeNumber&
Cy4TypeNumber::operator=(const Cy4TypeNumber& other)
{
    mNumber = other.mNumber;
    return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4TypeNumber::operator==(const Cy4TypeNumber& other) const
{
    return mNumber == other.mNumber;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4TypeNumber::operator!=(const Cy4TypeNumber& other) const
{
    return !operator==(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4TypeNumber::isValid(void) const
{
    return mNumber > 0 && mNumber < 100000;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4TypeNumber::isPrivate(void) const
{
    return mNumber >= 90000 && mNumber <= 99999;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
Cy4TypeNumber::toInteger(void) const
{
    return mNumber;
}

//////////////////////////////////////////////////////////////////////////////////////////
string
Cy4TypeNumber::toText(void) const
{
    if (!isValid()) return "-----";
    return printDecimal(mNumber, 5);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Anwendungsnummer  /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Cy4ApplicationType::Cy4ApplicationType(int num) : Cy4TypeNumber(num)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4ApplicationType::Cy4ApplicationType(const char* text) : Cy4TypeNumber()
{
    if (text[0] == 'A' || text[0] == 'a')
    {   text++;
        SInt32 value;
        if (parseDecimal(text, value)) mNumber = value;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4ApplicationType::Cy4ApplicationType(const string& text)
{
    *this = Cy4ApplicationType(text.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4ApplicationType::isProgramLoader(void) const
{
    return  mNumber > 0 && mNumber < 100;
}

//////////////////////////////////////////////////////////////////////////////////////////
string
Cy4ApplicationType::toText(void) const
{
    return 'A' + Cy4TypeNumber::toText();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Boardsnummer  /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Cy4BoardType::Cy4BoardType(int num) : Cy4TypeNumber(num)
{
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4BoardType::Cy4BoardType(const char* text) : Cy4TypeNumber()
{
    if (text[0] == 'B' || text[0] == 'b')
    {   text++;
        SInt32 value;
        if (parseDecimal(text, value)) mNumber = value;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4BoardType::Cy4BoardType(const string& text)
{
    *this = Cy4BoardType(text.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
string
Cy4BoardType::toText(void) const
{
    return 'B' + Cy4TypeNumber::toText();
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Version  //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Cy4Version::Cy4Version(int major, int minor)
{
    mVersion = (major<<4) | minor;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Version::Cy4Version(const char* text)
{
    mVersion = 0;                                          // Format: [V|v]#.#
    int i = 0;                                             //
    while (isblank(text[i])) i++;                          // Beispiele: "1.2"
    if (text[i] == 'V' || text[i] == 'v') i++;             //            "  1.2 "
    while (isblank(text[i])) i++;                          //            "v 1.2"
    if (!isdigit(text[i])) return;                         //            "V1.2"
    UInt08 major = UInt08(text[i]) - UInt08('0');
    i++;
    if (text[i] != '.') return;
    i++;
    if (!isdigit(text[i])) return;
    UInt08 minor = UInt08(text[i]) - UInt08('0');
    i++;
    while (isblank(text[i])) i++;
    if (minor == 0 && major == 0) return;  // V0.0 nicht erlaubt
    if (text[i] == '\0') mVersion = (major<<4) | minor;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Version::Cy4Version(const string& text)
{
    *this = Cy4Version(text.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Version::operator==(Cy4Version other) const
{
    return mVersion == other.mVersion;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Version::operator!=(Cy4Version other) const
{
    return !operator==(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Version::operator<(Cy4Version other) const
{
    return !operator>=(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Version::operator<=(Cy4Version other) const
{
    return !operator>(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Version::operator>(Cy4Version other) const
{
    return mVersion > other.mVersion;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Version::operator>=(Cy4Version other) const
{
    return operator>(other) || operator==(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Version::isValid(void) const
{
    int major = mVersion/16;
    int minor = mVersion&0x0F;
    if (major == 0 && minor == 0) return false;  // V0.0 nicht erlaubt
    return major >= 0 && major <= 9 && minor >= 0 && minor <= 9;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
Cy4Version::toInteger(void) const
{
    return mVersion;
}

//////////////////////////////////////////////////////////////////////////////////////////
string
Cy4Version::toText(void) const
{
    if (!isValid()) return "V-.-";
    return 'V' + printDecimal(mVersion/16) + '.' + printDecimal(mVersion&0x0F);
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Geräteinformation  ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Cy4TypePlate::Cy4TypePlate(const Cy4ApplicationType& applicationType,
                           Cy4Version                applicationVersion,
                           const Cy4BoardType&       boardType)
{
    mApplicationType = applicationType;
    mApplicationVersion = applicationVersion;
    mBoardType = boardType;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4TypePlate::Cy4TypePlate(const Cy4Message& message)
{
    if (message.isValid()                      &&
        message.access()   == Cy4Message::Read &&
        message.location() == 0xFFFFF0         &&
        message.count()    == 7)
    {   *this = Cy4TypePlate(message.data().data());
    }
    else
    {   *this = Cy4TypePlate();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4TypePlate::Cy4TypePlate(const ByteArray& data)
{
    *this = Cy4TypePlate(data.data());
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4TypePlate::Cy4TypePlate(const UInt08* data)
{
    *this = Cy4TypePlate();
    // Anwendungstypennummer: Axxxxxh -> xxxxx
    if ((data[0] & 0xF0) != 0xA0) return;
    int  typeNumber;
    Cy4TypePlate  tempPlate;
    typeNumber = ((int(data[0] & 0x0F))<<16) + (int(data[1])<<8) + data[2];
    tempPlate.mApplicationType = Cy4ApplicationType(fromBCD(typeNumber));
    if (!tempPlate.mApplicationType.isValid()) return;
    // Anwendungsversion: ijh -> Vi.j
    tempPlate.mApplicationVersion  = Cy4Version(data[3]>>4, data[3]&15);
    if (!tempPlate.mApplicationVersion.isValid()) return;
    // Boardtypennummer:  Bxxxxxh -> xxxxx
    if ((data[4] & 0xF0) != 0xB0) return;
    typeNumber = ((int(data[4] & 0x0F))<<16) + (int(data[5])<<8) + data[6];
    tempPlate.mBoardType = Cy4BoardType(fromBCD(typeNumber));
    if (!tempPlate.mBoardType.isValid()) return;
    *this = tempPlate;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4TypePlate::operator==(const Cy4TypePlate& other) const
{
    return mApplicationType    == other.mApplicationType    &&
           mApplicationVersion == other.mApplicationVersion &&
           mBoardType          == other.mBoardType;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4TypePlate::operator!=(const Cy4TypePlate& other) const
{
    return !operator==(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4TypePlate::isValid(void) const
{
    return mApplicationType.isValid()    &&
           mApplicationVersion.isValid() &&
           mBoardType.isValid();
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4TypePlate::isProgramLoader(void) const
{
    return isValid() && mApplicationType.isProgramLoader();
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4ApplicationType
Cy4TypePlate::applicationType(void) const
{
    return mApplicationType;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Version
Cy4TypePlate::applicationVersion(void) const
{
    return mApplicationVersion;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4BoardType
Cy4TypePlate::boardType(void) const
{
    return mBoardType;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Gerät  ////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Cy4Device::Cy4Device(Cy4Address address, const Cy4TypePlate& typePlate)
{
    mAddress   = address;
    mTypePlate = typePlate;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Device::operator==(const Cy4Device& other) const
{
    return mAddress == other.mAddress && mTypePlate == other.mTypePlate;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Device::operator!=(const Cy4Device& other) const
{
    return !operator==(other);
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Address
Cy4Device::address(void) const
{
    return mAddress;
}

//////////////////////////////////////////////////////////////////////////////////////////
const Cy4TypePlate&
Cy4Device::typePlate(void) const
{
    return mTypePlate;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Cy4Device::isValid(void) const
{
    return mAddress.isValid() && mTypePlate.isValid();
}

//////////////////////////////////////////////////////////////////////////////////////////
