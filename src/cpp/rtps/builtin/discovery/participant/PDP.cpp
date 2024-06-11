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
 * @file PDP.cpp
 *
 */

#include <rtps/builtin/discovery/participant/PDP.h>

#include <mutex>
#include <chrono>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/builtin/data/BuiltinEndpoints.hpp>
#include <fastdds/rtps/builtin/data/ParticipantProxyData.hpp>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>

#include <fastdds/builtin/type_lookup_service/TypeLookupManager.hpp>
#include <fastdds/utils/IPLocator.h>
#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/data/ProxyHashTables.hpp>
#include <rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <rtps/builtin/discovery/endpoint/EDPStatic.h>
#include <rtps/builtin/discovery/participant/PDPEndpoints.hpp>
#include <rtps/builtin/discovery/participant/PDPListener.h>
#include <rtps/builtin/liveliness/WLP.hpp>
#include <rtps/history/TopicPayloadPoolRegistry.hpp>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/reader/StatelessReader.hpp>
#include <rtps/writer/StatelessWriter.hpp>
#include <rtps/resources/TimedEvent.h>
#if HAVE_SECURITY
#include <rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#endif // if HAVE_SECURITY
#include <utils/shared_mutex.hpp>
#include <utils/TimeConversion.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

// Default configuration values for PDP reliable entities.
const Duration_t pdp_heartbeat_period{ 0, 350 * 1000000  }; // 350 milliseconds
const Duration_t pdp_nack_response_delay{ 0, 100 * 1000000  }; // 100 milliseconds
const Duration_t pdp_nack_supression_duration{ 0, 11 * 1000000 }; // 11 milliseconds
const Duration_t pdp_heartbeat_response_delay{ 0, 11 * 1000000 }; // 11 milliseconds

const int32_t pdp_initial_reserved_caches = 20;


PDP::PDP (
        BuiltinProtocols* built,
        const RTPSParticipantAllocationAttributes& allocation)
    : mp_builtin(built)
    , mp_RTPSParticipant(nullptr)
    , mp_EDP(nullptr)
    , participant_proxies_number_(allocation.participants.initial)
    , participant_proxies_(allocation.participants)
    , participant_proxies_pool_(allocation.participants)
    , reader_proxies_number_(allocation.total_readers().initial)
    , reader_proxies_pool_(allocation.total_readers())
    , writer_proxies_number_(allocation.total_writers().initial)
    , writer_proxies_pool_(allocation.total_writers())
    , m_hasChangedLocalPDP(true)
    , temp_reader_proxies_({
                allocation.locators.max_unicast_locators,
                allocation.locators.max_multicast_locators,
                allocation.data_limits,
                allocation.content_filter})
    , temp_writer_proxies_({
                allocation.locators.max_unicast_locators,
                allocation.locators.max_multicast_locators,
                allocation.data_limits})
    , mp_mutex(new std::recursive_mutex())
#ifdef FASTDDS_STATISTICS
    , proxy_observer_(nullptr)
#endif // ifdef FASTDDS_STATISTICS
    , resend_participant_info_event_(nullptr)
{
    size_t max_unicast_locators = allocation.locators.max_unicast_locators;
    size_t max_multicast_locators = allocation.locators.max_multicast_locators;

    for (size_t i = 0; i < allocation.participants.initial; ++i)
    {
        participant_proxies_pool_.push_back(new ParticipantProxyData(allocation));
    }

    for (size_t i = 0; i < allocation.total_readers().initial; ++i)
    {
        reader_proxies_pool_.push_back(new ReaderProxyData(max_unicast_locators, max_multicast_locators,
                allocation.data_limits, allocation.content_filter));
    }

    for (size_t i = 0; i < allocation.total_writers().initial; ++i)
    {
        writer_proxies_pool_.push_back(new WriterProxyData(max_unicast_locators, max_multicast_locators,
                allocation.data_limits));
    }
}

PDP::~PDP()
{
    delete resend_participant_info_event_;

    builtin_endpoints_->disable_pdp_readers(mp_RTPSParticipant);

    delete mp_EDP;

    builtin_endpoints_->delete_pdp_endpoints(mp_RTPSParticipant);
    builtin_endpoints_.reset();

    for (ParticipantProxyData* it : participant_proxies_)
    {
        delete it;
    }

    for (ParticipantProxyData* it : participant_proxies_pool_)
    {
        delete it;
    }

    for (ReaderProxyData* it : reader_proxies_pool_)
    {
        delete it;
    }

    for (WriterProxyData* it : writer_proxies_pool_)
    {
        delete it;
    }

    delete mp_mutex;
}

ParticipantProxyData* PDP::add_participant_proxy_data(
        const GUID_t& participant_guid,
        bool with_lease_duration,
        const ParticipantProxyData* participant_proxy_data)
{
    ParticipantProxyData* ret_val = nullptr;

    // Try to take one entry from the pool
    if (participant_proxies_pool_.empty())
    {
        size_t max_proxies = participant_proxies_.max_size();
        if (participant_proxies_number_ < max_proxies)
        {
            // Pool is empty but limit has not been reached, so we create a new entry.
            ++participant_proxies_number_;
            ret_val = new ParticipantProxyData(mp_RTPSParticipant->getRTPSParticipantAttributes().allocation);
            if (participant_guid != mp_RTPSParticipant->getGuid())
            {
                ret_val->lease_duration_event = new TimedEvent(mp_RTPSParticipant->getEventResource(),
                                [this, ret_val]() -> bool
                                {
                                    check_remote_participant_liveliness(ret_val);
                                    return false;
                                }, 0.0);
            }
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_PDP, "Maximum number of participant proxies (" << max_proxies << \
                    ") reached for participant " << mp_RTPSParticipant->getGuid() << std::endl);
            return nullptr;
        }
    }
    else
    {
        // Pool is not empty, use entry from pool
        ret_val = participant_proxies_pool_.back();
        participant_proxies_pool_.pop_back();
    }

    // Add returned entry to the collection
    ret_val->should_check_lease_duration = with_lease_duration;
    ret_val->m_guid = participant_guid;
    if (nullptr != participant_proxy_data)
    {
        ret_val->copy(*participant_proxy_data);
        ret_val->isAlive = true;
        // Notify discovery of remote participant
        getRTPSParticipant()->on_entity_discovery(participant_guid, ret_val->m_properties);
    }
    participant_proxies_.push_back(ret_val);

    return ret_val;
}

bool PDP::data_matches_with_prefix(
        const GuidPrefix_t& guid_prefix,
        const ParticipantProxyData& participant_data)
{
    bool ret_val = (guid_prefix == participant_data.m_guid.guidPrefix);

#if HAVE_SECURITY
    if (!ret_val)
    {
        GUID_t guid = GUID_t(guid_prefix, c_EntityId_RTPSParticipant);
        return getRTPSParticipant()->security_manager().check_guid_comes_from(participant_data.m_guid, guid);
    }
#endif  // HAVE_SECURITY

    return ret_val;
}

void PDP::initializeParticipantProxyData(
        ParticipantProxyData* participant_data)
{
    RTPSParticipantAttributes& attributes = mp_RTPSParticipant->getAttributes();
    bool announce_locators = !mp_RTPSParticipant->is_intraprocess_only();

    participant_data->m_leaseDuration = attributes.builtin.discovery_config.leaseDuration;
    //set_VendorId_eProsima(participant_data->m_VendorId);
    participant_data->m_VendorId = c_VendorId_eProsima;

    // TODO: participant_data->m_availableBuiltinEndpoints |= mp_builtin->available_builtin_endpoints();

    participant_data->m_availableBuiltinEndpoints |= builtin_endpoints_->builtin_endpoints();

    if (attributes.builtin.use_WriterLivelinessProtocol)
    {
        participant_data->m_availableBuiltinEndpoints |=
                fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
        participant_data->m_availableBuiltinEndpoints |=
                fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;

#if HAVE_SECURITY
        if (mp_RTPSParticipant->is_secure())
        {
            participant_data->m_availableBuiltinEndpoints |=
                    fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_WRITER;
            participant_data->m_availableBuiltinEndpoints |=
                    fastdds::rtps::BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_READER;
        }
#endif // if HAVE_SECURITY
    }
    participant_data->m_availableBuiltinEndpoints |=
            fastdds::rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_READER;
    participant_data->m_availableBuiltinEndpoints |=
            fastdds::rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_WRITER;

    participant_data->m_availableBuiltinEndpoints |=
            fastdds::rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REQUEST_DATA_WRITER;
    participant_data->m_availableBuiltinEndpoints |=
            fastdds::rtps::BUILTIN_ENDPOINT_TYPELOOKUP_SERVICE_REPLY_DATA_READER;

#if HAVE_SECURITY
    if (mp_RTPSParticipant->is_secure())
    {
        participant_data->m_availableBuiltinEndpoints |= mp_RTPSParticipant->security_manager().builtin_endpoints();
    }
#endif // if HAVE_SECURITY

    if (announce_locators)
    {
        participant_data->m_networkConfiguration = attributes.builtin.network_configuration;

        for (const Locator_t& loc : attributes.defaultUnicastLocatorList)
        {
            participant_data->default_locators.add_unicast_locator(loc);
        }
        for (const Locator_t& loc : attributes.defaultMulticastLocatorList)
        {
            participant_data->default_locators.add_multicast_locator(loc);
        }
    }
    participant_data->m_expectsInlineQos = false;
    participant_data->m_guid = mp_RTPSParticipant->getGuid();
    memcpy( participant_data->m_key.value, participant_data->m_guid.guidPrefix.value, 12);
    memcpy( participant_data->m_key.value + 12, participant_data->m_guid.entityId.value, 4);

    // Keep persistence Guid_Prefix_t in a specific property. This info must be propagated to all builtin endpoints
    {
        // Use user set persistence guid
        GuidPrefix_t persistent = mp_RTPSParticipant->get_persistence_guid_prefix();

        // If it has not been set, use guid
        if (persistent == c_GuidPrefix_Unknown)
        {
            persistent = attributes.prefix;
        }

        // If persistent is set, set it into the participant proxy
        if (persistent != c_GuidPrefix_Unknown)
        {
            participant_data->set_persistence_guid(
                GUID_t(
                    persistent,
                    c_EntityId_RTPSParticipant));
        }
    }

    participant_data->metatraffic_locators.unicast.clear();
    if (announce_locators)
    {
        for (const Locator_t& loc : mp_builtin->m_metatrafficUnicastLocatorList)
        {
            participant_data->metatraffic_locators.add_unicast_locator(loc);
        }
    }

    participant_data->metatraffic_locators.multicast.clear();
    if (announce_locators)
    {
        if (!m_discovery.avoid_builtin_multicast || participant_data->metatraffic_locators.unicast.empty())
        {
            for (const Locator_t& loc: mp_builtin->m_metatrafficMulticastLocatorList)
            {
                participant_data->metatraffic_locators.add_multicast_locator(loc);
            }
        }

        fastdds::rtps::network::external_locators::add_external_locators(*participant_data,
                attributes.builtin.metatraffic_external_unicast_locators,
                attributes.default_external_unicast_locators);
    }

    participant_data->m_participantName = std::string(attributes.getName());

    participant_data->m_userData = attributes.userData;

#if HAVE_SECURITY
    if (mp_RTPSParticipant->is_secure())
    {
        IdentityToken* identity_token = nullptr;
        if (mp_RTPSParticipant->security_manager().get_identity_token(&identity_token) && (nullptr != identity_token))
        {
            participant_data->identity_token_ = std::move(*identity_token);
            mp_RTPSParticipant->security_manager().return_identity_token(identity_token);
        }

        PermissionsToken* permissions_token = nullptr;
        if (mp_RTPSParticipant->security_manager().get_permissions_token(&permissions_token)
                && (nullptr != permissions_token))
        {
            participant_data->permissions_token_ = std::move(*permissions_token);
            mp_RTPSParticipant->security_manager().return_permissions_token(permissions_token);
        }

        const security::ParticipantSecurityAttributes& sec_attrs = mp_RTPSParticipant->security_attributes();
        participant_data->security_attributes_ = sec_attrs.mask();
        participant_data->plugin_security_attributes_ = sec_attrs.plugin_participant_attributes;
    }
    else
    {
        participant_data->security_attributes_ = 0UL;
        participant_data->plugin_security_attributes_ = 0UL;
    }
#endif // if HAVE_SECURITY

    // Set properties that will be sent to Proxy Data
    set_external_participant_properties_(participant_data);
}

bool PDP::initPDP(
        RTPSParticipantImpl* part)
{
    EPROSIMA_LOG_INFO(RTPS_PDP, "Beginning");
    mp_RTPSParticipant = part;
    m_discovery = mp_RTPSParticipant->getAttributes().builtin;
    initial_announcements_ = m_discovery.discovery_config.initial_announcements;
    //CREATE ENDPOINTS
    if (!createPDPEndpoints())
    {
        return false;
    }
    //UPDATE METATRAFFIC.
    update_builtin_locators();

    ParticipantProxyData* pdata = nullptr;
    {
        std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);
        pdata = add_participant_proxy_data(mp_RTPSParticipant->getGuid(), false, nullptr);
    }

    if (nullptr == pdata)
    {
        return false;
    }
    initializeParticipantProxyData(pdata);

    return true;
}

bool PDP::enable()
{
    // It is safe to call enable() on already enable PDPs
    if (enabled_)
    {
        return true;
    }

    // Create lease events on already created proxy data objects
    for (ParticipantProxyData* pool_item : participant_proxies_pool_)
    {
        pool_item->lease_duration_event = new TimedEvent(mp_RTPSParticipant->getEventResource(),
                        [this, pool_item]() -> bool
                        {
                            check_remote_participant_liveliness(pool_item);
                            return false;
                        }, 0.0);
    }

    resend_participant_info_event_ = new TimedEvent(mp_RTPSParticipant->getEventResource(),
                    [&]() -> bool
                    {
                        announceParticipantState(false);
                        set_next_announcement_interval();
                        return true;
                    },
                    0);

    set_initial_announcement_interval();

    enabled_.store(true);
    // Notify "self-discovery"
    getRTPSParticipant()->on_entity_discovery(mp_RTPSParticipant->getGuid(),
            get_participant_proxy_data(mp_RTPSParticipant->getGuid().guidPrefix)->m_properties);

    return builtin_endpoints_->enable_pdp_readers(mp_RTPSParticipant);
}

void PDP::disable()
{
    // Extract all the participant proxies excluding first one (ourselves)
    std::vector<ParticipantProxyData*> participants;
    {
        std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);
        participants.insert(participants.end(), participant_proxies_.begin() + 1, participant_proxies_.end());
        participant_proxies_.erase(participant_proxies_.begin() + 1, participant_proxies_.end());
    }

    // Unmatch all remote participants
    for (ParticipantProxyData* pdata : participants)
    {
        actions_on_remote_participant_removed(pdata, pdata->m_guid,
                ParticipantDiscoveryInfo::DISCOVERY_STATUS::REMOVED_PARTICIPANT, nullptr);
    }
}

void PDP::announceParticipantState(
        bool new_change,
        bool dispose /* = false */)
{
    WriteParams __wp = WriteParams::write_params_default();
    announceParticipantState(new_change, dispose, __wp);
}

void PDP::announceParticipantState(
        RTPSWriter& writer,
        WriterHistory& history,
        bool new_change,
        bool dispose,
        WriteParams& wparams)
{
    if (enabled_)
    {
        // EPROSIMA_LOG_INFO(RTPS_PDP, "Announcing RTPSParticipant State (new change: " << new_change << ")");
        CacheChange_t* change = nullptr;

        if (!dispose)
        {
            if (m_hasChangedLocalPDP.exchange(false) || new_change)
            {
                mp_mutex->lock();
                ParticipantProxyData* local_participant_data = getLocalParticipantProxyData();
                InstanceHandle_t key = local_participant_data->m_key;
                ParticipantProxyData proxy_data_copy(*local_participant_data);
                mp_mutex->unlock();

                if (history.getHistorySize() > 0)
                {
                    history.remove_min_change();
                }
                uint32_t cdr_size = proxy_data_copy.get_serialized_size(true);
                change = writer.new_change(
                    [cdr_size]() -> uint32_t
                    {
                        return cdr_size;
                    },
                    ALIVE, key);

                if (nullptr != change)
                {
                    CDRMessage_t aux_msg(change->serializedPayload);

#if __BIG_ENDIAN__
                    change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
                    aux_msg.msg_endian = BIGEND;
#else
                    change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
                    aux_msg.msg_endian =  LITTLEEND;
#endif // if __BIG_ENDIAN__

                    if (proxy_data_copy.writeToCDRMessage(&aux_msg, true))
                    {
                        change->serializedPayload.length = (uint16_t)aux_msg.length;

                        history.add_change(change, wparams);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(RTPS_PDP, "Cannot serialize ParticipantProxyData.");
                    }
                }
            }

        }
        else
        {
            mp_mutex->lock();
            ParticipantProxyData* local_participant_data = getLocalParticipantProxyData();
            InstanceHandle_t key = local_participant_data->m_key;
            ParticipantProxyData proxy_data_copy(*local_participant_data);
            mp_mutex->unlock();

            if (history.getHistorySize() > 0)
            {
                history.remove_min_change();
            }
            uint32_t cdr_size = proxy_data_copy.get_serialized_size(true);
            change = writer.new_change([cdr_size]() -> uint32_t
                            {
                                return cdr_size;
                            },
                            NOT_ALIVE_DISPOSED_UNREGISTERED, key);

            if (nullptr != change)
            {
                CDRMessage_t aux_msg(change->serializedPayload);

#if __BIG_ENDIAN__
                change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
                aux_msg.msg_endian = BIGEND;
#else
                change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
                aux_msg.msg_endian =  LITTLEEND;
#endif // if __BIG_ENDIAN__

                if (proxy_data_copy.writeToCDRMessage(&aux_msg, true))
                {
                    change->serializedPayload.length = (uint16_t)aux_msg.length;

                    history.add_change(change, wparams);
                }
                else
                {
                    EPROSIMA_LOG_ERROR(RTPS_PDP, "Cannot serialize ParticipantProxyData.");
                }
            }
        }
    }
}

void PDP::stopParticipantAnnouncement()
{
    if (resend_participant_info_event_)
    {
        resend_participant_info_event_->cancel_timer();
    }
}

void PDP::resetParticipantAnnouncement()
{
    if (resend_participant_info_event_)
    {
        resend_participant_info_event_->restart_timer();
    }
}

void PDP::notify_and_maybe_ignore_new_participant(
        ParticipantProxyData* pdata,
        bool& should_be_ignored)
{
    should_be_ignored = false;

    EPROSIMA_LOG_INFO(RTPS_PDP_DISCOVERY, "New participant "
            << pdata->m_guid << " at "
            << "MTTLoc: " << pdata->metatraffic_locators
            << " DefLoc:" << pdata->default_locators);

    RTPSParticipantListener* listener = getRTPSParticipant()->getListener();
    if (listener)
    {
        {
            std::lock_guard<std::mutex> cb_lock(callback_mtx_);
            ParticipantDiscoveryInfo info(*pdata);
            info.status = ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;


            listener->onParticipantDiscovery(
                getRTPSParticipant()->getUserRTPSParticipant(),
                std::move(info),
                should_be_ignored);
        }

        if (should_be_ignored)
        {
            getRTPSParticipant()->ignore_participant(pdata->m_guid.guidPrefix);
        }
    }
}

bool PDP::has_reader_proxy_data(
        const GUID_t& reader)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);
    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == reader.guidPrefix)
        {
            ProxyHashTable<ReaderProxyData>& readers = *pit->m_readers;
            return readers.find(reader.entityId) != readers.end();
        }
    }
    return false;
}

bool PDP::lookupReaderProxyData(
        const GUID_t& reader,
        ReaderProxyData& rdata)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);
    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == reader.guidPrefix)
        {
            auto rit = pit->m_readers->find(reader.entityId);
            if (rit != pit->m_readers->end())
            {
                rdata.copy(rit->second);
                return true;
            }
        }
    }
    return false;
}

bool PDP::has_writer_proxy_data(
        const GUID_t& writer)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);
    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == writer.guidPrefix)
        {
            ProxyHashTable<WriterProxyData>& writers = *pit->m_writers;
            return writers.find(writer.entityId) != writers.end();
        }
    }
    return false;
}

bool PDP::lookupWriterProxyData(
        const GUID_t& writer,
        WriterProxyData& wdata)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);
    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == writer.guidPrefix)
        {
            auto wit = pit->m_writers->find(writer.entityId);
            if ( wit != pit->m_writers->end())
            {
                wdata.copy(wit->second);
                return true;
            }
        }
    }
    return false;
}

bool PDP::removeReaderProxyData(
        const GUID_t& reader_guid)
{
    EPROSIMA_LOG_INFO(RTPS_PDP, "Removing reader proxy data " << reader_guid);
    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);

    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == reader_guid.guidPrefix)
        {
            auto rit = pit->m_readers->find(reader_guid.entityId);

            if (rit != pit->m_readers->end())
            {
                ReaderProxyData* pR = rit->second;
                mp_EDP->unpairReaderProxy(pit->m_guid, reader_guid);

                RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
                if (listener)
                {
                    RTPSParticipant* participant = mp_RTPSParticipant->getUserRTPSParticipant();
                    ReaderDiscoveryInfo info(std::move(*pR));
                    bool should_be_ignored = false;
                    info.status = ReaderDiscoveryInfo::REMOVED_READER;
                    listener->onReaderDiscovery(participant, std::move(info), should_be_ignored);
                }

                // Clear reader proxy data and move to pool in order to allow reuse
                pR->clear();
                pit->m_readers->erase(rit);
                reader_proxies_pool_.push_back(pR);
                return true;
            }
        }
    }

    return false;
}

bool PDP::removeReaderProxyData(
        const GUID_t& /*reader_guid*/,
        ReaderDiscoveryInfo::DISCOVERY_STATUS /*reason*/)
{
    return false;
}

bool PDP::removeWriterProxyData(
        const GUID_t& writer_guid)
{
    EPROSIMA_LOG_INFO(RTPS_PDP, "Removing writer proxy data " << writer_guid);
    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);

    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == writer_guid.guidPrefix)
        {
            auto wit = pit->m_writers->find(writer_guid.entityId);

            if (wit != pit->m_writers->end())
            {
                WriterProxyData* pW = wit->second;
                mp_EDP->unpairWriterProxy(pit->m_guid, writer_guid, false);

                RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
                if (listener)
                {
                    RTPSParticipant* participant = mp_RTPSParticipant->getUserRTPSParticipant();
                    WriterDiscoveryInfo info(std::move(*pW));
                    bool should_be_ignored = false;
                    info.status = WriterDiscoveryInfo::REMOVED_WRITER;
                    listener->onWriterDiscovery(participant, std::move(info), should_be_ignored);
                }

                // Clear writer proxy data and move to pool in order to allow reuse
                pW->clear();
                pit->m_writers->erase(wit);
                writer_proxies_pool_.push_back(pW);

                return true;
            }
        }
    }

    return false;
}

bool PDP::removeWriterProxyData(
        const GUID_t& /*writer_guid*/,
        WriterDiscoveryInfo::DISCOVERY_STATUS /*reason*/)
{
    return false;
}

bool PDP::lookup_participant_name(
        const GUID_t& guid,
        fastcdr::string_255& name)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);
    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid == guid)
        {
            name = pit->m_participantName;
            return true;
        }
    }
    return false;
}

bool PDP::lookup_participant_key(
        const GUID_t& participant_guid,
        InstanceHandle_t& key)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);
    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid == participant_guid)
        {
            key = pit->m_key;
            return true;
        }
    }
    return false;
}

ReaderProxyData* PDP::addReaderProxyData(
        const GUID_t& reader_guid,
        GUID_t& participant_guid,
        std::function<bool(ReaderProxyData*, bool, const ParticipantProxyData&)> initializer_func)
{
    EPROSIMA_LOG_INFO(RTPS_PDP, "Adding reader proxy data " << reader_guid);
    ReaderProxyData* ret_val = nullptr;

    // notify statistics module
    getRTPSParticipant()->on_entity_discovery(reader_guid, ParameterPropertyList_t());

    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);

    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == reader_guid.guidPrefix)
        {
            // Copy participant data to be used outside.
            participant_guid = pit->m_guid;

            // Check that it is not already there:
            auto rpi = pit->m_readers->find(reader_guid.entityId);

            if ( rpi != pit->m_readers->end())
            {
                ret_val = rpi->second;

                if (!initializer_func(ret_val, true, *pit))
                {
                    return nullptr;
                }

                RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
                if (listener)
                {
                    RTPSParticipant* participant = mp_RTPSParticipant->getUserRTPSParticipant();
                    ReaderDiscoveryInfo info(*ret_val);
                    bool should_be_ignored = false;
                    info.status = ReaderDiscoveryInfo::CHANGED_QOS_READER;
                    listener->onReaderDiscovery(participant, std::move(info), should_be_ignored);
                }

                return ret_val;
            }

            // Try to take one entry from the pool
            if (reader_proxies_pool_.empty())
            {
                size_t max_proxies = reader_proxies_pool_.max_size();
                if (reader_proxies_number_ < max_proxies)
                {
                    // Pool is empty but limit has not been reached, so we create a new entry.
                    ++reader_proxies_number_;
                    ret_val = new ReaderProxyData(
                        mp_RTPSParticipant->getAttributes().allocation.locators.max_unicast_locators,
                        mp_RTPSParticipant->getAttributes().allocation.locators.max_multicast_locators,
                        mp_RTPSParticipant->getAttributes().allocation.data_limits,
                        mp_RTPSParticipant->getAttributes().allocation.content_filter);
                }
                else
                {
                    EPROSIMA_LOG_WARNING(RTPS_PDP, "Maximum number of reader proxies (" << max_proxies <<
                            ") reached for participant " << mp_RTPSParticipant->getGuid() << std::endl);
                    return nullptr;
                }
            }
            else
            {
                // Pool is not empty, use entry from pool
                ret_val = reader_proxies_pool_.back();
                reader_proxies_pool_.pop_back();
            }

            // Copy network configuration from participant to reader proxy
            ret_val->networkConfiguration(pit->m_networkConfiguration);

            // Add to ParticipantProxyData
            (*pit->m_readers)[reader_guid.entityId] = ret_val;

            if (!initializer_func(ret_val, false, *pit))
            {
                return nullptr;
            }

            RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
            if (listener)
            {
                RTPSParticipant* participant = mp_RTPSParticipant->getUserRTPSParticipant();
                ReaderDiscoveryInfo info(*ret_val);
                bool should_be_ignored = false;
                info.status = ReaderDiscoveryInfo::DISCOVERED_READER;
                listener->onReaderDiscovery(participant, std::move(info), should_be_ignored);
            }

            return ret_val;
        }
    }

    return nullptr;
}

WriterProxyData* PDP::addWriterProxyData(
        const GUID_t& writer_guid,
        GUID_t& participant_guid,
        std::function<bool(WriterProxyData*, bool, const ParticipantProxyData&)> initializer_func)
{
    EPROSIMA_LOG_INFO(RTPS_PDP, "Adding writer proxy data " << writer_guid);
    WriterProxyData* ret_val = nullptr;

    // notify statistics module
    getRTPSParticipant()->on_entity_discovery(writer_guid, ParameterPropertyList_t());

    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);

    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == writer_guid.guidPrefix)
        {
            // Copy participant data to be used outside.
            participant_guid = pit->m_guid;

            // Check that it is not already there:
            auto wpi = pit->m_writers->find(writer_guid.entityId);

            if (wpi != pit->m_writers->end())
            {
                ret_val = wpi->second;

                if (!initializer_func(ret_val, true, *pit))
                {
                    return nullptr;
                }

                RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
                if (listener)
                {
                    RTPSParticipant* participant = mp_RTPSParticipant->getUserRTPSParticipant();
                    WriterDiscoveryInfo info(*ret_val);
                    bool should_be_ignored = false;
                    info.status = WriterDiscoveryInfo::CHANGED_QOS_WRITER;
                    listener->onWriterDiscovery(participant, std::move(info), should_be_ignored);
                }

                return ret_val;
            }

            // Try to take one entry from the pool
            if (writer_proxies_pool_.empty())
            {
                size_t max_proxies = writer_proxies_pool_.max_size();
                if (writer_proxies_number_ < max_proxies)
                {
                    // Pool is empty but limit has not been reached, so we create a new entry.
                    ++writer_proxies_number_;
                    ret_val = new WriterProxyData(
                        mp_RTPSParticipant->getAttributes().allocation.locators.max_unicast_locators,
                        mp_RTPSParticipant->getAttributes().allocation.locators.max_multicast_locators,
                        mp_RTPSParticipant->getAttributes().allocation.data_limits);
                }
                else
                {
                    EPROSIMA_LOG_WARNING(RTPS_PDP, "Maximum number of writer proxies (" << max_proxies <<
                            ") reached for participant " << mp_RTPSParticipant->getGuid() << std::endl);
                    return nullptr;
                }
            }
            else
            {
                // Pool is not empty, use entry from pool
                ret_val = writer_proxies_pool_.back();
                writer_proxies_pool_.pop_back();
            }

            // Copy network configuration from participant to writer proxy
            ret_val->networkConfiguration(pit->m_networkConfiguration);

            // Add to ParticipantProxyData
            (*pit->m_writers)[writer_guid.entityId] = ret_val;

            if (!initializer_func(ret_val, false, *pit))
            {
                return nullptr;
            }

            RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
            if (listener)
            {
                RTPSParticipant* participant = mp_RTPSParticipant->getUserRTPSParticipant();
                WriterDiscoveryInfo info(*ret_val);
                bool should_be_ignored = false;
                info.status = WriterDiscoveryInfo::DISCOVERED_WRITER;
                listener->onWriterDiscovery(participant, std::move(info), should_be_ignored);
            }

            return ret_val;
        }
    }

    return nullptr;
}

#if HAVE_SECURITY
bool PDP::pairing_remote_writer_with_local_reader_after_security(
        const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data)
{
    return mp_EDP->pairing_remote_writer_with_local_reader_after_security(local_reader, remote_writer_data);
}

bool PDP::pairing_remote_reader_with_local_writer_after_security(
        const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data)
{
    return mp_EDP->pairing_remote_reader_with_local_writer_after_security(local_writer, remote_reader_data);
}

#endif // HAVE_SECURITY

#ifdef FASTDDS_STATISTICS
bool PDP::get_all_local_proxies(
        std::vector<GUID_t>& guids)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);
    ParticipantProxyData* local_participant = getLocalParticipantProxyData();
    guids.reserve(local_participant->m_writers->size() +
            local_participant->m_readers->size() +
            1);

    //! Add the Participant entity to the local entities
    guids.push_back(local_participant->m_guid);

    // Add all the writers and readers belonging to the participant
    for (auto& writer : *(local_participant->m_writers))
    {
        guids.push_back(writer.second->guid());
    }

    for (auto& reader : *(local_participant->m_readers))
    {
        guids.push_back(reader.second->guid());
    }

    return true;
}

bool PDP::get_serialized_proxy(
        const GUID_t& guid,
        CDRMessage_t* msg)
{
    bool ret = false;
    bool found = false;

    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);

    if (guid.entityId == c_EntityId_RTPSParticipant)
    {
        for (auto part_proxy = participant_proxies_.begin();
                part_proxy != participant_proxies_.end(); ++part_proxy)
        {
            if ((*part_proxy)->m_guid == guid)
            {
                msg->msg_endian = LITTLEEND;
                msg->max_size = msg->reserved_size = (*part_proxy)->get_serialized_size(true);
                ret = (*part_proxy)->writeToCDRMessage(msg, true);
                found = true;
                break;
            }
        }

        if (!found)
        {
            EPROSIMA_LOG_ERROR(PDP, "Unknown participant proxy requested to serialize: " << guid);
        }
    }
    else if (guid.entityId.is_reader())
    {
        for (auto part_proxy = participant_proxies_.begin();
                part_proxy != participant_proxies_.end(); ++part_proxy)
        {
            if ((*part_proxy)->m_guid.guidPrefix == guid.guidPrefix)
            {
                for (auto& reader : *((*part_proxy)->m_readers))
                {
                    if (reader.second->guid() == guid)
                    {
                        msg->max_size = msg->reserved_size = reader.second->get_serialized_size(true);
                        ret = reader.second->writeToCDRMessage(msg, true);
                        found = true;
                        break;
                    }
                }
                break;
            }
        }

        if (!found)
        {
            EPROSIMA_LOG_ERROR(PDP, "Unknown reader proxy requested to serialize: " << guid);
        }
    }
    else if (guid.entityId.is_writer())
    {
        for (auto part_proxy = participant_proxies_.begin();
                part_proxy != participant_proxies_.end(); ++part_proxy)
        {
            if ((*part_proxy)->m_guid.guidPrefix == guid.guidPrefix)
            {
                for (auto& writer : *((*part_proxy)->m_writers))
                {
                    if (writer.second->guid() == guid)
                    {
                        msg->max_size = msg->reserved_size = writer.second->get_serialized_size(true);
                        ret = writer.second->writeToCDRMessage(msg, true);
                        found = true;
                        break;
                    }
                }
                break;
            }
        }

        if (!found)
        {
            EPROSIMA_LOG_ERROR(PDP, "Unknown writer proxy requested to serialize: " << guid);
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(PDP, "Unknown entitiy kind requested to serialize: " << guid);
    }

    return ret;
}

void PDP::set_proxy_observer(
        const fastdds::statistics::rtps::IProxyObserver* proxy_observer)
{
    proxy_observer_.store(proxy_observer);
}

#endif // FASTDDS_STATISTICS

bool PDP::remove_remote_participant(
        const GUID_t& partGUID,
        ParticipantDiscoveryInfo::DISCOVERY_STATUS reason)
{
    if (partGUID == getLocalParticipantProxyData()->m_guid)
    {
        // avoid removing our own data
        return false;
    }

    EPROSIMA_LOG_INFO(RTPS_PDP, partGUID );
    ParticipantProxyData* pdata = nullptr;

    // Remove it from our vector of RTPSParticipantProxies
    {
        std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);
        for (ResourceLimitedVector<ParticipantProxyData*>::iterator pit = participant_proxies_.begin();
                pit != participant_proxies_.end(); ++pit)
        {
            if ((*pit)->m_guid == partGUID)
            {
                pdata = *pit;
                participant_proxies_.erase(pit);
                break;
            }
        }
    }

    if (nullptr != pdata)
    {
        RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
        actions_on_remote_participant_removed(pdata, partGUID, reason, listener);
        return true;
    }

    return false;
}

void PDP::actions_on_remote_participant_removed(
        ParticipantProxyData* pdata,
        const GUID_t& partGUID,
        ParticipantDiscoveryInfo::DISCOVERY_STATUS reason,
        RTPSParticipantListener* listener)
{
    assert(nullptr != pdata);

    if (nullptr != mp_EDP)
    {
        for (auto pit : *pdata->m_readers)
        {
            ReaderProxyData* rit = pit.second;
            GUID_t reader_guid(rit->guid());
            if (reader_guid != c_Guid_Unknown)
            {
                mp_EDP->unpairReaderProxy(partGUID, reader_guid);

                if (listener)
                {
                    RTPSParticipant* participant = mp_RTPSParticipant->getUserRTPSParticipant();
                    ReaderDiscoveryInfo info(std::move(*rit));
                    bool should_be_ignored = false;
                    info.status = ReaderDiscoveryInfo::REMOVED_READER;
                    listener->onReaderDiscovery(participant, std::move(info), should_be_ignored);
                }
            }
        }
        for (auto pit : *pdata->m_writers)
        {
            WriterProxyData* wit = pit.second;
            GUID_t writer_guid(wit->guid());
            if (writer_guid != c_Guid_Unknown)
            {
                mp_EDP->unpairWriterProxy(partGUID, writer_guid,
                        reason == ParticipantDiscoveryInfo::DISCOVERY_STATUS::DROPPED_PARTICIPANT);

                if (listener)
                {
                    RTPSParticipant* participant = mp_RTPSParticipant->getUserRTPSParticipant();
                    WriterDiscoveryInfo info(std::move(*wit));
                    bool should_be_ignored = false;
                    info.status = WriterDiscoveryInfo::REMOVED_WRITER;
                    listener->onWriterDiscovery(participant, std::move(info), should_be_ignored);
                }
            }
        }
    }

    if (nullptr != mp_builtin->mp_WLP)
    {
        mp_builtin->mp_WLP->removeRemoteEndpoints(pdata);
    }

    mp_builtin->typelookup_manager_->remove_remote_endpoints(pdata);

    mp_EDP->removeRemoteEndpoints(pdata);
    removeRemoteEndpoints(pdata);

#if HAVE_SECURITY
    mp_builtin->mp_participantImpl->security_manager().remove_participant(*pdata);
#endif // if HAVE_SECURITY

    builtin_endpoints_->remove_from_pdp_reader_history(pdata->m_key);

    if (listener)
    {
        std::lock_guard<std::mutex> lock(callback_mtx_);
        ParticipantDiscoveryInfo info(*pdata);
        info.status = reason;
        bool should_be_ignored = false;
        listener->onParticipantDiscovery(mp_RTPSParticipant->getUserRTPSParticipant(), std::move(
                    info), should_be_ignored);
    }

    {
        std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);

        // Delete from sender resource list (TCP only)
        LocatorList_t remote_participant_locators;
        for (auto& remote_participant_default_locator : pdata->default_locators.unicast)
        {
            remote_participant_locators.push_back(remote_participant_default_locator);
        }
        for (auto& remote_participant_metatraffic_locator : pdata->metatraffic_locators.unicast)
        {
            remote_participant_locators.push_back(remote_participant_metatraffic_locator);
        }
        if (!remote_participant_locators.empty())
        {
            mp_RTPSParticipant->update_removed_participant(remote_participant_locators);
        }

        // Return reader proxy objects to pool
        for (auto pit : *pdata->m_readers)
        {
            pit.second->clear();
            reader_proxies_pool_.push_back(pit.second);
        }
        pdata->m_readers->clear();

        // Return writer proxy objects to pool
        for (auto pit : *pdata->m_writers)
        {
            pit.second->clear();
            writer_proxies_pool_.push_back(pit.second);
        }
        pdata->m_writers->clear();

        // Cancel lease event
        if (nullptr != pdata->lease_duration_event)
        {
            pdata->lease_duration_event->cancel_timer();
        }

        // Return proxy object to pool
        pdata->clear();
        participant_proxies_pool_.push_back(pdata);
    }
}

const BuiltinAttributes& PDP::builtin_attributes() const
{
    return mp_builtin->m_att;
}

void PDP::assert_remote_participant_liveliness(
        const GuidPrefix_t& remote_guid)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);

    for (ParticipantProxyData* it : participant_proxies_)
    {
        if (it->m_guid.guidPrefix == remote_guid)
        {
            // TODO Ricardo: Study if isAlive attribute is necessary.
            it->isAlive = true;
            it->assert_liveliness();
            break;
        }
    }
}

CDRMessage_t PDP::get_participant_proxy_data_serialized(
        Endianness_t endian)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*mp_mutex);
    CDRMessage_t cdr_msg(RTPSMESSAGE_DEFAULT_SIZE);
    cdr_msg.msg_endian = endian;

    if (!getLocalParticipantProxyData()->writeToCDRMessage(&cdr_msg, false))
    {
        cdr_msg.pos = 0;
        cdr_msg.length = 0;
    }

    return cdr_msg;
}

ParticipantProxyData* PDP::get_participant_proxy_data(
        const GuidPrefix_t& guid_prefix)
{
    for (auto pit = ParticipantProxiesBegin(); pit != ParticipantProxiesEnd(); ++pit)
    {
        if (data_matches_with_prefix(guid_prefix, **pit))
        {
            return *(pit);
        }
    }
    return nullptr;
}

std::list<eprosima::fastdds::rtps::RemoteServerAttributes>& PDP::remote_server_attributes()
{
    return mp_builtin->m_DiscoveryServers;
}

void PDP::check_remote_participant_liveliness(
        ParticipantProxyData* remote_participant)
{
    std::unique_lock<std::recursive_mutex> guard(*mp_mutex);

    if (remote_participant->should_check_lease_duration)
    {
        assert(GUID_t::unknown() != remote_participant->m_guid);
        // Check last received message's time_point plus lease duration time doesn't overcome now().
        // If overcame, remove participant.
        auto now = std::chrono::steady_clock::now();
        auto real_lease_tm = remote_participant->last_received_message_tm() +
                std::chrono::microseconds(fastdds::rtps::TimeConv::Duration_t2MicroSecondsInt64(remote_participant->
                                m_leaseDuration));
        if (now > real_lease_tm)
        {
            guard.unlock();
            remove_remote_participant(remote_participant->m_guid, ParticipantDiscoveryInfo::DROPPED_PARTICIPANT);
            return;
        }

        // Calculate next trigger.
        auto next_trigger = real_lease_tm - now;
        remote_participant->lease_duration_event->update_interval_millisec(
            (double)std::chrono::duration_cast<std::chrono::milliseconds>(next_trigger).count());
        remote_participant->lease_duration_event->restart_timer();
    }
}

void PDP::set_next_announcement_interval()
{
    if (initial_announcements_.count > 0)
    {
        --initial_announcements_.count;
        resend_participant_info_event_->update_interval(initial_announcements_.period);
    }
    else
    {
        resend_participant_info_event_->update_interval(m_discovery.discovery_config.leaseDuration_announcementperiod);
    }
}

void PDP::set_initial_announcement_interval()
{
    if ((initial_announcements_.count > 0) && (initial_announcements_.period <= c_TimeZero))
    {
        // Force a small interval (1ms) between initial announcements
        EPROSIMA_LOG_WARNING(RTPS_PDP, "Initial announcement period is not strictly positive. Changing to 1ms.");
        initial_announcements_.period = { 0, 1000000 };
    }
    set_next_announcement_interval();
}

void PDP::set_external_participant_properties_(
        ParticipantProxyData* participant_data)
{
    // For each property add it if it should be sent (it is propagated)
    for (auto const& property : mp_RTPSParticipant->getAttributes().properties.properties())
    {
        if (property.propagate())
        {
            participant_data->m_properties.push_back(property.name(), property.value());
        }
    }

    // Set participant type property
    // TODO: This could be done somewhere else that makes more sense.
    std::stringstream participant_type;
    participant_type << mp_RTPSParticipant->getAttributes().builtin.discovery_config.discoveryProtocol;
    auto ptype = participant_type.str();
    participant_data->m_properties.push_back(fastdds::dds::parameter_property_participant_type, ptype);

    // Add physical properties if present
    // TODO: This should be done using propagate value, however this cannot be done without breaking compatibility
    std::vector<std::string> physical_property_names = {
        fastdds::dds::parameter_policy_physical_data_host,
        fastdds::dds::parameter_policy_physical_data_user,
        fastdds::dds::parameter_policy_physical_data_process
    };
    for (auto physical_property_name : physical_property_names)
    {
        std::string* physical_property = PropertyPolicyHelper::find_property(
            mp_RTPSParticipant->getAttributes().properties, physical_property_name);
        if (nullptr != physical_property)
        {
            participant_data->m_properties.push_back(physical_property_name, *physical_property);
        }
    }
}

static void set_builtin_matched_allocation(
        ResourceLimitedContainerConfig& allocation,
        const RTPSParticipantAttributes& pattr)
{
    // Matched endpoints will depend on total number of participants
    allocation = pattr.allocation.participants;

    // As participants allocation policy includes the local participant, one has to be substracted
    if (allocation.initial > 1)
    {
        allocation.initial--;
    }
    if ((allocation.maximum > 1) &&
            (allocation.maximum < std::numeric_limits<size_t>::max()))
    {
        allocation.maximum--;
    }
}

static void set_builtin_endpoint_locators(
        EndpointAttributes& endpoint,
        const PDP* pdp,
        const BuiltinProtocols* builtin)
{
    const RTPSParticipantAttributes& pattr = pdp->getRTPSParticipant()->getRTPSParticipantAttributes();

    auto part_data = pdp->getLocalParticipantProxyData();
    if (nullptr == part_data)
    {
        // Local participant data has not yet been created.
        // This means we are creating the PDP endpoints, so we copy the locators from mp_builtin
        endpoint.multicastLocatorList = builtin->m_metatrafficMulticastLocatorList;
        endpoint.unicastLocatorList = builtin->m_metatrafficUnicastLocatorList;
    }
    else
    {
        // Locators are copied from the local participant metatraffic locators
        endpoint.unicastLocatorList.clear();
        for (const Locator_t& loc : part_data->metatraffic_locators.unicast)
        {
            endpoint.unicastLocatorList.push_back(loc);
        }
        endpoint.multicastLocatorList.clear();
        for (const Locator_t& loc : part_data->metatraffic_locators.multicast)
        {
            endpoint.multicastLocatorList.push_back(loc);
        }
    }

    // External locators are always taken from the same place
    endpoint.external_unicast_locators = pdp->builtin_attributes().metatraffic_external_unicast_locators;
    endpoint.ignore_non_matching_locators = pattr.ignore_non_matching_locators;
}

ReaderAttributes PDP::create_builtin_reader_attributes() const
{
    ReaderAttributes attributes;

    const RTPSParticipantAttributes& pattr = getRTPSParticipant()->getRTPSParticipantAttributes();
    set_builtin_matched_allocation(attributes.matched_writers_allocation, pattr);
    set_builtin_endpoint_locators(attributes.endpoint, this, mp_builtin);

    // Builtin endpoints are always reliable, transient local, keyed topics
    attributes.endpoint.reliabilityKind = RELIABLE;
    attributes.endpoint.durabilityKind = TRANSIENT_LOCAL;
    attributes.endpoint.topicKind = WITH_KEY;

    // Built-in readers never expect inline qos
    attributes.expects_inline_qos = false;

    return attributes;
}

WriterAttributes PDP::create_builtin_writer_attributes() const
{
    WriterAttributes attributes;

    const RTPSParticipantAttributes& pattr = getRTPSParticipant()->getRTPSParticipantAttributes();
    set_builtin_matched_allocation(attributes.matched_readers_allocation, pattr);
    set_builtin_endpoint_locators(attributes.endpoint, this, mp_builtin);

    // Builtin endpoints are always reliable, transient local, keyed topics
    attributes.endpoint.reliabilityKind = RELIABLE;
    attributes.endpoint.durabilityKind = TRANSIENT_LOCAL;
    attributes.endpoint.topicKind = WITH_KEY;

    return attributes;
}

#if HAVE_SECURITY
void PDP::add_builtin_security_attributes(
        ReaderAttributes& ratt,
        WriterAttributes& watt) const
{
    const security::ParticipantSecurityAttributes& part_attr = mp_RTPSParticipant->security_attributes();

    ratt.endpoint.security_attributes().is_submessage_protected = part_attr.is_discovery_protected;
    ratt.endpoint.security_attributes().plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;

    watt.endpoint.security_attributes().is_submessage_protected = part_attr.is_discovery_protected;
    watt.endpoint.security_attributes().plugin_endpoint_attributes = PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_VALID;

    if (part_attr.is_discovery_protected)
    {
        security::PluginParticipantSecurityAttributes plugin_part_attr(part_attr.plugin_participant_attributes);

        if (plugin_part_attr.is_discovery_encrypted)
        {
            ratt.endpoint.security_attributes().plugin_endpoint_attributes |=
                    PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
            watt.endpoint.security_attributes().plugin_endpoint_attributes |=
                    PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ENCRYPTED;
        }
        if (plugin_part_attr.is_discovery_origin_authenticated)
        {
            ratt.endpoint.security_attributes().plugin_endpoint_attributes |=
                    PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
            watt.endpoint.security_attributes().plugin_endpoint_attributes |=
                    PLUGIN_ENDPOINT_SECURITY_ATTRIBUTES_FLAG_IS_SUBMESSAGE_ORIGIN_AUTHENTICATED;
        }
    }
}

#endif // HAVE_SECURITY

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
