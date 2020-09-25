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

#include <fastrtps/utils/TimedMutex.hpp>

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
        IReaderDataFilter* pdp_filter = static_cast<ddb::PDPDataFilter<ddb::DiscoveryDataBase>*>(&discovery_db_);
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
    return discovery_db_.process_data_queue();
}

bool PDPServer2::server_update_routine()
{
    bool result = process_writers_acknowledgements();  // server + ddb(functor_with_ddb)
    process_data_queue();                              // all ddb
    result |= process_disposals();                     // server + ddb(get_disposals, clear_changes_to_disposes)
    result |= process_dirty_topics();                  // all ddb
    result |= process_to_send_lists();                 // server + ddb(get_to_send, remove_to_send_this)

    if (result)
    {
        awakeServerThread();
    }
    return result;
}

bool PDPServer2::process_writers_acknowledgements()
{
    /* PDP Writer's History */
    bool pending = process_history_acknowledgement(
        static_cast<fastrtps::rtps::StatefulWriter*>(mp_PDPWriter), mp_PDPWriterHistory);

    /* EDP Publications Writer's History */
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    pending |= process_history_acknowledgement(edp->publications_writer_.first, edp->publications_writer_.second);

    /* EDP Subscriptions Writer's History */
    pending |= process_history_acknowledgement(edp->subscriptions_writer_.first, edp->subscriptions_writer_.second);

    return pending;
}

bool PDPServer2::process_history_acknowledgement(
        fastrtps::rtps::StatefulWriter* writer,
        fastrtps::rtps::WriterHistory* writer_history)
{
    bool pending = false;
    std::unique_lock<fastrtps::RecursiveTimedMutex> lock(writer->getMutex());
    // Iterate over changes in writer's history
    for (auto chit = writer_history->changesRbegin(); chit != writer_history->changesRend(); chit++)
    {
        pending |= process_change_acknowledgement(
            *chit,
            writer,
            writer_history);
    }
    return pending;
}

bool PDPServer2::process_change_acknowledgement(
        fastrtps::rtps::CacheChange_t* c,
        fastrtps::rtps::StatefulWriter* writer,
        fastrtps::rtps::WriterHistory* writer_history)
{
    // DATA(p|w|r) case
    if (c->kind == fastrtps::rtps::ChangeKind_t::ALIVE)
    {
        // Call to `StatefulWriter::for_each_reader_proxy()`. This will update
        // `participants_|writers_|readers_[guid_prefix]::relevant_participants_builtin_ack_status`, and will also set
        // `pending` to whether the change is has been acknowledged by all readers.
        fastdds::rtps::ddb::DiscoveryDataBase::AckedFunctor func = discovery_db_.functor(c);
        writer->for_each_reader_proxy(c, func);

        // If the change has been acknowledge by everyone
        if (!func.pending())
        {
            // Remove the entry from writer history, but do not release the cache.
            // This CacheChange will only be released in the case that is substituted by a DATA(Up|Uw|Ur).
            writer_history->remove_change_and_reuse(c->sequenceNumber);
            return false;
        }
    }
    // DATA(Up|Uw|Ur) case. Currently, Fast DDS only considers CacheChange kinds ALIVE and NOT_ALIVE_DISPOSED, so if the
    // kind is not ALIVE, then we know is NOT_ALIVE_DISPOSED and so a DATA(Up|Uw|Ur). In the future we should handle the
    // cases where kind is NOT_ALIVE_UNREGISTERED or NOT_ALIVE_DISPOSED_UNREGISTERED.
    else
    {
        // If `StatefulWriter::is_acked_by_all()`:
        if (writer->is_acked_by_all(c))
        {
            // Remove entry from `participants_|writers_|readers_`
            discovery_db_.delete_entity_of_change(c);
            // Remove from writer's history
            writer_history->remove_change(c->sequenceNumber);
            return false;
        }
    }
    return true;
}

bool PDPServer2::process_disposals()
{
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    fastrtps::rtps::WriterHistory* pubs_history = edp->publications_writer_.second;
    fastrtps::rtps::WriterHistory* subs_history = edp->subscriptions_writer_.second;

    // Get list of disposals from database
    std::vector<fastrtps::rtps::CacheChange_t*> disposals = discovery_db_.changes_to_dispose();
    // Iterate over disposals
    for (auto change: disposals)
    {
        // No check is performed on whether the change is an actual disposal, leaving the responsability of correctly
        // populating the disposals list to discovery_db_.process_data_queue().

        // Get the identity of the participant from which the change came.
        fastrtps::rtps::GuidPrefix_t change_guid_prefix = discovery_db_.guid_from_change(change).guidPrefix;

        // DATA(Up) case
        if (discovery_db_.is_participant(change))
        {
            // Lock PDP writer
            std::unique_lock<fastrtps::RecursiveTimedMutex> lock(mp_PDPWriter->getMutex());

            // Remove all DATA(p) with the same sample identity as the DATA(Up) from PDP writer's history.
            remove_related_alive_from_history_nts(mp_PDPWriterHistory, change_guid_prefix);

            // Add DATA(Up) to PDP writer's history
            mp_PDPWriterHistory->add_change(change);
        }
        // DATA(Uw) case
        else if (discovery_db_.is_writer(change))
        {
            // Lock EDP publications writer
            std::unique_lock<fastrtps::RecursiveTimedMutex> lock(edp->publications_writer_.first->getMutex());

            // Remove all DATA(w) with the same sample identity as the DATA(Uw) from EDP publications writer's history
            remove_related_alive_from_history_nts(pubs_history, change_guid_prefix);

            // Check whether disposals contains a DATA(Up) from the same participant as the DATA(Uw).
            // If it does, then there is no need of adding the DATA(Uw).
            if (!announcement_from_same_participant_in_disposals(disposals, change_guid_prefix))
            {
                // Add DATA(Uw) to EDP publications writer's history.
                pubs_history->add_change(change);
            }
        }
        // DATA(Ur) case
        else if (discovery_db_.is_reader(change))
        {
            // Lock EDP subscriptions writer
            std::unique_lock<fastrtps::RecursiveTimedMutex> lock(edp->subscriptions_writer_.first->getMutex());

            // Remove all DATA(r) with the same sample identity as the DATA(Ur) from EDP subscriptions writer's history
            remove_related_alive_from_history_nts(subs_history, change_guid_prefix);

            // Check whether disposals contains a DATA(Up) from the same participant as the DATA(Ur).
            // If it does, then there is no need of adding the DATA(Ur).
            if (!announcement_from_same_participant_in_disposals(disposals, change_guid_prefix))
            {
                // Add DATA(Ur) to EDP subscriptions writer's history.
                subs_history->add_change(change);
            }
        }
        else
        {
            logError(PDPServer2, "Wrong DATA received from disposals");
        }
    }
    // Clear database disposals list
    discovery_db_.clear_changes_to_dispose();
    return false;
}

void PDPServer2::remove_related_alive_from_history_nts(
        fastrtps::rtps::WriterHistory* writer_history,
        const fastrtps::rtps::GuidPrefix_t& entity_guid_prefix)
{
    // Iterate over changes in writer_history
    for (auto chit = writer_history->changesBegin(); chit != writer_history->changesEnd(); chit++)
    {
        // Remove all DATA whose original sender was entity_guid_prefix from writer_history
        if (entity_guid_prefix == discovery_db_.guid_from_change(*chit).guidPrefix)
        {
            writer_history->remove_change(*chit);
        }
    }
}

bool PDPServer2::announcement_from_same_participant_in_disposals(
        const std::vector<fastrtps::rtps::CacheChange_t*>& disposals,
        const fastrtps::rtps::GuidPrefix_t& participant)
{
    for (auto change_: disposals)
    {
        if (discovery_db_.is_participant(change_) &&
                (discovery_db_.guid_from_change(change_).guidPrefix == participant))
        {
            return true;
        }
    }
    return false;
}

bool PDPServer2::process_dirty_topics()
{
    return discovery_db_.process_dirty_topics();
}

fastdds::rtps::ddb::DiscoveryDataBase& PDPServer2::discovery_db()
{
    return discovery_db_;
}

bool PDPServer2::process_to_send_lists()
{
    // Process pdp_to_send_
    process_to_send_list(discovery_db_.pdp_to_send(), mp_PDPWriter, mp_PDPWriterHistory);
    discovery_db_.clear_pdp_to_send();

    // Process edp_publications_to_send_
    EDPServer2* edp = static_cast<EDPServer2*>(mp_EDP);
    process_to_send_list(
        discovery_db_.edp_publications_to_send(),
        edp->publications_writer_.first,
        edp->publications_writer_.second);
    discovery_db_.clear_edp_publications_to_send();

    // Process edp_subscriptions_to_send_
    process_to_send_list(
        discovery_db_.edp_subscriptions_to_send(),
        edp->subscriptions_writer_.first,
        edp->subscriptions_writer_.second);
    discovery_db_.clear_edp_subscriptions_to_send();

    return false;
}

bool PDPServer2::process_to_send_list(
        const std::vector<eprosima::fastrtps::rtps::CacheChange_t*>& send_list,
        fastrtps::rtps::RTPSWriter* writer,
        fastrtps::rtps::WriterHistory* history)
{
    // Iterate over DATAs in send_list
    std::unique_lock<fastrtps::RecursiveTimedMutex> lock(writer->getMutex());
    for (auto change: send_list)
    {
        // If the DATA is already in the writer's history, then remove it.
        remove_change_from_history_nts(history, change);
        // Add DATA to writer's history.
        history->add_change(change);
    }
    return true;
}

bool PDPServer2::remove_change_from_history_nts(
        fastrtps::rtps::WriterHistory* history,
        fastrtps::rtps::CacheChange_t* change)
{
    for (auto chit = history->changesRbegin(); chit != history->changesRend(); chit++)
    {
        if (change->instanceHandle == (*chit)->instanceHandle)
        {
            history->remove_change(*chit);
            return true;
        }
    }
    return false;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
