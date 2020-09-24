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

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include "./DiscoveryParticipantInfo.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

void DiscoveryParticipantInfo::add_reader(
        const eprosima::fastrtps::rtps::GUID_t& guid)
{
    if (std::find(readers.begin(), readers.end(), guid) == readers.end())
    {
        readers.push_back(guid);
    }
}

void DiscoveryParticipantInfo::remove_reader(
        const eprosima::fastrtps::rtps::GUID_t& guid)
{
    auto it = std::find(readers.begin(), readers.end(), guid);
    if (it != readers.end())
    {
        readers.erase(it);
    }
}

void DiscoveryParticipantInfo::add_writer(
        const eprosima::fastrtps::rtps::GUID_t& guid)
{
    if (std::find(writers.begin(), writers.end(), guid) == writers.end())
    {
        writers.push_back(guid);
    }
}

void DiscoveryParticipantInfo::remove_writer(
        const eprosima::fastrtps::rtps::GUID_t& guid)
{
    auto it = std::find(writers.begin(), writers.end(), guid);
    if (it != writers.end())
    {
        writers.erase(it);
    }
}

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */