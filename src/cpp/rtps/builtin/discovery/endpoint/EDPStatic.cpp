// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file EDPStatic.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/endpoint/EDPStatic.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/xmlparser/XMLEndpointParser.h>

#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>

#include <fastrtps/log/Log.h>

#include <mutex>

#include <sstream>

namespace eprosima {
namespace fastrtps{
namespace rtps {


EDPStatic::EDPStatic(PDPSimple* p,RTPSParticipantImpl* part):
    EDP(p,part),
    mp_edpXML(nullptr)
{
}

EDPStatic::~EDPStatic()
{
    if(mp_edpXML != nullptr)
        delete(mp_edpXML);
}

bool EDPStatic::initEDP(BuiltinAttributes& attributes)
{
    logInfo(RTPS_EDP,"Beginning STATIC EndpointDiscoveryProtocol");
    m_attributes = attributes;
    mp_edpXML = new xmlparser::XMLEndpointParser();
    std::string filename(m_attributes.getStaticEndpointXMLFilename());
    return (this->mp_edpXML->loadXMLFile(filename) == xmlparser::XMLP_ret::XML_OK);
}

std::pair<std::string,std::string> EDPStaticProperty::toProperty(std::string type,std::string status,uint16_t id,const EntityId_t& ent)
{
    std::pair<std::string,std::string> prop;
    std::stringstream ss;
    ss << "eProsimaEDPStatic_"<<type<<"_"<<status<<"_ID_"<<id;
    prop.first = ss.str();
    ss.clear();
    ss.str(std::string());
    ss << (int)ent.value[0]<<".";
    ss << (int)ent.value[1]<<".";
    ss << (int)ent.value[2]<<".";
    ss << (int)ent.value[3];
    prop.second = ss.str();
    return prop;
}

bool EDPStaticProperty::fromProperty(std::pair<std::string,std::string> prop)
{
    if(prop.first.substr(0,17) == "eProsimaEDPStatic" && prop.first.substr(31,2) == "ID")
    {
        this->m_endpointType = prop.first.substr(18,6);
        this->m_status = prop.first.substr(25,5);
        this->m_userIdStr = prop.first.substr(34,100);
        std::stringstream ss;
        ss << m_userIdStr;
        ss >> m_userId;
        ss.clear();
        ss.str(std::string());
        ss << prop.second;
        int a,b,c,d;
        char ch;
        ss >> a >> ch >> b >> ch >> c >>ch >> d;
        m_entityId.value[0] = (octet)a;m_entityId.value[1] = (octet)b;
        m_entityId.value[2] = (octet)c;m_entityId.value[3] = (octet)d;
        return true;
    }
    return false;
}



bool EDPStatic::processLocalReaderProxyData(RTPSReader*, ReaderProxyData* rdata)
{
    logInfo(RTPS_EDP,rdata->guid().entityId<< " in topic: " <<rdata->topicName());
    mp_PDP->getMutex()->lock();
    //Add the property list entry to our local pdp
    ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
    localpdata->m_properties.properties.push_back(EDPStaticProperty::toProperty("Reader","ALIVE", rdata->userDefinedId(), rdata->guid().entityId));
    mp_PDP->getMutex()->unlock();
    this->mp_PDP->announceParticipantState(true);
    return true;
}

bool EDPStatic::processLocalWriterProxyData(RTPSWriter*, WriterProxyData* wdata)
{
    logInfo(RTPS_EDP ,wdata->guid().entityId << " in topic: " << wdata->topicName());
    mp_PDP->getMutex()->lock();
    //Add the property list entry to our local pdp
    ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
    localpdata->m_properties.properties.push_back(EDPStaticProperty::toProperty("Writer","ALIVE",
                wdata->userDefinedId(), wdata->guid().entityId));
    mp_PDP->getMutex()->unlock();
    this->mp_PDP->announceParticipantState(true);
    return true;
}

bool EDPStatic::removeLocalReader(RTPSReader* R)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_PDP->getMutex());
    ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
    for(std::vector<std::pair<std::string,std::string>>::iterator pit = localpdata->m_properties.properties.begin();
            pit!=localpdata->m_properties.properties.end();++pit)
    {
        EDPStaticProperty staticproperty;
        if(staticproperty.fromProperty(*pit))
        {
            if(staticproperty.m_entityId == R->getGuid().entityId)
            {
                *pit = EDPStaticProperty::toProperty("Reader","ENDED",R->getAttributes().getUserDefinedID(),
                        R->getGuid().entityId);
            }
        }
    }
    return false;
}

bool EDPStatic::removeLocalWriter(RTPSWriter*W)
{
    std::lock_guard<std::recursive_mutex> guard(*mp_PDP->getMutex());
    ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
    for(std::vector<std::pair<std::string,std::string>>::iterator pit = localpdata->m_properties.properties.begin();
            pit!=localpdata->m_properties.properties.end();++pit)
    {
        EDPStaticProperty staticproperty;
        if(staticproperty.fromProperty(*pit))
        {
            if(staticproperty.m_entityId == W->getGuid().entityId)
            {
                *pit = EDPStaticProperty::toProperty("Writer","ENDED",W->getAttributes().getUserDefinedID(),
                        W->getGuid().entityId);
            }
        }
    }
    return false;
}

void EDPStatic::assignRemoteEndpoints(const ParticipantProxyData& pdata)
{
    for(std::vector<std::pair<std::string,std::string>>::const_iterator pit = pdata.m_properties.properties.begin();
            pit!=pdata.m_properties.properties.end();++pit)
    {
        //cout << "STATIC EDP READING PROPERTY " << pit->first << "// " << pit->second << endl;
        EDPStaticProperty staticproperty;
        if(staticproperty.fromProperty(*pit))
        {
            if(staticproperty.m_endpointType == "Reader" && staticproperty.m_status=="ALIVE")
            {
                ReaderProxyData rdata;
                GUID_t guid(pdata.m_guid.guidPrefix,staticproperty.m_entityId);
                if(!this->mp_PDP->lookupReaderProxyData(guid ,rdata))//IF NOT FOUND, we CREATE AND PAIR IT
                {
                    newRemoteReader(pdata, staticproperty.m_userId,staticproperty.m_entityId);
                }
            }
            else if(staticproperty.m_endpointType == "Writer" && staticproperty.m_status == "ALIVE")
            {
                WriterProxyData wdata;
                GUID_t guid(pdata.m_guid.guidPrefix,staticproperty.m_entityId);
                if(!this->mp_PDP->lookupWriterProxyData(guid,wdata))//IF NOT FOUND, we CREATE AND PAIR IT
                {
                    newRemoteWriter(pdata,staticproperty.m_userId,staticproperty.m_entityId);
                }
            }
            else if(staticproperty.m_endpointType == "Reader" && staticproperty.m_status == "ENDED")
            {
                GUID_t guid(pdata.m_guid.guidPrefix,staticproperty.m_entityId);
                this->mp_PDP->removeReaderProxyData(guid);
            }
            else if(staticproperty.m_endpointType == "Writer" && staticproperty.m_status == "ENDED")
            {
                GUID_t guid(pdata.m_guid.guidPrefix,staticproperty.m_entityId);
                this->mp_PDP->removeWriterProxyData(guid);
            }
            else
            {
                logWarning(RTPS_EDP,"Property with type: "<<staticproperty.m_endpointType
                        << " and status "<<staticproperty.m_status << " not recognized");
            }
        }
        else
        {

        }
    }
}

bool EDPStatic::newRemoteReader(const ParticipantProxyData& pdata, uint16_t userId,EntityId_t entId)
{
    ReaderProxyData* rpd = NULL;
    if(mp_edpXML->lookforReader(pdata.m_participantName, userId, &rpd) == xmlparser::XMLP_ret::XML_OK)
    {
        logInfo(RTPS_EDP,"Activating: " << rpd->guid().entityId << " in topic " << rpd->topicName());
        ReaderProxyData newRPD(*rpd);
        newRPD.guid().guidPrefix = pdata.m_guid.guidPrefix;
        if(entId != c_EntityId_Unknown)
            newRPD.guid().entityId = entId;
        if(!checkEntityId(&newRPD))
        {
            logError(RTPS_EDP,"The provided entityId for Reader with ID: "
                    << newRPD.userDefinedId() << " does not match the topic Kind");
            return false;
        }
        newRPD.key() = newRPD.guid();
        newRPD.RTPSParticipantKey() = pdata.m_guid;
        GUID_t participant_guid;
        if(this->mp_PDP->addReaderProxyData(&newRPD, participant_guid))
        {
            this->pairing_reader_proxy_with_any_local_writer(participant_guid, &newRPD);
            return true;
        }
    }
    return false;
}

bool EDPStatic::newRemoteWriter(const ParticipantProxyData& pdata,uint16_t userId,EntityId_t entId)
{
    WriterProxyData* wpd = NULL;
    if(mp_edpXML->lookforWriter(pdata.m_participantName,userId,&wpd) == xmlparser::XMLP_ret::XML_OK)
    {
        logInfo(RTPS_EDP,"Activating: " << wpd->guid().entityId << " in topic " << wpd->topicName());
        WriterProxyData newWPD(*wpd);
        newWPD.guid().guidPrefix = pdata.m_guid.guidPrefix;
        if(entId != c_EntityId_Unknown)
            newWPD.guid().entityId = entId;
        if(!checkEntityId(&newWPD))
        {
            logError(RTPS_EDP,"The provided entityId for Writer with User ID: "
                    << newWPD.userDefinedId() << " does not match the topic Kind");
            return false;
        }
        newWPD.key() = newWPD.guid();
        newWPD.RTPSParticipantKey() = pdata.m_guid;
        GUID_t participant_guid;
        if(this->mp_PDP->addWriterProxyData(&newWPD, participant_guid))
        {
            this->pairing_writer_proxy_with_any_local_reader(participant_guid, &newWPD);
            return true;
        }
    }
    return false;
}

bool EDPStatic::checkEntityId(ReaderProxyData* rdata)
{
    if(rdata->topicKind() == WITH_KEY && rdata->guid().entityId.value[3] == 0x07)
        return true;
    if(rdata->topicKind() == NO_KEY && rdata->guid().entityId.value[3] == 0x04)
        return true;
    return false;
}

bool EDPStatic::checkEntityId(WriterProxyData* wdata)
{
    if(wdata->topicKind() == WITH_KEY && wdata->guid().entityId.value[3] == 0x02)
        return true;
    if(wdata->topicKind() == NO_KEY && wdata->guid().entityId.value[3] == 0x03)
        return true;
    return false;
}
} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
