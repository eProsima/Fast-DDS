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

#ifndef FASTDDS_RTPS_TRANSPORT_NETWORK__NETMASKFILTERKIND_HPP
#define FASTDDS_RTPS_TRANSPORT_NETWORK__NETMASKFILTERKIND_HPP

#include <ostream>

#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

enum class NetmaskFilterKind
{
    OFF,
    AUTO,
    ON
};

FASTDDS_EXPORTED_API std::ostream& operator <<(
        std::ostream& output,
        const NetmaskFilterKind& netmask_filter_kind);

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_TRANSPORT_NETWORK__NETMASKFILTERKIND_HPP
