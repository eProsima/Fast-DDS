// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file QosPolicyUtils.cpp
 *
 */

#include <fastdds/core/policy/QosPolicyUtils.hpp>

#include <utils/Host.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace utils {

// Compute the default DataSharing domain ID
uint64_t default_domain_id()
{
    uint64_t id = 0;
    Host::uint48 mac_id = Host::instance().mac_id();
    for (size_t i = 0; i < Host::mac_id_length; ++i)
    {
        id |= static_cast<uint64_t>(mac_id.value[i]) << (56 - (i * 8));
    }
    return id;
}

}  // namespace utils
}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

