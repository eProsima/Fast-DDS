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

#include <fastdds/rtps/builtin/data/TopicDescription.hpp>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/publisher/DataWriterHistory.hpp>
#include <fastdds/statistics/topic_names.hpp>

#include <rtps/history/CacheChangePool.h>
#include <rtps/history/PoolConfig.h>
#include <rtps/history/TopicPayloadPoolRegistry.hpp>
#include <rtps/RTPSDomainImpl.hpp>
#include <statistics/rtps/StatisticsBase.hpp>
#include <utils/TimeConversion.hpp>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

MonitorService::MonitorService(
        const fastdds::rtps::GUID_t& guid,
        IProxyQueryable* proxy_q,
        IConnectionsQueryable* conns_q,
        IStatusQueryable& status_q,
        endpoint_creator_t endpoint_creator,
        endpoint_registrator_t endpoint_registrator,
        ResourceEvent& event_service)
    : enabled_(false)
    , initialized_(false)
    , timer_active_(false)
    , local_participant_guid_(guid)
    , proxy_queryable_(proxy_q)
    , conns_queryable_(conns_q)
    , status_queryable_(status_q)
    , endpoint_creator_(endpoint_creator)
    , endpoint_registrator_(endpoint_registrator)
{
    if (!create_endpoint())
    {
        EPROSIMA_LOG_ERROR(MONITOR_SERVICE, "Could not create the monitor service endpoint");
    }

    event_.reset(new TimedEvent(event_service, [&]()
            {
                return spin_queue();
            }, MIN_TIME_BETWEEN_PUBS_MS));
}

MonitorService::~MonitorService()
{
    if (nullptr != listener_)
    {
        delete listener_;
    }
}

void MonitorService::release_payload_pool()
{
    if (status_writer_payload_pool_)
    {
        PoolConfig config = PoolConfig::from_history_attributes(status_writer_history_->m_att);
        status_writer_payload_pool_->release_history(config, true);
        status_writer_payload_pool_.reset();
    }
}

bool MonitorService::enable_monitor_service()
{
    bool ret = false;

    if (!enabled_.load())
    {
        if (!initialized_.load())
        {
            std::vector<GUID_t> local_guids;
            proxy_queryable_->get_all_local_proxies(local_guids);

            //! At least the participant and monitor service endpoints should exist
            assert(!local_guids.empty());

            {
                std::lock_guard<std::mutex> lock(mtx_);

                //! As we are later ignoring the monitor service endpoint, we are
                //! in fact reserving space for n+1
                changed_entities_.reserve(local_guids.size());

                for (auto& guid : local_guids)
                {
                    //! Ignore own writer
                    if (guid.entityId == monitor_service_status_writer)
                    {
                        continue;
                    }

                    initialize_entity(guid.entityId);
                }
            }

            event_->restart_timer();
            initialized_.store(true);
            timer_active_.store(true);
        }
        else
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (!changed_entities_.empty())
            {
                event_->restart_timer();
                timer_active_.store(true);
            }
        }

        ret = true;
        enabled_.store(true);
    }

    return ret;
}

bool MonitorService::disable_monitor_service()
{
    bool retcode = false;

    if (enabled_.load())
    {
        enabled_.store(false);
        retcode = true;
    }

    //! By design, the local_entities_ map keeps updating
    //! internally.

    return retcode;
}

bool MonitorService::remove_local_entity(
        const fastdds::rtps::EntityId_t& entity_id)
{
    // Remove the entity from the extended incompatible QoS collection
    {
        std::lock_guard<std::mutex> lock(extended_incompatible_qos_mtx_);
        GUID_t entity_guid = {local_participant_guid_.guidPrefix, entity_id};
        extended_incompatible_qos_collection_.erase(entity_guid);
    }

    // Remove the entity from the local entities
    {
        std::lock_guard<std::mutex> lock (mtx_);

        //! Add the entity to the changed entities if was not already present
        if (!local_entities_[entity_id].second)
        {
            changed_entities_.push_back(entity_id);
            if (!timer_active_.load())
            {
                event_->restart_timer();
                timer_active_.store(true);
            }
        }

        //! But remove it from the collection of entities
        //! This means a dispose
        local_entities_.erase(entity_id);
    }

    return true;
}

bool MonitorService::initialize_entity(
        const EntityId_t& entity_id)
{
    bool retcode = false;

    std::pair<EntityId_t, std::pair<std::bitset<StatusKind::STATUSES_SIZE>, bool>> local_entity;
    local_entity.second.first[StatusKind::PROXY] = 1;
    local_entity.second.first[StatusKind::CONNECTION_LIST] = 1;
    local_entity.first = entity_id;
    local_entity.second.second = true;

    auto ret = local_entities_.insert(local_entity);

    if (ret.second)
    {
        changed_entities_.push_back(ret.first->first);
        retcode = true;
    }
    else
    {
        EPROSIMA_LOG_ERROR(MONITOR_SERVICE, "Initializing an already existing entity");
    }

    return retcode;
}

bool MonitorService::push_entity_update(
        const fastdds::rtps::EntityId_t& entity_id,
        const uint32_t& status_id)
{
    bool ret = false;

    if (initialized_.load())
    {
        std::lock_guard<std::mutex> lock (mtx_);
        auto it = local_entities_.find(entity_id);

        if (it != local_entities_.end())
        {
            it->second.first[status_id] = 1;
            if (!it->second.second)
            {
                changed_entities_.push_back(it->first);
                it->second.second = true;
            }
            ret = true;
        }
        else
        {
            if (entity_id != monitor_service_status_writer && status_id != StatusKind::PROXY &&
                    status_id != StatusKind::CONNECTION_LIST)
            {
                EPROSIMA_LOG_ERROR(MONITOR_SERVICE,
                        "Trying to update the status of an entity without previously initialize it");
            }
            else
            {
                changed_entities_.reserve(changed_entities_.size() + 1);
                initialize_entity(entity_id);
                ret = true;
            }

        }

    }

    if (ret && enabled_.load() && !timer_active_.load())
    {
        event_->restart_timer();
        timer_active_.store(true);
    }

    return ret;
}

bool MonitorService::write_status(
        const fastdds::rtps::EntityId_t& entity_id,
        const std::bitset<StatusKind::STATUSES_SIZE>& changed_statuses,
        const bool& entity_disposed)
{
    if (!entity_disposed)
    {
        for (size_t i = 0; i < changed_statuses.size(); i++)
        {
            if (changed_statuses[i])
            {
                MonitorServiceStatusData status_data;
                MonitorServiceData data;

                status_data.local_entity(to_statistics_type({local_participant_guid_.guidPrefix, entity_id}));
                GUID_t local_entity_guid = {local_participant_guid_.guidPrefix, entity_id};

                bool status_retrieved = true;
                switch (i)
                {
                    case StatusKind::PROXY:
                    {
                        data.entity_proxy({});
                        CDRMessage_t msg;
                        //! Depending on the entity type [Participant, Writer, Reader]
                        //! the size will be accordingly calculated
                        status_retrieved = proxy_queryable_->get_serialized_proxy(local_entity_guid, &msg);
                        data.entity_proxy().assign(msg.buffer, msg.buffer + msg.length);
                        break;
                    }
                    case StatusKind::CONNECTION_LIST:
                    {
                        data.connection_list({});
                        std::vector<statistics::Connection> conns;
                        status_retrieved = conns_queryable_->get_entity_connections(local_entity_guid, conns);
                        data.connection_list(conns);
                        break;
                    }
                    case StatusKind::INCOMPATIBLE_QOS:
                    {
                        data.incompatible_qos_status(IncompatibleQoSStatus_s{});
                        status_retrieved = status_queryable_.get_monitoring_status(local_entity_guid, data);
                        break;
                    }
                    //Not triggered for the moment
                    case StatusKind::INCONSISTENT_TOPIC:
                    {
                        EPROSIMA_LOG_ERROR(MONITOR_SERVICE, "Inconsistent topic status not supported yet");
                        static_cast<void>(local_entity_guid);
                        break;
                    }
                    case StatusKind::LIVELINESS_LOST:
                    {
                        data.liveliness_lost_status(LivelinessLostStatus_s{});
                        status_retrieved = status_queryable_.get_monitoring_status(local_entity_guid, data);
                        break;
                    }
                    case StatusKind::LIVELINESS_CHANGED:
                    {
                        data.liveliness_changed_status(LivelinessChangedStatus_s{});
                        status_retrieved = status_queryable_.get_monitoring_status(local_entity_guid, data);
                        break;
                    }
                    case StatusKind::DEADLINE_MISSED:
                    {
                        data.deadline_missed_status(DeadlineMissedStatus_s{});
                        status_retrieved = status_queryable_.get_monitoring_status(local_entity_guid, data);
                        break;
                    }
                    case StatusKind::SAMPLE_LOST:
                    {
                        data.sample_lost_status(SampleLostStatus_s{});
                        status_retrieved = status_queryable_.get_monitoring_status(local_entity_guid, data);
                        break;
                    }
                    case StatusKind::EXTENDED_INCOMPATIBLE_QOS:
                    {
                        std::lock_guard<std::mutex> lock(extended_incompatible_qos_mtx_);
                        data.extended_incompatible_qos_status(extended_incompatible_qos_collection_[local_entity_guid]);
                        break;
                    }
                    default:
                    {
                        EPROSIMA_LOG_ERROR(MONITOR_SERVICE, "Referring to an unknown status");
                        static_cast<void>(local_entity_guid);
                        break;
                    }
                }

                if (status_retrieved)
                {
                    status_data.status_kind(static_cast<StatusKind::StatusKind>(i));
                    status_data.value(data);
                    add_change(status_data, false);
                }
                else
                {
                    EPROSIMA_LOG_ERROR(MONITOR_SERVICE, "Could not retrieve the status data for " << i << " of " <<
                            local_entity_guid);
                }
            }
        }
    }
    else
    {
        MonitorServiceStatusData status_data;
        MonitorServiceData data;

        status_data.local_entity(to_statistics_type({local_participant_guid_.guidPrefix, entity_id}));

        status_data.status_kind(StatusKind::PROXY);
        status_data.value().entity_proxy(std::vector<uint8_t>());

        //! Communicate the application what entity was removed
        //! by sending an empty proxy update
        add_change(status_data, false);

        //! Send a dispose for every statuskind of this entity
        for (uint32_t i = StatusKind::PROXY; i < StatusKind::STATUSES_SIZE; i++)
        {
            status_data.status_kind(i);
            add_change(status_data, true);
        }
    }

    return true;
}

bool MonitorService::add_change(
        MonitorServiceStatusData& status_data,
        const bool& disposed)
{
    InstanceHandle_t handle;
    type_.compute_key(&status_data, handle, false);

    CacheChange_t* change = status_writer_history_->create_change(
        (disposed ? fastdds::rtps::NOT_ALIVE_DISPOSED_UNREGISTERED : fastdds::rtps::ALIVE),
        handle);
    if (nullptr != change)
    {
        uint32_t cdr_size = type_.calculate_serialized_size(&status_data, fastdds::dds::DEFAULT_DATA_REPRESENTATION);
        if (!status_writer_payload_pool_->get_payload(cdr_size, change->serializedPayload))
        {
            status_writer_history_->release_change(change);
            change = nullptr;
        }
    }

    if (nullptr != change)
    {
        CDRMessage_t aux_msg(change->serializedPayload);

        if (!type_.serialize(&status_data, change->serializedPayload, fastdds::dds::DEFAULT_DATA_REPRESENTATION))
        {
            EPROSIMA_LOG_ERROR(MONITOR_SERVICE, "Serialization failed");
            status_writer_history_->release_change(change);
            return false;
        }

        WriteParams wp;
        auto datawriter_history = static_cast<eprosima::fastdds::dds::DataWriterHistory*>(status_writer_history_.get());

        std::unique_lock<RecursiveTimedMutex> lock(status_writer_->getMutex());
        auto max_blocking_time = std::chrono::steady_clock::now() +
                std::chrono::microseconds(fastdds::rtps::TimeConv::Time_t2MicroSecondsInt64(dds::Duration_t()));
        datawriter_history->add_pub_change(change, wp, lock, max_blocking_time);
    }
    else
    {
        EPROSIMA_LOG_ERROR(MONITOR_SERVICE, "Could not request a valid CacheChange for " <<
                status_data.status_kind() << " of " << to_fastdds_type(status_data.local_entity()));
        return false;
    }

    return true;
}

bool MonitorService::create_endpoint()
{
    EPROSIMA_LOG_INFO(MONITOR_SERVICE, "Creating Monitor Service endpoint");

    //! Design improvement: customize the endpoint attrs from a QoS

    bool created = false;

    RTPSWriter* tmp_writer = nullptr;

    WriterAttributes watts;
    watts.endpoint.endpointKind = fastdds::rtps::WRITER;
    watts.endpoint.durabilityKind = TRANSIENT_LOCAL;
    watts.endpoint.reliabilityKind = RELIABLE;
    watts.endpoint.topicKind = WITH_KEY;

    Property property;
    property.name("topic_name");
    property.value(MONITOR_SERVICE_TOPIC);
    watts.endpoint.properties.properties().push_back(std::move(property));

    HistoryAttributes hatt;
    hatt.payloadMaxSize = type_.max_serialized_type_size;
    hatt.memoryPolicy = MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    hatt.initialReservedCaches = 25;
    hatt.maximumReservedCaches = 0;

    dds::HistoryQosPolicy hqos;
    hqos.kind = dds::KEEP_LAST_HISTORY_QOS;
    hqos.depth = 1;

    TopicKind_t topic_kind = WITH_KEY;

    dds::ResourceLimitsQosPolicy rl_qos;
    rl_qos.max_instances = 0;
    rl_qos.max_samples_per_instance = 1;

    PoolConfig writer_pool_cfg = PoolConfig::from_history_attributes(hatt);
    status_writer_payload_pool_ = TopicPayloadPoolRegistry::get(MONITOR_SERVICE_TOPIC, writer_pool_cfg);
    status_writer_payload_pool_->reserve_history(writer_pool_cfg, false);

    status_writer_history_.reset(new eprosima::fastdds::dds::DataWriterHistory(
                status_writer_payload_pool_,
                std::make_shared<fastdds::rtps::CacheChangePool>(writer_pool_cfg),
                hqos, rl_qos, topic_kind, type_.max_serialized_type_size,
                MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE,
                [](
                    const InstanceHandle_t& ) -> void
                {
                }));

    listener_ = new MonitorServiceListener(this);

    created = endpoint_creator_(&tmp_writer,
                    watts,
                    status_writer_history_.get(),
                    listener_,
                    monitor_service_status_writer,
                    false);

    if (created)
    {
        status_writer_ = dynamic_cast<StatefulWriter*>(tmp_writer);

        //! Register the writer in the participant
        fastdds::dds::WriterQos wqos;

        wqos.m_reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
        wqos.m_durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;

        TopicDescription topic_desc;
        topic_desc.topic_name = MONITOR_SERVICE_TOPIC;
        topic_desc.type_name = type_.get_name();

        //! Register and propagate type object representation
        type_.register_type_object_representation();
        fastdds::dds::xtypes::TypeInformation type_info;
        if (fastdds::dds::RETCODE_OK ==
                RTPSDomainImpl::get_instance()->type_object_registry_observer().get_type_information(
                    type_.type_identifiers(), type_info))
        {
            topic_desc.type_information = type_info;
        }
        else
        {
            EPROSIMA_LOG_WARNING(MONITOR_SERVICE, "Failed to retrieve type information for " << MONITOR_SERVICE_TOPIC);
        }

        endpoint_registrator_(status_writer_, topic_desc, wqos);
    }
    else
    {
        status_writer_history_ = nullptr;
        release_payload_pool();
    }

    return created;
}

bool MonitorService::spin_queue()
{
    EntityId_t entity_id;
    bool re_schedule = false;
    std::bitset<StatusKind::STATUSES_SIZE> changed_statuses;
    bool local_instance_disposed = false;

    {
        std::lock_guard<std::mutex> lock(mtx_);

        //! Get EntityId from the queue
        entity_id = changed_entities_.front();
        changed_entities_.erase(changed_entities_.begin());

        //! Retrieve the entity from the map
        auto it_local_entities = local_entities_.find(entity_id);

        if (it_local_entities != local_entities_.end())
        {
            //! Item going to be processed, set the processing bool to false
            changed_statuses = it_local_entities->second.first;
            it_local_entities->second.second = false;
            it_local_entities->second.first.reset();
        }
        else
        {
            //! Item was removed, send dispose
            local_instance_disposed = true;
        }
    }

    write_status(entity_id, changed_statuses, local_instance_disposed);

    {
        std::lock_guard<std::mutex> lock(mtx_);

        if (!changed_entities_.empty())
        {
            re_schedule = true;
        }
        else
        {
            timer_active_.store(false);
        }
    }

    return re_schedule;
}

void MonitorService::on_incompatible_qos_matching(
        const fastdds::rtps::GUID_t& local_guid,
        const fastdds::rtps::GUID_t& remote_guid,
        const fastdds::dds::PolicyMask& incompatible_qos_policies)
{
    // Convert the PolicyMask to a vector of policy ids
    std::vector<uint32_t> incompatible_policies;
    for (uint32_t id = 1; id < dds::NEXT_QOS_POLICY_ID; ++id)
    {
        if (incompatible_qos_policies.test(id))
        {
            incompatible_policies.push_back(id);
        }
    }

    std::lock_guard<std::mutex> lock(extended_incompatible_qos_mtx_);

    if (!incompatible_policies.empty())
    {
        // Check if the local_guid is already in the collection. If not, create a new entry
        auto local_entity_incompatibilites =
                extended_incompatible_qos_collection_.insert({local_guid, {}});

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
                push_entity_update(local_guid.entityId, StatusKind::EXTENDED_INCOMPATIBLE_QOS);
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
            push_entity_update(local_guid.entityId, StatusKind::EXTENDED_INCOMPATIBLE_QOS);
        }
    }
    else
    {
        // Remove remote guid from the local guid incompatibilities collection
        auto it = extended_incompatible_qos_collection_.find(local_guid);

        if (it != extended_incompatible_qos_collection_.end())
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
                push_entity_update(local_guid.entityId, StatusKind::EXTENDED_INCOMPATIBLE_QOS);
            }
        }
    }
}

void MonitorService::on_remote_proxy_data_removed(
        const fastdds::rtps::GUID_t& removed_proxy_guid)
{
    auto& ext_incompatible_qos_collection = extended_incompatible_qos_collection_;
    std::lock_guard<std::mutex> lock(extended_incompatible_qos_mtx_);

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
            push_entity_update(local_entity.first.entityId, StatusKind::EXTENDED_INCOMPATIBLE_QOS);
        }
    }
}

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima
