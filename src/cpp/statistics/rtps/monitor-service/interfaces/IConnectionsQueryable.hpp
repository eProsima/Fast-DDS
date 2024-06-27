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
 *
 */

#ifndef _STATISTICS_MONITOR_SERVICE_INTERFACES_ICONNECTIONSQUERYABLE_HPP_
#define _STATISTICS_MONITOR_SERVICE_INTERFACES_ICONNECTIONSQUERYABLE_HPP_

#include <vector>

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/statistics/rtps/monitor_service/connections_fwd.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct IConnectionsQueryable
{
    /**
     * @brief Interface for retrieving a list of Connections
     * (announced locators, used_locators and mode)
     * of a given entity, by guid
     *
     * @param [in] guid The GUID_t identifying the target entity
     * @param [out] conns_list The output connection list
     * @return Whether the list of connnections could be retrieved
     */
    virtual bool get_entity_connections(
            const fastdds::rtps::GUID_t& guid,
            ConnectionList& conns_list) = 0;
};

} // rtps
} // statistics
} // fastdds
} // eprosima

#endif // _STATISTICS_MONITOR_SERVICE_INTERFACES_ICONNECTIONSQUERYABLE_HPP_
