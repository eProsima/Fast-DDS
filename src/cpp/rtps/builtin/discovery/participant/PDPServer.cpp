// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPServer.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/participant/PDPServer.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPListener.h>
#include <fastrtps/rtps/builtin/BuiltinProtocols.h>

#include <fastrtps/rtps/participant/RTPSParticipantListener.h>

#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/reader/RTPSReader.h>

#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastrtps/log/Log.h>


using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

// Default configuration values for PDP reliable entities.
const Duration_t pdp_heartbeat_period{ 1, 0 }; // 1 second
const Duration_t pdp_nack_response_delay{ 0, 400 * 1000 * 1000 }; // ~93 milliseconds
const Duration_t pdp_nack_supression_duration{ 0, 50 * 1000 * 1000 }; // ~11 milliseconds
const Duration_t pdp_heartbeat_response_delay{ 0, 50 * 1000 * 1000 }; // ~11 milliseconds

const int32_t pdp_initial_reserved_caches = 20;


GUID_t RemoteServerAttributes::GetPDPReader() const
{
    return GUID_t(guidPrefix, c_EntityId_SPDPReader);
}

GUID_t RemoteServerAttributes::GetPDPWriter() const
{
    return GUID_t(guidPrefix, c_EntityId_SPDPWriter);
}

GUID_t RemoteServerAttributes::GetEDPPublicationsReader() const
{
    return GUID_t(guidPrefix, c_EntityId_SEDPPubReader);
}

GUID_t RemoteServerAttributes::GetEDPSubscriptionsWriter() const
{
    return GUID_t(guidPrefix, c_EntityId_SEDPSubWriter);
}

GUID_t RemoteServerAttributes::GetEDPPublicationsWriter() const
{
    return GUID_t(guidPrefix, c_EntityId_SEDPPubWriter);
}

GUID_t RemoteServerAttributes::GetEDPSubscriptionsReader() const
{
    return GUID_t(guidPrefix, c_EntityId_SEDPSubReader);
}

PDPServer::PDPServer(BuiltinProtocols* built):
    PDP(built)
    {

    }

PDPServer::~PDPServer()
{

}

void PDPServer::initializeParticipantProxyData(ParticipantProxyData* participant_data)
{
    PDP::initializeParticipantProxyData(participant_data); // TODO: Remember that the PDP version USES security

    if(!getRTPSParticipant()->getAttributes().builtin.use_SERVER_DiscoveryProtocol)
    {
        logError(RTPS_PDP, "Using a PDP Server object with another user's settings");
    }

    if (getRTPSParticipant()->getAttributes().builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
    {
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
    }

    if (getRTPSParticipant()->getAttributes().builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
    {
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
    }

//#if HAVE_SECURITY
//    if (getRTPSParticipant()->getAttributes().builtin.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader)
//    {
//        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
//        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
//    }
//
//    if (getRTPSParticipant()->getAttributes().builtin.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader)
//    {
//        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
//        participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
//    }
//#endif

}

bool PDPServer::initPDP(RTPSParticipantImpl* part)
{
    if (!PDP::initPDP(part))
    {
        return false;
    }

    //INIT EDP
    mp_EDP = (EDP*)(new EDPSimple(this,mp_RTPSParticipant));
    if(!mp_EDP->initEDP(m_discovery))
    {
        logError(RTPS_PDP,"Endpoint discovery configuration failed");
        return false;
    }
 
    return true;
}


bool PDPServer::createPDPEndpoints()
{
    logInfo(RTPS_PDP,"Beginning");

    HistoryAttributes hatt;
    hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    mp_PDPReaderHistory = new ReaderHistory(hatt);

    ReaderAttributes ratt;
    ratt.expectsInlineQos = false;
    ratt.endpoint.endpointKind = READER;
    ratt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    ratt.endpoint.topicKind = WITH_KEY;
    ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    ratt.endpoint.reliabilityKind = RELIABLE;
    ratt.times.heartbeatResponseDelay = pdp_heartbeat_response_delay;

    mp_listener = new PDPListener(this); // TODO: Create a specific listener for Server Client

    if (mp_RTPSParticipant->createReader(&mp_PDPReader, ratt, mp_PDPReaderHistory, mp_listener, c_EntityId_SPDPReader, true, false)) //TODO: verify that enable = false (the framework should attach receiver resources later)
    {
//#if HAVE_SECURITY
//        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(rout, false);
//#endif
        // Initial peer list doesn't make sense in server scenario. Client should match its server list
        for (auto it = mp_builtin->m_DiscoveryServers.begin(); it != mp_builtin->m_DiscoveryServers.end(); ++it)
        {
            RemoteWriterAttributes rwatt;

            rwatt.guid = it->GetPDPWriter();
            rwatt.endpoint.multicastLocatorList.push_back(it->metatrafficMulticastLocatorList);
            rwatt.endpoint.unicastLocatorList.push_back(it->metatrafficUnicastLocatorList);
            rwatt.endpoint.topicKind = WITH_KEY;
            rwatt.endpoint.durabilityKind = TRANSIENT_LOCAL;
            rwatt.endpoint.reliabilityKind = RELIABLE;

            mp_PDPReader->matched_writer_add(rwatt);
        }
       
    }
    else
    {
        logError(RTPS_PDP, "PDPServer Reader creation failed");
        delete(mp_PDPReaderHistory);
        mp_PDPReaderHistory = nullptr;
        delete(mp_listener);
        mp_listener = nullptr;
        return false;
    }

    hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.writerHistoryMemoryPolicy;
    mp_PDPWriterHistory = new WriterHistory(hatt);

    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    watt.endpoint.reliabilityKind = RELIABLE;
    watt.endpoint.topicKind = WITH_KEY;
    watt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    watt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    watt.times.heartbeatPeriod = pdp_heartbeat_period;
    watt.times.nackResponseDelay = pdp_nack_response_delay;
    watt.times.nackSupressionDuration = pdp_nack_supression_duration;

    if (mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
        mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
    {
        watt.mode = ASYNCHRONOUS_WRITER;
    }

    if (mp_RTPSParticipant->createWriter(&mp_PDPWriter, watt, mp_PDPWriterHistory, nullptr, c_EntityId_SPDPWriter, true)) //TODO: verify that enable = false (the framework should attach receiver resources later)
    {
//#if HAVE_SECURITY
//        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(wout, false);
//#endif
        for (auto it = mp_builtin->m_DiscoveryServers.begin(); it != mp_builtin->m_DiscoveryServers.end(); ++it)
        {
            RemoteReaderAttributes rratt;

            rratt.guid = it->GetPDPReader();
            rratt.endpoint.multicastLocatorList.push_back(it->metatrafficMulticastLocatorList);
            rratt.endpoint.unicastLocatorList.push_back(it->metatrafficUnicastLocatorList);
            rratt.endpoint.topicKind = WITH_KEY;
            rratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
            rratt.endpoint.reliabilityKind = RELIABLE;

            mp_PDPWriter->matched_reader_add(rratt);
        }
        
    }
    else
    {
        logError(RTPS_PDP, "PDPServer Writer creation failed");
        delete(mp_PDPWriterHistory);
        mp_PDPWriterHistory = nullptr;
        return false;
    }
    logInfo(RTPS_PDP,"PDP Endpoints creation finished");
    return true;
}

void PDPServer::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
    (void)pdata;
    // Clients only exchange metatraffic with servers
}


void PDPServer::removeRemoteEndpoints(ParticipantProxyData* pdata)
{
    (void)pdata;
    // Clients only exchange metatraffic with servers
}


} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
