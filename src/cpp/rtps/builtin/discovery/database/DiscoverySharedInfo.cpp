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
 * @file DiscoverySharedInfo.cpp
 *
 */

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include "./DiscoverySharedInfo.hpp"

#include <json.hpp>
#include "backup/SharedBackupFunctions.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

DiscoverySharedInfo::DiscoverySharedInfo(
        eprosima::fastrtps::rtps::CacheChange_t* change,
        const eprosima::fastrtps::rtps::GuidPrefix_t& known_participant)
    : change_(change)
{
    // the server already knows every message
    add_or_update_ack_participant(known_participant, true);
}

eprosima::fastrtps::rtps::CacheChange_t* DiscoverySharedInfo::update_and_unmatch(
        eprosima::fastrtps::rtps::CacheChange_t* change)
{
    relevant_participants_builtin_ack_status_.unmatch_all();
    return update(change);
}

eprosima::fastrtps::rtps::CacheChange_t* DiscoverySharedInfo::update(
        eprosima::fastrtps::rtps::CacheChange_t* change)
{
    eprosima::fastrtps::rtps::CacheChange_t* old_change = change_;
    change_ = change;
    return old_change;
}

void DiscoverySharedInfo::to_json(
        nlohmann::json& j) const
{
    nlohmann::json j_change;
    nlohmann::json j_ack;

    ddb::to_json(j_change, *change_);
    relevant_participants_builtin_ack_status_.to_json(j_ack);
    j["change"] = j_change;
    j["ack_status"] = j_ack;
}

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
