//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Dienst                                                                              //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include "Conf.h"
#include "Global.h"
#include "KeyValues.h"
#include "Service.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////
//  Cy4NET Dienst  ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
Service::Service(const QString& name, Cy4Location location, int size, Access access)
{
    mName     = name;
    mLocation = location;
    if (size > 0 && size <= CFG::MaximumServiceSize) mCache = std::vector<CacheItem>(size);
    mAccess   = access;
    mBrief    = QString();
}

//////////////////////////////////////////////////////////////////////////////////////////
Service::Service(const QJsonObject& jsonObject)
{
    // Name
    {   if (jsonObject.contains(KEY::NAME))
        {   mName = Global::jsonLanguageEntry(jsonObject.value(KEY::NAME), true).trimmed();
        }
        else if (jsonObject.contains(DEPRECATED::TITLE))
        {   // Bis < V2.00 gab es keinen Dienst-Namen "NAME" sonder nur "TITEL"
            mName = Global::jsonLanguageEntry(jsonObject.value(DEPRECATED::TITLE), true).trimmed();
        }
        else
        {   mIssue = QObject::tr("Entry \"%1\" missing").arg(KEY::NAME);
        }
    }
    if (mName.isEmpty())
    {   mIssue = QObject::tr("Entry \"%1\" is empty").arg(KEY::NAME);
    }
    // Ort
    {   if (!jsonObject.contains(KEY::LOCATION))
        {   mIssue = QObject::tr("Entry \"%1\" missing").arg(KEY::LOCATION);
        }
        else
        {   mLocation = Cy4Location(Global::evaluate(jsonObject.value(KEY::LOCATION).toString()));
            if (mLocation < 0x100000 || mLocation > 0xFFFFFE)
            {   mIssue = QObject::tr("Value for \"%1\" out of range 100000h .. FFFFFEh")
                                     .arg(KEY::LOCATION);
            }
            if (!mLocation.isValid())
            {   mIssue = QObject::tr("Invalid entry in \"%1\" (must be 100000h .. "
                                     "FFFFFEh)").arg(KEY::LOCATION);
            }
        }
    }
    // Größe
    {   if (!jsonObject.contains(KEY::SIZE))
        {   mIssue = QObject::tr("Service at location %1h: Entry \"%2\" missing")
                                 .arg(mLocation.toText())
                                 .arg(KEY::SIZE);
        }
        else
        {   int size = jsonObject.value(KEY::SIZE).toInt(INVALID);
            if (size < 1 || size > CFG::MaximumServiceSize)
            {   mIssue = QObject::tr("Service at location %1h: Value for \"%2\" must be "
                                     "1 .. %3").arg(mLocation.toText())
                                                .arg(KEY::SIZE)
                                                .arg(CFG::MaximumServiceSize);
            }
            else
            {   mCache = std::vector<CacheItem>(size);
            }
        }
    }
    // Zugriff
    {   if (!jsonObject.contains(KEY::ACCESS))
        {   mIssue = QObject::tr("Service at location %1h: Entry \"%2\" missing")
                                 .arg(mLocation.toText())
                                 .arg(KEY::ACCESS);
        }
        else
        {   QString entryText = jsonObject.value(KEY::ACCESS).toString().trimmed().toLower();
            if      (entryText == KEY::READ_ONLY)  mAccess = Service::ReadOnly;
            else if (entryText == KEY::WRITE_ONLY) mAccess = Service::WriteOnly;
            else if (entryText == KEY::READ_WRITE) mAccess = Service::ReadWrite;
            else
            {   mIssue = QObject::tr("Service at location %1h: Value for \"%2\" must be "
                                     "\"%3\", \"%4\" or \"%5\"").arg(mLocation.toText(),
                                                                     KEY::ACCESS,
                                                                     KEY::READ_ONLY,
                                                                     KEY::WRITE_ONLY,
                                                                     KEY::READ_WRITE);
            }
        }
    }
    // Beschreibung (optional)
    {   mBrief = Global::jsonLanguageEntry(jsonObject.value(KEY::BRIEF));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Service::operator<(const Service& other) const
{
    return mLocation < other.mLocation;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Service::operator>(const Service& other) const
{
    return mLocation > other.mLocation;
}

//////////////////////////////////////////////////////////////////////////////////////////
const QString&
Service::name(void) const
{
    return mName;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Service::isValid(void) const
{
    return !mName.isEmpty() &&
           mAccess != Service::Unknown &&
           mLocation.isValid() &&
           size() > 0 &&
           mLocation + size() <= 0xFFFFFF;
}

//////////////////////////////////////////////////////////////////////////////////////////
const QString&
Service::issue(void) const
{
    return mIssue;
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location
Service::location(void) const
{
    return mLocation;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
Service::size(void) const
{
    return mCache.size();
}

//////////////////////////////////////////////////////////////////////////////////////////
Service::Access
Service::access(void) const
{
    return mAccess;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
Service::setBrief(const QString& brief)
{
    mBrief = brief;
}

//////////////////////////////////////////////////////////////////////////////////////////
const QString&
Service::brief(void) const
{
    return mBrief;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
Service::setDataAt(int index, UInt08 byte)
{
    assert(index >= 0 && index < int(mCache.size()));
    CacheItem& item = mCache[USize(index)];
    item.data   = byte;
    item.status = Touched;
}

//////////////////////////////////////////////////////////////////////////////////////////
UInt08
Service::dataAt(int index) const
{
    assert(index >= 0 && index < int(mCache.size()));
    return mCache[USize(index)].data;
}

//////////////////////////////////////////////////////////////////////////////////////////
Service::CacheStatus
Service::cacheStatusAt(int index) const
{
    assert(index >= 0 && index < int(mCache.size()));
    return mCache[USize(index)].status;
}

//////////////////////////////////////////////////////////////////////////////////////////
list<Cy4Message>
Service::toRequests(Cy4Address address, Cy4Message::Access access) const
{
    return toRequests(address, mLocation, mCache.size(), access);
}

//////////////////////////////////////////////////////////////////////////////////////////
list<Cy4Message>
Service::toRequests(Cy4Address address, Cy4Location location, int size, Cy4Message::Access access) const
{
    assert(size > 0);
    ::list<Cy4Message> list;
    if (!address.isValid() || !location.isValid()) return list;
    if (location < mLocation || location + size > mLocation + mCache.size()) return list;
    switch (access)
    {   case Cy4Message::Read:
        {   if (mAccess == Service::WriteOnly) return list;
            if (address.isBroadcast()) return list;
            // Liste mit Leseanforderungen aufstellen
            int remainingSize = size;
            do
            {   int payloadSize = ::min(remainingSize, CY4_PAYLOAD_CAPACITY);
                Cy4Message message(address, location, payloadSize);
                if (!message.isValid()) return ::list<Cy4Message>();
                list.push_back(message);
                location  += payloadSize;
                remainingSize -= payloadSize;
            } while (remainingSize > 0);
            break;
        }
        case Cy4Message::Write:
        {   if (mAccess == Service::ReadOnly) return list;
            // Liste mit Schreibaufforderung aufstellen
            int remainingSize = size;
            int offset = (location - mLocation).toInteger();
            do
            {   int payloadSize = ::min(remainingSize, CY4_PAYLOAD_CAPACITY);
                ByteArray payload(payloadSize);
                for (int i=0; i<payloadSize; i++)
                {   payload[i] = mCache.data()[offset+i].data;
                }
                offset += payloadSize;
                Cy4Message message(address, location, payload);
                if (!message.isValid()) return ::list<Cy4Message>();
                list.push_back(message);
                location += payloadSize;
                remainingSize -= payloadSize;
            } while (remainingSize > 0);
            break;
        }
        default:
            ;
    }
    return list;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
Service::fromResponse(const Cy4Message& message)
{
    if (!isValid()) return false;
    if (message.location() < mLocation) return false;
    if (message.location()+message.count() > mLocation + mCache.size()) return false;
    int offset = Cy4Location(message.location() - mLocation).toInteger();
    bool valid = message.isValid();
    for (int i=0; i<message.count(); i++)
    {   CacheItem& item = mCache[offset+i];
        if (message.access() == Cy4Message::Read)
        {   // 'message' ist eine Lesebestätigung. Daten importieren
            item.data = message[i];
        }
        if (valid) item.status = UptoDate;
        else       item.status = Failed;
    }
    return valid;
}

//////////////////////////////////////////////////////////////////////////////////////////
