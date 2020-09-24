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

PDPServer2 * PDPServerListener2::pdp_server()
{
    return static_cast<PDPServer2*>(parent_pdp_);
}


void PDPServerListener2::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    auto pdp_history = pdp_server()->mp_PDPReaderHistory;
    auto deleter = [pdp_history](CacheChange_t* p)
            {
                pdp_history->remove_change(p);
            };

    std::unique_ptr<CacheChange_t, decltype(deleter)> change((CacheChange_t*)(change_in), deleter);

    GUID_t writer_guid = change->writerGUID;
    logInfo(RTPS_PDP, "PDP Server Message received");

    // validate CacheChange
    if (change->instanceHandle == c_InstanceHandle_Unknown
        && !this->get_key(change.get()))
    {
        logWarning(RTPS_PDP, "Problem getting the key of the change, removing");
        return;
    }

    // Take GUID from instance handle
    GUID_t guid;
    iHandle2GUID(guid, change->instanceHandle);

    if (change->write_params.sample_identity() == SampleIdentity::unknown())
    {
        logWarning(RTPS_PDP, "CacheChange_t is not properly identified for client-server operation.");
        return;
    }

    if (change->kind == ALIVE)
    {
        // Ignore announcement from own RTPSParticipant
        if (guid == pdp_server()->getRTPSParticipant()->getGuid())
        {
            logInfo(RTPS_PDP, "Message from own RTPSParticipant, removing");
            return;
        }

        // Deserialized the payload to access the discovery info
        CDRMessage_t msg(change->serializedPayload);
        temp_participant_data_.clear();

        if (temp_participant_data_.readFromCDRMessage(
                &msg,
                true,
                pdp_server()->getRTPSParticipant()->network_factory(),
                pdp_server()->getRTPSParticipant()->has_shm_transport()))
        {
            if (change->instanceHandle == temp_participant_data_.m_key)
            {
                logInfo(RTPS_PDP, "Malformed PDP payload received, removing");
                return;
            }

            // Update the DiscoveryDabase
            if (pdp_server()->discovery_db().update(change.get()))
            {
                // assure processing time for the cache
                pdp_server()->awakeServerThread();

                // the discovery database takes ownership of the CacheChange_t
                // henceforth there are no references to the CacheChange_t
                pdp_history->remove_change_and_reuse(change.release());

                // TODO: when the DiscoveryDataBase allows updating capabilities we can dismissed old PDP processing
            }

            // At this point we can release reader lock.
            reader->getMutex().unlock();

            // grant atomic access to PDP inherited database
            std::unique_lock<std::recursive_mutex> lock(*pdp_server()->getMutex());

            // Check if participant already exists (updated info)
            ParticipantProxyData* pdata = nullptr;
            for (ParticipantProxyData* it : pdp_server()->participant_proxies_)
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
                // TODO: pending avoid builtin connections on client info relayed by other server

                logInfo(RTPS_PDP, "Registering a new participant: " << guid);

                // Create a new one when not found
                pdata = pdp_server()->createParticipantProxyData(temp_participant_data_, writer_guid);
                lock.unlock();

                // All builtins are connected, the database will avoid any EDP DATA to be send before having PDP DATA
                // acknowledgement
                if (pdata)
                {
                    pdp_server()->assignRemoteEndpoints(pdata);
                }
            }
            else
            {
                pdata->updateData(temp_participant_data_);
                pdata->isAlive = true;
                lock.unlock();

                // TODO: pending client liveliness management here
                // Included form symmetry with PDPListener to profit from a future updateInfoMatchesEDP override
                if (pdp_server()->updateInfoMatchesEDP())
                {
                    pdp_server()->mp_EDP->assignRemoteEndpoints(*pdata);
                }
            }

            if (pdata != nullptr)
            {
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
    else // Participant disposal
    {
        // remove_remote_participant will try to remove the cache from the history and destroy it. We do it beforehand
        // to grant DiscoveryDatabase ownership
        pdp_history->remove_change_and_reuse(change.get());

        // Update the DiscoveryDabase
        if (pdp_server()->discovery_db().update(change.get()))
        {
            // assure processing time for the cache
            pdp_server()->awakeServerThread();

            // the discovery database takes ownership of the CacheChange_t
            // henceforth there are no references to the CacheChange_t
            change.release();
        }

        reader->getMutex().unlock();
        if (pdp_server()->remove_remote_participant(guid, ParticipantDiscoveryInfo::REMOVED_PARTICIPANT))
        {
            reader->getMutex().lock();
            return;
        }
        reader->getMutex().lock();
    }

    // cache is removed from history (if still there) on leaving the scope
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
