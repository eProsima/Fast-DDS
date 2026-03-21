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

/*!
 * @file NetmaskFilterKind.cpp
 */

#include <ostream>
#include <string>

#include <fastdds/rtps/transport/network/NetmaskFilterKind.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

std::ostream& operator <<(
        std::ostream& output,
        const NetmaskFilterKind& netmask_filter_kind)
{
    switch (netmask_filter_kind)
    {
        case NetmaskFilterKind::OFF:
        {
            output << "OFF";
            break;
        }
        case NetmaskFilterKind::AUTO:
        {
            output << "AUTO";
            break;
        }
        case NetmaskFilterKind::ON:
        {
            output << "ON";
            break;
        }
        default:
        {
            output << "UNKNOWN";
            break;
        }
    }

    return output;
}

} // namsepace rtps
} // namespace fastdds
} // namespace eprosima
