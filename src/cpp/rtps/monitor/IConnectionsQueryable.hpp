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
 * @file IConnectionsQueryable.hpp
 */

#ifndef RTPS_MONITOR_ICONNECTIONSQUERYABLE_HPP
#define RTPS_MONITOR_ICONNECTIONSQUERYABLE_HPP

#include <vector>

#include <rtps/monitor/Connection.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

struct GUID_t;

typedef std::vector<Connection> ConnectionList;

class IConnectionsQueryable
{
public:

    IConnectionsQueryable() = default;

    virtual ~IConnectionsQueryable() = default;

    /**
     *
     * @param guid
     *
     * @return true if the entity status changed.
     */
    virtual ConnectionList get_entity_connections(
            const GUID_t& guid) = 0;

};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_MONITOR_ICONNECTIONSQUERYABLE_HPP
