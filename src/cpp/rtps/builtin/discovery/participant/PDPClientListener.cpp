// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PDPClientListener.cpp
 *
 */

 #include <rtps/builtin/discovery/participant/PDPClientListener.hpp>

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
 #include <rtps/builtin/discovery/participant/PDPClient.h>
 #include <rtps/builtin/discovery/participant/PDPEndpoints.hpp>
 #include <rtps/network/utils/external_locators.hpp>
 #include <rtps/participant/RTPSParticipantImpl.h>

 #include <mutex>

 #ifdef FASTDDS_STATISTICS
 #include <statistics/rtps/monitor-service/interfaces/IConnectionsObserver.hpp>
 #endif //FASTDDS_STATISTICS

 using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace eprosima::fastrtps::rtps;

PDPClientListener::PDPClientListener(
        PDPClient* in_PDP)
    : PDPListener(in_PDP)
{
}

PDPClient* PDPClientListener::pdp_client()
{
    return static_cast<PDPClient*>(parent_pdp_);
}

void PDPClientListener::onNewCacheChangeAdded(
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
        if (temp_participant_data_.readFromCDRMessage(&msg, true, parent_pdp_->getRTPSParticipant()->network_factory(),
                parent_pdp_->getRTPSParticipant()->has_shm_transport(), true, change_in->vendor_id))
        {
            // After correctly reading it
            change->instanceHandle = temp_participant_data_.m_key;
            guid = temp_participant_data_.m_guid;

            if (parent_pdp_->getRTPSParticipant()->is_participant_ignored(guid.guidPrefix))
            {
                return;
            }

            // Whether to drop the participant (if it is a server) due to persistence GUID mismatch
            bool should_be_dropped = false;

            // Whether to skip data Ps according to sequence number
            bool no_skip = false;

            fastrtps::rtps::GUID_t persistence_guid = fastrtps::rtps::GUID_t::unknown();
            fastrtps::ParameterPropertyList_t properties = temp_participant_data_.m_properties;
            auto _persistence_guid = std::find_if(
                properties.begin(),
                properties.end(),
                [](const dds::ParameterProperty_t& property)
                {
                    return property.first() == dds::parameter_property_persistence_guid;
                });

            if (_persistence_guid != properties.end())
            {
                std::istringstream(_persistence_guid->second()) >> persistence_guid;
                if (persistence_guid == fastrtps::rtps::GUID_t::unknown())
                {
                    // Unexpected: persistence GUID set but unknown
                }
                else
                {
                    auto pguid = persistence_guid_map.find(guid.guidPrefix);
                    if (pguid != persistence_guid_map.end())
                    {
                        if (pguid->second != persistence_guid.guidPrefix)
                        {
                            // Persistence guid mismatch -> Remove persistence GUID from map and drop participant
                            persistence_guid_map.erase(pguid);
                            should_be_dropped = true;

                            if (guid.guidPrefix == writer_guid.guidPrefix)
                            {
                                // Remove also sequence number map for this GUID
                                auto last_sn = sn_db.find(writer_guid.guidPrefix);
                                if (last_sn != sn_db.end())
                                {
                                    sn_db.erase(last_sn);
                                }
                            }
                        }
                    }
                    else
                    {
                        // First data P from unknown server -> add entry and avoid skipping based on SN
                        persistence_guid_map[guid.guidPrefix] = persistence_guid.guidPrefix;
                        no_skip = true;

                        // Force first SN in case the present server was dropped and SN resetted
                        sn_db[writer_guid.guidPrefix] = change->sequenceNumber;
                    }
                }
            }
            else
            {
                // Client case -> skip according to SN
            }

            if (should_be_dropped)
            {
                std::unique_lock<std::recursive_mutex> lock(*pdp_client()->getMutex());

                for (ParticipantProxyData* it : pdp_client()->participant_proxies_)
                {
                    if (guid == it->m_guid)
                    {
                        pdp_client()->remove_remote_participant(guid, ParticipantDiscoveryInfo::DROPPED_PARTICIPANT);
                        parent_pdp_->builtin_endpoints_->remove_from_pdp_reader_history(change);
                        return;
                    }
                }
            }

            if (!no_skip)
            {
                auto last_sn = sn_db.find(writer_guid.guidPrefix);
                if (last_sn != sn_db.end())
                {
                    if (change->sequenceNumber <= last_sn->second)
                    {
                        // Dropping already processed SN
                        parent_pdp_->builtin_endpoints_->remove_from_pdp_reader_history(change);
                        return;
                    }
                    else
                    {
                        // Newer SN -> update
                        sn_db[writer_guid.guidPrefix] = change->sequenceNumber;
                    }
                }
                else
                {
                    // First SN -> add
                    sn_db[writer_guid.guidPrefix] = change->sequenceNumber;
                }
            }

            // Filter locators
            const auto& pattr = parent_pdp_->getRTPSParticipant()->getAttributes();
            fastdds::rtps::network::external_locators::filter_remote_locators(temp_participant_data_,
                    pattr.builtin.metatraffic_external_unicast_locators, pattr.default_external_unicast_locators,
                    pattr.ignore_non_matching_locators);

            // Check if participant already exists (updated info)
            ParticipantProxyData* pdata = nullptr;
            bool already_processed = false;
            for (ParticipantProxyData* it : parent_pdp_->participant_proxies_)
            {
                if (guid == it->m_guid)
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
        if (parent_pdp_->remove_remote_participant(guid, ParticipantDiscoveryInfo::REMOVED_PARTICIPANT))
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

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
