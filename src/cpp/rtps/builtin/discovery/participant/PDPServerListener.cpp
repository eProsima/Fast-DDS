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
 * @file PDPServerListener.cpp
 *
 */

#include <memory>

#include <rtps/builtin/discovery/participant/PDPServerListener.hpp>

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/rtps/builtin/discovery/endpoint/EDP.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastdds/rtps/reader/RTPSReader.h>

#include <rtps/builtin/discovery/database/DiscoveryParticipantChangeData.hpp>
#include <rtps/builtin/discovery/participant/PDPServer.hpp>
#include <rtps/builtin/discovery/participant/DS/DiscoveryServerPDPEndpoints.hpp>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace eprosima::fastrtps::rtps;

PDPServerListener::PDPServerListener(
        PDPServer* in_PDP)
    : PDPListener(in_PDP)
{
}

PDPServer* PDPServerListener::pdp_server()
{
    return static_cast<PDPServer*>(parent_pdp_);
}

void PDPServerListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "");
    EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "------------------ PDP SERVER LISTENER START ------------------");
    EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER,
            "-------------------- " << pdp_server()->mp_RTPSParticipant->getGuid() <<
            " --------------------");
    EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "PDP Server Message received: " << change_in->instanceHandle);

    auto endpoints = static_cast<fastdds::rtps::DiscoveryServerPDPEndpoints*>(pdp_server()->builtin_endpoints_.get());
    // Get PDP reader history
    auto pdp_history = endpoints->reader.history_.get();
    // Get PDP reader to release change
    auto pdp_reader = endpoints->reader.reader_;

    bool routine_should_be_awake = false;

    // Create a delete function to clear the data associated with the unique pointer in case the change is not passed
    // to the database.
    auto deleter = [pdp_history](CacheChange_t* p)
            {
                // Remove change from reader history, returning it to the pool
                pdp_history->remove_change(p);
            };

    // Unique pointer to the change
    std::unique_ptr<CacheChange_t, decltype(deleter)> change((CacheChange_t*)(change_in), deleter);

    // Get GUID of the writer that sent the change
    GUID_t writer_guid = change->writerGUID;

    // DATA(p|Up) should have a unkown instance handle and no key
    if (change->instanceHandle == c_InstanceHandle_Unknown
            && !this->get_key(change.get()))
    {
        EPROSIMA_LOG_WARNING(RTPS_PDP_LISTENER, "Problem getting the key of the change, removing");
        EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "------------------ PDP SERVER LISTENER END ------------------");
        EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "");
        return;
    }

    // Take GUID from instance handle
    GUID_t guid = iHandle2GUID(change->instanceHandle);

    // DATA(p|Up) sample identity should not be unknown
    if (change->write_params.sample_identity() == SampleIdentity::unknown())
    {
        EPROSIMA_LOG_WARNING(RTPS_PDP_LISTENER, "CacheChange_t is not properly identified for client-server operation");
        EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "------------------ PDP SERVER LISTENER END ------------------");
        EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "");
        return;
    }

    // Related_sample_identity could be lost in message delivered, so we set as sample_identity
    // An empty related_sample_identity could lead into an empty sample_identity when resending this msg
    if (change->write_params.related_sample_identity() == SampleIdentity::unknown())
    {
        change->write_params.related_sample_identity(change->write_params.sample_identity());
    }

    // Reset the internal CacheChange_t union.
    change->writer_info.next = nullptr;
    change->writer_info.previous = nullptr;
    change->writer_info.num_sent_submessages = 0;

    // DATA(p) case
    if (change->kind == ALIVE)
    {
        // Ignore announcement from own RTPSParticipant
        if (guid == pdp_server()->getRTPSParticipant()->getGuid())
        {
            // Observation: It never reaches this point
            EPROSIMA_LOG_WARNING(RTPS_PDP_LISTENER, "Message from own RTPSParticipant, ignoring");
            EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "------------------ PDP SERVER LISTENER END ------------------");
            EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "");
            return;
        }

        // Deserialize the payload to access the discovery info
        CDRMessage_t msg(change->serializedPayload);
        temp_participant_data_.clear();
        auto participant_data = temp_participant_data_;

        if (participant_data.readFromCDRMessage(
                    &msg,
                    true,
                    pdp_server()->getRTPSParticipant()->network_factory(),
                    pdp_server()->getRTPSParticipant()->has_shm_transport(),
                    true,
                    change_in->vendor_id))
        {
            if (parent_pdp_->getRTPSParticipant()->is_participant_ignored(participant_data.m_guid.guidPrefix))
            {
                return;
            }

            const auto& pattr = pdp_server()->getRTPSParticipant()->getAttributes();
            fastdds::rtps::network::external_locators::filter_remote_locators(participant_data,
                    pattr.builtin.metatraffic_external_unicast_locators, pattr.default_external_unicast_locators,
                    pattr.ignore_non_matching_locators);

            /* Check PID_VENDOR_ID */
            if (participant_data.m_VendorId != fastrtps::rtps::c_VendorId_eProsima)
            {
                EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER,
                        "DATA(p|Up) from different vendor is not supported for Discover-Server operation");
                return;
            }

            fastrtps::ParameterPropertyList_t properties = participant_data.m_properties;

            /* Check DS_VERSION */
            auto ds_version = std::find_if(
                properties.begin(),
                properties.end(),
                [](const dds::ParameterProperty_t& property)
                {
                    return property.first() == dds::parameter_property_ds_version;
                });

            if (ds_version != properties.end())
            {
                if (std::stof(ds_version->second()) < 1.0)
                {
                    EPROSIMA_LOG_ERROR(RTPS_PDP_LISTENER, "Minimum " << dds::parameter_property_ds_version
                                                                     << " is 1.0, found: " << ds_version->second());
                    return;
                }
                EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "Participant " << dds::parameter_property_ds_version << ": "
                                                                    << ds_version->second());
            }
            else
            {
                EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, dds::parameter_property_ds_version << " is not set. Assuming 1.0");
            }

            /* Check PARTICIPANT_TYPE */
            bool is_client = true;
            auto participant_type = std::find_if(
                properties.begin(),
                properties.end(),
                [](const dds::ParameterProperty_t& property)
                {
                    return property.first() == dds::parameter_property_participant_type;
                });

            if (participant_type != properties.end())
            {
                if (participant_type->second() == ParticipantType::SERVER ||
                        participant_type->second() == ParticipantType::BACKUP ||
                        participant_type->second() == ParticipantType::SUPER_CLIENT)
                {
                    is_client = false;
                }
                else if (participant_type->second() == ParticipantType::SIMPLE)
                {
                    EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "Ignoring " << dds::parameter_property_participant_type << ": "
                                                                     << participant_type->second());
                    return;
                }
                else if (participant_type->second() != ParticipantType::CLIENT)
                {
                    EPROSIMA_LOG_ERROR(RTPS_PDP_LISTENER, "Wrong " << dds::parameter_property_participant_type << ": "
                                                                   << participant_type->second());
                    return;
                }
                EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "Participant type " << participant_type->second());
            }
            else
            {
                EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, dds::parameter_property_participant_type << " is not set");
                // Fallback to checking whether participant is a SERVER looking for the persistence GUID
                auto persistence_guid = std::find_if(
                    properties.begin(),
                    properties.end(),
                    [](const dds::ParameterProperty_t& property)
                    {
                        return property.first() == dds::parameter_property_persistence_guid;
                    });
                // The presence of persistence GUID property suggests a SERVER. This assumption is made to keep
                // backwards compatibility with Discovery Server v1.0. However, any participant that has been configured
                // as persistent will have this property.
                if (persistence_guid != properties.end())
                {
                    is_client = false;
                }
                EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "Participant is client: " << std::boolalpha << is_client);
            }

            // Check whether the participant is a client/server of this server or if it has been forwarded from
            //  another entity (server).
            // is_local means that the server is connected (or will be) with this entity directly
            bool is_local = true;

            // In case a new changes arrives from a local entity, but the ParticipantProxyData already exists
            //  because we know it from other server
            bool was_local = true;

            // If the instance handle is different from the writer GUID, then the change has been relayed
            if (iHandle2GUID(change->instanceHandle).guidPrefix != change->writerGUID.guidPrefix)
            {
                is_local = false;
            }
            else
            {
                // We already know that the writer and the entity are the same, so we can use writerGUID
                was_local = pdp_server()->discovery_db().is_participant_local(change->writerGUID.guidPrefix);
            }

            if (!pdp_server()->discovery_db().backup_in_progress())
            {
                // Notify the DiscoveryDataBase
                if (pdp_server()->discovery_db().update(
                            change.get(),
                            ddb::DiscoveryParticipantChangeData(
                                participant_data.metatraffic_locators,
                                is_client,
                                is_local)))
                {
                    // Remove change from PDP reader history, but do not return it to the pool. From here on, the discovery
                    // database takes ownership of the CacheChange_t. Henceforth there are no references to the change.
                    // Take change ownership away from the unique pointer, so that its destruction does not destroy the data
                    pdp_history->remove_change(pdp_history->find_change(change.release()), false);

                    // Ensure processing time for the cache by triggering the Server thread (which process the updates)
                    // The server does not have to postpone the execution of the routine if a change is received, i.e.
                    // the server routine is triggered instantly as the default value of the interval that the server has
                    // to wait is 0.
                    routine_should_be_awake = true;

                    // TODO: when the DiscoveryDataBase allows updating capabilities we can dismissed old PDP processing
                }
                else
                {
                    // If the database doesn't take the ownership, then return the CacheChante_t to the pool.
                    pdp_reader->releaseCache(change.release());
                }

            }
            else
            {
                // Release the unique pointer, not the change in the pool
                change.release();
            }

            // At this point we can release reader lock.
            reader->getMutex().unlock();

            // Grant atomic access to PDP inherited proxies database
            std::unique_lock<std::recursive_mutex> lock(*pdp_server()->getMutex());

            // Check if participant proxy already exists (means the DATA(p) brings updated info)
            ParticipantProxyData* pdata = nullptr;
            for (ParticipantProxyData* it : pdp_server()->participant_proxies_)
            {
                if (guid == it->m_guid)
                {
                    pdata = it;
                    break;
                }
            }

            // Store whether the participant is new or updated
            auto status = (pdata == nullptr) ? ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT :
                    ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT;

            // New participant case
            if (status == ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
            {
                // TODO: pending avoid builtin connections on client info relayed by other server

                EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "Registering a new participant: " << guid);

                // Create a new participant proxy entry
                pdata = pdp_server()->createParticipantProxyData(participant_data, writer_guid);
                // Realease PDP mutex
                lock.unlock();

                // All local builtins are connected, the database will avoid any EDP DATA to be send before having PDP
                // DATA acknowledgement
                if (pdata && is_local)
                {
                    pdp_server()->assignRemoteEndpoints(pdata);
                }
            }
            // Case ParticipantProxyData already exists but was known remotly and now must be local
            else if (is_local && !was_local)
            {
                // Realease PDP mutex
                lock.unlock();

                pdp_server()->assignRemoteEndpoints(pdata);
            }
            // Updated participant information case
            else
            {
                // Update proxy
                pdata->updateData(participant_data);
                pdata->isAlive = true;
                // Realease PDP mutex
                lock.unlock();

                // TODO: pending client liveliness management here
                // Included form symmetry with PDPListener to profit from a future updateInfoMatchesEDP override
                if (pdp_server()->updateInfoMatchesEDP() && is_local)
                {
                    pdp_server()->mp_EDP->assignRemoteEndpoints(*pdata, true);
                }
            }

            // Check whether the participant proxy data was created/updated correctly
            if (pdata != nullptr)
            {
                // Notify user of the discovery/update of the participant
                RTPSParticipantListener* listener = pdp_server()->getRTPSParticipant()->getListener();
                if (listener != nullptr)
                {
                    bool should_be_ignored = false;
                    {
                        std::lock_guard<std::mutex> cb_lock(pdp_server()->callback_mtx_);
                        ParticipantDiscoveryInfo info(*pdata);
                        info.status = status;

                        listener->onParticipantDiscovery(
                            pdp_server()->getRTPSParticipant()->getUserRTPSParticipant(),
                            std::move(info), should_be_ignored);
                    }
                    if (should_be_ignored)
                    {
                        parent_pdp_->getRTPSParticipant()->ignore_participant(guid.guidPrefix);
                    }
                }
            }

            // Take again the reader lock
            reader->getMutex().lock();
        }
    }
    // DATA(Up) case
    else
    {
        // remove_remote_participant will try to remove the cache from the history and destroy it. We do it beforehand
        // to grant DiscoveryDatabase ownership by not returning the change to the pool.
        pdp_history->remove_change(pdp_history->find_change(change.get()), false);

        // Notify the DiscoveryDatabase. DiscoveryParticipantChangeData is left as default since DATA(Up) does not have
        // serialized data
        if (pdp_server()->discovery_db().update(change.get(), ddb::DiscoveryParticipantChangeData()))
        {
            // Ensure processing time for the cache by triggering the Server thread (which process the updates
            // The server does not have to postpone the execution of the routine if a change is received, i.e.
            // the server routine is triggered instantly as the default value of the interval that the server has
            // to wait is 0.
            routine_should_be_awake = true;

            // From here on, the discovery database takes ownership of the CacheChange_t. Henceforth there are no
            // references to the change. Take change ownership away from the unique pointer, so that its destruction
            // does not destroy the data
            change.release();
        }

        // Remove participant from proxies
        reader->getMutex().unlock();
        pdp_server()->remove_remote_participant(guid, ParticipantDiscoveryInfo::REMOVED_PARTICIPANT);
        reader->getMutex().lock();
    }

    /*
     * Awake routine thread if needed.
     * Thread is awaken at the end of the listener as it is required to have created the Proxies before
     * the data is processed and the new messages added to history.
     * If not, could happen that a message is added to history in order to be sent to a relevant participant, and
     * this Participant still not have a ReaderProxy associated, and so it will miss the message and it wont be
     * sent again (because if there are no changes PDP is no sent again).
     */
    if (routine_should_be_awake)
    {
        pdp_server()->awake_routine_thread();
    }

    // cache is removed from history (if it's still there) and returned to the pool on leaving the scope, since the
    // unique pointer destruction grants it. If the ownership has been taken away from the unique pointer, then nothing
    // happens at this point

    EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER,
            "-------------------- " << pdp_server()->mp_RTPSParticipant->getGuid() <<
            " --------------------");
    EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "------------------ PDP SERVER LISTENER END ------------------");
    EPROSIMA_LOG_INFO(RTPS_PDP_LISTENER, "");
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
