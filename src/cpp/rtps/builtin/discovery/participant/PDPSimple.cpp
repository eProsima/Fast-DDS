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
 * @file PDPSimple.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimpleListener.h>

#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/builtin/liveliness/WLP.h>

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>

#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPStatic.h>

#include <fastrtps/rtps/resources/AsyncWriterThread.h>

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.h>


#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/reader/WriterProxy.h>

#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/history/ReaderHistory.h>


#include <fastrtps/utils/TimeConversion.h>

#include <fastrtps/log/Log.h>

#include <mutex>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {


PDPSimple::PDPSimple(BuiltinProtocols* built):
    mp_builtin(built),
    mp_RTPSParticipant(nullptr),
    mp_SPDPWriter(nullptr),
    mp_SPDPReader(nullptr),
    mp_EDP(nullptr),
    m_hasChangedLocalPDP(true),
    mp_resendParticipantTimer(nullptr),
    mp_listener(nullptr),
    mp_SPDPWriterHistory(nullptr),
    mp_SPDPReaderHistory(nullptr),
    mp_mutex(new std::recursive_mutex())
    {

    }

PDPSimple::~PDPSimple()
{
    mp_RTPSParticipant->deleteUserEndpoint(mp_SPDPWriter);
    mp_RTPSParticipant->deleteUserEndpoint(mp_SPDPReader);
}

void PDPSimple::initializeParticipantProxyData(std::shared_ptr<ParticipantProxyData> participant_data)
{
    participant_data->m_leaseDuration = mp_RTPSParticipant->getAttributes().builtin.leaseDuration;
    set_VendorId_eProsima(participant_data->m_VendorId);

    participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    if(mp_RTPSParticipant->getAttributes().builtin.use_WriterLivelinessProtocol)
    {
        participant_data->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
        participant_data->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
    }
    if(mp_RTPSParticipant->getAttributes().builtin.use_SIMPLE_EndpointDiscoveryProtocol)
    {
        if(mp_RTPSParticipant->getAttributes().builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
        {
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
        }
        if(mp_RTPSParticipant->getAttributes().builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
        {
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
        }
    }

#if HAVE_SECURITY
    participant_data->m_availableBuiltinEndpoints |= mp_RTPSParticipant->security_manager().builtin_endpoints();
#endif

    participant_data->m_defaultUnicastLocatorList = mp_RTPSParticipant->getAttributes().defaultUnicastLocatorList;
    participant_data->m_defaultMulticastLocatorList = mp_RTPSParticipant->getAttributes().defaultMulticastLocatorList;
    participant_data->m_expectsInlineQos = false;
    participant_data->m_guid = mp_RTPSParticipant->getGuid();
    for(uint8_t i = 0; i<16; ++i)
    {
        if(i<12)
            participant_data->m_key.value[i] = participant_data->m_guid.guidPrefix.value[i];
        else
            participant_data->m_key.value[i] = participant_data->m_guid.entityId.value[i - 12];
    }


    participant_data->m_metatrafficMulticastLocatorList = this->mp_builtin->m_metatrafficMulticastLocatorList;
    participant_data->m_metatrafficUnicastLocatorList = this->mp_builtin->m_metatrafficUnicastLocatorList;

    participant_data->m_participantName = std::string(mp_RTPSParticipant->getAttributes().getName());

    participant_data->m_userData = mp_RTPSParticipant->getAttributes().userData;

#if HAVE_SECURITY
    IdentityToken* identity_token = nullptr;
    if(mp_RTPSParticipant->security_manager().get_identity_token(&identity_token) && identity_token != nullptr)
    {
        participant_data->identity_token_ = std::move(*identity_token);
        mp_RTPSParticipant->security_manager().return_identity_token(identity_token);
    }
#endif
}

bool PDPSimple::initPDP(RTPSParticipantImpl* part)
{
    logInfo(RTPS_PDP,"Beginning");
    mp_RTPSParticipant = part;
    m_discovery = mp_RTPSParticipant->getAttributes().builtin;
    //CREATE ENDPOINTS
    if(!createSPDPEndpoints())
        return false;
    //UPDATE METATRAFFIC.
    mp_builtin->updateMetatrafficLocators(this->mp_SPDPReader->getAttributes()->unicastLocatorList);
    m_participantProxies.push_back(std::shared_ptr<ParticipantProxyData>(new ParticipantProxyData()));
    initializeParticipantProxyData(m_participantProxies.front());

    //INIT EDP
    if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
    {
        mp_EDP.reset(new EDPStatic(this,mp_RTPSParticipant));
        if(!mp_EDP->initEDP(m_discovery)){
            logError(RTPS_PDP,"Endpoint discovery configuration failed");
            return false;
        }

    }
    else if(m_discovery.use_SIMPLE_EndpointDiscoveryProtocol)
    {
        mp_EDP.reset(new EDPSimple(this,mp_RTPSParticipant));
        if(!mp_EDP->initEDP(m_discovery)){
            logError(RTPS_PDP,"Endpoint discovery configuration failed");
            return false;
        }
    }
    else
    {
        logWarning(RTPS_PDP,"No EndpointDiscoveryProtocol defined");
        return false;
    }

    if(!mp_RTPSParticipant->enableReader(mp_SPDPReader))
        return false;

    mp_resendParticipantTimer.reset(new ResendParticipantProxyDataPeriod(this,TimeConv::Time_t2MilliSecondsDouble(m_discovery.leaseDuration_announcementperiod)));

    return true;
}

void PDPSimple::stopParticipantAnnouncement()
{
    mp_resendParticipantTimer->cancel_timer();
}

void PDPSimple::resetParticipantAnnouncement()
{
    mp_resendParticipantTimer->restart_timer();
}

void PDPSimple::announceParticipantState(bool new_change, bool dispose)
{
    logInfo(RTPS_PDP,"Announcing RTPSParticipant State (new change: "<< new_change <<")");
    CacheChange_t* change = nullptr;

    if(!dispose)
    {
        if(new_change || m_hasChangedLocalPDP)
        {
            this->mp_mutex->lock();
            auto local_participant_data = getLocalParticipantProxyData();
            local_participant_data->m_manualLivelinessCount++;
            InstanceHandle_t key = local_participant_data->m_key;
            ParameterList_t parameter_list = local_participant_data->AllQostoParameterList();
            this->mp_mutex->unlock();

            if(mp_SPDPWriterHistory->getHistorySize() > 0)
                mp_SPDPWriterHistory->remove_min_change();
            // TODO(Ricardo) Change DISCOVERY_PARTICIPANT_DATA_MAX_SIZE with getLocalParticipantProxyData()->size().
            change = mp_SPDPWriter->new_change([]() -> uint32_t {return DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;}, ALIVE, key);

            if(change != nullptr)
            {
                CDRMessage_t aux_msg(0);
                aux_msg.wraps = true;
                aux_msg.buffer = change->serializedPayload.data;
                aux_msg.max_size = change->serializedPayload.max_size;

#if __BIG_ENDIAN__
                change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
                aux_msg.msg_endian = BIGEND;
#else
                change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
                aux_msg.msg_endian =  LITTLEEND;
#endif

                if(ParameterList::writeParameterListToCDRMsg(&aux_msg, &parameter_list, true))
                {
                    change->serializedPayload.length = (uint16_t)aux_msg.length;

                    mp_SPDPWriterHistory->add_change(change);
                }
                else
                {
                    logError(RTPS_PDP, "Cannot serialize ParticipantProxyData.");
                }
            }

            m_hasChangedLocalPDP = false;
        }
        else
        {
            mp_SPDPWriter->unsent_changes_reset();
        }
    }
    else
    {
        this->mp_mutex->lock();
        ParameterList_t parameter_list = getLocalParticipantProxyData()->AllQostoParameterList();
        this->mp_mutex->unlock();

        if(mp_SPDPWriterHistory->getHistorySize() > 0)
            mp_SPDPWriterHistory->remove_min_change();
        change = mp_SPDPWriter->new_change([]() -> uint32_t {return DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;}, NOT_ALIVE_DISPOSED_UNREGISTERED, getLocalParticipantProxyData()->m_key);

        if(change != nullptr)
        {
            CDRMessage_t aux_msg(0);
            aux_msg.wraps = true;
            aux_msg.buffer = change->serializedPayload.data;
            aux_msg.max_size = change->serializedPayload.max_size;

#if __BIG_ENDIAN__
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
            aux_msg.msg_endian = BIGEND;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
            aux_msg.msg_endian =  LITTLEEND;
#endif

            if(ParameterList::writeParameterListToCDRMsg(&aux_msg, &parameter_list, true))
            {
                change->serializedPayload.length = (uint16_t)aux_msg.length;

                mp_SPDPWriterHistory->add_change(change);
            }
            else
            {
                logError(RTPS_PDP, "Cannot serialize ParticipantProxyData.");
            }
        }
    }

}

bool PDPSimple::lookupReaderProxyData(const GUID_t& reader, ReaderProxyData& rdata, ParticipantProxyData& pdata)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
    for (auto pit = m_participantProxies.begin();
            pit != m_participantProxies.end();++pit)
    {
        for (auto rit = (*pit)->m_readers.begin();
                rit != (*pit)->m_readers.end();++rit)
        {
            if((*rit)->guid() == reader)
            {
                rdata.copy((*rit).get());
                pdata.copy(*(*pit).get());
                return true;
            }
        }
    }
    return false;
}

bool PDPSimple::lookupWriterProxyData(const GUID_t& writer, WriterProxyData& wdata, ParticipantProxyData& pdata)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
    for (auto pit = m_participantProxies.begin();
            pit != m_participantProxies.end(); ++pit)
    {
        for (auto wit = (*pit)->m_writers.begin();
                wit != (*pit)->m_writers.end(); ++wit)
        {
            if((*wit)->guid() == writer)
            {
                wdata.copy((*wit).get());
                pdata.copy(*(*pit).get());
                return true;
            }
        }
    }
    return false;
}

bool PDPSimple::removeReaderProxyData(const GUID_t& reader_guid)
{
    logInfo(RTPS_PDP, "Removing reader proxy data " << reader_guid);
    std::unique_lock<std::recursive_mutex> guardPDP(*this->mp_mutex);

    for (auto pit = m_participantProxies.begin();
            pit != m_participantProxies.end(); ++pit)
    {
        for (auto rit = (*pit)->m_readers.begin();
                rit != (*pit)->m_readers.end(); ++rit)
        {
            if((*rit)->guid() == reader_guid)
            {
                const GUID_t& participant_guid = (*pit)->m_guid;
                (*pit)->m_readers.erase(rit);
                // We don't need the guard anymore
                guardPDP.unlock();
                mp_EDP->unpairReaderProxy(participant_guid, reader_guid);
                return true;
            }
        }
    }

    return false;
}

bool PDPSimple::removeWriterProxyData(const GUID_t& writer_guid)
{
    logInfo(RTPS_PDP, "Removing writer proxy data " << writer_guid);
    std::unique_lock<std::recursive_mutex> guardPDP(*this->mp_mutex);

    for (auto pit = m_participantProxies.begin();
            pit != m_participantProxies.end(); ++pit)
    {
        for (auto wit = (*pit)->m_writers.begin();
                wit != (*pit)->m_writers.end(); ++wit)
        {
            if((*wit)->guid() == writer_guid)
            {
                const GUID_t& participant_guid = (*pit)->m_guid;
                (*pit)->m_writers.erase(wit);
                // At this point, we do not need the lock anymore
                guardPDP.unlock();
                mp_EDP->unpairWriterProxy(participant_guid, writer_guid);
                return true;
            }
        }
    }

    return false;
}


bool PDPSimple::lookupParticipantProxyData(const GUID_t& pguid, ParticipantProxyData& pdata)
{
    logInfo(RTPS_PDP,pguid);
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
    for(auto pit = m_participantProxies.begin();
            pit!=m_participantProxies.end();++pit)
    {
        if((*pit)->m_guid == pguid)
        {
            pdata.copy(**pit);
            return true;
        }
    }
    return false;
}

bool PDPSimple::createSPDPEndpoints()
{
    logInfo(RTPS_PDP,"Beginning");
    //SPDP BUILTIN RTPSParticipant WRITER
    HistoryAttributes hatt;
    hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
    hatt.initialReservedCaches = 20;
    hatt.maximumReservedCaches = 100;
    mp_SPDPWriterHistory.reset(new WriterHistory(hatt));
    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    watt.endpoint.reliabilityKind = BEST_EFFORT;
    watt.endpoint.topicKind = WITH_KEY;
    if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
            mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
        watt.mode = ASYNCHRONOUS_WRITER;
    RTPSWriter* wout;
    if(mp_RTPSParticipant->createWriter(&wout,watt,mp_SPDPWriterHistory.get(),nullptr,c_EntityId_SPDPWriter,true))
    {
#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(wout, false);
#endif
        mp_SPDPWriter = dynamic_cast<StatelessWriter*>(wout);
        for(LocatorListIterator lit = mp_builtin->m_initialPeersList.begin();
                lit != mp_builtin->m_initialPeersList.end(); ++lit)
            mp_SPDPWriter->add_locator(*lit);
    }
    else
    {
        logError(RTPS_PDP,"SimplePDP Writer creation failed");
        mp_SPDPWriterHistory.reset();
        return false;
    }
    hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
    hatt.initialReservedCaches = 250;
    hatt.maximumReservedCaches = 5000;
    mp_SPDPReaderHistory.reset(new ReaderHistory(hatt));
    ReaderAttributes ratt;
    ratt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    ratt.endpoint.topicKind = WITH_KEY;
    ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    ratt.endpoint.reliabilityKind = BEST_EFFORT;
    mp_listener.reset(new PDPSimpleListener(this));
    RTPSReader* rout;
    if(mp_RTPSParticipant->createReader(&rout,ratt,mp_SPDPReaderHistory.get(),mp_listener.get(),c_EntityId_SPDPReader,true, false))
    {
#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(rout, false);
#endif
        mp_SPDPReader = dynamic_cast<StatelessReader*>(rout);
    }
    else
    {
        logError(RTPS_PDP,"SimplePDP Reader creation failed");
        mp_SPDPReaderHistory.reset();
        mp_listener.reset();
        return false;
    }

    logInfo(RTPS_PDP,"SPDP Endpoints creation finished");
    return true;
}

bool PDPSimple::addReaderProxyData(ReaderProxyData* rdata, ParticipantProxyData& pdata)
{
    logInfo(RTPS_PDP, "Adding reader proxy data " << rdata->guid());

    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);

    for(auto pit = m_participantProxies.begin();
            pit!=m_participantProxies.end();++pit)
    {
        if((*pit)->m_guid.guidPrefix == rdata->guid().guidPrefix)
        {
            // Set locators information if not defined by ReaderProxyData.
            if(rdata->unicastLocatorList().empty() && rdata->multicastLocatorList().empty())
            {
                rdata->unicastLocatorList((*pit)->m_defaultUnicastLocatorList);
                rdata->multicastLocatorList((*pit)->m_defaultMulticastLocatorList);
            }
            // Set as alive.
            rdata->isAlive(true);

            // Copy participant data to be used outside.
            pdata.copy(**pit);

            // Check that it is not already there:
            for(auto rit = (*pit)->m_readers.begin();
                    rit!=(*pit)->m_readers.end();++rit)
            {
                if((*rit)->guid().entityId == rdata->guid().entityId)
                {
                    (*rit)->update(rdata);
                    return true;
                }
            }

            std::shared_ptr<ReaderProxyData> newRPD(new ReaderProxyData(*rdata));
            (*pit)->m_readers.push_back(newRPD);
            return true;
        }
    }

    return false;
}

bool PDPSimple::addWriterProxyData(WriterProxyData* wdata, ParticipantProxyData& pdata)
{
    logInfo(RTPS_PDP, "Adding writer proxy data " << wdata->guid());

    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);

    for(auto pit = m_participantProxies.begin();
            pit!=m_participantProxies.end();++pit)
    {
        if((*pit)->m_guid.guidPrefix == wdata->guid().guidPrefix)
        {
            // Set locators information if not defined by ReaderProxyData.
            if(wdata->unicastLocatorList().empty() && wdata->multicastLocatorList().empty())
            {
                wdata->unicastLocatorList((*pit)->m_defaultUnicastLocatorList);
                wdata->multicastLocatorList((*pit)->m_defaultMulticastLocatorList);
            }
            // Set as alive.
            wdata->isAlive(true);

            // Copy participant data to be used outside.
            pdata.copy(**pit);

            //CHECK THAT IT IS NOT ALREADY THERE:
            for(auto wit = (*pit)->m_writers.begin();
                    wit!=(*pit)->m_writers.end();++wit)
            {
                if((*wit)->guid().entityId == wdata->guid().entityId)
                {
                    (*wit)->update(wdata);
                    return true;
                }
            }

            std::shared_ptr<WriterProxyData> newWPD(new WriterProxyData(*wdata));
            (*pit)->m_writers.push_back(newWPD);
            return true;
        }
    }
    return false;
}

void PDPSimple::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
    logInfo(RTPS_PDP,"For RTPSParticipant: "<<pdata->m_guid.guidPrefix);
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    if(auxendp!=0)
    {
        RemoteWriterAttributes watt(pdata->m_VendorId);
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SPDPWriter;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = BEST_EFFORT;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_SPDPReader->matched_writer_add(watt);
    }
    auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    if(auxendp!=0)
    {
        RemoteReaderAttributes ratt(pdata->m_VendorId);
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SPDPReader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.reliabilityKind = BEST_EFFORT;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_SPDPWriter->matched_reader_add(ratt);
    }

#if HAVE_SECURITY
    // Validate remote participant
    mp_RTPSParticipant->security_manager().discovered_participant(*pdata);
#else
    //Inform EDP of new RTPSParticipant data:
    notifyAboveRemoteEndpoints(*pdata);
#endif
}

void PDPSimple::notifyAboveRemoteEndpoints(const GUID_t& participant_guid)
{
    ParticipantProxyData participant_data;
    bool found_participant = false;

    this->mp_mutex->lock();
    for(auto pit = m_participantProxies.begin();
            pit != m_participantProxies.end(); ++pit)
    {
        if((*pit)->m_guid == participant_guid)
        {
            participant_data.copy(**pit);
            found_participant = true;
            break;
        }
    }
    this->mp_mutex->unlock();

    if(found_participant)
    {
        notifyAboveRemoteEndpoints(participant_data);
    }
}

void PDPSimple::notifyAboveRemoteEndpoints(const ParticipantProxyData& pdata)
{
    //Inform EDP of new RTPSParticipant data:
    if(mp_EDP!=nullptr)
        mp_EDP->assignRemoteEndpoints(pdata);
    if(mp_builtin->mp_WLP !=nullptr)
        mp_builtin->mp_WLP->assignRemoteEndpoints(pdata);
}


void PDPSimple::removeRemoteEndpoints(ParticipantProxyData* pdata)
{
    logInfo(RTPS_PDP,"For RTPSParticipant: "<<pdata->m_guid);
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    if(auxendp!=0)
    {
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SPDPWriter;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = BEST_EFFORT;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_SPDPReader->matched_writer_remove(watt);
    }
    auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    if(auxendp!=0)
    {
        RemoteReaderAttributes ratt;
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SPDPReader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.reliabilityKind = BEST_EFFORT;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        mp_SPDPWriter->matched_reader_remove(ratt);
    }
}

bool PDPSimple::removeRemoteParticipant(GUID_t& partGUID)
{
    logInfo(RTPS_PDP,partGUID );
    std::shared_ptr<ParticipantProxyData> pdata = nullptr;

    //Remove it from our vector or RTPSParticipantProxies:
    this->mp_mutex->lock();
    for(auto pit = m_participantProxies.begin();
            pit!=m_participantProxies.end();++pit)
    {
        if((*pit)->m_guid == partGUID)
        {
            pdata = *pit;
            m_participantProxies.erase(pit);
            break;
        }
    }
    this->mp_mutex->unlock();

    if(pdata !=nullptr)
    {
        if(mp_EDP!=nullptr)
        {
            for(auto rit = pdata->m_readers.begin();
                    rit != pdata->m_readers.end();++rit)
            {
                mp_EDP->unpairReaderProxy(partGUID, (*rit)->guid());
            }
            for(auto wit = pdata->m_writers.begin();
                    wit !=pdata->m_writers.end();++wit)
            {
                mp_EDP->unpairWriterProxy(partGUID, (*wit)->guid());
            }
        }

#if HAVE_SECURITY
        mp_builtin->mp_participantImpl->security_manager().remove_participant(*pdata);
#endif

        if(mp_builtin->mp_WLP != nullptr)
            this->mp_builtin->mp_WLP->removeRemoteEndpoints(pdata.get());
        this->mp_EDP->removeRemoteEndpoints(pdata.get());
        this->removeRemoteEndpoints(pdata.get());

        this->mp_SPDPReaderHistory->getMutex()->lock();
        for(std::vector<CacheChange_t*>::iterator it=this->mp_SPDPReaderHistory->changesBegin();
                it!=this->mp_SPDPReaderHistory->changesEnd();++it)
        {
            if((*it)->instanceHandle == pdata->m_key)
            {
                this->mp_SPDPReaderHistory->remove_change(*it);
                break;
            }
        }
        this->mp_SPDPReaderHistory->getMutex()->unlock();

        return true;
    }

    return false;
}

void PDPSimple::assertRemoteParticipantLiveliness(const GuidPrefix_t& guidP)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
    for(auto it = this->m_participantProxies.begin();
            it!=this->m_participantProxies.end();++it)
    {
        if((*it)->m_guid.guidPrefix == guidP)
        {
            logInfo(RTPS_LIVELINESS,"RTPSParticipant "<< (*it)->m_guid << " is Alive");
            // TODO Ricardo: Study if isAlive attribute is necessary.
            (*it)->isAlive = true;
            if((*it)->mp_leaseDurationTimer != nullptr)
            {
                (*it)->mp_leaseDurationTimer->cancel_timer();
                (*it)->mp_leaseDurationTimer->restart_timer();
            }
            break;
        }
    }
}

void PDPSimple::assertLocalWritersLiveliness(LivelinessQosPolicyKind kind)
{
    logInfo(RTPS_LIVELINESS,"of type " << (kind==AUTOMATIC_LIVELINESS_QOS?"AUTOMATIC":"")
            <<(kind==MANUAL_BY_PARTICIPANT_LIVELINESS_QOS?"MANUAL_BY_PARTICIPANT":""));
    std::lock_guard<std::recursive_mutex> guard(*this->mp_mutex);
    for(auto wit = this->m_participantProxies.front()->m_writers.begin();
            wit!=this->m_participantProxies.front()->m_writers.end();++wit)
    {
        if((*wit)->m_qos.m_liveliness.kind == kind)
        {
            logInfo(RTPS_LIVELINESS,"Local Writer "<< (*wit)->guid().entityId << " marked as ALIVE");
            (*wit)->isAlive(true);
        }
    }
}

void PDPSimple::assertRemoteWritersLiveliness(GuidPrefix_t& guidP,LivelinessQosPolicyKind kind)
{
    std::lock_guard<std::recursive_mutex> guardP(*mp_RTPSParticipant->getParticipantMutex());
    std::lock_guard<std::recursive_mutex> pguard(*this->mp_mutex);
    logInfo(RTPS_LIVELINESS,"of type " << (kind==AUTOMATIC_LIVELINESS_QOS?"AUTOMATIC":"")
            <<(kind==MANUAL_BY_PARTICIPANT_LIVELINESS_QOS?"MANUAL_BY_PARTICIPANT":""));

    for(auto pit=this->m_participantProxies.begin();
            pit!=this->m_participantProxies.end();++pit)
    {
        if((*pit)->m_guid.guidPrefix == guidP)
        {
            for(auto wit = (*pit)->m_writers.begin();
                    wit != (*pit)->m_writers.end();++wit)
            {
                if((*wit)->m_qos.m_liveliness.kind == kind)
                {
                    (*wit)->isAlive(true);
                    for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
                            rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
                    {
                        if((*rit)->getAttributes()->reliabilityKind == RELIABLE)
                        {
                            StatefulReader* sfr = (StatefulReader*)(*rit);
                            WriterProxy* WP;
                            if(sfr->matched_writer_lookup((*wit)->guid(), &WP))
                            {
                                WP->assertLiveliness();
                                continue;
                            }
                        }
                    }
                }
            }
            break;
        }
    }
}

bool PDPSimple::newRemoteEndpointStaticallyDiscovered(const GUID_t& pguid, int16_t userDefinedId,EndpointKind_t kind)
{
    ParticipantProxyData pdata;
    if(lookupParticipantProxyData(pguid, pdata))
    {
        if(kind == WRITER)
            std::dynamic_pointer_cast<EDPStatic>(mp_EDP)->newRemoteWriter(pdata,userDefinedId);
        else
            std::dynamic_pointer_cast<EDPStatic>(mp_EDP)->newRemoteReader(pdata,userDefinedId);
    }
    return false;
}

CDRMessage_t PDPSimple::get_participant_proxy_data_serialized(Endianness_t endian)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
    CDRMessage_t cdr_msg;
    cdr_msg.msg_endian = endian;

    ParameterList_t parameter_list = getLocalParticipantProxyData()->AllQostoParameterList();
    if(!ParameterList::writeParameterListToCDRMsg(&cdr_msg, &parameter_list, true))
    {
        cdr_msg.pos = 0;
        cdr_msg.length = 0;
    }

    return cdr_msg;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
