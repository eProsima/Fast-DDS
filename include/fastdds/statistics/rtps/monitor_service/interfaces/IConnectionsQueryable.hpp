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

#ifndef _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ICONNECTIONSQUERYABLE_HPP_
#define _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ICONNECTIONSQUERYABLE_HPP_

#include <vector>

#include <fastdds/rtps/common/Guid.h>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

using namespace eprosima::fastdds::statistics;

class Connection;

struct IConnectionsQueryable
{
    using ConnectionList = std::vector<Connection>;

    /**
    * @brief Interface for retrieving a list of Connections
    * (announced locators, used_locators and mode)
    * of a given entity, by guid
    *
    * @param guid The GUID_t identifying the target entity
    * @return ConnectionList The Connections collection
    */
    virtual ConnectionList get_entity_connections(const fastrtps::rtps::GUID_t& guid) = 0;
};

} // rtps
} // statistics
} // fastdds
} // eprosima

#endif // _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ICONNECTIONSQUERYABLE_HPP_

