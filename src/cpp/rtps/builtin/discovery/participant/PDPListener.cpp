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

#include <fastdds/rtps/builtin/discovery/participant/PDPListener.h>

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/rtps/builtin/discovery/endpoint/EDP.h>
#include <fastdds/rtps/builtin/discovery/participant/PDP.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.h>
#include <fastdds/rtps/participant/RTPSParticipantListener.h>
#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/resources/TimedEvent.h>

#include <fastrtps/utils/TimeConversion.h>

#include <fastdds/core/policy/ParameterList.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>

#include <mutex>

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastrtps {
namespace rtps {

PDPListener::PDPListener(
        PDP* parent)
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
    logInfo(RTPS_PDP, "SPDP Message received from: " << change_in->writerGUID);

    // Make sure we have an instance handle (i.e GUID)
    if (change->instanceHandle == c_InstanceHandle_Unknown)
    {
        if (!this->get_key(change))
        {
            logWarning(RTPS_PDP, "Problem getting the key of the change, removing");
            parent_pdp_->mp_PDPReaderHistory->remove_change(change);
            return;
        }
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

        // Release reader lock to avoid ABBA lock. PDP mutex should always be first.
        // Keep change information on local variables to check consistency later
        SequenceNumber_t seq_num = change->sequenceNumber;
        reader->getMutex().unlock();
        std::unique_lock<std::recursive_mutex> lock(*parent_pdp_->getMutex());
        reader->getMutex().lock();

        // If change is not consistent, it will be processed on the thread that has overriten it
        if ((ALIVE != change->kind) || (seq_num != change->sequenceNumber) || (writer_guid != change->writerGUID))
        {
            return;
        }

        // Access to temp_participant_data_ is protected by reader lock

        // Load information on temp_participant_data_
        CDRMessage_t msg(change->serializedPayload);
        temp_participant_data_.clear();
        if (temp_participant_data_.readFromCDRMessage(&msg, true, parent_pdp_->getRTPSParticipant()->network_factory(),
            parent_pdp_->getRTPSParticipant()->has_shm_transport()))
        {
            // After correctly reading it
            change->instanceHandle = temp_participant_data_.m_key;
            guid = temp_participant_data_.m_guid;

            // Check if participant already exists (updated info)
            ParticipantProxyData* pdata = nullptr;
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
                // Create a new one when not found
                pdata = parent_pdp_->createParticipantProxyData(temp_participant_data_, writer_guid);
                if (pdata != nullptr)
                {
                    reader->getMutex().unlock();
                    lock.unlock();

                    logInfo(RTPS_PDP_DISCOVERY, "New participant " << pdata->m_guid << " at " << "MTTLoc: "
                            << pdata->metatraffic_locators << " DefLoc:" << pdata->default_locators);

                    // Assigning remote endpoints implies sending a DATA(p) to all matched and fixed readers, since
                    // StatelessWriter::matched_reader_add marks the entire history as unsent if the added reader's
                    // durability is bigger or equal to TRANSIENT_LOCAL_DURABILITY_QOS (TRANSIENT_LOCAL or TRANSIENT),
                    // which is the case of ENTITYID_BUILTIN_SDP_PARTICIPANT_READER (TRANSIENT_LOCAL). If a remote
                    // participant is discovered before creating the first DATA(p) change (which happens at the end of
                    // BuiltinProtocols::initBuiltinProtocols), then StatelessWriter::matched_reader_add ends up marking
                    // no changes as unsent (since the history is empty), which is OK because this can only happen if a
                    // participant is discovered in the middle of BuiltinProtocols::initBuiltinProtocols, which will
                    // create the first DATA(p) upon finishing, thus triggering the sent to all fixed and matched
                    // readers anyways.
                    parent_pdp_->assignRemoteEndpoints(pdata);
                }
            }
            else
            {
                pdata->updateData(temp_participant_data_);
                pdata->isAlive = true;
                reader->getMutex().unlock();
                lock.unlock();

                logInfo(RTPS_PDP_DISCOVERY, "Update participant " << pdata->m_guid << " at " << "MTTLoc: "
                        << pdata->metatraffic_locators << " DefLoc:" << pdata->default_locators);

                if (parent_pdp_->updateInfoMatchesEDP())
                {
                    parent_pdp_->mp_EDP->assignRemoteEndpoints(*pdata);
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
        reader->getMutex().unlock();
        if (parent_pdp_->remove_remote_participant(guid, ParticipantDiscoveryInfo::REMOVED_PARTICIPANT))
        {
            reader->getMutex().lock();
            // All changes related with this participant have been removed from history by remove_remote_participant
            return;
        }
        reader->getMutex().lock();
    }

    //Remove change form history.
    parent_pdp_->mp_PDPReaderHistory->remove_change(change);
}

bool PDPListener::get_key(
        CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, fastdds::dds::PID_PARTICIPANT_GUID);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
