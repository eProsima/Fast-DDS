
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
 * @file DiscoveryEndpointInfo.cpp
 *
 */

#include <vector>

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include "./DiscoveryEndpointInfo.hpp"
#include "../json_dump/SharedDumpFunctions.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

nlohmann::json DiscoveryEndpointInfo::json_dump() const
{
    nlohmann::json j = DiscoverySharedInfo::json_dump();
    
    j["topic"] = topic_;

    return j;
}

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
