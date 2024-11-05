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

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/common/Guid.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct IProxyObserver
{
    /**
     * @brief Interface used to notify about any updates
     * on the local entities (updates in the proxy,
     * new matches, unpairs,...)
     *
     * @param guid The GUID_t identifying the target entity
     * @param is_alive Whether this entity is alive
     * @return Whether the implementor has been properly notified
     */
    virtual bool on_local_entity_change(
            const fastdds::rtps::GUID_t& guid,
            bool is_alive) const = 0;

    /**
     * @brief Interface used to notify about any updates
     * regarding remote entities incompatible QoS matching.
     *
     * @param local_guid The GUID_t identifying the local entity
     * @param remote_guid The GUID_t identifying the remote entity
     * @param incompatible_qos The PolicyMask with the incompatible QoS
     *
     */
    virtual void on_incompatible_qos_matching(
            const fastdds::rtps::GUID_t& local_guid,
            const fastdds::rtps::GUID_t& remote_guid,
            const fastdds::dds::PolicyMask& incompatible_qos) const = 0;

    /**
     * @brief Method to notify the implementor that a remote proxy
     * data has been removed. This is interesting to notify proxy removals
     * independently of the the remote entity being matched or not.
     *
     * @param removed_proxy_guid GUID of the removed proxy.
     */
    virtual void on_remote_proxy_data_removed(
            const fastdds::rtps::GUID_t& removed_proxy_guid) const = 0;
};

} // rtps
} // statistics
} // fastdds
} // eprosima

#endif // _FASTDDS_STATISTICS_MONITOR_SERVICE_INTERFACES_IPROXYOBSERVER_HPP_

