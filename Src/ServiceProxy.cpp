//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Dienst-Stellvertreter                                                               //
//  _________________________________                                                   //
//                                                                                      //
//  Copyright (c) Reimar Grasbon                                                        //
//  SPDX-License-Identifier: BSD-2-Clause												//
//////////////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include "Global.h"
#include "ServiceProxy.h"
#include "ServiceMap.h"

//////////////////////////////////////////////////////////////////////////////////////////
static Service*
relatedService(const ServiceProxy* serviceProxy)
{
    int index = serviceMap().indexOf(serviceProxy->location(),
                                     serviceProxy->size(),
                                     serviceProxy->access());
    if (index == INVALID) return nullptr;
    return &serviceMap().at(index);
}

//////////////////////////////////////////////////////////////////////////////////////////
ServiceProxy::ServiceProxy(const QJsonObject& jsonObject)
{
    Service tmpService = Service(jsonObject);
    mLocation = tmpService.location();
    mSize     = tmpService.size();
    mAccess   = tmpService.access();
    mBrief    = tmpService.brief();
}

//////////////////////////////////////////////////////////////////////////////////////////
QString
ServiceProxy::name(void) const
{
    Service* service = relatedService(this);
    if (service == nullptr) return QString();
    return service->name();
}

//////////////////////////////////////////////////////////////////////////////////////////
Cy4Location
ServiceProxy::location(void) const
{
    return mLocation;
}

//////////////////////////////////////////////////////////////////////////////////////////
int
ServiceProxy::size(void) const
{
    return mSize;
}

//////////////////////////////////////////////////////////////////////////////////////////
Service::Access
ServiceProxy::access(void) const
{
    return mAccess;
}

//////////////////////////////////////////////////////////////////////////////////////////
ByteArray
ServiceProxy::read(void) const
{
    Service* service = relatedService(this);
    if (service == nullptr || service->access() == Service::WriteOnly) return ByteArray();
    int offset = Cy4Location(mLocation - service->location()).toInteger();
    ByteArray data(mSize);
    for (USize i=0; i<data.size(); i++)
    {   int index = offset + i;
        data[i] = service->dataAt(index);
    }
    return data;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceProxy::write(int startIndex, const ByteArray& data) const
{
    assert(startIndex + data.size() <= USize(mSize));
    Service* service = relatedService(this);
    if (service == nullptr || service->access() == Service::ReadOnly) return;
    int offset = startIndex + Cy4Location(mLocation - service->location()).toInteger();
    for (USize i=0; i<data.size(); i++) service->setDataAt(offset + i, data[i]);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
ServiceProxy::accessFailed(void) const
{
    Service* service = relatedService(this);
    if (service == nullptr) return true;
    int offset = Cy4Location(mLocation - service->location()).toInteger();
    for (int i=0; i<mSize; i++)
    {   if (service->cacheStatusAt(offset + i) == Service::Failed) return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool
ServiceProxy::isUpToDate(void) const
{
    Service* service = relatedService(this);
    if (service == nullptr) return false;
    if (service->access() == Service::WriteOnly) return false;
    int offset = Cy4Location(mLocation - service->location()).toInteger();
    for (int i=0; i<mSize; i++)
    {   if (service->cacheStatusAt(offset + i) != Service::UptoDate) return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
void
ServiceProxy::setBrief(const QString& brief)
{
    mBrief = brief;
}

//////////////////////////////////////////////////////////////////////////////////////////
const QString&
ServiceProxy::brief(void) const
{
    return mBrief;
}

//////////////////////////////////////////////////////////////////////////////////////////
