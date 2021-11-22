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

#include <fastdds/rtps/reader/RTPSReader.h>

#include <fastdds/rtps/history/ReaderHistory.h>

#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>

#include <fastrtps/utils/TimeConversion.h>

#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>

#include <rtps/participant/RTPSParticipantImpl.h>

#include <mutex>

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/rtps/builtin/discovery/participant/PDPServerListener.h>
#include <fastdds/rtps/builtin/discovery/participant/PDPServer.h>


namespace eprosima {
namespace fastrtps {
namespace rtps {

PDPServerListener::PDPServerListener(
        PDPServer* in_PDP)
    : PDPListener(in_PDP)
    , parent_server_pdp_(in_PDP)
{
}

void PDPServerListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
<<<<<<< HEAD
    CacheChange_t* change = (CacheChange_t*)(change_in);
=======
    logInfo(RTPS_PDP_LISTENER, "");
    logInfo(RTPS_PDP_LISTENER, "------------------ PDP SERVER LISTENER START ------------------");
    logInfo(RTPS_PDP_LISTENER,
            "-------------------- " << pdp_server()->mp_RTPSParticipant->getGuid() <<
            " --------------------");
    logInfo(RTPS_PDP_LISTENER, "PDP Server Message received: " << change_in->instanceHandle);

    // Get PDP reader history
    auto pdp_history = pdp_server()->mp_PDPReaderHistory;
    // Get PDP reader to release change
    auto pdp_reader = pdp_server()->mp_PDPReader;

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
>>>>>>> 436826000 (Discovery Server fix reconnection (#2246))
    GUID_t writer_guid = change->writerGUID;
    logInfo(RTPS_PDP, "SPDP Message received");

    if (change->instanceHandle == c_InstanceHandle_Unknown)
    {
        if (!this->get_key(change))
        {
            logWarning(RTPS_PDP, "Problem getting the key of the change, removing");
            parent_pdp_->mp_PDPReaderHistory->remove_change(change);
            return;
        }
    }

    // update the PDP Writer with this reader info
    if (!parent_server_pdp_->addRelayedChangeToHistory(*change))
    {
        logInfo(RTPS_PDP, "Ignoring a DATA(p) that was already received");

        parent_pdp_->mp_PDPReaderHistory->remove_change(change);
        return; // already there
    }

    // Take GUID from instance handle
    GUID_t guid;
    iHandle2GUID(guid, change->instanceHandle);

    if (change->kind == ALIVE)
    {
        // Ignore announcement from own RTPSParticipant
        if (guid == parent_pdp_->getRTPSParticipant()->getGuid())
        {
            logInfo(RTPS_PDP, "Message from own RTPSParticipant, removing");
            parent_pdp_->mp_PDPReaderHistory->remove_change(change);
            return;
        }

        ParticipantProxyData local_data(parent_pdp_->getRTPSParticipant()->getRTPSParticipantAttributes().allocation);

        // Load information on local_data
        CDRMessage_t msg(change->serializedPayload);
        if (local_data.readFromCDRMessage(&msg, true,
                parent_pdp_->getRTPSParticipant()->network_factory(),
                parent_pdp_->getRTPSParticipant()->has_shm_transport()))
        {
<<<<<<< HEAD
            change->instanceHandle = local_data.m_key;
            guid = local_data.m_guid;
=======
            /* Check PID_VENDOR_ID */
            if (participant_data.m_VendorId != fastrtps::rtps::c_VendorId_eProsima)
            {
                logInfo(RTPS_PDP_LISTENER,
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
                    logError(RTPS_PDP_LISTENER, "Minimum " << dds::parameter_property_ds_version
                                                           << " is 1.0, found: " << ds_version->second());
                    return;
                }
                logInfo(RTPS_PDP_LISTENER, "Participant " << dds::parameter_property_ds_version << ": "
                                                          << ds_version->second());
            }
            else
            {
                logInfo(RTPS_PDP_LISTENER, dds::parameter_property_ds_version << " is not set. Assuming 1.0");
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
                    logInfo(RTPS_PDP_LISTENER, "Ignoring " << dds::parameter_property_participant_type << ": "
                                                           << participant_type->second());
                    return;
                }
                else if (participant_type->second() != ParticipantType::CLIENT)
                {
                    logError(RTPS_PDP_LISTENER, "Wrong " << dds::parameter_property_participant_type << ": "
                                                         << participant_type->second());
                    return;
                }
                logInfo(RTPS_PDP_LISTENER, "Participant type " << participant_type->second());
            }
            else
            {
                logInfo(RTPS_PDP_LISTENER, dds::parameter_property_participant_type << " is not set");
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
                logInfo(RTPS_PDP_LISTENER, "Participant is client: " << std::boolalpha << is_client);
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

>>>>>>> 436826000 (Discovery Server fix reconnection (#2246))
            // At this point we can release reader lock.
            reader->getMutex().unlock();

            // Check if participant already exists (updated info)
            ParticipantProxyData* pdata = nullptr;
            std::unique_lock<std::recursive_mutex> lock(*parent_pdp_->getMutex());
            for (ParticipantProxyData* it : parent_pdp_->participant_proxies_)
            {
                if (guid == it->m_guid)
                {
                    pdata = it;
                    break;
                }
            }

            auto status = (pdata == nullptr) ? ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT :
                    ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT;

            if (pdata == nullptr)
            {
                logInfo(RTPS_PDP, "Registering a new participant: " <<
                        change->write_params.sample_identity().writer_guid());

                // Create a new one when not found
                pdata = parent_pdp_->createParticipantProxyData(local_data, writer_guid);
                lock.unlock();

                if (pdata != nullptr)
                {
                    // Dismiss any client data relayed by a server
                    if (pdata->m_guid.guidPrefix == writer_guid.guidPrefix)
                    {
                        // This call would be needed again if the clients known not the server prefix
                        //  parent_pdp_->announceParticipantState(false);
                        parent_pdp_->assignRemoteEndpoints(pdata);
                        parent_server_pdp_->queueParticipantForEDPMatch(pdata);
                    }
                }
            }
            else
            {
                pdata->updateData(local_data);
                pdata->isAlive = true;
                // activate lease duration if the DATA(p) comes directly from the client
                bool previous_lease_check_status = pdata->should_check_lease_duration;
                pdata->should_check_lease_duration = writer_guid.guidPrefix == pdata->m_guid.guidPrefix;
                lock.unlock();

                // Included for symmetry with PDPListener to profit from a future updateInfoMatchesEDP override
                // right now servers update matching on clients that were previously relayed by a server
                if ( previous_lease_check_status != pdata->should_check_lease_duration
                        || parent_pdp_->updateInfoMatchesEDP())
                {
                    parent_pdp_->assignRemoteEndpoints(pdata);
                    parent_server_pdp_->queueParticipantForEDPMatch(pdata);
                }
            }

            if (pdata != nullptr)
            {
                RTPSParticipantListener* listener = parent_pdp_->getRTPSParticipant()->getListener();
                if (listener != nullptr)
                {
                    std::lock_guard<std::mutex> cb_lock(parent_pdp_->callback_mtx_);
                    ParticipantDiscoveryInfo info(*pdata);
                    info.status = status;

                    listener->onParticipantDiscovery(
                        parent_pdp_->getRTPSParticipant()->getUserRTPSParticipant(),
                        std::move(info));
                }
            }

            // Take again the reader lock
            reader->getMutex().lock();
        }
    }
    else
    {
        InstanceHandle_t key;

        if (!parent_pdp_->lookup_participant_key(guid, key))
        {
<<<<<<< HEAD
            logWarning(RTPS_PDP, "PDPServerListener received DATA(p) NOT_ALIVE_DISPOSED from unknown participant");
            parent_pdp_->mp_PDPReaderHistory->remove_change(change);
            return;
        }
=======
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
>>>>>>> 436826000 (Discovery Server fix reconnection (#2246))

        if (parent_pdp_->remove_remote_participant(guid, ParticipantDiscoveryInfo::REMOVED_PARTICIPANT))
        {
            // all changes related with this participant have been removed from history by removeRemoteParticipant
            return;
        }
    }

    //Remove change form history.
    parent_pdp_->mp_PDPReaderHistory->remove_change(change);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
