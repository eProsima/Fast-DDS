// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPSecurityInitiatorListener.cpp
 *
 */

#include <rtps/builtin/discovery/participant/DS/PDPSecurityInitiatorListener.hpp>

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
#include <rtps/builtin/discovery/participant/PDPEndpoints.hpp>
#include <rtps/network/ExternalLocatorsProcessor.hpp>
#include <rtps/participant/RTPSParticipantImpl.h>

#include <mutex>

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastrtps {
namespace rtps {

PDPSecurityInitiatorListener::PDPSecurityInitiatorListener(
        PDP* parent,
        SecurityInitiatedCallback response_cb)
    : parent_pdp_(parent)
    , temp_participant_data_(parent->getRTPSParticipant()->getRTPSParticipantAttributes().allocation)
    , response_cb_(response_cb)
{
}

void PDPSecurityInitiatorListener::onNewCacheChangeAdded(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    CacheChange_t* change = const_cast<CacheChange_t*>(change_in);
    GUID_t writer_guid = change->writerGUID;
    EPROSIMA_LOG_INFO(RTPS_PDP, "SPDP Message received from: " << change_in->writerGUID);

    // Make sure we have an instance handle (i.e GUID)
    if (change->instanceHandle == c_InstanceHandle_Unknown)
    {
        if (!this->get_key(change))
        {
            EPROSIMA_LOG_WARNING(RTPS_PDP, "Problem getting the key of the change, removing");
            parent_pdp_->builtin_endpoints_->remove_from_pdp_reader_history(change);
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
            EPROSIMA_LOG_INFO(RTPS_PDP, "Message from own RTPSParticipant, removing");
            parent_pdp_->builtin_endpoints_->remove_from_pdp_reader_history(change);
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

            // Filter locators
            const auto& pattr = parent_pdp_->getRTPSParticipant()->getAttributes();
            fastdds::rtps::ExternalLocatorsProcessor::filter_remote_locators(temp_participant_data_,
                    pattr.builtin.metatraffic_external_unicast_locators, pattr.default_external_unicast_locators,
                    pattr.ignore_non_matching_locators);

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

            if (pdata == nullptr)
            {
                // Create a new one when not found

                reader->getMutex().unlock();
                lock.unlock();

                //! notify security manager in order to start handshake
                bool ret = parent_pdp_->getRTPSParticipant()->security_manager().discovered_participant(
                    temp_participant_data_);

                //! Reply to the remote participant
                if (ret)
                {
                    response_cb_(temp_participant_data_);
                }

                // Take again the reader lock
                reader->getMutex().lock();

            } //! Do nothing if already discovered

        }

    } //! Do nothing on participant removal

    //Remove change form history.
    parent_pdp_->builtin_endpoints_->remove_from_pdp_reader_history(change);
}

bool PDPSecurityInitiatorListener::get_key(
        CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, fastdds::dds::PID_PARTICIPANT_GUID);
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
