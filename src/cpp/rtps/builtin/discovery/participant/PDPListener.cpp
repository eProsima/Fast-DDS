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

#include <rtps/builtin/discovery/participant/PDPListener.h>

#include <mutex>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>
#include <fastdds/rtps/participant/RTPSParticipantListener.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/discovery/endpoint/EDP.h>
#include <rtps/builtin/discovery/participant/PDP.h>
#include <rtps/builtin/discovery/participant/PDPEndpoints.hpp>
#include <rtps/network/utils/external_locators.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/resources/TimedEvent.h>
#ifdef FASTDDS_STATISTICS
#include <statistics/rtps/monitor-service/interfaces/IConnectionsObserver.hpp>
#endif //FASTDDS_STATISTICS

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastdds {
namespace rtps {

PDPListener::PDPListener(
        PDP* parent)
    : parent_pdp_(parent)
    , temp_participant_data_(parent->getRTPSParticipant()->get_attributes().allocation)
{
}

void PDPListener::on_new_cache_change_added(
        RTPSReader* reader,
        const CacheChange_t* const change_in)
{
    CacheChange_t* change = const_cast<CacheChange_t*>(change_in);
    GUID_t writer_guid = change->writerGUID;
    EPROSIMA_LOG_INFO(RTPS_PDP, "SPDP Message received from: " << writer_guid);

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
        if (temp_participant_data_.read_from_cdr_message(&msg, true,
                parent_pdp_->getRTPSParticipant()->network_factory(),
                true, change_in->vendor_id))
        {
            // After correctly reading it
            change->instanceHandle = temp_participant_data_.m_key;
            guid = temp_participant_data_.guid;

            if (parent_pdp_->getRTPSParticipant()->is_participant_ignored(guid.guidPrefix))
            {
                return;
            }

            if (!check_discovery_conditions(temp_participant_data_))
            {
                return;
            }

            // Filter locators
            const auto& pattr = parent_pdp_->getRTPSParticipant()->get_attributes();
            fastdds::rtps::network::external_locators::filter_remote_locators(temp_participant_data_,
                    pattr.builtin.metatraffic_external_unicast_locators, pattr.default_external_unicast_locators,
                    pattr.ignore_non_matching_locators);

            // Check if participant already exists (updated info)
            ParticipantProxyData* pdata = nullptr;
            bool already_processed = false;
            for (ParticipantProxyData* it : parent_pdp_->participant_proxies_)
            {
                if (guid == it->guid)
                {
                    pdata = it;

                    // This means this is the same DATA(p) that we have already processed.
                    // We do not compare sample_identity directly because it is not properly filled
                    // in the change during desearialization.
                    if (it->m_sample_identity.writer_guid() == change->writerGUID &&
                            it->m_sample_identity.sequence_number() == change->sequenceNumber)
                    {
                        already_processed = true;
                    }

                    break;
                }
            }

            // Only process the DATA(p) if it is not a repeated one
            if (!already_processed)
            {
                temp_participant_data_.m_sample_identity.writer_guid(change->writerGUID);
                temp_participant_data_.m_sample_identity.sequence_number(change->sequenceNumber);
                process_alive_data(pdata, temp_participant_data_, writer_guid, reader, lock);
            }
        }
    }
    else if (reader->matched_writer_is_matched(writer_guid))
    {
        reader->getMutex().unlock();
        if (parent_pdp_->remove_remote_participant(guid, ParticipantDiscoveryStatus::REMOVED_PARTICIPANT))
        {
#ifdef FASTDDS_STATISTICS
            //! Removal of a participant proxy should trigger
            //! a connections update on the local participant connection list
            if (nullptr != parent_pdp_->getRTPSParticipant()->get_connections_observer())
            {
                parent_pdp_->getRTPSParticipant()->get_connections_observer()->on_local_entity_connections_change(
                    parent_pdp_->getRTPSParticipant()->getGuid());
            }
#endif //FASTDDS_STATISTICS
            reader->getMutex().lock();
            // All changes related with this participant have been removed from history by remove_remote_participant
            return;
        }
        reader->getMutex().lock();
    }

    //Remove change form history.
    parent_pdp_->builtin_endpoints_->remove_from_pdp_reader_history(change);
}

void PDPListener::process_alive_data(
        ParticipantProxyData* old_data,
        ParticipantProxyData& new_data,
        GUID_t& writer_guid,
        RTPSReader* reader,
        std::unique_lock<std::recursive_mutex>& lock)
{
    GUID_t participant_guid = new_data.guid;

    if (old_data == nullptr)
    {
        // Create a new one when not found
        old_data = parent_pdp_->createParticipantProxyData(new_data, writer_guid);

        if (old_data != nullptr)
        {
            // Copy proxy to be passed forward before releasing PDP mutex
            ParticipantProxyData old_data_copy(*old_data);

            reader->getMutex().unlock();
            lock.unlock();

            // Assigning remote endpoints implies sending a DATA(p) to all matched and fixed readers, since
            // StatelessWriter::matched_reader_add_edp marks the entire history as unsent if the added reader's
            // durability is bigger or equal to TRANSIENT_LOCAL_DURABILITY_QOS (TRANSIENT_LOCAL or TRANSIENT),
            // which is the case of ENTITYID_BUILTIN_SDP_PARTICIPANT_READER (TRANSIENT_LOCAL). If a remote
            // participant is discovered before creating the first DATA(p) change (which happens at the end of
            // BuiltinProtocols::initBuiltinProtocols), then StatelessWriter::matched_reader_add_edp ends up marking
            // no changes as unsent (since the history is empty), which is OK because this can only happen if a
            // participant is discovered in the middle of BuiltinProtocols::initBuiltinProtocols, which will
            // create the first DATA(p) upon finishing, thus triggering the sent to all fixed and matched
            // readers anyways.
            parent_pdp_->assignRemoteEndpoints(&old_data_copy);
        }
        else
        {
            reader->getMutex().unlock();
            lock.unlock();
        }
    }
    else
    {
        old_data->update_data(new_data);
        old_data->is_alive = true;

        reader->getMutex().unlock();

        EPROSIMA_LOG_INFO(RTPS_PDP_DISCOVERY, "Update participant "
                << old_data->guid << " at "
                << "MTTLoc: " << old_data->metatraffic_locators
                << " DefLoc:" << old_data->default_locators);

        if (parent_pdp_->updateInfoMatchesEDP())
        {
            parent_pdp_->mp_EDP->assignRemoteEndpoints(*old_data, true);
        }

        // Copy proxy to be passed forward before releasing PDP mutex
        ParticipantBuiltinTopicData old_proxy_data_copy(*old_data);

        lock.unlock();

        RTPSParticipantListener* listener = parent_pdp_->getRTPSParticipant()->getListener();
        if (listener != nullptr)
        {
            bool should_be_ignored = false;

            {
                std::lock_guard<std::mutex> cb_lock(parent_pdp_->callback_mtx_);

                listener->on_participant_discovery(
                    parent_pdp_->getRTPSParticipant()->getUserRTPSParticipant(),
                    ParticipantDiscoveryStatus::CHANGED_QOS_PARTICIPANT,
                    old_proxy_data_copy,
                    should_be_ignored);
            }
            if (should_be_ignored)
            {
                parent_pdp_->getRTPSParticipant()->ignore_participant(participant_guid.guidPrefix);
            }
        }
    }

#ifdef FASTDDS_STATISTICS
    //! Addition or update of a participant proxy should trigger
    //! a connections update on the local participant connection list
    if (nullptr != parent_pdp_->getRTPSParticipant()->get_connections_observer())
    {
        parent_pdp_->getRTPSParticipant()->get_connections_observer()->on_local_entity_connections_change(
            parent_pdp_->getRTPSParticipant()->getGuid());
    }
#endif //FASTDDS_STATISTICS

    // Take again the reader lock
    reader->getMutex().lock();
}

bool PDPListener::check_discovery_conditions(
        ParticipantProxyData& participant_data)
{
    bool ret = true;
    uint32_t remote_participant_domain_id = participant_data.domain_id;

    // In PDPSimple, do not match if the participant is from a different domain.
    // If the domain id is unknown, it is assumed to be the same domain
    if (remote_participant_domain_id != parent_pdp_->getRTPSParticipant()->get_domain_id() &&
            remote_participant_domain_id != fastdds::dds::DOMAIN_ID_UNKNOWN)
    {
        EPROSIMA_LOG_INFO(RTPS_PDP_DISCOVERY, "Received participant with different domain id ("
                << remote_participant_domain_id << ") than ours ("
                << parent_pdp_->getRTPSParticipant()->get_domain_id() << ")");
        ret = false;
    }

    return ret;
}

bool PDPListener::get_key(
        CacheChange_t* change)
{
    return ParameterList::readInstanceHandleFromCDRMsg(change, fastdds::dds::PID_PARTICIPANT_GUID);
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
