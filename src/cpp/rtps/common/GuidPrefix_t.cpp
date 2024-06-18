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

#include <rtps/common/GuidUtils.hpp>

#include <fastdds/rtps/common/GuidPrefix_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

bool GuidPrefix_t::is_on_same_host_as(
        const GuidPrefix_t& other_guid_prefix) const
{
    return memcmp(value, other_guid_prefix.value, 4) == 0;
}

bool GuidPrefix_t::is_from_this_host() const
{
    return is_on_same_host_as(GuidUtils::instance().prefix());
}

bool GuidPrefix_t::is_on_same_process_as(
        const GuidPrefix_t& other_guid_prefix) const
{
    return memcmp(value, other_guid_prefix.value, 8) == 0;
}

bool GuidPrefix_t::is_from_this_process() const
{
    return is_on_same_process_as(GuidUtils::instance().prefix());
}

} // namsepace rtps
} // namespace fastdds
} // namespace eprosima
