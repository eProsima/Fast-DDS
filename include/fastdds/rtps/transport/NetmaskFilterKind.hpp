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
 * @file NetmaskFilterKind.hpp
 */

#ifndef _FASTDDS_RTPS_TRANSPORT_NETMASKFILTER_HPP_
#define _FASTDDS_RTPS_TRANSPORT_NETMASKFILTER_HPP_

#include <ostream>
#include <string>
#include <unordered_map>

#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

enum RTPS_DllAPI NetmaskFilterKind
{
    OFF,
    AUTO,
    ON
};

inline std::ostream& operator <<(
        std::ostream& output,
        const NetmaskFilterKind& netmask_filter_kind)
{
    std::string netmask_filter_kind_str = "UNKNOWN";
    static const std::unordered_map<NetmaskFilterKind, std::string> conversion_map =
    {
        {NetmaskFilterKind::OFF, "OFF"},
        {NetmaskFilterKind::AUTO, "AUTO"},
        {NetmaskFilterKind::ON, "ON"}
    };

    auto it = conversion_map.find(netmask_filter_kind);
    if (it != conversion_map.end())
    {
        netmask_filter_kind_str = it->second;
    }

    return output << netmask_filter_kind_str;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_RTPS_TRANSPORT_NETMASKFILTER_HPP_
