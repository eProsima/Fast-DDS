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

#include <nlohmann/json.hpp>

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

    DiscoveryParticipantsAckStatus() = default;

    ~DiscoveryParticipantsAckStatus() = default;

    enum class ParticipantState : uint8_t
    {
        PENDING_SEND,  // Data(p) has not been sent yet
        WAITING_ACK,   // Data(p) has already been sent but ACK has not been received
        ACKED          // Data(p) has been acked
    };

    void add_or_update_participant(
            const GuidPrefix_t& guid_p,
            ParticipantState status);

    void remove_participant(
            const GuidPrefix_t& guid_p);

    void unmatch_all();

    bool is_waiting_ack(
            const GuidPrefix_t& guid_p) const;

    bool is_matched(
            const GuidPrefix_t& guid_p) const;

    bool is_relevant_participant(
            const GuidPrefix_t& guid_p) const;

    bool is_acked_by_all() const;

    std::vector<GuidPrefix_t> relevant_participants() const;

    void to_json(
            nlohmann::json& j) const;

private:

    std::map<GuidPrefix_t, ParticipantState> relevant_participants_map_;
};

inline std::ostream& operator <<(
        std::ostream& os,
        DiscoveryParticipantsAckStatus::ParticipantState child)
{
    switch (child)
    {
        case DiscoveryParticipantsAckStatus::ParticipantState::PENDING_SEND:
            os << "PENDING_SEND";
            break;
        case DiscoveryParticipantsAckStatus::ParticipantState::WAITING_ACK:
            os << "WAITING_ACK";
            break;
        case DiscoveryParticipantsAckStatus::ParticipantState::ACKED:
            os << "ACKED";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_DISCOVERY_PARTICIPANT_ACK_STATUS_H_ */
