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
 * @file IProxyObserver.hpp
 *
 */

#ifndef _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_IPROXYOBSERVER_HPP_
#define _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_IPROXYOBSERVER_HPP_

#include <fastdds/rtps/common/Guid.h>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct IProxyObserver
{
    /**
     * @brief Interface use to notify about any updates
     * on the local entities (updates in the proxy,
     * new matches, unpairs,...)
     *
     * @param guid The GUID_t identifying the target entity
     * @param is_alive Whether this entity is alive
     * @return Whether the implementor has been properly notified
     */
    virtual bool on_local_entity_change(
            const fastrtps::rtps::GUID_t& guid,
            bool is_alive) const = 0;
};

} // rtps
} // statistics
} // fastdds
} // eprosima

#endif // _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_IPROXYOBSERVER_HPP_

