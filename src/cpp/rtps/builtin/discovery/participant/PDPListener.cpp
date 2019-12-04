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
 * @file PDPListener.cpp
 *
 */

#include <fastrtps/rtps/reader/RTPSReader.h>

#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastrtps/utils/TimeConversion.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDP.h>
#include <fastrtps/rtps/builtin/discovery/endpoint/EDP.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPListener.h>
#include <fastrtps/rtps/resources/TimedEvent.h>

#include <fastrtps/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>

#include "../../../participant/RTPSParticipantImpl.h"

#include <mutex>

#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

PDPListener::PDPListener(PDP* parent)
    : parent_pdp_(parent)
    , temp_participant_data_(parent->getRTPSParticipant()->getRTPSParticipantAttributes().allocation)
{
}

void PDPListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    CacheChange_t* change = const_cast<CacheChange_t*>(change_in);
    GUID_t writer_guid = change->writerGUID;
    logInfo(RTPS_PDP,"SPDP Message received");

    // Make sure we have an instance handle (i.e GUID)
    if(change->instanceHandle == c_InstanceHandle_Unknown)
    {
        if(!this->get_key(change))
        {
            logWarning(RTPS_PDP,"Problem getting the key of the change, removing");
            parent_pdp_->mp_PDPReaderHistory->remove_change(change);
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
            parent_pdp_->mp_PDPReaderHistory->remove_change(change);
            return;
        }

        // Release reader lock to avoid ABBA lock. PDP mutex should always be first.
        // Keep change information on local variables to check consistency later
        SequenceNumber_t seq_num = change->sequenceNumber;
        ParticipantDiscoveryInfo::DISCOVERY_STATUS status;
        reader->getMutex().unlock();

        // changes may arise here on reader status!!!

        std::unique_lock<std::recursive_mutex> lock(*parent_pdp_->getMutex());
        reader->getMutex().lock();

        // Check state 1. If change is not consistent, it will be processed on the thread that has overriten it
        if((ALIVE != change->kind) || (seq_num != change->sequenceNumber) || (writer_guid != change->writerGUID))
        {
            return;
        }

        // Check if there is a pool ParticipantProxyData already associated with this change
        // 1- search in the local collection
        std::shared_ptr<ParticipantProxyData> pdata = parent_pdp_->get_from_local_proxies(guid.guidPrefix);
        bool create = !pdata;

        // 2- If not found search in the pool (maybe other participant created it)
        if(!pdata)
        {
            pdata = PDP::get_from_proxy_pool(guid.guidPrefix));
        }

        // 3 - Deserialize if needed
        bool deserialize = false;
        if( !pdata || pdata->version_ < seq_num )
        { 
            // Access to temp_participant_data_ is protected by reader lock
            // deserialize on temp_participant_data if new info
            temp_participant_data_.clear();

            // Load information on temp_participant_data_
            CDRMessage_t msg(change->serializedPayload);
            if(!temp_participant_data_.readFromCDRMessage(&msg, true, parent_pdp_->getRTPSParticipant()->network_factory()))
            {
                temp_participant_data_.clear();
            }
            else
            {
                deserialize = true;

                // After correctly reading it
                change->instanceHandle = temp_participant_data_.m_key;
                guid = temp_participant_data_.m_guid;

                // if new create and copy the temp_participant_data
                status = !pdata ? ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT :
                    ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT;
            }
        }

        if( create )
        {
            if(deserialize)
            {   // we must create a new proxy
                pdata = parent_pdp_->createParticipantProxyData( temp_participant_data_, writer_guid);
                // createParticipantProxyData returns with ParticipantProxyData mutex ownership
            }
            else
            {   // already around add to the locals
                parent_pdp_->add_participant_proxy_data(pdata);
                pdata->ppd_mutex_.lock();
            }
            // Release mutexes ownership
            reader->getMutex().unlock();
            lock.unlock();

            parent_pdp_->announceParticipantState(false);
            parent_pdp_->assignRemoteEndpoints(pdata.get());

            pdata->ppd_mutex_.unlock(); // got by createParticipantProxyData
        }
        else if ( deserialize )
        {
            std::lock_guard<std::recursive_mutex> ppd_lock(pdata->ppd_mutex_);

            // Participant proxy data mutex was lock above to keep version_
            pdata->updateData(temp_participant_data_);
            pdata->isAlive = true;

            // Release mutexes ownership
            reader->getMutex().unlock();
            lock.unlock();

            if(parent_pdp_->updateInfoMatchesEDP())
            {
                parent_pdp_->mp_EDP->assignRemoteEndpoints(*pdata);
            }
        }
        else
        {
            // Release mutexes ownership
            reader->getMutex().unlock();
            lock.unlock();
        }
        
        if( pdata && temp_participant_data_.m_guid != GUID_t::unknown())
        {
            RTPSParticipantListener* listener = parent_pdp_->getRTPSParticipant()->getListener();
            if(listener != nullptr)
            {
                std::lock_guard<std::mutex> cb_lock(parent_pdp_->callback_mtx_);
                std::lock_guard<std::recursive_mutex> ppd_lock(pdata->ppd_mutex_);

                ParticipantDiscoveryInfo info(*pdata);
                info.status = status;

                listener->onParticipantDiscovery(
                    parent_pdp_->getRTPSParticipant()->getUserRTPSParticipant(),
                    std::move(info));
            }
        }

        reader->getMutex().lock();
    }
    else
    {
        reader->getMutex().unlock();
        if (parent_pdp_->remove_remote_participant(guid, ParticipantDiscoveryInfo::REMOVED_PARTICIPANT))
        {
            reader->getMutex().lock();
            return; // all changes related with this participant have been removed from history by remove_remote_participant
        }
        reader->getMutex().lock();
    }

    //Remove change form history.
    parent_pdp_->mp_PDPReaderHistory->remove_change(change);
}

bool PDPListener::get_key(CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, PID_PARTICIPANT_GUID);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
