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
 * @file EDPSimple.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include "EDPSimpleListeners.h"


#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>

#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>


#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/history/WriterHistory.h>


#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>


#include <fastrtps/log/Log.h>

#include <mutex>

namespace eprosima {
namespace fastrtps{
namespace rtps {


EDPSimple::EDPSimple(PDPSimple* p,RTPSParticipantImpl* part):
    EDP(p,part),
    mp_pubListen(nullptr),
    mp_subListen(nullptr)

    {
        // TODO Auto-generated constructor stub

    }

EDPSimple::~EDPSimple()
{
    if(this->mp_PubReader.first !=nullptr)
    {
        delete(mp_PubReader.first);
        delete(mp_PubReader.second);
    }
    if(this->mp_SubReader.first !=nullptr)
    {
        delete(mp_SubReader.first);
        delete(mp_SubReader.second);
    }
    if(this->mp_PubWriter.first !=nullptr)
    {
        delete(mp_PubWriter.first);
        delete(mp_PubWriter.second);
    }
    if(this->mp_SubWriter.first !=nullptr)
    {
        delete(mp_SubWriter.first);
        delete(mp_SubWriter.second);
    }
    if(mp_pubListen!=nullptr)
        delete(mp_pubListen);
    if(mp_subListen !=nullptr)
        delete(mp_subListen);
}


bool EDPSimple::initEDP(BuiltinAttributes& attributes)
{
    logInfo(RTPS_EDP,"Beginning Simple Endpoint Discovery Protocol");
    m_discovery = attributes;

    if(!createSEDPEndpoints())
    {
        logError(RTPS_EDP,"Problem creation SimpleEDP endpoints");
        return false;
    }
    return true;
}


bool EDPSimple::createSEDPEndpoints()
{
    logInfo(RTPS_EDP,"Beginning");
    WriterAttributes watt;
    ReaderAttributes ratt;
    HistoryAttributes hatt;
    bool created = true;
    RTPSReader* raux = nullptr;
    RTPSWriter* waux = nullptr;
    if(m_discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
    {
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 5000;
        hatt.payloadMaxSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
        mp_PubWriter.second = new WriterHistory(hatt);
        //Wparam.pushMode = true;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.times.nackResponseDelay.seconds = 0;
        watt.times.nackResponseDelay.fraction = 0;
        watt.times.initialHeartbeatDelay.seconds = 0;
        watt.times.initialHeartbeatDelay.fraction = 0;
        if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
                mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
            watt.mode = ASYNCHRONOUS_WRITER;
        created &=this->mp_RTPSParticipant->createWriter(&waux,watt,mp_PubWriter.second,nullptr,c_EntityId_SEDPPubWriter,true);
        if(created)
        {
            mp_PubWriter.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP,"SEDP Publication Writer created");
        }
        else
        {
            delete(mp_PubWriter.second);
            mp_PubWriter.second = nullptr;
        }
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 1000000;
        hatt.payloadMaxSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
        mp_SubReader.second = new ReaderHistory(hatt);
        //Rparam.historyMaxSize = 100;
        ratt.expectsInlineQos = false;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.topicKind = WITH_KEY;
        ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.times.heartbeatResponseDelay.seconds = 0;
        ratt.times.heartbeatResponseDelay.fraction = 0;
        ratt.times.initialAcknackDelay.seconds = 0;
        ratt.times.initialAcknackDelay.fraction = 0;
        this->mp_subListen = new EDPSimpleSUBListener(this);
        created &=this->mp_RTPSParticipant->createReader(&raux,ratt,mp_SubReader.second,mp_subListen,c_EntityId_SEDPSubReader,true);
        if(created)
        {
            mp_SubReader.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP,"SEDP Subscription Reader created");
        }
        else
        {
            delete(mp_SubReader.second);
            mp_SubReader.second = nullptr;
            delete(mp_subListen);
            mp_subListen = nullptr;
        }
    }
    if(m_discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
    {
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 1000000;
        hatt.payloadMaxSize = DISCOVERY_PUBLICATION_DATA_MAX_SIZE;
        mp_PubReader.second = new ReaderHistory(hatt);
        //Rparam.historyMaxSize = 100;
        ratt.expectsInlineQos = false;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.topicKind = WITH_KEY;
        ratt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.times.heartbeatResponseDelay.seconds = 0;
        ratt.times.heartbeatResponseDelay.fraction = 0;
        ratt.times.initialAcknackDelay.seconds = 0;
        ratt.times.initialAcknackDelay.fraction = 0;
        this->mp_pubListen = new EDPSimplePUBListener(this);
        created &=this->mp_RTPSParticipant->createReader(&raux,ratt,mp_PubReader.second,mp_pubListen,c_EntityId_SEDPPubReader,true);
        if(created)
        {
            mp_PubReader.first = dynamic_cast<StatefulReader*>(raux);
            logInfo(RTPS_EDP,"SEDP Publication Reader created");

        }
        else
        {
            delete(mp_PubReader.second);
            mp_PubReader.second = nullptr;
            delete(mp_pubListen);
            mp_pubListen = nullptr;
        }
        hatt.initialReservedCaches = 100;
        hatt.maximumReservedCaches = 5000;
        hatt.payloadMaxSize = DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;
        mp_SubWriter.second = new WriterHistory(hatt);
        //Wparam.pushMode = true;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.topicKind = WITH_KEY;
        watt.endpoint.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        watt.times.nackResponseDelay.seconds = 0;
        watt.times.nackResponseDelay.fraction = 0;
        watt.times.initialHeartbeatDelay.seconds = 0;
        watt.times.initialHeartbeatDelay.fraction = 0;
        if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
                mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
            watt.mode = ASYNCHRONOUS_WRITER;
        created &=this->mp_RTPSParticipant->createWriter(&waux,watt,mp_SubWriter.second,nullptr,c_EntityId_SEDPSubWriter,true);
        if(created)
        {
            mp_SubWriter.first = dynamic_cast<StatefulWriter*>(waux);
            logInfo(RTPS_EDP,"SEDP Subscription Writer created");

        }
        else
        {
            delete(mp_SubWriter.second);
            mp_SubWriter.second = nullptr;
        }
    }
    logInfo(RTPS_EDP,"Creation finished");
    return created;
}


bool EDPSimple::processLocalReaderProxyData(ReaderProxyData* rdata)
{
    logInfo(RTPS_EDP,rdata->guid().entityId);
    if(mp_SubWriter.first !=nullptr)
    {
        // TODO(Ricardo) Write a getCdrSerializedPayload for ReaderProxyData.
        CacheChange_t* change = mp_SubWriter.first->new_change([]() -> uint32_t {return DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;}, ALIVE,rdata->key());

        if(change !=nullptr)
        {
            rdata->toParameterList();

            CDRMessage_t aux_msg(0);
            aux_msg.wraps = true;
            aux_msg.buffer = change->serializedPayload.data;
            aux_msg.max_size = change->serializedPayload.max_size;

#if EPROSIMA_BIG_ENDIAN
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
            aux_msg.msg_endian = BIGEND;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
            aux_msg.msg_endian =  LITTLEEND;
#endif

            ParameterList::writeParameterListToCDRMsg(&aux_msg, &rdata->m_parameterList, true);
            change->serializedPayload.length = (uint16_t)aux_msg.length;

            {
                std::unique_lock<std::recursive_mutex> lock(*mp_SubWriter.second->getMutex());
                for(auto ch = mp_SubWriter.second->changesBegin();ch!=mp_SubWriter.second->changesEnd();++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        mp_SubWriter.second->remove_change(*ch);
                        break;
                    }
                }
            }

            if(this->mp_subListen->getAttachedListener() != nullptr)
                this->mp_subListen->getAttachedListener()->onNewCacheChangeAdded(mp_SubReader.first, change);

            mp_SubWriter.second->add_change(change);

            return true;
        }
        return false;
    }
    return true;
}
bool EDPSimple::processLocalWriterProxyData(WriterProxyData* wdata)
{
    logInfo(RTPS_EDP, wdata->guid().entityId);
    if(mp_PubWriter.first !=nullptr)
    {
        CacheChange_t* change = mp_PubWriter.first->new_change([]() -> uint32_t {return DISCOVERY_PUBLICATION_DATA_MAX_SIZE;}, ALIVE, wdata->key());
        if(change != nullptr)
        {
            wdata->toParameterList();

            CDRMessage_t aux_msg(0);
            aux_msg.wraps = true;
            aux_msg.buffer = change->serializedPayload.data;
            aux_msg.max_size = change->serializedPayload.max_size;

#if EPROSIMA_BIG_ENDIAN
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
            aux_msg.msg_endian = BIGEND;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
            aux_msg.msg_endian =  LITTLEEND;
#endif

            ParameterList::writeParameterListToCDRMsg(&aux_msg, &wdata->m_parameterList, true);
            change->serializedPayload.length = (uint16_t)aux_msg.length;

            {
                std::unique_lock<std::recursive_mutex> lock(*mp_PubWriter.second->getMutex());
                for(auto ch = mp_PubWriter.second->changesBegin();ch!=mp_PubWriter.second->changesEnd();++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        mp_PubWriter.second->remove_change(*ch);
                        break;
                    }
                }
            }

            if(this->mp_pubListen->getAttachedListener() != nullptr)
                this->mp_pubListen->getAttachedListener()->onNewCacheChangeAdded(mp_PubReader.first, change);

            mp_PubWriter.second->add_change(change);

            return true;
        }
        return false;
    }
    return true;
}

bool EDPSimple::removeLocalWriter(RTPSWriter* W)
{
    logInfo(RTPS_EDP,W->getGuid().entityId);
    if(mp_PubWriter.first!=nullptr)
    {
        InstanceHandle_t iH;
        iH = W->getGuid();
        CacheChange_t* change = mp_PubWriter.first->new_change([]() -> uint32_t {return DISCOVERY_PUBLICATION_DATA_MAX_SIZE;}, NOT_ALIVE_DISPOSED_UNREGISTERED,iH);
        if(change != nullptr)
        {
            {
                std::lock_guard<std::recursive_mutex> guard(*mp_PubWriter.second->getMutex());
                for(auto ch = mp_PubWriter.second->changesBegin();ch!=mp_PubWriter.second->changesEnd();++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        mp_PubWriter.second->remove_change(*ch);
                        break;
                    }
                }

            }

            if(this->mp_pubListen->getAttachedListener() != nullptr)
                this->mp_pubListen->getAttachedListener()->onNewCacheChangeAdded(mp_PubReader.first, change);

            mp_PubWriter.second->add_change(change);
        }
    }
    return removeWriterProxy(W->getGuid());
}

bool EDPSimple::removeLocalReader(RTPSReader* R)
{
    logInfo(RTPS_EDP,R->getGuid().entityId);
    if(mp_SubWriter.first!=nullptr)
    {
        InstanceHandle_t iH;
        iH = (R->getGuid());
        CacheChange_t* change = mp_SubWriter.first->new_change([]() -> uint32_t {return DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE;}, NOT_ALIVE_DISPOSED_UNREGISTERED,iH);
        if(change != nullptr)
        {
            {
                std::lock_guard<std::recursive_mutex> guard(*mp_SubWriter.second->getMutex());
                for(auto ch = mp_SubWriter.second->changesBegin();ch!=mp_SubWriter.second->changesEnd();++ch)
                {
                    if((*ch)->instanceHandle == change->instanceHandle)
                    {
                        mp_SubWriter.second->remove_change(*ch);
                        break;
                    }
                }
            }

            if(this->mp_subListen->getAttachedListener() != nullptr)
                this->mp_subListen->getAttachedListener()->onNewCacheChangeAdded(mp_SubReader.first, change);

            mp_SubWriter.second->add_change(change);
        }
    }
    return removeReaderProxy(R->getGuid());
}



void EDPSimple::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
    logInfo(RTPS_EDP,"New DPD received, adding remote endpoints to our SimpleEDP endpoints");
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    std::lock_guard<std::recursive_mutex> guard(*pdata->mp_mutex);
    if(auxendp!=0 && mp_PubReader.first!=nullptr) //Exist Pub Writer and i have pub reader
    {
        logInfo(RTPS_EDP,"Adding SEDP Pub Writer to my Pub Reader");
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SEDPPubWriter;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        pdata->m_builtinWriters.push_back(watt);
        mp_PubReader.first->matched_writer_add(watt);
    }
    auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && mp_PubWriter.first!=nullptr) //Exist Pub Detector
    {
        logInfo(RTPS_EDP,"Adding SEDP Pub Reader to my Pub Writer");
        RemoteReaderAttributes ratt;
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SEDPPubReader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.reliabilityKind = RELIABLE;
        pdata->m_builtinReaders.push_back(ratt);
        mp_PubWriter.first->matched_reader_add(ratt);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && mp_SubReader.first!=nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Writer to my Sub Reader");
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SEDPSubWriter;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        pdata->m_builtinWriters.push_back(watt);
        mp_SubReader.first->matched_writer_add(watt);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    //FIXME: FIX TO NOT FAIL WITH BAD BUILTIN ENDPOINT SET
    //auxendp = 1;
    if(auxendp!=0 && mp_SubWriter.first!=nullptr) //Exist Pub Announcer
    {
        logInfo(RTPS_EDP,"Adding SEDP Sub Reader to my Sub Writer");
        RemoteReaderAttributes ratt;
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SEDPSubReader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.reliabilityKind = RELIABLE;
        pdata->m_builtinReaders.push_back(ratt);
        mp_SubWriter.first->matched_reader_add(ratt);
    }
}


void EDPSimple::removeRemoteEndpoints(ParticipantProxyData* pdata)
{
    logInfo(RTPS_EDP,"For RTPSParticipant: "<<pdata->m_guid);
    std::lock_guard<std::recursive_mutex> guard(*pdata->mp_mutex);
    for (auto it = pdata->m_builtinReaders.begin(); it != pdata->m_builtinReaders.end();++it)
    {
        if(it->guid.entityId == c_EntityId_SEDPPubReader && this->mp_PubWriter.first !=nullptr)
        {
            mp_PubWriter.first->matched_reader_remove(*it);
            continue;
        }
        if(it->guid.entityId == c_EntityId_SEDPSubReader && this->mp_SubWriter.first !=nullptr)
        {
            mp_SubWriter.first->matched_reader_remove(*it);
            continue;
        }
    }

    for (auto it = pdata->m_builtinWriters.begin(); it != pdata->m_builtinWriters.end(); ++it)
    {
        if(it->guid.entityId == c_EntityId_SEDPPubWriter && this->mp_PubReader.first !=nullptr)
        {
            mp_PubReader.first->matched_writer_remove(*it);
            continue;
        }
        if(it->guid.entityId == c_EntityId_SEDPSubWriter && this->mp_SubReader.first !=nullptr)
        {
            mp_SubReader.first->matched_writer_remove(*it);
            continue;
        }
    }
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
