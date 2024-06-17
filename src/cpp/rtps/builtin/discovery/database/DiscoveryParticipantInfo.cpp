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
 * @file DiscoveryParticipantInfo.cpp
 *
 */

#include <vector>

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include <rtps/builtin/discovery/database/DiscoveryParticipantInfo.hpp>

#include <nlohmann/json.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

CacheChange_t* DiscoveryParticipantInfo::update(
        CacheChange_t* change,
        DiscoveryParticipantChangeData participant_change_data)
{
    participant_change_data_ = participant_change_data;
    return update(change);
}

CacheChange_t* DiscoveryParticipantInfo::update_and_unmatch(
        CacheChange_t* change,
        DiscoveryParticipantChangeData participant_change_data)
{
    participant_change_data_ = participant_change_data;
    return update_and_unmatch(change);
}

void DiscoveryParticipantInfo::add_reader(
        const GUID_t& guid)
{
    if (std::find(readers_.begin(), readers_.end(), guid) == readers_.end())
    {
        readers_.push_back(guid);
    }
}

void DiscoveryParticipantInfo::remove_reader(
        const GUID_t& guid)
{
    // erase it from the back to accelerate participant removal
    auto rit = std::find(readers_.rbegin(), readers_.rend(), guid);
    if (rit != readers_.rend())
    {
        // prev because the inverse iterator
        readers_.erase(std::prev(rit.base()));
    }
}

void DiscoveryParticipantInfo::add_writer(
        const GUID_t& guid)
{
    if (std::find(writers_.begin(), writers_.end(), guid) == writers_.end())
    {
        writers_.push_back(guid);
    }
}

void DiscoveryParticipantInfo::remove_writer(
        const GUID_t& guid)
{
    // erase it from the back to accelerate participant removal
    auto rit = std::find(writers_.rbegin(), writers_.rend(), guid);
    if (rit != writers_.rend())
    {
        // prev because the inverse iterator
        writers_.erase(std::prev(rit.base()));
    }
}

void DiscoveryParticipantInfo::to_json(
        nlohmann::json& j) const
{
    DiscoverySharedInfo::to_json(j);
    participant_change_data_.to_json(j);
}

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
