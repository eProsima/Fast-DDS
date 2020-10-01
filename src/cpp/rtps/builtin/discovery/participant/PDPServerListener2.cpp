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
 * @file PDPServerListener2.cpp
 *
 */

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>

#include "./PDPServerListener2.hpp"
#include "./PDPServer2.hpp"
#include "../database/DiscoveryParticipantChangeData.hpp"

#include <memory>

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace eprosima::fastrtps::rtps;

PDPServerListener2::PDPServerListener2(
        PDPServer2* in_PDP)
    : PDPListener(in_PDP)
{
}

PDPServer2* PDPServerListener2::pdp_server()
{
    return static_cast<PDPServer2*>(parent_pdp_);
}

void PDPServerListener2::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    logInfo(RTPS_PDP_LISTENER, "");
    logInfo(RTPS_PDP_LISTENER, "------------------ PDP SERVER LISTENER START ------------------");
    logInfo(RTPS_PDP_LISTENER, "PDP Server Message received: " << change_in->instanceHandle);

    // Get PDP reader history
    auto pdp_history = pdp_server()->mp_PDPReaderHistory;

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
        logWarning(RTPS_PDP_LISTENER, "Problem getting the key of the change, removing");
        logInfo(RTPS_PDP_LISTENER, "------------------ PDP SERVER LISTENER END ------------------");
        logInfo(RTPS_PDP_LISTENER, "");
        return;
    }

    // Take GUID from instance handle
    GUID_t guid = iHandle2GUID(change->instanceHandle);

    // DATA(p|Up) sample identity should not be unknown
    if (change->write_params.sample_identity() == SampleIdentity::unknown())
    {
        logWarning(RTPS_PDP_LISTENER, "CacheChange_t is not properly identified for client-server operation");
        logInfo(RTPS_PDP_LISTENER, "------------------ PDP SERVER LISTENER END ------------------");
        logInfo(RTPS_PDP_LISTENER, "");
        return;
    }

    // DATA(p) case
    if (change->kind == ALIVE)
    {
        // Ignore announcement from own RTPSParticipant
        if (guid == pdp_server()->getRTPSParticipant()->getGuid())
        {
            logInfo(RTPS_PDP_LISTENER, "Message from own RTPSParticipant, ignoring");
            logInfo(RTPS_PDP_LISTENER, "------------------ PDP SERVER LISTENER END ------------------");
            logInfo(RTPS_PDP_LISTENER, "");
            return;
        }

        // Deserialize the payload to access the discovery info
        CDRMessage_t msg(change->serializedPayload);
        temp_participant_data_.clear();

        if (temp_participant_data_.readFromCDRMessage(
                    &msg,
                    true,
                    pdp_server()->getRTPSParticipant()->network_factory(),
                    pdp_server()->getRTPSParticipant()->has_shm_transport()))
        {
            // // Key should not match instance handle
            // if (change->instanceHandle == temp_participant_data_.m_key)
            // {
            //     logInfo(RTPS_PDP_LISTENER, "Malformed PDP payload received, ignoring: " << change->instanceHandle);
            //     return;
            // }

            // Check whether the participant is a CLIENT or a SERVER
            bool is_client = true;
            fastrtps::ParameterPropertyList_t properties = temp_participant_data_.m_properties;
            for (auto property : properties)
            {
                // If property PID_PERSISTENCE_GUID is set, then the participant is not a CLIENT
                if (property.first() == "PID_PERSISTENCE_GUID")
                {
                    is_client = false;
                    break;
                }
            }

            // Check whether the participant is a client of this server, or it has been discovered through another
            // server
            bool is_my_client = true;
            // If the instance handle is different from the writer GUID, then the change has been relayed. That means,
            // that the participants is somebody else's client
            if (change->instanceHandle != change->writerGUID)
            {
                is_my_client = false;
            }
            // If the change has NOT been relayed, then look for it in this server's list of remote servers (servers
            // for which this sever is a client).
            //    1. If the participant is there, it means that this server is a CLIENT of the remote one, thus the
            //       remote is not my client.
            //    2. If the participant is not there, it means that the remote server is a CLIENT of this server
            else
            {
                // Iterate over the servers for which I'm a CLIENT
                for (auto server : pdp_server()->servers())
                {
                    // If the change's participant is in the list, then it means I'm a CLIENT to it.
                    if (iHandle2GUID(change->instanceHandle).guidPrefix == server.guidPrefix)
                    {
                        is_my_client = false;
                        break;
                    }
                }
            }

            // Notify the DiscoveryDataBase
            if (pdp_server()->discovery_db().update(
                change.get(),
                ddb::DiscoveryParticipantChangeData(
                    temp_participant_data_.metatraffic_locators,
                    is_client,
                    is_my_client)))
            {
                // Remove change from PDP reader history, but do not return it to the pool. From here on, the discovery
                // database takes ownership of the CacheChange_t. Henceforth there are no references to the change.
                // Take change ownership away from the unique pointer, so that its destruction does not destroy the data
                pdp_history->remove_change(pdp_history->find_change(change.release()), false);

                // Ensure processing time for the cache by triggering the Server thread (which process the updates
                pdp_server()->awakeServerThread();

                // TODO: when the DiscoveryDataBase allows updating capabilities we can dismissed old PDP processing
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

                logInfo(RTPS_PDP_LISTENER, "Registering a new participant: " << guid);

                // Create a new participant proxy entry
                pdata = pdp_server()->createParticipantProxyData(temp_participant_data_, writer_guid);
                // Realease PDP mutex
                lock.unlock();

                // All builtins are connected, the database will avoid any EDP DATA to be send before having PDP DATA
                // acknowledgement
                if (pdata)
                {
                    pdp_server()->assignRemoteEndpoints(pdata);
                }
            }
            // Updated participant information case
            else
            {
                // Update proxy
                pdata->updateData(temp_participant_data_);
                pdata->isAlive = true;
                // Realease PDP mutex
                lock.unlock();

                // TODO: pending client liveliness management here
                // Included form symmetry with PDPListener to profit from a future updateInfoMatchesEDP override
                if (pdp_server()->updateInfoMatchesEDP())
                {
                    pdp_server()->mp_EDP->assignRemoteEndpoints(*pdata);
                }
            }

            // Check whether the participant proxy data was created/updated correctly
            if (pdata != nullptr)
            {
                // Notify user of the discovery/update of the participant
                RTPSParticipantListener* listener = pdp_server()->getRTPSParticipant()->getListener();
                if (listener != nullptr)
                {
                    std::lock_guard<std::mutex> cb_lock(pdp_server()->callback_mtx_);
                    ParticipantDiscoveryInfo info(*pdata);
                    info.status = status;

                    listener->onParticipantDiscovery(
                        pdp_server()->getRTPSParticipant()->getUserRTPSParticipant(),
                        std::move(info));
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
            pdp_server()->awakeServerThread();

            // From here on, the discovery database takes ownership of the CacheChange_t. Henceforth there are no
            // references to the change. Take change ownership away from the unique pointer, so that its destruction
            // does not destroy the data
            change.release();
        }

        // Remove participant from proxies
        reader->getMutex().unlock();
        if (pdp_server()->remove_remote_participant(guid, ParticipantDiscoveryInfo::REMOVED_PARTICIPANT))
        {
            reader->getMutex().lock();
            return;
        }
        reader->getMutex().lock();
    }
    // cache is removed from history (if it's still there) and returned to the pool on leaving the scope, since the
    // unique pointer destruction grants it. If the ownership has been taken away from the unique pointer, then nothing
    // happens at this point

    logInfo(RTPS_PDP_LISTENER, "------------------ PDP SERVER LISTENER END ------------------");
    logInfo(RTPS_PDP_LISTENER, "");
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
