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
 * @file DiscoveryParticipantsAckStatus.hpp
 *
 */

#ifndef _FASTDDS_RTPS_DISCOVERY_PARTICIPANT_ACK_STATUS_H_
#define _FASTDDS_RTPS_DISCOVERY_PARTICIPANT_ACK_STATUS_H_

#include <map>
#include <vector>

#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include <json.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

/**
 * Class to manage the relevant_participants_builtin_ack_status structure from DiscoveryDataBase
 *@ingroup DISCOVERY_MODULE
 */
class DiscoveryParticipantsAckStatus
{

public:

    DiscoveryParticipantsAckStatus()
    {
    }

    ~DiscoveryParticipantsAckStatus()
    {
    }

    void add_or_update_participant(
            const eprosima::fastrtps::rtps::GuidPrefix_t& guid_p,
            bool status);

    void remove_participant(
            const eprosima::fastrtps::rtps::GuidPrefix_t& guid_p);

    void unmatch_all();

    bool is_matched(
            const eprosima::fastrtps::rtps::GuidPrefix_t& guid_p) const;

    bool is_relevant_participant(
            const eprosima::fastrtps::rtps::GuidPrefix_t& guid_p) const;

    bool is_acked_by_all() const;

    std::vector<eprosima::fastrtps::rtps::GuidPrefix_t> relevant_participants() const;

    void to_json(
            nlohmann::json& j) const;

private:

    std::map<eprosima::fastrtps::rtps::GuidPrefix_t, bool> relevant_participants_map_;
};

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_DISCOVERY_PARTICIPANT_ACK_STATUS_H_ */
