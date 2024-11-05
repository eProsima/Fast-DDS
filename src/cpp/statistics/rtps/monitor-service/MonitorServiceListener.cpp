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
#include <statistics/rtps/StatisticsBase.hpp>

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
        const fastdds::rtps::GUID_t& guid,
        const uint32_t& id) const
{
    return monitor_srv_->push_entity_update(guid.entityId, id);
}

bool MonitorServiceListener::on_local_entity_change(
        const fastdds::rtps::GUID_t& guid,
        bool is_alive) const
{
    bool ret = false;

    if (is_alive)
    {
        ret = monitor_srv_->push_entity_update(guid.entityId, StatusKind::PROXY);
    }
    else
    {
        ret = monitor_srv_->remove_local_entity(guid.entityId);
    }

    return ret;
}

bool MonitorServiceListener::on_local_entity_connections_change(
        const fastdds::rtps::GUID_t& guid) const
{
    return monitor_srv_->push_entity_update(guid.entityId, StatusKind::CONNECTION_LIST);
}

void MonitorServiceListener::on_writer_matched(
        fastdds::rtps::RTPSWriter*,
        const fastdds::rtps::MatchingInfo& info)
{
    if (info.status == eprosima::fastdds::rtps::MATCHED_MATCHING)
    {
        EPROSIMA_LOG_INFO(MONITOR_SERVICE, "Status writer matched with " << info.remoteEndpointGuid);
    }
    else if (info.status == eprosima::fastdds::rtps::REMOVED_MATCHING)
    {
        EPROSIMA_LOG_INFO(MONITOR_SERVICE, "Status writer unmatched with " << info.remoteEndpointGuid);
    }
}

void MonitorServiceListener::on_writer_change_received_by_all(
        fastdds::rtps::RTPSWriter* writer,
        fastdds::rtps::CacheChange_t* change)
{
    //! Do nothing for the moment, no relevant info
    static_cast<void>(writer);
    static_cast<void>(change);
}

bool MonitorServiceListener::on_incompatible_qos_matching(
        const fastdds::rtps::GUID_t& local_guid,
        const fastdds::rtps::GUID_t& remote_guid,
        const fastdds::dds::PolicyMask& incompatible_qos_policies) const
{
    bool ret = true;

    // Convert the PolicyMask to a vector of policy ids
    std::vector<uint32_t> incompatible_policies;
    for (uint32_t id = 1; id < dds::NEXT_QOS_POLICY_ID; ++id)
    {
        if (incompatible_qos_policies.test(id))
        {
            incompatible_policies.push_back(id);
        }
    }

    if (monitor_srv_)
    {
        auto& ext_incompatible_qos_collection = monitor_srv_->get_extended_incompatible_qos_collection();
        std::lock_guard<std::mutex> lock(monitor_srv_->get_extended_incompatible_qos_mtx());

        if (!incompatible_policies.empty())
        {
            // Check if the local_guid is already in the collection. If not, create a new entry
            auto local_entity_incompatibilites =
                    ext_incompatible_qos_collection.insert({local_guid, {}});

            bool first_incompatibility_with_remote = false;

            // Local entity already in the collection (has any incompatible QoS with any remote entity)
            if (!local_entity_incompatibilites.second)
            {
                // Check if the local entitiy already had an incompatibility with this remote entity
                auto it = std::find_if(
                    local_entity_incompatibilites.first->second.begin(),
                    local_entity_incompatibilites.first->second.end(),
                    [&remote_guid](const ExtendedIncompatibleQoSStatus_s& status)
                    {
                        return to_fastdds_type(status.remote_guid()) == remote_guid;
                    });

                if (it == local_entity_incompatibilites.first->second.end())
                {
                    // First incompatibility with that remote entity
                    first_incompatibility_with_remote = true;
                }
                else
                {
                    // Already had an incompatibility with that remote entity.
                    // Update them
                    it->current_incompatible_policies(incompatible_policies);
                    monitor_srv_->push_entity_update(local_guid.entityId, StatusKind::EXTENDED_INCOMPATIBLE_QOS);
                }
            }
            else
            {
                // This will be the first incompatibility of this entity
                first_incompatibility_with_remote = true;
            }

            if (first_incompatibility_with_remote)
            {
                ExtendedIncompatibleQoSStatus_s status;
                status.remote_guid(to_statistics_type(remote_guid));
                status.current_incompatible_policies(incompatible_policies);
                local_entity_incompatibilites.first->second.emplace_back(status);
                monitor_srv_->push_entity_update(local_guid.entityId, StatusKind::EXTENDED_INCOMPATIBLE_QOS);
            }
        }
        else
        {
            // Remove remote guid from the local guid incompatibilities collection
            auto it = ext_incompatible_qos_collection.find(local_guid);

            if (it != ext_incompatible_qos_collection.end())
            {
                auto it_remote = std::find_if(
                    it->second.begin(),
                    it->second.end(),
                    [&remote_guid](const ExtendedIncompatibleQoSStatus_s& status)
                    {
                        return to_fastdds_type(status.remote_guid()) == remote_guid;
                    });

                if (it_remote != it->second.end())
                {
                    it->second.erase(it_remote);
                    monitor_srv_->push_entity_update(local_guid.entityId, StatusKind::EXTENDED_INCOMPATIBLE_QOS);
                }
            }
        }
    }

    return ret;
}

bool MonitorServiceListener::on_remote_proxy_data_removed(
        const fastdds::rtps::GUID_t& removed_proxy_guid) const
{
    auto& ext_incompatible_qos_collection = monitor_srv_->get_extended_incompatible_qos_collection();
    std::lock_guard<std::mutex> lock(monitor_srv_->get_extended_incompatible_qos_mtx());

    for (auto& local_entity : ext_incompatible_qos_collection)
    {
        auto it = std::find_if(
            local_entity.second.begin(),
            local_entity.second.end(),
            [&removed_proxy_guid](const ExtendedIncompatibleQoSStatus_s& status)
            {
                return to_fastdds_type(status.remote_guid()) == removed_proxy_guid;
            });

        if (it != local_entity.second.end())
        {
            local_entity.second.erase(it);
            monitor_srv_->push_entity_update(local_entity.first.entityId, StatusKind::EXTENDED_INCOMPATIBLE_QOS);
        }
    }
    return false;
}

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima
