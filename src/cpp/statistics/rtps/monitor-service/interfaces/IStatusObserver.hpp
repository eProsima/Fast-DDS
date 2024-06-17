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
 * @file IStatusObserver.hpp
 *
 */

#ifndef _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ISTATUSOBSERVER_HPP_
#define _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ISTATUSOBSERVER_HPP_

#include <fastdds/rtps/common/Guid.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct IStatusObserver
{
    /**
     * @brief Interface used to notify about any change in the statuses
     * (IncompatibleQoS, InconsistenTopic,...) of a local entity
     *
     * @param guid The GUID_t identifying the target entity
     * @param id The id of the status changing
     * @return Whether the implementor has been properly notified
     */
    virtual bool on_local_entity_status_change(
            const fastdds::rtps::GUID_t& guid,
            const uint32_t& id) const = 0;
};

} // rtps
} // statistics
} // fastdds
} // eprosima

#endif // _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_ISTATUSOBSERVER_HPP_

