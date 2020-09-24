// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPServer2.cpp
 *
 */

#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/liveliness/WLP.h>

#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastdds/rtps/reader/StatefulReader.h>
#include <fastdds/rtps/writer/StatefulWriter.h>

#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/history/ReaderHistory.h>

#include <fastrtps/utils/TimeConversion.h>

#include <rtps/builtin/discovery/participant/DirectMessageSender.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>

#include <fastdds/dds/log/Log.hpp>

#include "./PDPServer2.hpp"
#include "./PDPServerListener2.hpp"
#include "./DServerEvent2.hpp"
#include "../endpoint/EDPServer2.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace eprosima::fastrtps::rtps;

PDPServer2::PDPServer2(
        BuiltinProtocols* builtin,
        const RTPSParticipantAllocationAttributes& allocation)
    : PDP(builtin, allocation)
    , mp_sync(nullptr)
{
}

PDPServer2::~PDPServer2()
{
    delete(mp_sync);
}

bool PDPServer2::init(
        RTPSParticipantImpl* part)
{
    if (!PDP::initPDP(part))
    {
        return false;
    }

    //INIT EDP
    mp_EDP = new EDPServer2(this, mp_RTPSParticipant);
    if (!mp_EDP->initEDP(m_discovery))
    {
        logError(RTPS_PDP, "Endpoint discovery configuration failed");
        return false;
    }

    /*
        Given the fact that a participant is either a client or a server the
        discoveryServer_client_syncperiod parameter has a context defined meaning.
     */
    mp_sync = new DServerEvent2(this,
                    TimeConv::Duration_t2MilliSecondsDouble(m_discovery.discovery_config.
                    discoveryServer_client_syncperiod));
    awakeServerThread();

    return true;
}

ParticipantProxyData* PDPServer2::createParticipantProxyData(
        const ParticipantProxyData& participant_data,
        const GUID_t& writer_guid)
{
    std::lock_guard<std::recursive_mutex> lock(*getMutex());

    // lease duration is controlled for owned clients or linked servers
    // other clients liveliness is provided through server's PDP discovery data

    // check if the DATA msg is relayed by another server
    bool do_lease = participant_data.m_guid.guidPrefix == writer_guid.guidPrefix;

    if (!do_lease)
    {
        // if not a client verify this participant is a server
        for (auto& svr : mp_builtin->m_DiscoveryServers)
        {
            if (svr.guidPrefix == participant_data.m_guid.guidPrefix)
            {
                do_lease = true;
            }
        }
    }

    ParticipantProxyData* pdata = add_participant_proxy_data(participant_data.m_guid, do_lease);
    if (pdata != nullptr)
    {
        pdata->copy(participant_data);
        pdata->isAlive = true;
        if (do_lease)
        {
            pdata->lease_duration_event->update_interval(pdata->m_leaseDuration);
            pdata->lease_duration_event->restart_timer();
        }
    }

    return pdata;
}

bool PDPServer2::createPDPEndpoints()
{
    logInfo(RTPS_PDP, "Beginning PDPServer Endpoints creation");

    /***********************************
    * PDP READER
    ***********************************/
    // PDP Reader History
    HistoryAttributes hatt;
    hatt.payloadMaxSize = mp_builtin->m_att.readerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    mp_PDPReaderHistory = new ReaderHistory(hatt);

    // PDP Reader Attributes
    ReaderAttributes ratt;
    ratt.expectsInlineQos = false;
    ratt.endpoint.endpointKind = READER;
    ratt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    ratt.endpoint.topicKind = WITH_KEY;
    ratt.endpoint.durabilityKind = VOLATILE;
    ratt.endpoint.reliabilityKind = RELIABLE;
    ratt.times.heartbeatResponseDelay = pdp_heartbeat_response_delay;

    // PDP Listener
    mp_listener = new PDPServerListener2(this);

    // Create PDP Reader
    if (mp_RTPSParticipant->createReader(&mp_PDPReader, ratt, mp_PDPReaderHistory,
            mp_listener, c_EntityId_SPDPReader, true, false))
    {
        // Enable unknown clients to reach this reader
        mp_PDPReader->enableMessagesFromUnkownWriters(true);
    }
    // Could not create PDP Reader, so return false
    else
    {
        logError(RTPS_PDP, "PDPServer Reader creation failed");
        delete(mp_PDPReaderHistory);
        mp_PDPReaderHistory = nullptr;
        delete(mp_listener);
        mp_listener = nullptr;
        return false;
    }

    /***********************************
    * PDP WRITER
    ***********************************/

    // PDP Writer History
    hatt.payloadMaxSize = mp_builtin->m_att.writerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.writerHistoryMemoryPolicy;
    mp_PDPWriterHistory = new WriterHistory(hatt);

    // PDP Writer Attributes
    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.durabilityKind = VOLATILE;
    watt.endpoint.reliabilityKind = RELIABLE;
    watt.endpoint.topicKind = WITH_KEY;
    watt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    watt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    watt.times.heartbeatPeriod = pdp_heartbeat_period;
    watt.times.nackResponseDelay = pdp_nack_response_delay;
    watt.times.nackSupressionDuration = pdp_nack_supression_duration;
    watt.mode = ASYNCHRONOUS_WRITER;

    // Create PDP Writer
    if (mp_RTPSParticipant->createWriter(&mp_PDPWriter, watt, mp_PDPWriterHistory,
            nullptr, c_EntityId_SPDPWriter, true))
    {
        // Set pdp filter to writer
        IReaderDataFilter* pdp_filter = static_cast<ddb::PDPDataFilter<ddb::DiscoveryDataBase>*>(&discovery_db);
        static_cast<StatefulWriter*>(mp_PDPWriter)->reader_data_filter(pdp_filter);
        // Enable separate sending so the filter can be called for each change and reader proxy
        mp_PDPWriter->set_separate_sending(true);
    }
    // Could not create PDP Writer, so return false
    else
    {
        logError(RTPS_PDP, "PDPServer Writer creation failed");
        delete(mp_PDPWriterHistory);
        mp_PDPWriterHistory = nullptr;
        return false;
    }
    logInfo(RTPS_PDP, "PDPServer Endpoints creation finished");
    return true;
}

void PDPServer2::initializeParticipantProxyData(
        ParticipantProxyData* participant_data)
{
    PDP::initializeParticipantProxyData(participant_data);

    if (!(getRTPSParticipant()->getAttributes().builtin.discovery_config.discoveryProtocol !=
            DiscoveryProtocol_t::CLIENT))
    {
        logError(RTPS_PDP, "Using a PDP Server object with another user's settings");
    }

    // A PDP server should always be provided with all EDP endpoints
    // because it must relay all clients EDP info
    participant_data->m_availableBuiltinEndpoints
        |= DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER
            | DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR
            | DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR
            | DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;

    const SimpleEDPAttributes& se = getRTPSParticipant()->getAttributes().builtin.discovery_config.m_simpleEDP;

    if (!(se.use_PublicationWriterANDSubscriptionReader && se.use_PublicationReaderANDSubscriptionWriter))
    {
        logWarning(RTPS_PDP, "SERVER or BACKUP PDP requires always all EDP endpoints creation.");
    }
}

void PDPServer2::assignRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    (void)pdata;
    // TODO DISCOVERY SERVER VERSION 2
}

void PDPServer2::notifyAboveRemoteEndpoints(
        const ParticipantProxyData& pdata)
{
    (void)pdata;
    // TODO DISCOVERY SERVER VERSION 2

    // No EDP notification needed. EDP endpoints would be match when PDP synchronization is granted
    if (mp_builtin->mp_WLP != nullptr)
    {
        mp_builtin->mp_WLP->assignRemoteEndpoints(pdata);
    }
}

void PDPServer2::removeRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    (void)pdata;
    // TODO DISCOVERY SERVER VERSION 2
}

#if HAVE_SQLITE3
std::string PDPServer2::GetPersistenceFileName()
{
    assert(getRTPSParticipant());

    std::ostringstream filename(std::ios_base::ate);
    std::string prefix;

    // . is not suitable separator for filenames
    filename << "server-" << getRTPSParticipant()->getGuid().guidPrefix;
    prefix = filename.str();
    std::replace(prefix.begin(), prefix.end(), '.', '-');
    filename.str(std::move(prefix));
    filename << ".db";

    return filename.str();
}

#endif // HAVE_SQLITE3

void PDPServer2::announceParticipantState(
        bool new_change,
        bool dispose /* = false */,
        WriteParams& )
{
    (void)new_change;
    (void)dispose;
    // TODO DISCOVERY SERVER VERSION 2
}

/**
 * This method removes a remote RTPSParticipant and all its writers and readers.
 * @param participant_guid GUID_t of the remote RTPSParticipant.
 * @param reason Why the participant is being removed (dropped vs removed)
 * @return true if correct.
 */
bool PDPServer2::remove_remote_participant(
        const GUID_t& partGUID,
        ParticipantDiscoveryInfo::DISCOVERY_STATUS reason)
{
    (void)partGUID;
    (void)reason;
    // TODO DISCOVERY SERVER VERSION 2
    return true;
}

bool PDPServer2::process_data_queue()
{
    return discovery_db.process_data_queue();
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
