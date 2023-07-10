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
 * @file MonitorService.cpp
 */

#include <statistics/rtps/monitor-service/MonitorService.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

MonitorService::MonitorService(
        const fastrtps::rtps::GUID_t& guid,
        IProxyQueryable* proxy_q,
        IConnectionsQueryable* conns_q,
        const IStatusQueryable& status_q)
    : local_participant_guid_(guid)
    , proxy_queryable_(proxy_q)
    , conns_queryable_(conns_q)
    , status_queryeble_(status_q)
{

}

MonitorService::~MonitorService()
{
    //!TODO
}

bool MonitorService::enable_monitor_service()
{
    //!TODO
    return false;
}

bool MonitorService::disable_monitor_service()
{
    //!TODO
    return false;
}

bool MonitorService::remove_local_entity(
        const fastrtps::rtps::EntityId_t& entity_id)
{
    //!TODO
    static_cast<void>(entity_id);
    return false;
}

bool MonitorService::push_entity_update(
        const fastrtps::rtps::EntityId_t& entity_id,
        const uint32_t& status_id)
{
    //!TODO
    static_cast<void>(entity_id);
    static_cast<void>(status_id);
    return false;
}

bool MonitorService::write_status(
        const fastrtps::rtps::EntityId_t& entity_id)
{
    //!TODO
    static_cast<void>(entity_id);
    return false;
}

bool MonitorService::create_endpoint()
{
    //!TODO
    return false;
}

bool MonitorService::spin_queue()
{
    //!TODO
    return false;
}

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima