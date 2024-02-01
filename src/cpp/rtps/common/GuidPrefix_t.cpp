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
 * @file GuidPrefix_t.cpp
 */

#include <fastdds/rtps/common/Types.h>
#include <utils/SystemInfo.hpp>

#include <fastdds/rtps/common/GuidPrefix_t.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

bool GuidPrefix_t::is_from_this_host() const
{
    uint16_t host_id = SystemInfo::instance().host_id();
    return (value[2] == static_cast<octet>(host_id & 0xFF) &&
           (value[3]) == static_cast<octet>((host_id >> 8) & 0xFF));
}

} // namsepace rtps
} // namespace fastrtps
} // namespace eprosima
