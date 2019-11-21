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

#include <fastrtps/rtps/builtin/discovery/participant/PDP.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPListener.h>

#include <fastrtps/rtps/builtin/BuiltinProtocols.h>
#include <fastrtps/rtps/builtin/liveliness/WLP.h>

#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>
#include <fastrtps/rtps/resources/TimedEvent.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/WriterProxyData.h>

#include <fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDPStatic.h>

#include <fastrtps/rtps/resources/AsyncWriterThread.h>

#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/history/ReaderHistory.h>


#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/utils/IPLocator.h>

#include <fastrtps/log/Log.h>

#include <mutex>
#include <chrono>

namespace eprosima {
namespace fastrtps {
namespace rtps {

// Default configuration values for PDP reliable entities.

const Duration_t pdp_heartbeat_period{ 0, 350 * 1000  }; // 350 milliseconds
const Duration_t pdp_nack_response_delay{ 0, 100 * 1000  }; // 100 milliseconds
const Duration_t pdp_nack_supression_duration{ 0, 11 * 1000 }; // ~11 milliseconds
const Duration_t pdp_heartbeat_response_delay{ 0, 11 * 1000 }; // ~11 milliseconds

const int32_t pdp_initial_reserved_caches = 20;


PDP::PDP (
        BuiltinProtocols* built,
        const RTPSParticipantAllocationAttributes& allocation)
    : mp_builtin(built)
    , mp_RTPSParticipant(nullptr)
    , mp_PDPWriter(nullptr)
    , mp_PDPReader(nullptr)
    , mp_EDP(nullptr)
    , participant_proxies_number_(allocation.participants.initial)
    , participant_proxies_(allocation.participants)
    , participant_proxies_pool_(allocation.participants)
    , reader_proxies_number_(allocation.total_readers().initial)
    , reader_proxies_pool_(allocation.total_readers())
    , writer_proxies_number_(allocation.total_writers().initial)
    , writer_proxies_pool_(allocation.total_writers())
    , m_hasChangedLocalPDP(true)
    , mp_listener(nullptr)
    , mp_PDPWriterHistory(nullptr)
    , mp_PDPReaderHistory(nullptr)
    , temp_reader_data_(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
    , temp_writer_data_(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
    , mp_mutex(new std::recursive_mutex())
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
        reader_proxies_pool_.push_back(new ReaderProxyData(max_unicast_locators, max_multicast_locators));
    }

    for (size_t i = 0; i < allocation.total_writers().initial; ++i)
    {
        writer_proxies_pool_.push_back(new WriterProxyData(max_unicast_locators, max_multicast_locators));
    }
}

PDP::~PDP()
{
    delete resend_participant_info_event_;
    mp_RTPSParticipant->disableReader(mp_PDPReader);
    delete mp_EDP;
    mp_RTPSParticipant->deleteUserEndpoint(mp_PDPWriter);
    mp_RTPSParticipant->deleteUserEndpoint(mp_PDPReader);
    delete mp_PDPWriterHistory;
    delete mp_PDPReaderHistory;
    delete mp_listener;

    for(ParticipantProxyData* it : participant_proxies_)
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
        bool with_lease_duration)
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
                        [this, ret_val](TimedEvent::EventCode code) -> bool
                        {
                            if (TimedEvent::EVENT_SUCCESS == code)
                            {
                                check_remote_participant_liveliness(ret_val);
                            }

                            return false;
                        }, 0.0);
            }
        }
        else
        {
            logWarning(RTPS_PDP, "Maximum number of participant proxies (" << max_proxies << \
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
    participant_proxies_.push_back(ret_val);

    return ret_val;
}

void PDP::initializeParticipantProxyData(ParticipantProxyData* participant_data)
{
    participant_data->m_leaseDuration = mp_RTPSParticipant->getAttributes().builtin.discovery_config.leaseDuration;
    //set_VendorId_eProsima(participant_data->m_VendorId);
    participant_data->m_VendorId = c_VendorId_eProsima;

    participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;

#if HAVE_SECURITY
    participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_ANNOUNCER;
    participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PARTICIPANT_SECURE_DETECTOR;
#endif

    if(mp_RTPSParticipant->getAttributes().builtin.use_WriterLivelinessProtocol)
    {
        participant_data->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER;
        participant_data->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER;

#if HAVE_SECURITY
        participant_data->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_WRITER;
        participant_data->m_availableBuiltinEndpoints |= BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_SECURE_DATA_READER;
#endif
    }

#if HAVE_SECURITY
    participant_data->m_availableBuiltinEndpoints |= mp_RTPSParticipant->security_manager().builtin_endpoints();
#endif

    for (const Locator_t& loc : mp_RTPSParticipant->getAttributes().defaultUnicastLocatorList)
    {
        participant_data->default_locators.add_unicast_locator(loc);
    }
    for (const Locator_t& loc : mp_RTPSParticipant->getAttributes().defaultMulticastLocatorList)
    {
        participant_data->default_locators.add_multicast_locator(loc);
    }
    participant_data->m_expectsInlineQos = false;
    participant_data->m_guid = mp_RTPSParticipant->getGuid();
    for(uint8_t i = 0; i<16; ++i)
    {
        if(i<12)
            participant_data->m_key.value[i] = participant_data->m_guid.guidPrefix.value[i];
        else
            participant_data->m_key.value[i] = participant_data->m_guid.entityId.value[i - 12];
    }

    // Keep persistence Guid_Prefix_t in a specific property. This info must be propagated to all builtin endpoints
    {
        GuidPrefix_t persistent = mp_RTPSParticipant->getAttributes().prefix;

        if(persistent != c_GuidPrefix_Unknown)
        {
            participant_data->set_persistence_guid(
                GUID_t(
                    persistent,
                    c_EntityId_RTPSParticipant));
        }
    }

    participant_data->metatraffic_locators.unicast.clear();
    for (const Locator_t& loc : this->mp_builtin->m_metatrafficUnicastLocatorList)
    {
        participant_data->metatraffic_locators.add_unicast_locator(loc);
    }

    participant_data->metatraffic_locators.multicast.clear();
    if (!m_discovery.avoid_builtin_multicast || participant_data->metatraffic_locators.unicast.empty())
    {
        for(const Locator_t& loc: this->mp_builtin->m_metatrafficMulticastLocatorList)
        {
            participant_data->metatraffic_locators.add_multicast_locator(loc);
        }
    }

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
    if(mp_RTPSParticipant->security_manager().get_permissions_token(&permissions_token)
        && permissions_token != nullptr)
    {
        participant_data->permissions_token_ = std::move(*permissions_token);
        mp_RTPSParticipant->security_manager().return_permissions_token(permissions_token);
    }

    if (mp_RTPSParticipant->is_secure())
    {
        const security::ParticipantSecurityAttributes & sec_attrs = mp_RTPSParticipant->security_attributes();
        participant_data->security_attributes_ = sec_attrs.mask();
        participant_data->plugin_security_attributes_ = sec_attrs.plugin_participant_attributes;
    }
    else
    {
        participant_data->security_attributes_ = 0UL;
        participant_data->plugin_security_attributes_ = 0UL;
    }
#endif
}

bool PDP::initPDP(
    RTPSParticipantImpl* part)
{
    logInfo(RTPS_PDP,"Beginning");
    mp_RTPSParticipant = part;
    m_discovery = mp_RTPSParticipant->getAttributes().builtin;
    initial_announcements_ = m_discovery.discovery_config.initial_announcements;
    //CREATE ENDPOINTS
    if (!createPDPEndpoints())
    {
        return false;
    }
    //UPDATE METATRAFFIC.
    mp_builtin->updateMetatrafficLocators(this->mp_PDPReader->getAttributes().unicastLocatorList);
    ParticipantProxyData* pdata = add_participant_proxy_data(part->getGuid(), true);
    if (pdata == nullptr)
    {
        return false;
    }
    initializeParticipantProxyData(pdata);

    // Create lease events on already created proxy data objects
    for (ParticipantProxyData* pool_item : participant_proxies_pool_)
    {
        pool_item->lease_duration_event = new TimedEvent(mp_RTPSParticipant->getEventResource(),
                [this, pool_item](TimedEvent::EventCode code) -> bool
                {
                    if (TimedEvent::EVENT_SUCCESS == code)
                    {
                        check_remote_participant_liveliness(pool_item);
                    }

                    return false;
                }, 0.0);
    }

    resend_participant_info_event_ = new TimedEvent(mp_RTPSParticipant->getEventResource(),
            [&](TimedEvent::EventCode code) -> bool
            {
                if (TimedEvent::EVENT_SUCCESS == code)
                {
                    announceParticipantState(false);
                    set_next_announcement_interval();
                    return true;
                }

                return false;
            },
            0);

    set_initial_announcement_interval();

    return true;
}

bool PDP::enable()
{
    return mp_RTPSParticipant->enableReader(mp_PDPReader);
}

void PDP::announceParticipantState(
    bool new_change,
    bool dispose,
    WriteParams& wparams)
{
    logInfo(RTPS_PDP,"Announcing RTPSParticipant State (new change: "<< new_change <<")");
    CacheChange_t* change = nullptr;

    if(!dispose)
    {
        if(m_hasChangedLocalPDP.exchange(false) || new_change)
        {
            this->mp_mutex->lock();
            ParticipantProxyData* local_participant_data = getLocalParticipantProxyData();
            InstanceHandle_t key = local_participant_data->m_key;
            ParticipantProxyData proxy_data_copy(*local_participant_data);
            this->mp_mutex->unlock();

            if(mp_PDPWriterHistory->getHistorySize() > 0)
                mp_PDPWriterHistory->remove_min_change();
            // TODO(Ricardo) Change DISCOVERY_PARTICIPANT_DATA_MAX_SIZE with getLocalParticipantProxyData()->size().
            change = mp_PDPWriter->new_change([]() -> uint32_t
                {
                    return DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
                }
            , ALIVE, key);

            if(change != nullptr)
            {
                CDRMessage_t aux_msg(change->serializedPayload);

#if __BIG_ENDIAN__
                change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
                aux_msg.msg_endian = BIGEND;
#else
                change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
                aux_msg.msg_endian =  LITTLEEND;
#endif

                if (proxy_data_copy.writeToCDRMessage(&aux_msg, true))
                {
                    change->serializedPayload.length = (uint16_t)aux_msg.length;

                   mp_PDPWriterHistory->add_change(change, wparams);
                }
                else
                {
                    logError(RTPS_PDP, "Cannot serialize ParticipantProxyData.");
                }
            }
        }

    }
    else
    {
        this->mp_mutex->lock();
        ParticipantProxyData proxy_data_copy(*getLocalParticipantProxyData());
        this->mp_mutex->unlock();

        if(mp_PDPWriterHistory->getHistorySize() > 0)
            mp_PDPWriterHistory->remove_min_change();
        change = mp_PDPWriter->new_change([]() -> uint32_t
            {
                return DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
            }
        , NOT_ALIVE_DISPOSED_UNREGISTERED, getLocalParticipantProxyData()->m_key);

        if(change != nullptr)
        {
            CDRMessage_t aux_msg(change->serializedPayload);

#if __BIG_ENDIAN__
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_BE;
            aux_msg.msg_endian = BIGEND;
#else
            change->serializedPayload.encapsulation = (uint16_t)PL_CDR_LE;
            aux_msg.msg_endian =  LITTLEEND;
#endif

            if (proxy_data_copy.writeToCDRMessage(&aux_msg, true))
            {
                change->serializedPayload.length = (uint16_t)aux_msg.length;

                mp_PDPWriterHistory->add_change(change, wparams);
            }
            else
            {
                logError(RTPS_PDP, "Cannot serialize ParticipantProxyData.");
            }
        }
    }

}

void PDP::stopParticipantAnnouncement()
{
    resend_participant_info_event_->cancel_timer();
}

void PDP::resetParticipantAnnouncement()
{
    resend_participant_info_event_->restart_timer();
}

bool PDP::has_reader_proxy_data(const GUID_t& reader)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == reader.guidPrefix)
        {
            for (ReaderProxyData* rit : pit->m_readers)
            {
                if (rit->guid() == reader)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool PDP::lookupReaderProxyData(const GUID_t& reader, ReaderProxyData& rdata)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == reader.guidPrefix)
        {
            for (ReaderProxyData* rit : pit->m_readers)
            {
                if (rit->guid() == reader)
                {
                    rdata.copy(rit);
                    return true;
                }
            }
        }
    }
    return false;
}

bool PDP::has_writer_proxy_data(const GUID_t& writer)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == writer.guidPrefix)
        {
            for (WriterProxyData* wit : pit->m_writers)
            {
                if (wit->guid() == writer)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool PDP::lookupWriterProxyData(const GUID_t& writer, WriterProxyData& wdata)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == writer.guidPrefix)
        {
            for (WriterProxyData* wit : pit->m_writers)
            {
                if (wit->guid() == writer)
                {
                    wdata.copy(wit);
                    return true;
                }
            }
        }
    }
    return false;
}

bool PDP::removeReaderProxyData(const GUID_t& reader_guid)
{
    logInfo(RTPS_PDP, "Removing reader proxy data " << reader_guid);
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);

    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == reader_guid.guidPrefix)
        {
            for (ReaderProxyData* rit : pit->m_readers)
            {
                if (rit->guid() == reader_guid)
                {
                    mp_EDP->unpairReaderProxy(pit->m_guid, reader_guid);

                    RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
                    if (listener)
                    {
                        ReaderDiscoveryInfo info(std::move(*rit));
                        info.status = ReaderDiscoveryInfo::REMOVED_READER;
                        listener->onReaderDiscovery(mp_RTPSParticipant->getUserRTPSParticipant(), std::move(info));
                    }

                    // Clear reader proxy data and move to pool in order to allow reuse
                    rit->clear();
                    pit->m_readers.remove(rit);
                    reader_proxies_pool_.push_back(rit);
                    return true;
                }
            }
        }
    }

    return false;
}

bool PDP::removeWriterProxyData(const GUID_t& writer_guid)
{
    logInfo(RTPS_PDP, "Removing writer proxy data " << writer_guid);
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);

    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == writer_guid.guidPrefix)
        {
            for (WriterProxyData* wit : pit->m_writers)
            {
                if (wit->guid() == writer_guid)
                {
                    mp_EDP->unpairWriterProxy(pit->m_guid, writer_guid);

                    RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
                    if (listener)
                    {
                        WriterDiscoveryInfo info(std::move(*wit));
                        info.status = WriterDiscoveryInfo::REMOVED_WRITER;
                        listener->onWriterDiscovery(mp_RTPSParticipant->getUserRTPSParticipant(), std::move(info));
                    }

                    // Clear writer proxy data and move to pool in order to allow reuse
                    wit->clear();
                    pit->m_writers.remove(wit);
                    writer_proxies_pool_.push_back(wit);
                    return true;
                }
            }
        }
    }

    return false;
}

bool PDP::lookup_participant_name(
        const GUID_t& guid,
        string_255& name)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
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
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
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
    logInfo(RTPS_PDP, "Adding reader proxy data " << reader_guid);
    ReaderProxyData* ret_val = nullptr;

    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);

    for(ParticipantProxyData* pit : participant_proxies_)
    {
        if(pit->m_guid.guidPrefix == reader_guid.guidPrefix)
        {
            // Copy participant data to be used outside.
            participant_guid = pit->m_guid;

            // Check that it is not already there:
            for(ReaderProxyData* rit : pit->m_readers)
            {
                if(rit->guid().entityId == reader_guid.entityId)
                {
                    if (!initializer_func(rit, true, *pit))
                    {
                        return nullptr;
                    }

                    ret_val = rit;

                    RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
                    if(listener)
                    {
                        ReaderDiscoveryInfo info(*ret_val);
                        info.status = ReaderDiscoveryInfo::CHANGED_QOS_READER;
                        listener->onReaderDiscovery(mp_RTPSParticipant->getUserRTPSParticipant(), std::move(info));
                    }

                    return ret_val;
                }
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
                        mp_RTPSParticipant->getAttributes().allocation.locators.max_multicast_locators);
                }
                else
                {
                    logWarning(RTPS_PDP, "Maximum number of reader proxies (" << max_proxies <<
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

            // Add to ParticipantProxyData
            pit->m_readers.push_back(ret_val);

            if (!initializer_func(ret_val, false, *pit))
            {
                return nullptr;
            }

            RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
            if(listener)
            {
                ReaderDiscoveryInfo info(*ret_val);
                info.status = ReaderDiscoveryInfo::DISCOVERED_READER;
                listener->onReaderDiscovery(mp_RTPSParticipant->getUserRTPSParticipant(), std::move(info));
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
    logInfo(RTPS_PDP, "Adding reader proxy data " << writer_guid);
    WriterProxyData* ret_val = nullptr;

    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);

    for (ParticipantProxyData* pit : participant_proxies_)
    {
        if (pit->m_guid.guidPrefix == writer_guid.guidPrefix)
        {
            // Copy participant data to be used outside.
            participant_guid = pit->m_guid;

            // Check that it is not already there:
            for (WriterProxyData* wit : pit->m_writers)
            {
                if (wit->guid().entityId == writer_guid.entityId)
                {
                    if (!initializer_func(wit, true, *pit))
                    {
                        return nullptr;
                    }

                    ret_val = wit;

                    RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
                    if (listener)
                    {
                        WriterDiscoveryInfo info(*ret_val);
                        info.status = WriterDiscoveryInfo::CHANGED_QOS_WRITER;
                        listener->onWriterDiscovery(mp_RTPSParticipant->getUserRTPSParticipant(), std::move(info));
                    }

                    return ret_val;
                }
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
                        mp_RTPSParticipant->getAttributes().allocation.locators.max_multicast_locators);
                }
                else
                {
                    logWarning(RTPS_PDP, "Maximum number of writer proxies (" << max_proxies <<
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

            // Add to ParticipantProxyData
            pit->m_writers.push_back(ret_val);

            if (!initializer_func(ret_val, false, *pit))
            {
                return nullptr;
            }

            RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();
            if (listener)
            {
                WriterDiscoveryInfo info(*ret_val);
                info.status = WriterDiscoveryInfo::DISCOVERED_WRITER;
                listener->onWriterDiscovery(mp_RTPSParticipant->getUserRTPSParticipant(), std::move(info));
            }

            return ret_val;
        }
    }

    return nullptr;
}

bool PDP::remove_remote_participant(
        const GUID_t& partGUID,
        ParticipantDiscoveryInfo::DISCOVERY_STATUS reason)
{
    if (partGUID == getLocalParticipantProxyData()->m_guid)
    {   // avoid removing our own data
        return false;
    }

    logInfo(RTPS_PDP,partGUID );
    ParticipantProxyData* pdata = nullptr;

    //Remove it from our vector or RTPSParticipantProxies:
    this->mp_mutex->lock();
    for(ResourceLimitedVector<ParticipantProxyData*>::iterator pit = participant_proxies_.begin();
            pit!=participant_proxies_.end();++pit)
    {
        if((*pit)->m_guid == partGUID)
        {
            pdata = *pit;
            participant_proxies_.erase(pit);
            break;
        }
    }
    this->mp_mutex->unlock();

    if(pdata !=nullptr)
    {
        if(mp_EDP!=nullptr)
        {
            RTPSParticipantListener* listener = mp_RTPSParticipant->getListener();

            for(ReaderProxyData* rit : pdata->m_readers)
            {
                GUID_t reader_guid(rit->guid());
                if (reader_guid != c_Guid_Unknown)
                {
                    mp_EDP->unpairReaderProxy(partGUID, reader_guid);

                    if (listener)
                    {
                        ReaderDiscoveryInfo info(std::move(*rit));
                        info.status = ReaderDiscoveryInfo::REMOVED_READER;
                        listener->onReaderDiscovery(mp_RTPSParticipant->getUserRTPSParticipant(), std::move(info));
                    }
                }
            }
            for(WriterProxyData* wit : pdata->m_writers)
            {
                GUID_t writer_guid(wit->guid());
                if (writer_guid != c_Guid_Unknown)
                {
                    mp_EDP->unpairWriterProxy(partGUID, writer_guid);

                    if (listener)
                    {
                        WriterDiscoveryInfo info(std::move(*wit));
                        info.status = WriterDiscoveryInfo::REMOVED_WRITER;
                        listener->onWriterDiscovery(mp_RTPSParticipant->getUserRTPSParticipant(), std::move(info));
                    }
                }
            }
        }

        if(mp_builtin->mp_WLP != nullptr)
            this->mp_builtin->mp_WLP->removeRemoteEndpoints(pdata);
        this->mp_EDP->removeRemoteEndpoints(pdata);
        this->removeRemoteEndpoints(pdata);

#if HAVE_SECURITY
        mp_builtin->mp_participantImpl->security_manager().remove_participant(*pdata);
#endif

        this->mp_PDPReaderHistory->getMutex()->lock();
        for(std::vector<CacheChange_t*>::iterator it=this->mp_PDPReaderHistory->changesBegin();
                it!=this->mp_PDPReaderHistory->changesEnd();++it)
        {
            if((*it)->instanceHandle == pdata->m_key)
            {
                this->mp_PDPReaderHistory->remove_change(*it);
                break;
            }
        }
        this->mp_PDPReaderHistory->getMutex()->unlock();

        auto listener =  mp_RTPSParticipant->getListener();
        if (listener != nullptr)
        {
            std::lock_guard<std::mutex> lock(callback_mtx_);
            ParticipantDiscoveryInfo info(*pdata);
            info.status = reason;
            listener->onParticipantDiscovery(mp_RTPSParticipant->getUserRTPSParticipant(), std::move(info));
        }

        this->mp_mutex->lock();

        // Return reader proxy objects to pool
        for (ReaderProxyData* rit : pdata->m_readers)
        {
            reader_proxies_pool_.push_back(rit);
        }
        pdata->m_readers.clear();

        // Return writer proxy objects to pool
        for (WriterProxyData* wit : pdata->m_writers)
        {
            writer_proxies_pool_.push_back(wit);
        }
        pdata->m_writers.clear();

        // Cancel lease event
        if (pdata->lease_duration_event != nullptr)
        {
            pdata->lease_duration_event->cancel_timer();
        }

        // Return proxy object to pool
        pdata->clear();
        participant_proxies_pool_.push_back(pdata);

        this->mp_mutex->unlock();
        return true;
    }

    return false;
}

const BuiltinAttributes& PDP::builtin_attributes() const
{
    return mp_builtin->m_att;
}

void PDP::assert_remote_participant_liveliness(
        const GuidPrefix_t& remote_guid)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);

    for (ParticipantProxyData* it : this->participant_proxies_)
    {
        if(it->m_guid.guidPrefix == remote_guid)
        {
            // TODO Ricardo: Study if isAlive attribute is necessary.
            it->isAlive = true;
            it->assert_liveliness();
            break;
        }
    }
}

CDRMessage_t PDP::get_participant_proxy_data_serialized(Endianness_t endian)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
    CDRMessage_t cdr_msg;
    cdr_msg.msg_endian = endian;

    if (!getLocalParticipantProxyData()->writeToCDRMessage(&cdr_msg, false))
    {
        cdr_msg.pos = 0;
        cdr_msg.length = 0;
    }

    return cdr_msg;
}

void PDP::check_remote_participant_liveliness(
        ParticipantProxyData* remote_participant)
{
    std::unique_lock<std::recursive_mutex> guard(*this->mp_mutex);

    if(GUID_t::unknown() != remote_participant->m_guid)
    {
        // Check last received message's time_point plus lease duration time doesn't overcome now().
        // If overcame, remove participant.
        auto now = std::chrono::steady_clock::now();
        auto real_lease_tm = remote_participant->last_received_message_tm() +
                std::chrono::microseconds(TimeConv::Duration_t2MicroSecondsInt64(remote_participant->m_leaseDuration));
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
        logWarning(RTPS_PDP, "Initial announcement period is not strictly positive. Changing to 1ms.");
        initial_announcements_.period = { 0, 1000000 };
    }
    set_next_announcement_interval();
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
