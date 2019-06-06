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

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {


PDPSimple::PDPSimple (
        BuiltinProtocols* built,
        const RTPSParticipantAllocationAttributes& allocation)
    : mp_builtin(built)
    , mp_RTPSParticipant(nullptr)
    , mp_SPDPWriter(nullptr)
    , mp_SPDPReader(nullptr)
    , mp_EDP(nullptr)
    , participant_proxies_number_(allocation.participants.initial)
    , participant_proxies_(allocation.participants)
    , participant_proxies_pool_(allocation.participants)
    , reader_proxies_number_(allocation.total_readers().initial)
    , reader_proxies_pool_(allocation.total_readers())
    , writer_proxies_number_(allocation.total_writers().initial)
    , writer_proxies_pool_(allocation.total_writers())
    , m_hasChangedLocalPDP(true)
    , resend_participant_info_event_(nullptr)
    , mp_listener(nullptr)
    , mp_SPDPWriterHistory(nullptr)
    , mp_SPDPReaderHistory(nullptr)
    , temp_reader_data_(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
    , temp_writer_data_(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
    , mp_mutex(new std::recursive_mutex())
{
    size_t max_unicast_locators = allocation.locators.max_unicast_locators;
    size_t max_multicast_locators = allocation.locators.max_multicast_locators;

    for (size_t i = 0; i < allocation.participants.initial; ++i)
    {
        participant_proxies_pool_.push_back(new ParticipantProxyData(allocation));
    }

    for (size_t i = 0; i < allocation.readers.initial; ++i)
    {
        reader_proxies_pool_.push_back(new ReaderProxyData(max_unicast_locators, max_multicast_locators));
    }

    for (size_t i = 0; i < allocation.writers.initial; ++i)
    {
        writer_proxies_pool_.push_back(new WriterProxyData(max_unicast_locators, max_multicast_locators));
    }
}

PDPSimple::~PDPSimple()
{
    if(resend_participant_info_event_ != nullptr)
    {
        delete(resend_participant_info_event_);
    }

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

    delete(mp_mutex);
}

ParticipantProxyData* PDPSimple::add_participant_proxy_data(const GUID_t& participant_guid)
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
                        [&, participant_guid](TimedEvent::EventCode code) -> bool
                        {
                            if (TimedEvent::EVENT_SUCCESS == code)
                            {
                                remove_remote_participant(participant_guid, ParticipantDiscoveryInfo::DROPPED_PARTICIPANT);
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
    ret_val->m_guid = participant_guid;
    participant_proxies_.push_back(ret_val);

    return ret_val;
}

void PDPSimple::initializeParticipantProxyData(ParticipantProxyData* participant_data)
{
    participant_data->m_leaseDuration = mp_RTPSParticipant->getAttributes().builtin.leaseDuration;
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

#if HAVE_SECURITY
        if(mp_RTPSParticipant->getAttributes().builtin.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader)
        {
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_ANNOUNCER;
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_DETECTOR;
        }

        if(mp_RTPSParticipant->getAttributes().builtin.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader)
        {
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_SECURE_ANNOUNCER;
            participant_data->m_availableBuiltinEndpoints |= DISC_BUILTIN_ENDPOINT_PUBLICATION_SECURE_DETECTOR;
        }
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


    participant_data->metatraffic_locators.multicast.clear();
    for(const Locator_t& loc: this->mp_builtin->m_metatrafficMulticastLocatorList)
    { 
        participant_data->metatraffic_locators.add_multicast_locator(loc);
    }
    participant_data->metatraffic_locators.unicast.clear();
    for (const Locator_t& loc : this->mp_builtin->m_metatrafficUnicastLocatorList)
    {
        participant_data->metatraffic_locators.add_unicast_locator(loc);
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
    if(mp_RTPSParticipant->security_manager().get_permissions_token(&permissions_token) && permissions_token != nullptr)
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

bool PDPSimple::initPDP(RTPSParticipantImpl* part)
{
    logInfo(RTPS_PDP,"Beginning");
    mp_RTPSParticipant = part;
    m_discovery = mp_RTPSParticipant->getAttributes().builtin;
    //CREATE ENDPOINTS
    if (!createSPDPEndpoints())
    {
        return false;
    }
    //UPDATE METATRAFFIC.
    mp_builtin->updateMetatrafficLocators(this->mp_SPDPReader->getAttributes().unicastLocatorList);
    ParticipantProxyData* pdata = add_participant_proxy_data(part->getGuid());
    if (pdata == nullptr)
    {
        return false;
    }
    initializeParticipantProxyData(pdata);

    // Create lease events on already created proxy data objects
    for (ParticipantProxyData* pool_item : participant_proxies_pool_)
    {
        const GUID_t guid = pool_item->m_guid;
        pool_item->lease_duration_event = new TimedEvent(mp_RTPSParticipant->getEventResource(),
                [&, guid](TimedEvent::EventCode code) -> bool
                {
                    if (TimedEvent::EVENT_SUCCESS == code)
                    {
                        remove_remote_participant(guid, ParticipantDiscoveryInfo::DROPPED_PARTICIPANT);
                    }

                    return false;
                }, 0.0);
    }

    //INIT EDP
    if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
    {
        mp_EDP = (EDP*)(new EDPStatic(this,mp_RTPSParticipant));
        if(!mp_EDP->initEDP(m_discovery))
        {
            logError(RTPS_PDP,"Endpoint discovery configuration failed");
            return false;
        }
    }
    else if(m_discovery.use_SIMPLE_EndpointDiscoveryProtocol)
    {
        mp_EDP = (EDP*)(new EDPSimple(this,mp_RTPSParticipant));
        if(!mp_EDP->initEDP(m_discovery))
        {
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

    resend_participant_info_event_ = new TimedEvent(mp_RTPSParticipant->getEventResource(),
            [&](TimedEvent::EventCode code) -> bool
            {
                if (TimedEvent::EVENT_SUCCESS == code)
                {
                    {
                        std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
                        getLocalParticipantProxyData()->m_manualLivelinessCount++;
                    }
                    announceParticipantState(false);
                    return true;
                }

                return false;
            },
            TimeConv::Duration_t2MilliSecondsDouble(m_discovery.leaseDuration_announcementperiod));

    return true;
}

void PDPSimple::stopParticipantAnnouncement()
{
    resend_participant_info_event_->cancel_timer();
}

void PDPSimple::resetParticipantAnnouncement()
{
    resend_participant_info_event_->restart_timer();
}

void PDPSimple::announceParticipantState(bool new_change, bool dispose)
{
    logInfo(RTPS_PDP,"Announcing RTPSParticipant State (new change: "<< new_change <<")");
    CacheChange_t* change = nullptr;

    if(!dispose)
    {
        if(new_change || m_hasChangedLocalPDP.load())
        {
            this->mp_mutex->lock();
            ParticipantProxyData* local_participant_data = getLocalParticipantProxyData();
            local_participant_data->m_manualLivelinessCount++;
            InstanceHandle_t key = local_participant_data->m_key;
            ParticipantProxyData proxy_data_copy(*local_participant_data);
            this->mp_mutex->unlock();

            if(mp_SPDPWriterHistory->getHistorySize() > 0)
                mp_SPDPWriterHistory->remove_min_change();
            // TODO(Ricardo) Change DISCOVERY_PARTICIPANT_DATA_MAX_SIZE with getLocalParticipantProxyData()->size().
            change = mp_SPDPWriter->new_change([]() -> uint32_t {return DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;}, ALIVE, key);

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

                if(proxy_data_copy.writeToCDRMessage(&aux_msg, true))
                {
                    change->serializedPayload.length = (uint16_t)aux_msg.length;

                    mp_SPDPWriterHistory->add_change(change);
                }
                else
                {
                    logError(RTPS_PDP, "Cannot serialize ParticipantProxyData.");
                }
            }

            m_hasChangedLocalPDP.exchange(false);
        }
        else
        {
            mp_SPDPWriter->unsent_changes_reset();
        }
    }
    else
    {
        this->mp_mutex->lock();
        ParticipantProxyData proxy_data_copy(*getLocalParticipantProxyData());
        this->mp_mutex->unlock();

        if(mp_SPDPWriterHistory->getHistorySize() > 0)
            mp_SPDPWriterHistory->remove_min_change();
        change = mp_SPDPWriter->new_change([]() -> uint32_t {return DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;}, NOT_ALIVE_DISPOSED_UNREGISTERED, getLocalParticipantProxyData()->m_key);

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

                mp_SPDPWriterHistory->add_change(change);
            }
            else
            {
                logError(RTPS_PDP, "Cannot serialize ParticipantProxyData.");
            }
        }
    }

}

bool PDPSimple::has_reader_proxy_data(const GUID_t& reader)
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

bool PDPSimple::lookupReaderProxyData(const GUID_t& reader, ReaderProxyData& rdata)
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

bool PDPSimple::has_writer_proxy_data(const GUID_t& writer)
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

bool PDPSimple::lookupWriterProxyData(const GUID_t& writer, WriterProxyData& wdata)
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

bool PDPSimple::removeReaderProxyData(const GUID_t& reader_guid)
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

bool PDPSimple::removeWriterProxyData(const GUID_t& writer_guid)
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


bool PDPSimple::lookup_participant_name(
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

bool PDPSimple::createSPDPEndpoints()
{
    logInfo(RTPS_PDP,"Beginning");

    const RTPSParticipantAllocationAttributes& allocation =
        mp_RTPSParticipant->getRTPSParticipantAttributes().allocation;

    //SPDP BUILTIN RTPSParticipant READER
    HistoryAttributes hatt;
    hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
    hatt.memoryPolicy = mp_builtin->m_att.readerHistoryMemoryPolicy;
    hatt.initialReservedCaches = 25;
    if (allocation.participants.initial > 0)
    {
        hatt.initialReservedCaches = (int32_t)allocation.participants.initial;
    }
    if (allocation.participants.maximum < std::numeric_limits<size_t>::max())
    {
        hatt.maximumReservedCaches = (int32_t)allocation.participants.maximum;
    }

    mp_SPDPReaderHistory = new ReaderHistory(hatt);
    ReaderAttributes ratt;
    ratt.endpoint.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
    ratt.endpoint.topicKind = WITH_KEY;
    ratt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    ratt.endpoint.reliabilityKind = BEST_EFFORT;
    ratt.matched_writers_allocation = allocation.participants;
    mp_listener = new PDPSimpleListener(this);
    RTPSReader* rout;
    if (mp_RTPSParticipant->createReader(&rout, ratt, mp_SPDPReaderHistory, mp_listener, c_EntityId_SPDPReader, true, false))
    {
#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(rout, false);
#endif
        mp_SPDPReader = dynamic_cast<StatelessReader*>(rout);
        if (mp_SPDPReader != nullptr)
        {
            temp_writer_data_.clear();
            temp_writer_data_.guid().guidPrefix = mp_RTPSParticipant->getGuid().guidPrefix;
            temp_writer_data_.guid().entityId = c_EntityId_SPDPWriter;
            temp_writer_data_.topicKind(WITH_KEY);
            temp_writer_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            temp_writer_data_.m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
            mp_SPDPReader->matched_writer_add(temp_writer_data_);
        }
    }
    else
    {
        logError(RTPS_PDP, "SimplePDP Reader creation failed");
        delete(mp_SPDPReaderHistory);
        mp_SPDPReaderHistory = nullptr;
        delete(mp_listener);
        mp_listener = nullptr;
        return false;
    }

    //SPDP BUILTIN RTPSParticipant WRITER
    hatt.payloadMaxSize = DISCOVERY_PARTICIPANT_DATA_MAX_SIZE;
    hatt.initialReservedCaches = 1;
    hatt.maximumReservedCaches = 1;
    hatt.memoryPolicy = mp_builtin->m_att.writerHistoryMemoryPolicy;
    mp_SPDPWriterHistory = new WriterHistory(hatt);
    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.durabilityKind = TRANSIENT_LOCAL;
    watt.endpoint.reliabilityKind = BEST_EFFORT;
    watt.endpoint.topicKind = WITH_KEY;
    watt.endpoint.remoteLocatorList = m_discovery.initialPeersList;
    watt.matched_readers_allocation = allocation.participants;

    if (mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
        mp_RTPSParticipant->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
    {
        watt.mode = ASYNCHRONOUS_WRITER;
    }

    RTPSWriter* wout;
    if (mp_RTPSParticipant->createWriter(&wout, watt, mp_SPDPWriterHistory, nullptr, c_EntityId_SPDPWriter, true))
    {
#if HAVE_SECURITY
        mp_RTPSParticipant->set_endpoint_rtps_protection_supports(wout, false);
#endif
        mp_SPDPWriter = dynamic_cast<StatelessWriter*>(wout);
        if (mp_SPDPWriter != nullptr)
        {
            mp_SPDPWriter->set_fixed_locators(mp_builtin->m_initialPeersList);

            const NetworkFactory& network = mp_RTPSParticipant->network_factory();
            Locator_t local_locator;
            temp_reader_data_.clear();
            temp_reader_data_.guid().guidPrefix = mp_RTPSParticipant->getGuid().guidPrefix;
            temp_reader_data_.guid().entityId = c_EntityId_SPDPReader;
            for (auto it = mp_builtin->m_initialPeersList.begin(); it != mp_builtin->m_initialPeersList.end(); ++it)
            {
                if (network.transform_remote_locator(*it, local_locator))
                {
                    if (IPLocator::isMulticast(local_locator))
                    {
                        temp_reader_data_.add_multicast_locator(local_locator);
                    }
                    else
                    {
                        temp_reader_data_.add_unicast_locator(local_locator);
                    }
                }
            }
            temp_reader_data_.topicKind(WITH_KEY);
            temp_reader_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
            temp_reader_data_.m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
            mp_SPDPWriter->matched_reader_add(temp_reader_data_);
        }
    }
    else
    {
        logError(RTPS_PDP, "SimplePDP Writer creation failed");
        delete(mp_SPDPWriterHistory);
        mp_SPDPWriterHistory = nullptr;
        return false;
    }
    logInfo(RTPS_PDP,"SPDP Endpoints creation finished");
    return true;
}

ReaderProxyData* PDPSimple::addReaderProxyData(
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

WriterProxyData* PDPSimple::addWriterProxyData(
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

void PDPSimple::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
    logInfo(RTPS_PDP,"For RTPSParticipant: "<< pdata->m_guid.guidPrefix);

    const NetworkFactory& network = mp_RTPSParticipant->network_factory();
    uint32_t endp = pdata->m_availableBuiltinEndpoints;
    uint32_t auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER;
    if(auxendp!=0)
    {
        std::lock_guard<std::mutex> data_guard(temp_data_lock_);
        temp_writer_data_.clear();
        temp_writer_data_.guid().guidPrefix = pdata->m_guid.guidPrefix;
        temp_writer_data_.guid().entityId = c_EntityId_SPDPWriter;
        temp_writer_data_.persistence_guid(temp_writer_data_.guid());
        temp_writer_data_.set_locators(pdata->metatraffic_locators, network, true);
        temp_writer_data_.m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
        temp_writer_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        mp_SPDPReader->matched_writer_add(temp_writer_data_);
    }
    auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    if(auxendp!=0)
    {
        std::lock_guard<std::mutex> data_guard(temp_data_lock_);
        temp_reader_data_.clear();
        temp_reader_data_.m_expectsInlineQos = false;
        temp_reader_data_.guid().guidPrefix = pdata->m_guid.guidPrefix;
        temp_reader_data_.guid().entityId = c_EntityId_SPDPReader;
        temp_reader_data_.set_locators(pdata->metatraffic_locators, network, true);
        temp_reader_data_.m_qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
        temp_reader_data_.m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        mp_SPDPWriter->matched_reader_add(temp_reader_data_);
    }

#if HAVE_SECURITY
    // Validate remote participant
    mp_RTPSParticipant->security_manager().discovered_participant(*pdata);
#else
    //Inform EDP of new RTPSParticipant data:
    notifyAboveRemoteEndpoints(*pdata);
#endif
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
        GUID_t writer_guid(pdata->m_guid.guidPrefix, c_EntityId_SPDPWriter);
        mp_SPDPReader->matched_writer_remove(writer_guid);
    }
    auxendp = endp;
    auxendp &=DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR;
    if(auxendp!=0)
    {
        GUID_t reader_guid(pdata->m_guid.guidPrefix, c_EntityId_SPDPReader);
        mp_SPDPWriter->matched_reader_remove(reader_guid);
    }
}

bool PDPSimple::remove_remote_participant(
        const GUID_t& partGUID,
        ParticipantDiscoveryInfo::DISCOVERY_STATUS reason)
{
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

        auto listener =  mp_RTPSParticipant->getListener();
        if (listener != nullptr)
        {
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

const BuiltinAttributes& PDPSimple::builtin_attributes() const
{
    return mp_builtin->m_att;
}

void PDPSimple::assertRemoteParticipantLiveliness(const GuidPrefix_t& guidP)
{
    std::lock_guard<std::recursive_mutex> guardPDP(*this->mp_mutex);
    for(ParticipantProxyData* it : this->participant_proxies_)
    {
        if(it->m_guid.guidPrefix == guidP)
        {
            logInfo(RTPS_LIVELINESS,"RTPSParticipant "<< it->m_guid << " is Alive");
            // TODO Ricardo: Study if isAlive attribute is necessary.
            it->isAlive = true;
            if(it->lease_duration_event != nullptr)
            {
                it->lease_duration_event->cancel_timer();
                it->lease_duration_event->restart_timer();
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
    for(WriterProxyData* wit : this->participant_proxies_.front()->m_writers)
    {
        if(wit->m_qos.m_liveliness.kind == kind)
        {
            logInfo(RTPS_LIVELINESS,"Local Writer "<< wit->guid().entityId << " marked as ALIVE");
            wit->isAlive(true);
        }
    }
}

void PDPSimple::assertRemoteWritersLiveliness(GuidPrefix_t& guidP,LivelinessQosPolicyKind kind)
{
    std::lock_guard<std::recursive_mutex> guardP(*mp_RTPSParticipant->getParticipantMutex());
    std::lock_guard<std::recursive_mutex> pguard(*this->mp_mutex);
    logInfo(RTPS_LIVELINESS,"of type " << (kind==AUTOMATIC_LIVELINESS_QOS?"AUTOMATIC":"")
            <<(kind==MANUAL_BY_PARTICIPANT_LIVELINESS_QOS?"MANUAL_BY_PARTICIPANT":""));

    for(ParticipantProxyData* pit : participant_proxies_)
    {
        if(pit->m_guid.guidPrefix == guidP)
        {
            for(WriterProxyData* wit : pit->m_writers)
            {
                if(wit->m_qos.m_liveliness.kind == kind)
                {
                    wit->isAlive(true);
                    for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
                            rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
                    {
                        if((*rit)->getAttributes().reliabilityKind == RELIABLE)
                        {
                            StatefulReader* sfr = (StatefulReader*)(*rit);
                            sfr->assert_liveliness(wit->guid());
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
    string_255 pname;
    if(lookup_participant_name(pguid, pname))
    {
        if(kind == WRITER)
            dynamic_cast<EDPStatic*>(mp_EDP)->newRemoteWriter(pguid, pname,userDefinedId);
        else
            dynamic_cast<EDPStatic*>(mp_EDP)->newRemoteReader(pguid, pname,userDefinedId);
    }
    return false;
}

CDRMessage_t PDPSimple::get_participant_proxy_data_serialized(Endianness_t endian)
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

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
