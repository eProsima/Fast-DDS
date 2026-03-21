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
 * @file IConnectionsObserver.hpp
 *
 */

#ifndef _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ICONNECTIONSOBSERVER_HPP_
#define _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ICONNECTIONSOBSERVER_HPP_

#include <vector>

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/statistics/rtps/monitor_service/connections_fwd.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct IConnectionsObserver
{
    /**
     * @brief Interface use to notify any connection change
     * in an entity
     *
     * @param guid The GUID_t identifying the target entity
     * @return Whether the implementor has been properly notified
     */
    virtual bool on_local_entity_connections_change(
            const fastdds::rtps::GUID_t& guid) const = 0;
};

} // rtps
} // statistics
} // fastdds
} // eprosima

#endif // _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ICONNECTIONSOBSERVER_HPP_

