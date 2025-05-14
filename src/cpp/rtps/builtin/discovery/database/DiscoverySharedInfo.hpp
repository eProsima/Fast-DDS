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
 * @file DiscoverySharedInfo.hpp
 *
 */

#ifndef _FASTDDS_RTPS_DISCOVERY_SHARED_INFO_H_
#define _FASTDDS_RTPS_DISCOVERY_SHARED_INFO_H_

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/dds/log/Log.hpp>

#include <rtps/builtin/discovery/database/DiscoveryParticipantsAckStatus.hpp>

#include <nlohmann/json.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

/**
 * Class to join the shared info from Participant and Endpoint
 *@ingroup DISCOVERY_MODULE
 */
class DiscoverySharedInfo
{

public:

    DiscoverySharedInfo(
            CacheChange_t* change,
            const GuidPrefix_t& known_participant);

    ~DiscoverySharedInfo() = default;

    virtual CacheChange_t* update_and_unmatch(
            CacheChange_t* change);

    virtual CacheChange_t* update(
            CacheChange_t* change);

    void add_or_update_ack_participant(
            const GuidPrefix_t& guid_p,
            DiscoveryParticipantsAckStatus::ParticipantState status = DiscoveryParticipantsAckStatus::ParticipantState::PENDING_SEND)
    {
        EPROSIMA_LOG_INFO(
            DISCOVERY_DATABASE,
            "Adding relevant participant " << guid_p
                                           << " with status " << status
                                           << " to " << fastdds::rtps::iHandle2GUID(change_->instanceHandle));
        relevant_participants_builtin_ack_status_.add_or_update_participant(guid_p, status);
    }

    void remove_participant(
            const GuidPrefix_t& guid_p)
    {
        relevant_participants_builtin_ack_status_.remove_participant(guid_p);
    }

    bool is_waiting_ack(
            const GuidPrefix_t& guid_p) const
    {
        return relevant_participants_builtin_ack_status_.is_waiting_ack(guid_p);
    }

    bool is_matched(
            const GuidPrefix_t& guid_p) const
    {
        return relevant_participants_builtin_ack_status_.is_matched(guid_p);
    }

    bool is_relevant_participant(
            const GuidPrefix_t& guid_p) const
    {
        return relevant_participants_builtin_ack_status_.is_relevant_participant(guid_p);
    }

    CacheChange_t* change() const
    {
        return change_;
    }

    std::vector<GuidPrefix_t> relevant_participants() const
    {
        return relevant_participants_builtin_ack_status_.relevant_participants();
    }

    bool is_acked_by_all() const
    {
        return relevant_participants_builtin_ack_status_.is_acked_by_all();
    }

    virtual void to_json(
            nlohmann::json& j) const;

protected:

    CacheChange_t* change_;

    // new class is used in order to could change it in the future for a more efficient implementation
    ddb::DiscoveryParticipantsAckStatus
            relevant_participants_builtin_ack_status_;

};

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_DISCOVERY_SHARED_INFO_H_ */
