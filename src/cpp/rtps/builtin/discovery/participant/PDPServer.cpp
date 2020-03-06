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

#include <fastdds/rtps/builtin/discovery/participant/timedevent/DServerEvent.h>
#include <fastdds/rtps/builtin/discovery/participant/PDPServerListener.h>
#include <fastdds/rtps/builtin/discovery/participant/PDPServer.h>
#include <fastdds/rtps/builtin/discovery/endpoint/EDPServer.h>


#include <fastdds/rtps/writer/ReaderProxy.h>

#include <algorithm>
#include <forward_list>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps {
namespace rtps {

PDPServer::PDPServer(
        BuiltinProtocols* builtin,
        const RTPSParticipantAllocationAttributes& allocation,
        DurabilityKind_t durability_kind)
    : PDP(builtin, allocation)
    , _durability(durability_kind)
    , mp_sync(nullptr)
    , PDP_callback_(false)
{

}

PDPServer::~PDPServer()
{
    delete(mp_sync);
}

bool PDPServer::init(
        RTPSParticipantImpl* part)
{
    if (!PDP::initPDP(part))
    {
        return false;
    }

    //INIT EDP
    mp_EDP = new EDPServer(this, mp_RTPSParticipant, _durability);
    if (!mp_EDP->initEDP(m_discovery))
    {
        logError(RTPS_PDP, "Endpoint discovery configuration failed");
        return false;
    }

    /*
        Given the fact that a participant is either a client or a server the
        discoveryServer_client_syncperiod parameter has a context defined meaning.
     */
    mp_sync = new DServerEvent(this,
                    TimeConv::Duration_t2MilliSecondsDouble(m_discovery.discovery_config.
                    discoveryServer_client_syncperiod));
    awakeServerThread();
    // the timer is also restart from removeRemoteParticipant, remove(Publisher|Subscriber)FromHistory
    // and queueParticipantForEDPMatch

    return true;
}

ParticipantProxyData* PDPServer::createParticipantProxyData(
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

bool PDPServer::createPDPEndpoints()
{
    logInfo(RTPS_PDP, "Beginning PDPServer Endpoints creation");

    const NetworkFactory& network = mp_RTPSParticipant->network_factory();

    HistoryAttributes hatt;
    hatt.payloadMaxSize = mp_builtin->m_att.readerPayloadSize;
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

    mp_listener = new PDPServerListener(this);

    if (mp_RTPSParticipant->createReader(&mp_PDPReader, ratt, mp_PDPReaderHistory,
            mp_listener, c_EntityId_SPDPReader, true, false))
    {
        // enable unknown clients to reach this reader
        mp_PDPReader->enableMessagesFromUnkownWriters(true);

        std::lock_guard<std::mutex> data_guard(temp_data_lock_);
        for (const RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
        {
            temp_writer_data_.clear();
            temp_writer_data_.guid(it.GetPDPWriter());
            temp_writer_data_.set_multicast_locators(it.metatrafficMulticastLocatorList, network);
            temp_writer_data_.set_remote_unicast_locators(it.metatrafficUnicastLocatorList, network);
            temp_writer_data_.m_qos.m_durability.durabilityKind(_durability);
            temp_writer_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            mp_PDPReader->matched_writer_add(temp_writer_data_);
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

    hatt.payloadMaxSize = mp_builtin->m_att.writerPayloadSize;
    hatt.initialReservedCaches = pdp_initial_reserved_caches;
    hatt.memoryPolicy = mp_builtin->m_att.writerHistoryMemoryPolicy;
    mp_PDPWriterHistory = new WriterHistory(hatt);

    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.durabilityKind = _durability;

#if HAVE_SQLITE3
    watt.endpoint.properties.properties().push_back(Property("dds.persistence.plugin", "builtin.SQLITE3"));
    watt.endpoint.properties.properties().push_back(Property("dds.persistence.sqlite3.filename",
            GetPersistenceFileName()));
#endif

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

    if (mp_RTPSParticipant->createWriter(&mp_PDPWriter, watt, mp_PDPWriterHistory,
            nullptr, c_EntityId_SPDPWriter, true))
    {
        std::lock_guard<std::mutex> data_guard(temp_data_lock_);
        for (const RemoteServerAttributes& it : mp_builtin->m_DiscoveryServers)
        {
            temp_reader_data_.clear();
            temp_reader_data_.guid(it.GetPDPReader());
            temp_reader_data_.set_multicast_locators(it.metatrafficMulticastLocatorList, network);
            temp_reader_data_.set_remote_unicast_locators(it.metatrafficUnicastLocatorList, network);
            temp_reader_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            temp_reader_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

            mp_PDPWriter->matched_reader_add(temp_reader_data_);
        }

    }
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

void PDPServer::initializeParticipantProxyData(
        ParticipantProxyData* participant_data)
{
    PDP::initializeParticipantProxyData(participant_data); // TODO: Remember that the PDP version USES security

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

void PDPServer::assignRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();

    {
        std::unique_lock<std::recursive_mutex> lock(*getMutex());

        // Verify if this participant is a server
        for (auto& svr : mp_builtin->m_DiscoveryServers)
        {
            if (svr.guidPrefix == pdata->m_guid.guidPrefix)
            {
                svr.proxy = pdata;

                lock.unlock();

                // servers are already match in PDPServer::createPDPEndpoints
                // Notify another endpoints
                notifyAboveRemoteEndpoints(*pdata);
                return;
            }
        }
    }

    // boilerplate, note that PDPSimple version doesn't use RELIABLE entities
    logInfo(RTPS_PDP, "For RTPSParticipant: " << pdata->m_guid.guidPrefix);

    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    if (auxendp != 0)
    {
        std::lock_guard<std::mutex> data_guard(temp_data_lock_);
        temp_writer_data_.clear();
        temp_writer_data_.guid().guidPrefix = pdata->m_guid.guidPrefix;
        temp_writer_data_.guid().entityId = c_EntityId_SPDPWriter;
        temp_writer_data_.persistence_guid(pdata->get_persistence_guid());
        temp_writer_data_.set_persistence_entity_id(c_EntityId_SPDPWriter);
        temp_writer_data_.set_remote_locators(pdata->metatraffic_locators, network, true);
        temp_writer_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        temp_writer_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;

        mp_PDPReader->matched_writer_add(temp_writer_data_);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    if (auxendp != 0)
    {
        std::lock_guard<std::mutex> data_guard(temp_data_lock_);
        temp_reader_data_.clear();
        temp_reader_data_.m_expectsInlineQos = false;
        temp_reader_data_.guid().guidPrefix = pdata->m_guid.guidPrefix;
        temp_reader_data_.guid().entityId = c_EntityId_SPDPReader;
        temp_reader_data_.set_remote_locators(pdata->metatraffic_locators, network, true);
        temp_reader_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        temp_reader_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;

        mp_PDPWriter->matched_reader_add(temp_reader_data_);

        // TODO: remove when the Writer API issue is resolved
        std::lock_guard<std::recursive_mutex> lock(*getMutex());
        // Waiting till we remove C++11 restriction:
        // clients_.insert_or_assign(temp_reader_data_.guid(), temp_reader_data_);
        auto emplace_result = clients_.emplace(temp_reader_data_.guid(), temp_reader_data_);
        if (!emplace_result.second)
        {
            emplace_result.first->second = temp_reader_data_;
        }
    }

    // Notify another endpoints
    notifyAboveRemoteEndpoints(*pdata);

}

void PDPServer::notifyAboveRemoteEndpoints(
        const ParticipantProxyData& pdata)
{
    // No EDP notification needed. EDP endpoints would be match when PDP synchronization is granted
    if (mp_builtin->mp_WLP != nullptr)
    {
        mp_builtin->mp_WLP->assignRemoteEndpoints(pdata);
    }
}

void PDPServer::removeRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    // EDP endpoints have been already unmatch by the associated listener
    assert(!mp_EDP->areRemoteEndpointsMatched(pdata));

    // Verify if this participant is a server
    bool is_server = false;
    {
        std::lock_guard<std::recursive_mutex> lock(*getMutex());

        for (RemoteServerAttributes& svr : mp_builtin->m_DiscoveryServers)
        {
            if (svr.guidPrefix == pdata->m_guid.guidPrefix)
            {
                svr.proxy = nullptr; // reasign when we receive again server DATA(p)
                is_server = true;
            }
        }
    }

    // Clients should be unmatch and
    // servers unmatch and match in order to renew its associated proxies
    logInfo(RTPS_PDP, "For unmatching for server: " << pdata->m_guid);
    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;

    if (auxendp != 0)
    {
        GUID_t wguid;

        wguid.guidPrefix = pdata->m_guid.guidPrefix;
        wguid.entityId = c_EntityId_SPDPWriter;

        safe_PDP_matched_writer_remove(wguid);

        /*
            When a server acts like a client to another server it should never
            stop receiving meta data from him
         */
        if (is_server)
        {
            std::lock_guard<std::mutex> data_guard(temp_data_lock_);
            temp_writer_data_.clear();
            temp_writer_data_.guid(wguid);
            temp_writer_data_.persistence_guid(temp_writer_data_.guid());
            temp_writer_data_.set_remote_locators(pdata->metatraffic_locators, network, true);
            temp_writer_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            temp_writer_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            mp_PDPReader->matched_writer_add(temp_writer_data_);
        }

    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

    if (auxendp != 0)
    {
        GUID_t rguid;
        rguid.guidPrefix = pdata->m_guid.guidPrefix;
        rguid.entityId = c_EntityId_SPDPReader;
        mp_PDPWriter->matched_reader_remove(rguid);

        /*
           When a server acts like a client to another server it should never
           stop sending meta data from him
         */
        if (is_server)
        {
            std::lock_guard<std::mutex> data_guard(temp_data_lock_);
            temp_reader_data_.clear();
            temp_reader_data_.m_expectsInlineQos = false;
            temp_reader_data_.guid(rguid);
            temp_reader_data_.set_remote_locators(pdata->metatraffic_locators, network, true);
            temp_reader_data_.m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            temp_reader_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            mp_PDPWriter->matched_reader_add(temp_reader_data_);
        }
        else
        {
            std::unique_lock<std::recursive_mutex> lock(*getMutex());

            // TODO: remove when the Writer API issue is resolved
            clients_.erase(rguid);
        }
    }

}

bool PDPServer::all_clients_acknowledge_PDP()
{
    // check if already initialized
    assert( mp_PDPWriter && dynamic_cast<StatefulWriter*>(mp_PDPWriter));

    return dynamic_cast<StatefulWriter*>(mp_PDPWriter)->all_readers_updated();
}

void PDPServer::match_all_clients_EDP_endpoints()
{
    // PDP must have been initialize
    assert(mp_EDP);

    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);

    if (!pendingEDPMatches())
    {
        return;
    }

    for (auto p: _p2match)
    {
        assert( p != nullptr);
        mp_EDP->assignRemoteEndpoints(*p);
    }

    _p2match.clear();
}

bool PDPServer::trimWriterHistory()
{
    assert(mp_mutex && mp_PDPWriter);
    std::lock_guard<std::recursive_mutex> guardP(*getMutex());

    EDPServer* pEDP = dynamic_cast<EDPServer*>(mp_EDP);
    assert(pEDP);

    bool istrim = true;

    istrim &= trimPDPWriterHistory();
    istrim &= pEDP->trimPUBWriterHistory();
    istrim &= pEDP->trimSUBWriterHistory();

    return istrim;
}

bool PDPServer::trimPDPWriterHistory()
{
    // trim demises container
    key_list disposal, aux;

    if (_demises.empty())
    {
        return true;
    }

    // sweep away any resurrected participant
    std::for_each(ParticipantProxiesBegin(), ParticipantProxiesEnd(),
            [&disposal](const ParticipantProxyData* pD) {
                    disposal.insert(pD->m_key);
                });
    std::set_difference(_demises.cbegin(), _demises.cend(), disposal.cbegin(), disposal.cend(),
            std::inserter(aux, aux.begin()));
    _demises.swap(aux);

    if (_demises.empty())
    {
        return true;
    }

    // traverse the WriterHistory searching CacheChanges_t with demised keys
    std::forward_list<CacheChange_t*> removal;
    std::lock_guard<RecursiveTimedMutex> guardW(mp_PDPWriter->getMutex());

    std::copy_if(mp_PDPWriterHistory->changesBegin(),
            mp_PDPWriterHistory->changesBegin(), std::front_inserter(removal),
            [this](const CacheChange_t* chan)
                {
                    return _demises.find(chan->instanceHandle) != _demises.cend();
                });

    if (removal.empty())
    {
        return true;
    }

    aux.clear();
    key_list& pending = aux;

    // remove outdate CacheChange_ts
    for (auto pC : removal)
    {
        if (mp_PDPWriter->is_acked_by_all(pC))
        {
            mp_PDPWriterHistory->remove_change(pC);
        }
        else
        {
            pending.insert(pC->instanceHandle);
        }
    }

    // update demises
    _demises.swap(pending);

    return _demises.empty(); // finish?
}

// CacheChange_t's ParticipantProxyData wouldn't be loaded when this function is called
bool PDPServer::addRelayedChangeToHistory(
        CacheChange_t& c)
{
    assert(mp_PDPWriter && c.serializedPayload.max_size);

    std::lock_guard<RecursiveTimedMutex> lock(mp_PDPWriter->getMutex());
    CacheChange_t* pCh = nullptr;

    // validate the sample, if no sample data update it
    WriteParams& wp = c.write_params;
    SampleIdentity& sid = wp.sample_identity();
    if (sid == SampleIdentity::unknown())
    {
        sid.writer_guid(c.writerGUID);
        sid.sequence_number(c.sequenceNumber);
        logError(RTPS_PDP,
                "A DATA(p) received by server " << mp_PDPWriter->getGuid()
                    << " from participant " << c.writerGUID
                    << " without a valid SampleIdentity");
    }

    if (wp.related_sample_identity() == SampleIdentity::unknown())
    {
        wp.related_sample_identity(sid);
    }

    // See if this sample is already in the cache.
    // TODO: Accelerate this search by using a PublisherHistory as mp_PDPWriterHistory
    auto it = std::find_if(
        mp_PDPWriterHistory->changesRbegin(),
        mp_PDPWriterHistory->changesRend(),
        [&sid] (CacheChange_t* c) {
                    return sid == c->write_params.sample_identity();
                });

    if (it == mp_PDPWriterHistory->changesRend())
    {
        if (mp_PDPWriterHistory->reserve_Cache(&pCh, c.serializedPayload.max_size) && pCh && pCh->copy(&c))
        {
            pCh->writerGUID = mp_PDPWriter->getGuid();
            // keep the original sample identity by using wp
            return mp_PDPWriterHistory->add_change(pCh, wp);
        }
    }
    return false;
}

// Always call after PDP proxies update
void PDPServer::removeParticipantFromHistory(
        const InstanceHandle_t& key)
{
    std::lock_guard<std::recursive_mutex> guardP(*mp_mutex);

    _demises.insert(key);
    trimWriterHistory();
}

void PDPServer::queueParticipantForEDPMatch(
        const ParticipantProxyData* pdata)
{
    assert(pdata != nullptr);

    std::lock_guard<std::recursive_mutex> guardP(*mp_mutex);

    // add the new client or server to the EDP matching list
    _p2match.insert(pdata);
    awakeServerThread();
    // the timer is also restart from removeRemoteParticipant, remove(Publisher|Subscriber)FromHistory
    // and initPDP

    logInfo(PDP_SERVER, "participant " << pdata->m_participantName << " prefix: " << pdata->m_guid
                                       << " waiting for EDP match with server "
                                       << getRTPSParticipant()->getRTPSParticipantAttributes().getName());
}

void PDPServer::removeParticipantForEDPMatch(
        const GUID_t& guid)
{
    std::lock_guard<std::recursive_mutex> guardP(*mp_mutex);

    // remove the deceased client to the EDP matching list
    for (pending_matches_list::iterator it = _p2match.begin(); it != _p2match.end(); ++it)
    {
        if ((*it)->m_guid == guid)
        {
            _p2match.erase(it);
            return;
        }
    }

}

#if HAVE_SQLITE3
std::string PDPServer::GetPersistenceFileName()
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
#endif

bool PDPServer::all_servers_acknowledge_PDP()
{
    // check if already initialized
    assert(mp_PDPWriterHistory && mp_PDPWriter);

    // First check if all servers have been discovered
    bool discovered = true;

    for (auto& s : mp_builtin->m_DiscoveryServers)
    {
        discovered &= (s.proxy != nullptr);
    }

    if (!discovered)
    {
        // The first change in the PDP WriterHistory is this server ParticipantProxyData
        // see BuiltinProtocols::initBuiltinProtocols call to PDPXXX::announceParticipantState(true)
        CacheChange_t* pPD;
        if (mp_PDPWriterHistory->get_min_change(&pPD))
        {
            // This answer includes also clients but is accurate enough
            return mp_PDPWriter->is_acked_by_all(pPD);
        }
        else
        {
            logError(RTPS_PDP,
                    "ParticipantProxy data should have been added to client PDP history cache by a previous call to announceParticipantState()");
            return false;
        }
    }

    return true;
}

bool PDPServer::is_all_servers_PDPdata_updated()
{
    StatefulReader* pR = dynamic_cast<StatefulReader*>(mp_PDPReader);
    assert(pR);

    // This answer includes also clients but is accurate enough
    return pR->isInCleanState();
}

bool PDPServer::match_servers_EDP_endpoints()
{
    std::lock_guard<std::recursive_mutex> lock(*getMutex());
    bool all = true; // have all servers been discovered?

    for (auto& svr : mp_builtin->m_DiscoveryServers)
    {
        all &= (svr.proxy != nullptr);

        if (svr.proxy && !mp_EDP->areRemoteEndpointsMatched(svr.proxy))
        {
            queueParticipantForEDPMatch(svr.proxy);
        }
    }

    return all;
}

void PDPServer::announceParticipantState(
        bool new_change,
        bool dispose /* = false */,
        WriteParams& )
{
    logInfo(RTPS_PDP, "Announcing RTPSParticipant State (new change: " << new_change << ")");

    StatefulWriter* pW = dynamic_cast<StatefulWriter*>(mp_PDPWriter);
    assert(pW);

    /*
       Protect writer sequence number. Make sure in order to prevent AB BA deadlock that the
       writer mutex is systematically lock before the PDP one (if needed):
        - transport callbacks on PDPListener
        - initialization and removal on BuiltinProtocols::initBuiltinProtocols and ~BuiltinProtocols
        - DSClientEvent (own thread)
        - ResendParticipantProxyDataPeriod (participant event thread)
     */
    std::lock_guard<RecursiveTimedMutex> wlock(pW->getMutex());

    // Servers only send direct DATA(p) to servers in order to allow discovery
    if (new_change)
    {
        // only builtinprotocols uses new_change = true, delegate in base class
        // in order to get the ParticipantProxyData into the WriterHistory and broadcast the first DATA(p)

        WriteParams wp;
        SampleIdentity local;
        local.writer_guid(mp_PDPWriter->getGuid());
        local.sequence_number(mp_PDPWriterHistory->next_sequence_number());
        wp.sample_identity(local);
        wp.related_sample_identity(local);

        if (!dispose)
        {
            PDP::announceParticipantState(new_change, dispose, wp);
        }
        else
        {   // we must assure when the server is dying that all client are send at least a DATA(p)
            // note here we can no longer receive and DATA or ACKNACK from clients.
            // In order to avoid that we send the message directly as in the standard stateless PDP

            CacheChange_t* change = nullptr;

            if ((change = pW->new_change(
                [this]() -> uint32_t
                {
                    return mp_builtin->m_att.writerPayloadSize;
                },
                NOT_ALIVE_DISPOSED_UNREGISTERED, getLocalParticipantProxyData()->m_key)))
            {
                // update the sequence number
                change->sequenceNumber = mp_PDPWriterHistory->next_sequence_number();
                change->write_params = wp;

                std::vector<GUID_t> remote_readers;
                LocatorList_t locators;

                // TODO: modify announcement mechanism to allow direct message sending
                //for (auto it = pW->matchedReadersBegin(); it != pW->matchedReadersEnd(); ++it)
                //{
                //    RemoteReaderAttributes & att = (*it)->m_att;
                //    remote_readers.push_back(att.guid);

                //    EndpointAttributes & ep = att.endpoint;
                //    locators.push_back(ep.unicastLocatorList);
                //    //locators.push_back(ep.multicastLocatorList);
                //}

                // TODO: remove when the Writer API issue is resolved
                std::lock_guard<std::recursive_mutex> lock(*getMutex());

                for (auto client : clients_)
                {
                    ReaderProxyData& rat = client.second;
                    remote_readers.push_back(rat.guid());
                    for (const Locator_t& loc : rat.remote_locators().unicast)
                    {
                        locators.push_back(loc);
                    }
                    // locators.push_back(rat.endpoint.multicastLocatorList);
                }

                for (auto& svr : mp_builtin->m_DiscoveryServers)
                {
                    if (svr.proxy != nullptr)
                    {
                        remote_readers.push_back(svr.GetPDPReader());
                        // locators.push_back(svr.metatrafficMulticastLocatorList);
                        locators.push_back(svr.metatrafficUnicastLocatorList);
                    }
                }

                DirectMessageSender sender(getRTPSParticipant(), &remote_readers, &locators);
                RTPSMessageGroup group(getRTPSParticipant(), mp_PDPWriter, sender);

                if (!group.add_data(*change, false))
                {
                    logError(RTPS_PDP, "Error sending announcement from server to clients");
                }
            }

            // free change
            mp_PDPWriterHistory->release_Cache(change);
        }

    }
    else
    {
        // retrieve the participant discovery data
        CacheChange_t* pPD;
        if (mp_PDPWriterHistory->get_min_change(&pPD))
        {
            std::lock_guard<std::recursive_mutex> lock(*getMutex());

            std::vector<GUID_t> remote_readers;
            LocatorList_t locators;

            // TODO: modify announcement mechanism to allow direct message sending
            //for (auto it = pW->matchedReadersBegin(); it != pW->matchedReadersEnd(); ++it)
            //{
            //    RemoteReaderAttributes & att = (*it)->m_att;
            //    remote_readers.push_back(att.guid);

            //    EndpointAttributes & ep = att.endpoint;
            //    locators.push_back(ep.unicastLocatorList);
            //    locators.push_back(ep.multicastLocatorList);
            //}

            // TODO: remove when the Writer API issue is resolved
            for (auto client : clients_)
            {
                ReaderProxyData& rat = client.second;
                remote_readers.push_back(rat.guid());
                for (const Locator_t& loc : rat.remote_locators().unicast)
                {
                    locators.push_back(loc);
                }
                for (const Locator_t& loc : rat.remote_locators().multicast)
                {
                    locators.push_back(loc);
                }
            }

            for (auto& svr : mp_builtin->m_DiscoveryServers)
            {
                if (svr.proxy == nullptr)
                {
                    remote_readers.push_back(svr.GetPDPReader());
                    locators.push_back(svr.metatrafficMulticastLocatorList);
                    locators.push_back(svr.metatrafficUnicastLocatorList);
                }
            }

            DirectMessageSender sender(getRTPSParticipant(), &remote_readers, &locators);
            RTPSMessageGroup group(getRTPSParticipant(), mp_PDPWriter, sender);

            if (!group.add_data(*pPD, false))
            {
                logError(RTPS_PDP, "Error sending announcement from server to servers");
            }
        }
        else
        {
            logError(RTPS_PDP,
                    "ParticipantProxy data should have been added to client PDP history cache by a previous call to announceParticipantState()");
        }
    }
}

/**
 * This method removes a remote RTPSParticipant and all its writers and readers.
 * @param participant_guid GUID_t of the remote RTPSParticipant.
 * @param reason Why the participant is being removed (dropped vs removed)
 * @return true if correct.
 */
bool PDPServer::remove_remote_participant(
        const GUID_t& partGUID,
        ParticipantDiscoveryInfo::DISCOVERY_STATUS reason)
{
    InstanceHandle_t key;

    if (partGUID == getLocalParticipantProxyData()->m_guid
            || !lookup_participant_key(partGUID, key))
    {   // verify it's a known participant
        return false;
    }

    {
        // prevent mp_PDPReaderHistory from been clean up by the PDPServerListener
        std::lock_guard<RecursiveTimedMutex> lock(mp_PDPReader->getMutex());

        // Notify everybody of this demise if it's a lease Duration one
        CacheChange_t* pC;

        // Check if the DATA(p[UD]) is already in Reader
        if (!(mp_PDPReaderHistory->get_max_change(&pC) &&
                pC->kind == NOT_ALIVE_DISPOSED_UNREGISTERED && // last message received is aun DATA(p[UD])
                pC->instanceHandle == key )) // from the same participant I'm going to report
        {   // We must create the DATA(p[UD])
            if ((pC = mp_PDPWriter->new_change(
                [this]() -> uint32_t
                {
                    return mp_builtin->m_att.writerPayloadSize;
                },
                NOT_ALIVE_DISPOSED_UNREGISTERED, key)))
            {
                // Use this server identity in order to hint clients it's a lease duration demise
                WriteParams wp;
                SampleIdentity local;
                local.writer_guid(mp_PDPWriter->getGuid());
                local.sequence_number(mp_PDPWriterHistory->next_sequence_number());
                wp.sample_identity(local);
                wp.related_sample_identity(local);

                if (mp_PDPWriterHistory->add_change(pC, wp))
                {
                    // Impersonate
                    pC->writerGUID = GUID_t(partGUID.guidPrefix, c_EntityId_SPDPWriter);

                    logInfo(RTPS_PDP, "Server created a DATA(p[UD]) for a lease duration casualty.")
                }
            }
        }

    }

    // Trigger the WriterHistory cleaning mechanism of demised participants DATA. Note that
    // only DATA acknowledge by all clients would be actually removed
    {
        std::lock_guard<std::recursive_mutex> lock(*getMutex());

        InstanceHandle_t ih;

        removeParticipantFromHistory(ih = partGUID);
        removeParticipantForEDPMatch(partGUID);

        // awake server event thread
        awakeServerThread();
        // the timer is also restart from initPDP, remove(Publisher|Subscriber)FromHistory
        // and queueParticipantForEDPMatch
    }

    return PDP::remove_remote_participant(partGUID, reason);
}

bool PDPServer::pendingHistoryCleaning()
{
    std::lock_guard<std::recursive_mutex> guardP(*getMutex());

    EDPServer* pEDP = dynamic_cast<EDPServer*>(mp_EDP);
    assert(pEDP);

    return !_demises.empty() || pEDP->pendingHistoryCleaning();
}

// ! returns a unique_ptr to an object that handles PDP_callback_ in a RAII fashion
std::unique_ptr<PDPServer::InPDPCallback> PDPServer::signalCallback()
{
    // TODO: change when C++14 available
    //return std::make_unique<PDPServer::InPDPCallback>(*this);
    return std::unique_ptr<InPDPCallback>(new InPDPCallback(*this));
}

// ! calls PDP Reader matched_writer_remove preventing deadlocks
bool PDPServer::safe_PDP_matched_writer_remove(
        const GUID_t& wguid)
{
    bool res;
    std::unique_lock<std::recursive_mutex> guardP(*getMutex());

    if (PDP_callback_)
    {
        // If we are in a transport callback the reader mutex is already lock
        // and we cannot remove the writer proxies
        RecursiveTimedMutex& mtx = mp_PDPReader->getMutex();

        mtx.unlock();
        res = mp_PDPReader->matched_writer_remove(wguid);
        mtx.lock();
    }
    else
    {
        // if a lease callback takes place while transport callback is going on
        // an before it has the unmatch done we must prevent a deadlock in PDP mutex
        guardP.unlock();
        res = mp_PDPReader->matched_writer_remove(wguid);
    }

    return res;
}

PDPServer::InPDPCallback::InPDPCallback(
        PDPServer& svr)
    : server_(svr)
{
    std::lock_guard<std::recursive_mutex> lock(*server_.getMutex());

    server_.PDP_callback_ = true;
}

PDPServer::InPDPCallback::~InPDPCallback()
{
    std::lock_guard<std::recursive_mutex> lock(*server_.getMutex());

    server_.PDP_callback_ = false;
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
