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
 * @file DiscoveryParticipantsAckStatus.cpp
 *
 */

#include <map>

#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include "./DiscoveryParticipantsAckStatus.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

bool DiscoveryParticipantsAckStatus::is_matched(
        const eprosima::fastrtps::rtps::GuidPrefix_t& guid_p) const
{
    auto it = relevant_participants_map_.find(guid_p);
    if (it != relevant_participants_map_.end())
    {
        return it->second;
    }
    return false;
}

void DiscoveryParticipantsAckStatus::match_all()
{
    for (auto it = relevant_participants_map_.begin(); it != relevant_participants_map_.end(); ++it)
    {
        it->second = true;
    }
}

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
