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
 * @file MonitorServiceListener.cpp
 */

#include <statistics/rtps/monitor-service/MonitorServiceListener.hpp>

#include <statistics/rtps/monitor-service/MonitorService.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

MonitorServiceListener::MonitorServiceListener(
        MonitorService* ms)
    : monitor_srv_(ms)
{

}

bool MonitorServiceListener::on_local_entity_status_change(
        const fastrtps::rtps::GUID_t& guid,
        const uint32_t& id) const
{
    return monitor_srv_->push_entity_update(guid.entityId, id);
}

bool MonitorServiceListener::on_local_entity_change(
        const fastrtps::rtps::GUID_t& guid,
        bool is_alive) const
{
    bool ret = false;

    if (is_alive)
    {
        ret = monitor_srv_->push_entity_update(guid.entityId, PROXY);
    }
    else
    {
        ret = monitor_srv_->remove_local_entity(guid.entityId);
    }

    return ret;
}

bool MonitorServiceListener::on_local_entity_connections_change(
        const fastrtps::rtps::GUID_t& guid) const
{
    return monitor_srv_->push_entity_update(guid.entityId, CONNECTION_LIST);
}

void MonitorServiceListener::onWriterMatched(
        fastrtps::rtps::RTPSWriter*,
        fastrtps::rtps::MatchingInfo& info)
{
    if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
    {
        EPROSIMA_LOG_INFO(MONITOR_SERVICE, "Status writer matched with " << info.remoteEndpointGuid);
    }
    else if (info.status == eprosima::fastrtps::rtps::REMOVED_MATCHING)
    {
        EPROSIMA_LOG_INFO(MONITOR_SERVICE, "Status writer unmatched with " << info.remoteEndpointGuid);
    }
}

void MonitorServiceListener::onWriterChangeReceivedByAll(
        fastrtps::rtps::RTPSWriter* writer,
        fastrtps::rtps::CacheChange_t* change)
{
    //! Do nothing for the moment, no relevant info
    static_cast<void>(writer);
    static_cast<void>(change);
}

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima
