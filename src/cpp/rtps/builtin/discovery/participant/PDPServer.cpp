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

#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/builtin/liveliness/WLP.h>

#include <fastrtps/rtps/participant/RTPSParticipantListener.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastrtps/utils/TimeConversion.h>

#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/log/Log.h>

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/DServerEvent.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPServerListener.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPServer.h>
#include <fastrtps/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPServer.h>


#include <fastrtps/rtps/writer/ReaderProxy.h>

#include <algorithm>
#include <forward_list>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

PDPServer::PDPServer(
        BuiltinProtocols* built,
        DurabilityKind_t durability_kind)
    : PDP(built)
    , _durability(durability_kind)
    , _msgbuffer(DISCOVERY_PARTICIPANT_DATA_MAX_SIZE,built->mp_participantImpl->getGuid().guidPrefix)
    , mp_sync(nullptr)
{

}

PDPServer::~PDPServer()
{
    if (mp_sync != nullptr)
        delete(mp_sync);
}

bool PDPServer::initPDP(RTPSParticipantImpl* part)
{
    if (!PDP::initPDP(part,true))
    {
        return false;
    }

    //INIT EDP
    mp_EDP = (EDP*)(new EDPServer(this, mp_RTPSParticipant, _durability));
    if (!mp_EDP->initEDP(m_discovery))
    {
        logError(RTPS_PDP, "Endpoint discovery configuration failed");
        return false;
    }

    /*
        Given the fact that a participant is either a client or a server the
        discoveryServer_client_syncperiod parameter has a context defined meaning.
    */
    mp_sync = new DServerEvent(this, TimeConv::Time_t2MilliSecondsDouble(m_discovery.discoveryServer_client_syncperiod));
    awakeServerThread(); 
    // the timer is also restart from removeRemoteParticipant, remove(Publisher|Subscriber)FromHistory
    // and queueParticipantForEDPMatch
    
    return true;
}

ParticipantProxyData * PDPServer::createParticipantProxyData(const ParticipantProxyData & participant_data, const CacheChange_t & change)
{
    std::unique_lock<std::recursive_mutex> lock(*getMutex());

    // lease duration is controlled for owned clients or linked servers
    // other clients liveliness is provided through server's PDP discovery data

    // check if the DATA msg is relayed by another server
    bool do_lease = participant_data.m_guid.guidPrefix == change.writerGUID.guidPrefix;

    if (!do_lease)
    {
        // if not a client verify this participant is a server
        for (auto & svr : mp_builtin->m_DiscoveryServers)
        {
            if (svr.guidPrefix == participant_data.m_guid.guidPrefix)
            {
                do_lease = true;
            }
        }
    }

    ParticipantProxyData * pdata = new ParticipantProxyData(participant_data);
    pdata->isAlive = true;

    if (do_lease)
    {
        pdata->mp_leaseDurationTimer = new RemoteParticipantLeaseDuration(this,
            pdata,
            TimeConv::Time_t2MilliSecondsDouble(pdata->m_leaseDuration));
        pdata->mp_leaseDurationTimer->restart_timer();
    }
    else
    {
        pdata->mp_leaseDurationTimer = nullptr;
    }

    m_participantProxies.push_back(pdata);

    return pdata;
}

bool PDPServer::createPDPEndpoints()
{
    logInfo(RTPS_PDP, "Beginning PDPServer Endpoints creation");

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

    mp_listener = new PDPServerListener(this);

    if (mp_RTPSParticipant->createReader(&mp_PDPReader, ratt, mp_PDPReaderHistory, mp_listener, c_EntityId_SPDPReader, true, false))
    {
        // enable unknown clients to reach this reader
        mp_PDPReader->enableMessagesFromUnkownWriters(true);

        for (auto it = mp_builtin->m_DiscoveryServers.begin(); it != mp_builtin->m_DiscoveryServers.end(); ++it)
        {
            RemoteWriterAttributes rwatt;

            rwatt.guid = it->GetPDPWriter();
            rwatt.endpoint.multicastLocatorList.push_back(it->metatrafficMulticastLocatorList);
            rwatt.endpoint.unicastLocatorList.push_back(it->metatrafficUnicastLocatorList);
            rwatt.endpoint.topicKind = WITH_KEY;
            rwatt.endpoint.durabilityKind = _durability; // Server Information must be persistent
            rwatt.endpoint.reliabilityKind = RELIABLE;

            // TODO: remove the join when Reader and Writer match functions are updated
            rwatt.endpoint.remoteLocatorList.push_back(it->metatrafficMulticastLocatorList);
            rwatt.endpoint.remoteLocatorList.push_back(it->metatrafficUnicastLocatorList);

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
    watt.endpoint.durabilityKind = _durability;
    watt.endpoint.properties.properties().push_back(Property("dds.persistence.plugin", "builtin.SQLITE3"));
    watt.endpoint.properties.properties().push_back(Property("dds.persistence.sqlite3.filename", GetPersistenceFileName()));
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

    if (mp_RTPSParticipant->createWriter(&mp_PDPWriter, watt, mp_PDPWriterHistory, nullptr, c_EntityId_SPDPWriter, true))
    {

        for (auto it = mp_builtin->m_DiscoveryServers.begin(); it != mp_builtin->m_DiscoveryServers.end(); ++it)
        {
            RemoteReaderAttributes rratt;

            rratt.guid = it->GetPDPReader();
            rratt.endpoint.multicastLocatorList.push_back(it->metatrafficMulticastLocatorList);
            rratt.endpoint.unicastLocatorList.push_back(it->metatrafficUnicastLocatorList);
            rratt.endpoint.topicKind = WITH_KEY;
            rratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
            rratt.endpoint.reliabilityKind = RELIABLE;

            // TODO: remove the join when Reader and Writer match functions are updated
            rratt.endpoint.remoteLocatorList.push_back(it->metatrafficMulticastLocatorList);
            rratt.endpoint.remoteLocatorList.push_back(it->metatrafficUnicastLocatorList);

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
    logInfo(RTPS_PDP, "PDPServer Endpoints creation finished");
    return true;
}

void PDPServer::initializeParticipantProxyData(ParticipantProxyData* participant_data)
{
    PDP::initializeParticipantProxyData(participant_data); // TODO: Remember that the PDP version USES security

    if (!(getRTPSParticipant()->getAttributes().builtin.discoveryProtocol != PDPType_t::CLIENT))
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
}


void PDPServer::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
    // Verify if this participant is a server
    for (auto & svr : mp_builtin->m_DiscoveryServers)
    {
        if (svr.guidPrefix == pdata->m_guid.guidPrefix)
        {
            svr.proxy = pdata;
            // servers are already match in PDPServer::createPDPEndpoints
            // Notify another endpoints
            notifyAboveRemoteEndpoints(*pdata);
            return;
        }
    }

    // boilerplate, note that PDPSimple version doesn't use RELIABLE entities
    logInfo(RTPS_PDP, "For RTPSParticipant: " << pdata->m_guid.guidPrefix);
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    if (auxendp != 0)
    {
        RemoteWriterAttributes watt(pdata->m_VendorId);
        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SPDPWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = TRANSIENT_LOCAL;

        // TODO remove the join when Reader and Writer match functions are updated
        watt.endpoint.remoteLocatorList.push_back(pdata->m_metatrafficUnicastLocatorList);
        watt.endpoint.remoteLocatorList.push_back(pdata->m_metatrafficMulticastLocatorList);

        mp_PDPReader->matched_writer_add(watt);
    }
    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    if (auxendp != 0)
    {
        RemoteReaderAttributes ratt(pdata->m_VendorId);
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SPDPReader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;

        mp_PDPWriter->matched_reader_add(ratt);
    }

    // Notify another endpoints
    notifyAboveRemoteEndpoints(*pdata);

}

void PDPServer::notifyAboveRemoteEndpoints(const ParticipantProxyData& pdata)
{
    // No EDP notification needed. EDP endpoints would be match when PDP synchronization is granted
    if (mp_builtin->mp_WLP != nullptr)
        mp_builtin->mp_WLP->assignRemoteEndpoints(pdata);
}


void PDPServer::removeRemoteEndpoints(ParticipantProxyData* pdata)
{
    // EDP endpoints have been already unmatch by the associated listener
    assert(!mp_EDP->areRemoteEndpointsMatched(pdata));

    // Verify if this participant is a server
    bool is_server = false;
    {
        std::unique_lock<std::recursive_mutex> lock(*getMutex());

        for (auto & svr : mp_builtin->m_DiscoveryServers)
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
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;

    if (auxendp != 0)
    {
        RemoteWriterAttributes watt;

        watt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        watt.guid.entityId = c_EntityId_SPDPWriter;
        watt.endpoint.persistence_guid = watt.guid;
        watt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = is_server ? TRANSIENT : TRANSIENT_LOCAL;
        watt.endpoint.topicKind = WITH_KEY;

        mp_PDPReader->matched_writer_remove(watt);

        if (is_server)
        {
            mp_PDPReader->matched_writer_add(watt,false);
        }

    }

    auxendp = endp;
    auxendp &= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

    if (auxendp != 0)
    {
        RemoteReaderAttributes ratt;

        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = pdata->m_guid.guidPrefix;
        ratt.guid.entityId = c_EntityId_SPDPReader;
        ratt.endpoint.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
        ratt.endpoint.topicKind = WITH_KEY;

        mp_PDPWriter->matched_reader_remove(ratt);

        if (is_server)
        {
            mp_PDPWriter->matched_reader_add(ratt);
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
        return;

    for (auto p: _p2match)
    {
       assert( p != nullptr);
       mp_EDP->assignRemoteEndpoints(*p);
    }

    _p2match.clear();
}

bool PDPServer::trimWriterHistory()
{
    EDPServer * pEDP = dynamic_cast<EDPServer*>(mp_EDP);
    assert(pEDP);

    bool istrim = true;

    istrim &= trimPDPWriterHistory();
    istrim &= pEDP->trimPUBWriterHistory();
    istrim &= pEDP->trimSUBWriterHistory();

    return istrim;
}


bool PDPServer::trimPDPWriterHistory()
{
    assert(mp_mutex && mp_PDPWriter && mp_PDPWriter->getMutex());

    // trim demises container
    key_list disposal, aux;

    if (_demises.empty())
        return true;

    std::lock_guard<std::recursive_mutex> guardP(*getMutex());

    // sweep away any resurrected participant
    std::for_each(ParticipantProxiesBegin(), ParticipantProxiesEnd(),
        [&disposal](const ParticipantProxyData* pD) { disposal.insert(pD->m_key); });
    std::set_difference(_demises.cbegin(), _demises.cend(), disposal.cbegin(), disposal.cend(),
        std::inserter(aux,aux.begin()));
    _demises.swap(aux);

    if (_demises.empty())
        return true;

    // traverse the WriterHistory searching CacheChanges_t with demised keys
    std::forward_list<CacheChange_t*> removal;
    std::lock_guard<std::recursive_mutex> guardW(*mp_PDPWriter->getMutex());

    std::copy_if(mp_PDPWriterHistory->changesBegin(), mp_PDPWriterHistory->changesBegin(), std::front_inserter(removal),
        [this](const CacheChange_t* chan) { return _demises.find(chan->instanceHandle) != _demises.cend();  });

    if (removal.empty())
        return true;

    aux.clear();
    key_list & pending = aux;

    // remove outdate CacheChange_ts
    for (auto pC : removal)
    {
        if (mp_PDPWriter->is_acked_by_all(pC))
            mp_PDPWriterHistory->remove_change(pC);
        else
            pending.insert(pC->instanceHandle);
    }

    // update demises
    _demises.swap(pending);

    return _demises.empty(); // finish?
}

// CacheChange_t's ParticipantProxyData wouldn't be loaded when this function is called
bool PDPServer::addRelayedChangeToHistory( CacheChange_t & c)
{
    assert(mp_PDPWriter && mp_PDPWriter->getMutex() && c.serializedPayload.max_size);

    std::lock_guard<std::recursive_mutex> lock(*mp_PDPWriter->getMutex());
    CacheChange_t * pCh = nullptr;

    // validate the sample, if no sample data update it
    WriteParams & wp = c.write_params;
    SampleIdentity & sid = wp.sample_identity();
    if (sid == SampleIdentity::unknown())
    {
        sid.writer_guid(c.writerGUID);
        sid.sequence_number(c.sequenceNumber);
        logError(RTPS_PDP, "A DATA(p) received by server " << mp_PDPWriter->getGuid()
            << " from participant " << c.writerGUID << " without a valid SampleIdentity");
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
        // mp_PDPWriterHistory->reserve_Cache(&pCh, DISCOVERY_PARTICIPANT_DATA_MAX_SIZE)
        if (mp_PDPWriterHistory->reserve_Cache(&pCh, c.serializedPayload.max_size) && pCh && pCh->copy(&c))
        {
            pCh->writerGUID = mp_PDPWriter->getGuid();
            // keep the original sample identity by using wp
            return mp_PDPWriterHistory->add_change(pCh,wp);
        }
    }
    return false;
}

// Always call after PDP proxies update
void PDPServer::removeParticipantFromHistory(const InstanceHandle_t & key)
{
    std::lock_guard<std::recursive_mutex> guardP(*mp_mutex);

    _demises.insert(key);
    trimWriterHistory();
}

void PDPServer::queueParticipantForEDPMatch(const ParticipantProxyData * pdata)
{
    assert(pdata != nullptr);

    std::lock_guard<std::recursive_mutex> guardP(*mp_mutex);

    // add the new client or server to the EDP matching list
    _p2match.insert(pdata);
    awakeServerThread();
    // the timer is also restart from removeRemoteParticipant, remove(Publisher|Subscriber)FromHistory
    // and initPDP

    logInfo(PDP_SERVER, "participant " << pdata->m_participantName << " prefix: " << pdata->m_guid
        << " waiting for EDP match with server " << this->getRTPSParticipant()->getRTPSParticipantAttributes().getName());
}

void PDPServer::removeParticipantForEDPMatch(const ParticipantProxyData * pdata)
{
    assert(pdata != nullptr);

    std::lock_guard<std::recursive_mutex> guardP(*mp_mutex);

    // remove the deceased client to the EDP matching list
    for (pending_matches_list::iterator it = _p2match.begin(); it != _p2match.end(); ++it)
    {
        if ((*it)->m_guid == pdata->m_guid)
        {
            _p2match.erase(it);
            return;
        }
    }
    
}

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

bool PDPServer::all_servers_acknowledge_PDP()
{
    // check if already initialized
    assert(mp_PDPWriterHistory && mp_PDPWriter);

    // First check if all servers have been discovered
    bool discovered = true;

    for (auto & s : mp_builtin->m_DiscoveryServers)
    {
        discovered &= (s.proxy != nullptr);
    }

    if (!discovered)
    {
        // The first change in the PDP WriterHistory is this server ParticipantProxyData
        // see BuiltinProtocols::initBuiltinProtocols call to PDPXXX::announceParticipantState(true)
        CacheChange_t * pPD;
        if (mp_PDPWriterHistory->get_min_change(&pPD))
        {
            // This answer includes also clients but is accurate enough
            return mp_PDPWriter->is_acked_by_all(pPD);
        }
        else
        {
            logError(RTPS_PDP, "ParticipantProxy data should have been added to client PDP history cache by a previous call to announceParticipantState()");
            return false;
        }
    }

    return true;
}


bool PDPServer::is_all_servers_PDPdata_updated()
{
    StatefulReader * pR = dynamic_cast<StatefulReader *>(mp_PDPReader);
    assert(pR);

    // This answer includes also clients but is accurate enough
    return pR->isInCleanState();
}


bool PDPServer::match_servers_EDP_endpoints()
{
    std::lock_guard<std::recursive_mutex> lock(*getMutex());
    bool all = true; // have all servers been discovered?

    for (auto & svr : mp_builtin->m_DiscoveryServers)
    {
        all &= (svr.proxy != nullptr);

        if (svr.proxy && !mp_EDP->areRemoteEndpointsMatched(svr.proxy))
        {
            this->queueParticipantForEDPMatch(svr.proxy);
        }
    }

    return all;
}

void PDPServer::announceParticipantState(bool new_change, bool dispose /* = false */, WriteParams& )
{
    StatefulWriter * pW = dynamic_cast<StatefulWriter*>(mp_PDPWriter);
    assert(pW);

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

            if ((change = pW->new_change([]() -> uint32_t {return DISCOVERY_PARTICIPANT_DATA_MAX_SIZE; },
                NOT_ALIVE_DISPOSED_UNREGISTERED, getLocalParticipantProxyData()->m_key)))
            {
                // update the sequence number
                change->sequenceNumber = mp_PDPWriterHistory->next_sequence_number();
                change->write_params = wp;

                std::lock_guard<std::recursive_mutex> wlock(*pW->getMutex());

                RTPSMessageGroup group(getRTPSParticipant(), mp_PDPWriter, RTPSMessageGroup::WRITER, _msgbuffer);

                std::vector<GUID_t> remote_readers;
                LocatorList_t locators;

                for (auto it = pW->matchedReadersBegin(); it != pW->matchedReadersEnd(); ++it)
                {
                    RemoteReaderAttributes & att = (*it)->m_att;
                    remote_readers.push_back(att.guid);

                    EndpointAttributes & ep = att.endpoint;
                    locators.push_back(ep.unicastLocatorList);
                    //locators.push_back(ep.multicastLocatorList);
                }

                if (!group.add_data(*change, remote_readers, locators, false))
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
        CacheChange_t * pPD;
        if (mp_PDPWriterHistory->get_min_change(&pPD))
        {
            std::lock_guard<std::recursive_mutex> lock(*getMutex());

            RTPSMessageGroup group(getRTPSParticipant(), mp_PDPWriter, RTPSMessageGroup::WRITER, _msgbuffer);

            std::vector<GUID_t> remote_readers;
            LocatorList_t locators;

            for (auto it = pW->matchedReadersBegin(); it != pW->matchedReadersEnd(); ++it)
            {
                RemoteReaderAttributes & att = (*it)->m_att;
                remote_readers.push_back(att.guid);

                EndpointAttributes & ep = att.endpoint;
                locators.push_back(ep.unicastLocatorList);
                locators.push_back(ep.multicastLocatorList);
            }

            for (auto & svr : mp_builtin->m_DiscoveryServers)
            {
                if (svr.proxy == nullptr)
                {
                    remote_readers.push_back(svr.GetPDPReader());
                    locators.push_back(svr.metatrafficMulticastLocatorList);
                    locators.push_back(svr.metatrafficUnicastLocatorList);
                }
            }

            if (!group.add_data(*pPD, remote_readers, locators, false))
            {
                logError(RTPS_PDP, "Error sending announcement from server to servers");
            }
        }
        else
        {
            logError(RTPS_PDP, "ParticipantProxy data should have been added to client PDP history cache by a previous call to announceParticipantState()");
        }
    }
}

bool PDPServer::removeRemoteParticipant(GUID_t& partGUID)
{
    // verify it's a known participant
    ParticipantProxyData info;

    if (!lookupParticipantProxyData(partGUID, info))
    {
        return false;
    }

    // Notify everybody of this demise if it's a lease Duration one
    CacheChange_t *pC;

    // Check if the DATA(p[UD]) is already in Reader
    if (!mp_PDPReaderHistory->get_max_change(&pC))
    {   // We must create the DATA(p[UD])
        if ((pC = mp_PDPWriter->new_change([]() -> uint32_t {return DISCOVERY_PARTICIPANT_DATA_MAX_SIZE; },
            NOT_ALIVE_DISPOSED_UNREGISTERED, info.m_key)))
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
                pC->writerGUID = GUID_t(info.m_guid.guidPrefix, c_EntityId_SPDPWriter);

                logInfo(RTPS_PDP, "Server created a DATA(p[UD]) for a lease duration casualty.")
            }
        }
    }
    
    // Trigger the WriterHistory cleaning mechanism of demised participants DATA. Note that
    // only DATA acknowledge by all clients would be actually removed
    {   
        std::lock_guard<std::recursive_mutex> lock(*getMutex());

        InstanceHandle_t ih;

        removeParticipantFromHistory(ih = partGUID);
        removeParticipantForEDPMatch(&info);

        // awake server event thread
        awakeServerThread();
        // the timer is also restart from initPDP, remove(Publisher|Subscriber)FromHistory
        // and queueParticipantForEDPMatch
    }

    return PDP::removeRemoteParticipant(partGUID);
}


bool PDPServer::pendingHistoryCleaning()
{
    EDPServer * pEDP = dynamic_cast<EDPServer*>(mp_EDP);
    assert(pEDP);

    return !_demises.empty() || pEDP->pendingHistoryCleaning();
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
