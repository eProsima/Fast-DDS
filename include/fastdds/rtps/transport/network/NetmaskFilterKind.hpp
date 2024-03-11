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

#ifndef _FASTDDS_RTPS_TRANSPORT_NETWORK_NETMASKFILTER_HPP_
#define _FASTDDS_RTPS_TRANSPORT_NETWORK_NETMASKFILTER_HPP_

#include <ostream>

#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

enum class NetmaskFilterKind
{
    OFF,
    AUTO,
    ON
};

RTPS_DllAPI std::ostream& operator <<(
        std::ostream& output,
        const NetmaskFilterKind& netmask_filter_kind);

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_RTPS_TRANSPORT_NETWORK_NETMASKFILTER_HPP_
