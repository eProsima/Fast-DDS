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
 * @file PDPSimpleListener.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimpleListener.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/builtin/discovery/endpoint/EDP.h>
#include <fastrtps/rtps/reader/RTPSReader.h>

#include <fastrtps/rtps/resources/TimedEvent.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>

#include <fastrtps/utils/TimeConversion.h>


#include <mutex>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

PDPSimpleListener::PDPSimpleListener(PDPSimple* parent)
    : parent_pdp_(parent)
    , temp_participant_data_(parent->getRTPSParticipant()->getRTPSParticipantAttributes().allocation)
{
}

void PDPSimpleListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t * const change_in)
{
    CacheChange_t* change = (CacheChange_t*)(change_in);
    logInfo(RTPS_PDP,"SPDP Message received");

    // Make sure we have an instance handle (i.e GUID)
    if(change->instanceHandle == c_InstanceHandle_Unknown)
    {
        if(!this->get_key(change))
        {
            logWarning(RTPS_PDP,"Problem getting the key of the change, removing");
            parent_pdp_->mp_SPDPReaderHistory->remove_change(change);
            return;
        }
    }

    // Take GUID from instance handle
    GUID_t guid;
    iHandle2GUID(guid, change->instanceHandle);

    if(change->kind == ALIVE)
    {
        // Ignore announcement from own RTPSParticipant
        if (guid == parent_pdp_->getRTPSParticipant()->getGuid())
        {
            logInfo(RTPS_PDP, "Message from own RTPSParticipant, removing");
            parent_pdp_->mp_SPDPReaderHistory->remove_change(change);
            return;
        }
        
        // Protecting access to temp_participant_data_
        std::unique_lock<std::mutex> data_lock(mutex_);

        // Load information on temp_participant_data_
        CDRMessage_t msg(change->serializedPayload);
        if(temp_participant_data_.readFromCDRMessage(&msg))
        {
            // After correctly reading it
            change->instanceHandle = temp_participant_data_.m_key;

            // At this point we can release reader lock.
            reader->getMutex().unlock();

            // Check if participant already exists (updated info)
            ParticipantProxyData* pdata = nullptr;
            std::unique_lock<std::recursive_mutex> lock(*parent_pdp_->getMutex());
            for (ParticipantProxyData* it : parent_pdp_->participant_proxies_)
            {
                if(temp_participant_data_.m_guid == it->m_guid)
                {
                    pdata = it;
                    break;
                }
            }

            auto status = (pdata == nullptr) ? ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT :
                ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT;

            if(pdata == nullptr)
            {
                // Create a new one when not found
                pdata = parent_pdp_->add_participant_proxy_data(temp_participant_data_.m_guid);
                if (pdata != nullptr)
                {
                    pdata->copy(temp_participant_data_);
                    pdata->isAlive = true;
                    pdata->lease_duration_event->update_interval(pdata->m_leaseDuration);
                    pdata->lease_duration_event->restart_timer();
                    lock.unlock();

                    parent_pdp_->announceParticipantState(false);
                    parent_pdp_->assignRemoteEndpoints(pdata);
                }
            }
            else
            {
                pdata->updateData(temp_participant_data_);
                pdata->isAlive = true;
                lock.unlock();

                if(parent_pdp_->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
                    parent_pdp_->mp_EDP->assignRemoteEndpoints(*pdata);
            }

            if (pdata != nullptr)
            {
                auto listener = parent_pdp_->getRTPSParticipant()->getListener();
                if (listener != nullptr)
                {
                    ParticipantDiscoveryInfo info(*pdata);
                    info.status = status;

                    listener->onParticipantDiscovery(parent_pdp_->getRTPSParticipant()->getUserRTPSParticipant(), std::move(info));
                }
            }

            // Take again the reader lock
            reader->getMutex().lock();
        }
    }
    else
    {
        if(parent_pdp_->remove_remote_participant(guid, ParticipantDiscoveryInfo::REMOVED_PARTICIPANT))
        {
            return; // change already removed from history
        }
    }

    //Remove change form history.
    parent_pdp_->mp_SPDPReaderHistory->remove_change(change);
}

bool PDPSimpleListener::get_key(CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, PID_PARTICIPANT_GUID);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
