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

#include <rtps/builtin/discovery/participant/PDPSimple.h>

#include <mutex>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/builtin/data/BuiltinEndpoints.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/rtps/participant/RTPSParticipantListener.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/data/NetworkConfiguration.hpp>
#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <rtps/builtin/discovery/endpoint/EDPStatic.h>
#include <rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.hpp>
#include <rtps/builtin/discovery/participant/PDPListener.h>
#include <rtps/builtin/discovery/participant/simple/SimplePDPEndpoints.hpp>
#include <rtps/builtin/discovery/participant/simple/SimplePDPEndpointsSecure.hpp>
#include <rtps/builtin/liveliness/WLP.hpp>
#include <rtps/history/TopicPayloadPoolRegistry.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/reader/StatelessReader.hpp>
#include <rtps/resources/TimedEvent.h>
#include <rtps/writer/BaseWriter.hpp>
#include <rtps/writer/StatelessWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

static HistoryAttributes pdp_reader_history_attributes(
        const BuiltinAttributes& builtin_att,
        const RTPSParticipantAllocationAttributes& allocation)
{
    HistoryAttributes hatt;
    hatt.payloadMaxSize = builtin_att.readerPayloadSize;
    hatt.memoryPolicy = builtin_att.readerHistoryMemoryPolicy;
    hatt.initialReservedCaches = 25;
    if (allocation.participants.initial > 0)
    {
        hatt.initialReservedCaches = (int32_t)allocation.participants.initial;
    }
    if (allocation.participants.maximum < std::numeric_limits<size_t>::max())
    {
        hatt.maximumReservedCaches = (int32_t)allocation.participants.maximum;
    }

    return hatt;
}

static HistoryAttributes pdp_writer_history_attributes(
        const BuiltinAttributes& builtin_att)
{
    HistoryAttributes hatt;
    hatt.payloadMaxSize = builtin_att.writerPayloadSize;
    hatt.memoryPolicy = builtin_att.writerHistoryMemoryPolicy;
    hatt.initialReservedCaches = 1;
    hatt.maximumReservedCaches = 1;

    return hatt;
}

PDPSimple::PDPSimple (
        BuiltinProtocols* built,
        const RTPSParticipantAllocationAttributes& allocation)
    : PDP(built, allocation)
{
}

PDPSimple::~PDPSimple()
{
}

void PDPSimple::update_builtin_locators()
{
    auto endpoints = static_cast<fastdds::rtps::SimplePDPEndpoints*>(builtin_endpoints_.get());
    mp_builtin->updateMetatrafficLocators(endpoints->reader.reader_->getAttributes().unicastLocatorList);
}

void PDPSimple::initializeParticipantProxyData(
        ParticipantProxyData* participant_data)
{
    PDP::initializeParticipantProxyData(participant_data);

    auto discovery_config = getRTPSParticipant()->get_attributes().builtin.discovery_config;

    if (discovery_config.use_SIMPLE_EndpointDiscoveryProtocol)
    {
        if (discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
        {
            participant_data->m_available_builtin_endpoints |=
                    fastdds::rtps::DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
            participant_data->m_available_builtin_endpoints |=
                    fastdds::rtps::DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
        }

        if (discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
        {
            participant_data->m_available_builtin_endpoints |=
                    fastdds::rtps::DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
            participant_data->m_available_builtin_endpoints |=
                    fastdds::rtps::DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
        }

#if HAVE_SECURITY
        if (discovery_config.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader)
        {
            participant_data->m_available_builtin_endpoints |=
                    fastdds::rtps::DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
            participant_data->m_available_builtin_endpoints |=
                    fastdds::rtps::DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
        }

        if (discovery_config.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader)
        {
            participant_data->m_available_builtin_endpoints |=
                    fastdds::rtps::DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
            participant_data->m_available_builtin_endpoints |=
                    fastdds::rtps::DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
        }
#endif // if HAVE_SECURITY
    }
    else if (!discovery_config.use_STATIC_EndpointDiscoveryProtocol)
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "Neither EDP simple nor EDP static enabled. Endpoints will not be discovered.");
    }
}

bool PDPSimple::init(
        RTPSParticipantImpl* part)
{
    // The DATA(p) must be processed after EDP endpoint creation
    if (!PDP::initPDP(part))
    {
        return false;
    }

    //INIT EDP
    if (m_discovery.discovery_config.use_STATIC_EndpointDiscoveryProtocol)
    {
        mp_EDP = new EDPStatic(this, mp_RTPSParticipant);
        if (!mp_EDP->initEDP(m_discovery))
        {
            EPROSIMA_LOG_ERROR(RTPS_PDP, "Endpoint discovery configuration failed");
            delete mp_EDP;
            mp_EDP = nullptr;
            return false;
        }
    }
    else if (m_discovery.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol)
    {
        mp_EDP = new EDPSimple(this, mp_RTPSParticipant);
        if (!mp_EDP->initEDP(m_discovery))
        {
            EPROSIMA_LOG_ERROR(RTPS_PDP, "Endpoint discovery configuration failed");
            delete mp_EDP;
            mp_EDP = nullptr;
            return false;
        }
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTPS_PDP, "No EndpointDiscoveryProtocol defined");
        return false;
    }

    return true;
}

ParticipantProxyData* PDPSimple::createParticipantProxyData(
        const ParticipantProxyData& participant_data,
        const GUID_t&)
{
    std::unique_lock<std::recursive_mutex> lock(*getMutex());

    // decide if we dismiss the participant using the ParticipantFilteringFlags
    const ParticipantFilteringFlags& flags = m_discovery.discovery_config.ignoreParticipantFlags;
    const GUID_t& remote = participant_data.guid;
    const GUID_t& local = getLocalParticipantProxyData()->guid;
    bool is_same_host = local.is_on_same_host_as(remote);
    bool is_same_process = local.is_on_same_process_as(remote);

    // Discard participants on different process when they don't have metatraffic locators
    if (participant_data.metatraffic_locators.multicast.empty() &&
            participant_data.metatraffic_locators.unicast.empty() &&
            !is_same_process)
    {
        return nullptr;
    }

    if (flags != ParticipantFilteringFlags::NO_FILTER)
    {
        if (!is_same_host)
        {
            if (flags & ParticipantFilteringFlags::FILTER_DIFFERENT_HOST)
            {
                return nullptr;
            }
        }
        else
        {
            bool filter_same = (flags& ParticipantFilteringFlags::FILTER_SAME_PROCESS) != 0;
            bool filter_different = (flags& ParticipantFilteringFlags::FILTER_DIFFERENT_PROCESS) != 0;

            if (filter_same && filter_different)
            {
                return nullptr;
            }

            if ((filter_same && is_same_process) || (filter_different && !is_same_process))
            {
                return nullptr;
            }
        }
    }

    ParticipantProxyData* pdata = add_participant_proxy_data(participant_data.guid, true, &participant_data);
    if (pdata != nullptr)
    {
        pdata->lease_duration_event->update_interval(pdata->lease_duration);
        pdata->lease_duration_event->restart_timer();
    }

    return pdata;
}

// EDPStatic requires matching on ParticipantProxyData property updates
bool PDPSimple::updateInfoMatchesEDP()
{
    return dynamic_cast<EDPStatic*>(mp_EDP) != nullptr;
}

void PDPSimple::announceParticipantState(
        bool new_change,
        bool dispose /* = false */)
{
    WriteParams __wp = WriteParams::write_params_default();
    announceParticipantState(new_change, dispose, __wp);
}

void PDPSimple::announceParticipantState(
        bool new_change,
        bool dispose,
        WriteParams& wp)
{
    if (enabled_)
    {
        new_change |= m_hasChangedLocalPDP.exchange(false);

#if HAVE_SECURITY
        if (mp_RTPSParticipant->is_secure())
        {
            auto secure = dynamic_cast<fastdds::rtps::SimplePDPEndpointsSecure*>(builtin_endpoints_.get());
            assert(nullptr != secure);

            WriterHistory& history = *(secure->secure_writer.history_);
            PDP::announceParticipantState(history, new_change, dispose, wp);
        }
#endif // HAVE_SECURITY

        auto endpoints = dynamic_cast<fastdds::rtps::SimplePDPEndpoints*>(builtin_endpoints_.get());
        WriterHistory& history = *(endpoints->writer.history_);
        PDP::announceParticipantState(history, new_change, dispose, wp);

        if (!(dispose || new_change))
        {
            endpoints->writer.writer_->send_periodic_announcement();
        }
    }
}

bool PDPSimple::createPDPEndpoints()
{
    EPROSIMA_LOG_INFO(RTPS_PDP, "Beginning");

    fastdds::rtps::SimplePDPEndpoints* endpoints = nullptr;
#if HAVE_SECURITY
    fastdds::rtps::SimplePDPEndpointsSecure* secure_endpoints = nullptr;
    bool is_secure = mp_RTPSParticipant->is_secure();
    if (is_secure)
    {
        secure_endpoints = new fastdds::rtps::SimplePDPEndpointsSecure();
        secure_endpoints->secure_reader.listener_.reset(new PDPListener(this));

        endpoints = secure_endpoints;
        endpoints->reader.listener_.reset(new PDPSecurityInitiatorListener(this,
                [this](const ParticipantProxyData& participant_data)
                {
                    match_pdp_remote_endpoints(participant_data, false, true);
                }));
    }
    else
#endif  // HAVE_SECURITY
    {
        endpoints = new fastdds::rtps::SimplePDPEndpoints();
        endpoints->reader.listener_.reset(new PDPListener(this));
    }
    builtin_endpoints_.reset(endpoints);

    bool ret_val = create_dcps_participant_endpoints();
#if HAVE_SECURITY
    if (ret_val && is_secure)
    {
        create_dcps_participant_secure_endpoints();
    }
#endif  // HAVE_SECURITY
    EPROSIMA_LOG_INFO(RTPS_PDP, "SPDP Endpoints creation finished");
    return ret_val;
}

bool PDPSimple::create_dcps_participant_endpoints()
{
    const RTPSParticipantAttributes& pattr = mp_RTPSParticipant->get_attributes();
    const RTPSParticipantAllocationAttributes& allocation = pattr.allocation;
    const BuiltinAttributes& builtin_att = mp_builtin->m_att;
    auto endpoints = dynamic_cast<fastdds::rtps::SimplePDPEndpoints*>(builtin_endpoints_.get());
    assert(nullptr != endpoints);

    constexpr const char* topic_name = "DCPSParticipant";
    const EntityId_t reader_entity_id = c_EntityId_SPDPReader;
    const EntityId_t writer_entity_id = c_EntityId_SPDPWriter;

    // BUILTIN DCPSParticipant READER
    auto& reader = endpoints->reader;
    HistoryAttributes hatt;
    hatt = pdp_reader_history_attributes(builtin_att, allocation);

    PoolConfig reader_pool_cfg = PoolConfig::from_history_attributes(hatt);
    reader.payload_pool_ = TopicPayloadPoolRegistry::get(topic_name, reader_pool_cfg);
    reader.payload_pool_->reserve_history(reader_pool_cfg, true);
    reader.history_.reset(new ReaderHistory(hatt));

    ReaderAttributes ratt = create_builtin_reader_attributes();
    ratt.endpoint.reliabilityKind = BEST_EFFORT;

    RTPSReader* rtps_reader = nullptr;
    if (mp_RTPSParticipant->createReader(&rtps_reader, ratt, reader.payload_pool_, reader.history_.get(),
            reader.listener_.get(), reader_entity_id, true, false))
    {
        reader.reader_ = dynamic_cast<StatelessReader*>(rtps_reader);
        assert(nullptr != reader.reader_);

#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(rtps_reader, false);
#endif // if HAVE_SECURITY
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "'" << topic_name << "' builtin reader creation failed");
        reader.release();
        return false;
    }

    // BUILTIN DCPSParticipant WRITER
    auto& writer = endpoints->writer;
    hatt = pdp_writer_history_attributes(builtin_att);

    PoolConfig writer_pool_cfg = PoolConfig::from_history_attributes(hatt);
    writer.payload_pool_ = TopicPayloadPoolRegistry::get(topic_name, writer_pool_cfg);
    writer.payload_pool_->reserve_history(writer_pool_cfg, false);
    writer.history_.reset(new WriterHistory(hatt, writer.payload_pool_));

    WriterAttributes watt = create_builtin_writer_attributes();
    watt.endpoint.reliabilityKind = BEST_EFFORT;
    if (!m_discovery.initialPeersList.empty())
    {
        auto entry = LocatorSelectorEntry::create_fully_selected_entry(
            m_discovery.initialPeersList);
        mp_RTPSParticipant->createSenderResources(entry);
    }


    RTPSWriter* rtps_writer = nullptr;
    if (mp_RTPSParticipant->createWriter(&rtps_writer, watt, writer.history_.get(),
            nullptr, writer_entity_id, true))
    {
        writer.writer_ = dynamic_cast<PDPStatelessWriter*>(rtps_writer);
        assert(nullptr != writer.writer_);

#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(rtps_writer, false);
#endif // if HAVE_SECURITY

        const NetworkFactory& network = mp_RTPSParticipant->network_factory();
        LocatorList_t fixed_locators;
        for (const Locator_t& loc : mp_builtin->m_initialPeersList)
        {
            if (network.is_locator_remote_or_allowed(loc))
            {
                // Add initial peers locator without transformation as we don't know whether the
                // remote transport will allow localhost
                fixed_locators.push_back(loc);

                /**
                 * TCP special case:
                 *
                 * In TCP, it is not possible to open a socket with 'any' (0.0.0.0) address as it's done
                 * in UDP, so when the TCP transports receive a locator with 'any', they open an input
                 * channel for the specified port in each of the machine interfaces (with the exception
                 * of localhost). In fact, a participant with a TCP transport will only listen on localhost
                 * if localhost is the address of any of the initial peers.
                 *
                 * However, when the TCP enabled participant does not have a whitelist (or localhost is in
                 * it), it allows for transformation of its locators to localhost for performance optimizations.
                 * In this case, the remote TCP participant it will send data using a socket in localhost,
                 * and for that the participant with the initial peers list needs to be listening there
                 * to receive it.
                 *
                 * That means:
                 *   1. Checking that the initial peer is not already localhost
                 *   2. Checking that the initial peer locator is of TCP kind
                 *   3. Checking that the network configuration allows for localhost locators
                 */
                Locator_t local_locator;
                network.transform_remote_locator(loc, local_locator,
                        DISC_NETWORK_CONFIGURATION_LISTENING_LOCALHOST_ALL);
                if (loc != local_locator
                        && (loc.kind == LOCATOR_KIND_TCPv4 || loc.kind == LOCATOR_KIND_TCPv6)
                        && network.is_locator_allowed(local_locator))
                {
                    fixed_locators.push_back(local_locator);
                }
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTPS_PDP, "Ignoring initial peers locator " << loc << " : not allowed.");
            }
        }
        writer.writer_->set_initial_peers(fixed_locators);
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "'" << topic_name << "' builtin writer creation failed");
        writer.release();
        return false;
    }
    return true;
}

#if HAVE_SECURITY
bool PDPSimple::create_dcps_participant_secure_endpoints()
{
    const RTPSParticipantAttributes& pattr = mp_RTPSParticipant->get_attributes();
    const RTPSParticipantAllocationAttributes& allocation = pattr.allocation;
    const BuiltinAttributes& builtin_att = mp_builtin->m_att;
    auto endpoints = dynamic_cast<fastdds::rtps::SimplePDPEndpointsSecure*>(builtin_endpoints_.get());
    assert(nullptr != endpoints);

    constexpr const char* topic_name = "DCPSParticipantsSecure";
    const EntityId_t reader_entity_id = c_EntityId_spdp_reliable_participant_secure_reader;
    const EntityId_t writer_entity_id = c_EntityId_spdp_reliable_participant_secure_writer;

    // BUILTIN DCPSParticipantsSecure READER
    auto& reader = endpoints->secure_reader;
    HistoryAttributes hatt;
    hatt = pdp_reader_history_attributes(builtin_att, allocation);

    PoolConfig reader_pool_cfg = PoolConfig::from_history_attributes(hatt);
    reader.payload_pool_ = TopicPayloadPoolRegistry::get(topic_name, reader_pool_cfg);
    reader.payload_pool_->reserve_history(reader_pool_cfg, true);
    reader.history_.reset(new ReaderHistory(hatt));

    ReaderAttributes ratt = create_builtin_reader_attributes();
    WriterAttributes watt = create_builtin_writer_attributes();
    add_builtin_security_attributes(ratt, watt);

    RTPSReader* rtps_reader = nullptr;
    if (mp_RTPSParticipant->createReader(&rtps_reader, ratt, reader.payload_pool_, reader.history_.get(),
            reader.listener_.get(), reader_entity_id, true, false))
    {
        reader.reader_ = dynamic_cast<StatefulReader*>(rtps_reader);
        assert(nullptr != reader.reader_);
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "'" << topic_name << "' builtin reader creation failed");
        reader.release();
        return false;
    }

    // SPDP BUILTIN RTPSParticipant WRITER
    auto& writer = endpoints->secure_writer;
    hatt = pdp_writer_history_attributes(builtin_att);

    PoolConfig writer_pool_cfg = PoolConfig::from_history_attributes(hatt);
    writer.payload_pool_ = TopicPayloadPoolRegistry::get(topic_name, writer_pool_cfg);
    writer.payload_pool_->reserve_history(writer_pool_cfg, false);
    writer.history_.reset(new WriterHistory(hatt, writer.payload_pool_));

    RTPSWriter* rtps_writer = nullptr;
    if (mp_RTPSParticipant->createWriter(&rtps_writer, watt, writer.history_.get(),
            nullptr, writer_entity_id, true))
    {
        writer.writer_ = dynamic_cast<StatefulWriter*>(rtps_writer);
        assert(nullptr != writer.writer_);
    }
    else
    {
        EPROSIMA_LOG_ERROR(RTPS_PDP, "'" << topic_name << "' builtin writer creation failed");
        writer.release();
        return false;
    }
    return true;
}

#endif  // HAVE_SECURITY

void PDPSimple::assignRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    bool ignored = false;
    notify_and_maybe_ignore_new_participant(pdata, ignored);
    if (!ignored)
    {
#if HAVE_SECURITY
        auto endpoints = dynamic_cast<fastdds::rtps::SimplePDPEndpointsSecure*>(builtin_endpoints_.get());
        if (nullptr != endpoints)
        {
            // This participant is secure.
            // PDP should have been matched inside notifyAboveRemoteEndpoints after completing the authentication process.
            // We now match the other builtin endpoints.
            GUID_t remote_guid = pdata->guid;
            remote_guid.entityId = c_EntityId_spdp_reliable_participant_secure_writer;
            bool notify_secure = endpoints->secure_reader.reader_->matched_writer_is_matched(remote_guid);
            assign_low_level_remote_endpoints(*pdata, notify_secure);
        }
        else
#endif // if HAVE_SECURITY
        {
            // This participant is not secure.
            // Match PDP and other builtin endpoints.
            match_pdp_remote_endpoints(*pdata, false, false);
            assign_low_level_remote_endpoints(*pdata, false);
        }
    }
}

void PDPSimple::removeRemoteEndpoints(
        ParticipantProxyData* pdata)
{
    EPROSIMA_LOG_INFO(RTPS_PDP, "For RTPSParticipant: " << pdata->guid);
    unmatch_pdp_remote_endpoints(pdata->guid);
}

void PDPSimple::unmatch_pdp_remote_endpoints(
        const GUID_t& participant_guid)
{
    GUID_t guid = participant_guid;

    {
        auto endpoints = dynamic_cast<fastdds::rtps::SimplePDPEndpoints*>(builtin_endpoints_.get());
        assert(nullptr != endpoints);

        guid.entityId = c_EntityId_SPDPWriter;
        endpoints->reader.reader_->matched_writer_remove(guid);

        guid.entityId = c_EntityId_SPDPReader;
        endpoints->writer.writer_->matched_reader_remove(guid);
    }

#if HAVE_SECURITY
    auto endpoints = dynamic_cast<fastdds::rtps::SimplePDPEndpointsSecure*>(builtin_endpoints_.get());
    if (nullptr != endpoints)
    {
        guid.entityId = c_EntityId_spdp_reliable_participant_secure_writer;
        endpoints->secure_reader.reader_->matched_writer_remove(guid);

        guid.entityId = c_EntityId_spdp_reliable_participant_secure_reader;
        endpoints->secure_writer.writer_->matched_reader_remove(guid);
    }
#endif // HAVE_SECURITY
}

void PDPSimple::notifyAboveRemoteEndpoints(
        const ParticipantProxyData& pdata,
        bool notify_secure_endpoints)
{
    if (notify_secure_endpoints)
    {
        unmatch_pdp_remote_endpoints(pdata.guid);
        match_pdp_remote_endpoints(pdata, true, false);
    }
    else
    {
        // Add remote participant data
        GUID_t writer_guid{ pdata.guid.guidPrefix, c_EntityId_SPDPWriter };
        ParticipantProxyData* part_data = createParticipantProxyData(pdata, writer_guid);
        if (part_data != nullptr)
        {
            bool ignored = false;
            notify_and_maybe_ignore_new_participant(part_data, ignored);
            if (!ignored)
            {
                match_pdp_remote_endpoints(*part_data, false, false);
                assign_low_level_remote_endpoints(*part_data, false);
            }
        }
    }

}

void PDPSimple::match_pdp_remote_endpoints(
        const ParticipantProxyData& pdata,
        bool notify_secure_endpoints,
        bool writer_only)
{
#if !HAVE_SECURITY
    static_cast<void>(notify_secure_endpoints);
#endif // !HAVE_SECURITY

    auto endpoints = static_cast<fastdds::rtps::SimplePDPEndpoints*>(builtin_endpoints_.get());

    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    bool use_multicast_locators = !mp_RTPSParticipant->get_attributes().builtin.avoid_builtin_multicast ||
            pdata.metatraffic_locators.unicast.empty();
    const uint32_t endp = pdata.m_available_builtin_endpoints;

    // Default to values for non-secure endpoints
    auto reliability_kind = dds::BEST_EFFORT_RELIABILITY_QOS;
    uint32_t pdp_reader_mask = fastdds::rtps::DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    uint32_t pdp_writer_mask = fastdds::rtps::DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    EntityId_t reader_entity_id = c_EntityId_SPDPReader;
    EntityId_t writer_entity_id = c_EntityId_SPDPWriter;
    BaseReader* reader = endpoints->reader.reader_;
    BaseWriter* writer = endpoints->writer.writer_;

#if HAVE_SECURITY
    // If the other participant has been authenticated, use values for secure endpoints
    if (notify_secure_endpoints)
    {
        auto secure_endpoints = static_cast<fastdds::rtps::SimplePDPEndpointsSecure*>(builtin_endpoints_.get());
        reliability_kind = dds::RELIABLE_RELIABILITY_QOS;
        pdp_reader_mask = fastdds::rtps::DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_DETECTOR;
        pdp_writer_mask = fastdds::rtps::DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_ANNOUNCER;
        reader_entity_id = c_EntityId_spdp_reliable_participant_secure_reader;
        writer_entity_id = c_EntityId_spdp_reliable_participant_secure_writer;
        reader = secure_endpoints->secure_reader.reader_;
        writer = secure_endpoints->secure_writer.writer_;
    }
#endif // HAVE_SECURITY

    if (!writer_only && (0 != (endp & pdp_writer_mask)))
    {
        auto temp_writer_data = get_temporary_writer_proxies_pool().get();

        temp_writer_data->clear();
        temp_writer_data->guid.guidPrefix = pdata.guid.guidPrefix;
        temp_writer_data->guid.entityId = writer_entity_id;
        temp_writer_data->persistence_guid = pdata.get_persistence_guid();
        temp_writer_data->set_persistence_entity_id(writer_entity_id);
        temp_writer_data->set_remote_locators(pdata.metatraffic_locators, network, use_multicast_locators,
                pdata.is_from_this_host());
        temp_writer_data->reliability.kind = reliability_kind;
        temp_writer_data->durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
#if HAVE_SECURITY
        if (notify_secure_endpoints)
        {
            if (!mp_RTPSParticipant->security_manager().discovered_builtin_writer(
                        reader->getGuid(), pdata.guid, *temp_writer_data,
                        reader->getAttributes().security_attributes()))
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Security manager returns an error for writer " <<
                        temp_writer_data->guid);
            }
        }
        else
#endif // HAVE_SECURITY
        {
            reader->matched_writer_add_edp(*temp_writer_data);
        }
    }

    if (0 != (endp & pdp_reader_mask))
    {
        auto temp_reader_data = get_temporary_reader_proxies_pool().get();

        temp_reader_data->clear();
        temp_reader_data->expects_inline_qos = false;
        temp_reader_data->guid.guidPrefix = pdata.guid.guidPrefix;
        temp_reader_data->guid.entityId = reader_entity_id;
        temp_reader_data->set_remote_locators(pdata.metatraffic_locators, network, use_multicast_locators,
                pdata.is_from_this_host());
        temp_reader_data->reliability.kind = reliability_kind;
        temp_reader_data->durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
#if HAVE_SECURITY
        if (notify_secure_endpoints)
        {
            if (!mp_RTPSParticipant->security_manager().discovered_builtin_reader(
                        writer->getGuid(), pdata.guid, *temp_reader_data,
                        writer->getAttributes().security_attributes()))
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Security manager returns an error for reader " <<
                        temp_reader_data->guid);
            }
        }
        else
#endif // HAVE_SECURITY
        {
            writer->matched_reader_add_edp(*temp_reader_data);
        }
    }
}

void PDPSimple::assign_low_level_remote_endpoints(
        const ParticipantProxyData& pdata,
        bool notify_secure_endpoints)
{
    if (mp_builtin->mp_WLP != nullptr)
    {
        mp_builtin->mp_WLP->assignRemoteEndpoints(pdata, notify_secure_endpoints);
    }

    if (nullptr != mp_builtin->typelookup_manager_)
    {
        mp_builtin->typelookup_manager_->assign_remote_endpoints(pdata);
    }

    if (mp_EDP != nullptr)
    {
        mp_EDP->assignRemoteEndpoints(pdata, notify_secure_endpoints);
    }
}

#if HAVE_SECURITY
bool PDPSimple::pairing_remote_writer_with_local_reader_after_security(
        const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
{
    auto endpoints = dynamic_cast<fastdds::rtps::SimplePDPEndpointsSecure*>(builtin_endpoints_.get());
    if ((nullptr != endpoints) && (local_reader == endpoints->secure_reader.reader_->getGuid()))
    {
        endpoints->secure_reader.reader_->matched_writer_add_edp(remote_writer_data);
        return true;
    }

    return PDP::pairing_remote_writer_with_local_reader_after_security(local_reader, remote_writer_data);
}

bool PDPSimple::pairing_remote_reader_with_local_writer_after_security(
        const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    auto endpoints = dynamic_cast<fastdds::rtps::SimplePDPEndpointsSecure*>(builtin_endpoints_.get());
    if ((nullptr != endpoints) && (local_writer == endpoints->secure_writer.writer_->getGuid()))
    {
        endpoints->secure_writer.writer_->matched_reader_add_edp(remote_reader_data);
        return true;
    }

    return PDP::pairing_remote_reader_with_local_writer_after_security(local_writer, remote_reader_data);
}

#endif // HAVE_SECURITY

bool PDPSimple::newRemoteEndpointStaticallyDiscovered(
        const GUID_t& pguid,
        int16_t userDefinedId,
        EndpointKind_t kind)
{
    fastcdr::string_255 pname;
    if (lookup_participant_name(pguid, pname))
    {
        if (kind == WRITER)
        {
            dynamic_cast<EDPStatic*>(mp_EDP)->newRemoteWriter(pguid, pname, userDefinedId);
        }
        else
        {
            dynamic_cast<EDPStatic*>(mp_EDP)->newRemoteReader(pguid, pname, userDefinedId);
        }
    }
    return false;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
