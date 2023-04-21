// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file IProxyQueryable.hpp
 */

#ifndef RTPS_MONITOR_IPROXYQUERYABLE_HPP
#define RTPS_MONITOR_IPROXYQUERYABLE_HPP

#include <vector>

#include <fastdds/rtps/common/CDRMessage_t.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

struct GUID_t;

class IProxyQueryable
{
public:

    IProxyQueryable() = default;

    virtual ~IProxyQueryable() = default;

    /**
     *
     * @param guid
     *
     * @return true if the entity status changed.
     */
    virtual bool get_serialized_proxy(
            const GUID_t& guid,
            CDRMessage_t* message) = 0;

    /**
     *
     * @param guid
     *
     * @return true if the entity status changed.
     */
    virtual bool get_all_local_proxies(
            std::vector<GUID_t>& proxies_map) = 0;


};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_MONITOR_IPROXYQUERYABLE_HPP
