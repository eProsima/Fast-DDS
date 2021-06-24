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
    CacheChange_t* change = (CacheChange_t*)(change_in);
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
            change->instanceHandle = local_data.m_key;
            guid = local_data.m_guid;
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
            logWarning(RTPS_PDP, "PDPServerListener received DATA(p) NOT_ALIVE_DISPOSED from unknown participant");
            parent_pdp_->mp_PDPReaderHistory->remove_change(change);
            return;
        }

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
