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
    if(mp_resendParticipantTimer != nullptr)
        delete(mp_resendParticipantTimer);

    mp_RTPSParticipant->disableReader(mp_SPDPReader);

    if(mp_EDP!=nullptr)
    {
        delete(mp_EDP);
    }

    mp_RTPSParticipant->deleteUserEndpoint(mp_SPDPWriter);
    mp_RTPSParticipant->deleteUserEndpoint(mp_SPDPReader);
    delete(mp_SPDPWriterHistory);
    delete(mp_SPDPReaderHistory);

    delete(mp_listener);
    for(auto it = this->m_participantProxies.begin();
            it!=this->m_participantProxies.end();++it)
    {
        delete(*it);
    }

    delete(mp_mutex);
}

void PDPSimple::initializeParticipantProxyData(BuiltinAttributes& discovery_attr,
        ParticipantProxyData* participant_data)
{
    participant_data->m_leaseDuration = discovery_attr.leaseDuration;
    set_VendorId_eProsima(participant_data->m_VendorId);

    participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

    if(discovery_attr.use_WriterLivelinessProtocol)
    {
        participant_data->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
        participant_data->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;
    }

    if(discovery_attr.use_SIMPLE_EndpointDiscoveryProtocol)
    {
        if(discovery_attr.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
        {
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
        }

        if(discovery_attr.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
        {
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
        }

#if HAVE_SECURITY
        if(discovery_attr.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader)
        {
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
        }

        if(discovery_attr.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader)
        {
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
        }
#endif
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


    participant_data->m_metatrafficMulticastLocatorList = discovery_attr.metatrafficMulticastLocatorList;
    participant_data->m_metatrafficUnicastLocatorList = discovery_attr.metatrafficUnicastLocatorList;

    participant_data->m_participantName = std::string(mp_RTPSParticipant->getAttributes().getName());

    participant_data->m_userData = mp_RTPSParticipant->getAttributes().userData;

#if HAVE_SECURITY
    IdentityToken* identity_token = nullptr;
    if(mp_RTPSParticipant->security_manager().get_identity_token(&identity_token) && identity_token != nullptr)
    {
        participant_data->identity_token_ = std::move(*identity_token);
        mp_RTPSParticipant->security_manager().return_identity_token(identity_token);
    }

    PermissionsToken* permissions_token = nullptr;
    if(mp_RTPSParticipant->security_manager().get_permissions_token(&permissions_token) && permissions_token != nullptr)
    {
        participant_data->permissions_token_ = std::move(*permissions_token);
        mp_RTPSParticipant->security_manager().return_permissions_token(permissions_token);
    }
#endif
}

bool PDPSimple::initPDP(RTPSParticipantImpl* part, BuiltinAttributes& discovery_attr)
{
    logInfo(RTPS_PDP,"Beginning");
    mp_RTPSParticipant = part;

    //CREATE ENDPOINTS
    if(!createSPDPEndpoints(discovery_attr))
        return false;

    //Create ReceiverResources now and update the list with the REAL used ones
    //createReceiverResources(m_att.builtin.metatrafficMulticastLocatorList, true);
    part->network_factory().NormalizeLocators(discovery_attr.metatrafficMulticastLocatorList);

    //Create ReceiverResources now and update the list with the REAL used ones
    //createReceiverResources(m_att.builtin.metatrafficUnicastLocatorList, true);
    part->network_factory().NormalizeLocators(discovery_attr.metatrafficUnicastLocatorList);

    //UPDATE METATRAFFIC.
    discovery_attr.metatrafficUnicastLocatorList = this->mp_SPDPReader->getAttributes()->unicastLocatorList;
    m_participantProxies.push_back(new ParticipantProxyData());
    initializeParticipantProxyData(discovery_attr, m_participantProxies.front());

    //INIT EDP
    if(discovery_attr.use_STATIC_EndpointDiscoveryProtocol)
    {
        mp_EDP = (EDP*)(new EDPStatic(this,mp_RTPSParticipant));
        if(!mp_EDP->initEDP(discovery_attr)){
            logError(RTPS_PDP,"Endpoint discovery configuration failed");
            return false;
        }

    }
    else if(discovery_attr.use_SIMPLE_EndpointDiscoveryProtocol)
    {
        mp_EDP = (EDP*)(new EDPSimple(this,mp_RTPSParticipant));
        if(!mp_EDP->initEDP(discovery_attr)){
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

    mp_resendParticipantTimer = new ResendParticipantProxyDataPeriod(this,TimeConv::Time_t2MilliSecondsDouble(discovery_attr.leaseDuration_announcementperiod));

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
            ParticipantProxyData* local_participant_data = getLocalParticipantProxyData();
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
                rdata.copy(*rit);
                pdata.copy(**pit);
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
                wdata.copy(*wit);
                pdata.copy(**pit);
                return true;
            }
        }
    }
    return false;
}

bool PDPSimple::removeReaderProxyData(const GUID_t& reader_guid)
{
    logInfo(RTPS_PDP, "Removing reader proxy data " << reader_guid);
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);

    for (auto pit = m_participantProxies.begin();
            pit != m_participantProxies.end(); ++pit)
    {
        for (auto rit = (*pit)->m_readers.begin();
                rit != (*pit)->m_readers.end(); ++rit)
        {
            if((*rit)->guid() == reader_guid)
            {
                mp_EDP->unpairReaderProxy((*pit)->m_guid, reader_guid);
                delete *rit;
                (*pit)->m_readers.erase(rit);
                return true;
            }
        }
    }

    return false;
}

bool PDPSimple::removeWriterProxyData(const GUID_t& writer_guid)
{
    logInfo(RTPS_PDP, "Removing writer proxy data " << writer_guid);
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);

    for (auto pit = m_participantProxies.begin();
            pit != m_participantProxies.end(); ++pit)
    {
        for (auto wit = (*pit)->m_writers.begin();
                wit != (*pit)->m_writers.end(); ++wit)
        {
            if((*wit)->guid() == writer_guid)
            {
                mp_EDP->unpairWriterProxy((*pit)->m_guid, writer_guid);
                delete *wit;
                (*pit)->m_writers.erase(wit);
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
    for(std::vector<ParticipantProxyData*>::iterator pit = m_participantProxies.begin();
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

bool PDPSimple::createSPDPEndpoints(BuiltinAttributes& discovery_attr)
{
    logInfo(RTPS_PDP,"Beginning");
    //SPDP BUILTIN RTPSParticipant WRITER
    HistoryAttributes hatt;
    hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
    hatt.initialReservedCaches = 20;
    hatt.maximumReservedCaches = 100;
    hatt.memoryPolicy = mp_builtin->m_att.writerHistoryMemoryPolicy;
    mp_SPDPWriterHistory = new WriterHistory(hatt);
    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    watt.endpoint.reliabilityKind = BEST_EFFORT;
    watt.endpoint.topicKind = WITH_KEY;
    if(mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
            mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
        watt.mode = ASYNCHRONOUS_WRITER;
    RTPSWriter* wout;
    if(mp_RTPSParticipant->createWriter(&wout,watt,mp_SPDPWriterHistory,nullptr,c_EntityId_SPDPWriter,true))
    {
#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(wout, false);
#endif
        mp_SPDPWriter = dynamic_cast<StatelessWriter*>(wout);
        for(LocatorListIterator lit = discovery_attr.initialPeersList.begin();
                lit != discovery_attr.initialPeersList.end(); ++lit)
        {
            mp_SPDPWriter->add_locator(*lit);
        }
    }
    else
    {
        logError(RTPS_PDP,"SimplePDP Writer creation failed");
        delete(mp_SPDPWriterHistory);
        mp_SPDPWriterHistory = nullptr;
        return false;
    }
    hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
    hatt.initialReservedCaches = 250;
    hatt.maximumReservedCaches = 5000;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    mp_SPDPReaderHistory = new ReaderHistory(hatt);
    ReaderAttributes ratt;
    ratt.endpoint.multicastLocatorList = discovery_attr.metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = discovery_attr.metatrafficUnicastLocatorList;
    ratt.endpoint.topicKind = WITH_KEY;
    ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    ratt.endpoint.reliabilityKind = BEST_EFFORT;
    mp_listener = new PDPSimpleListener(this);
    RTPSReader* rout;
