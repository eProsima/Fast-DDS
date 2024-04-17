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

#include <fastdds/publisher/DataWriterHistory.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <fastrtps/utils/TimeConversion.h>

#include <rtps/history/PoolConfig.h>
#include <rtps/history/TopicPayloadPoolRegistry.hpp>
#include <statistics/rtps/StatisticsBase.hpp>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

MonitorService::MonitorService(
        const fastrtps::rtps::GUID_t& guid,
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
        const fastrtps::rtps::EntityId_t& entity_id)
{
    {
        std::lock_guard<std::mutex> lock (mtx_);

        //! Add the entity to the changed entities if was not already present
        if (!local_entities_[entity_id].second)
        {
            changed_entities_.push_back(entity_id);
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

    std::pair<EntityId_t, std::pair<std::bitset<STATUSES_SIZE>, bool>> local_entity;
    local_entity.second.first[PROXY] = 1;
    local_entity.second.first[CONNECTION_LIST] = 1;
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
        const fastrtps::rtps::EntityId_t& entity_id,
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
            if (entity_id != monitor_service_status_writer && status_id != PROXY && status_id != CONNECTION_LIST)
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
        const fastrtps::rtps::EntityId_t& entity_id,
        const std::bitset<STATUSES_SIZE>& changed_statuses,
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
                    case PROXY:
                    {
                        CDRMessage_t msg;
                        //! Depending on the entity type [Participant, Writer, Reader]
                        //! the size will be accordingly calculated
                        status_retrieved = proxy_queryable_->get_serialized_proxy(local_entity_guid, &msg);
                        data.entity_proxy().assign(msg.buffer, msg.buffer + msg.length);
                        break;
                    }
                    case CONNECTION_LIST:
                    {
                        std::vector<statistics::Connection> conns;
                        status_retrieved = conns_queryable_->get_entity_connections(local_entity_guid, conns);
                        data.connection_list(conns);
                        break;
                    }
                    case INCOMPATIBLE_QOS:
                    {
                        data.incompatible_qos_status(IncompatibleQoSStatus_s{});
                        status_retrieved = status_queryable_.get_monitoring_status(local_entity_guid, data);
                        break;
                    }
                    //Not triggered for the moment
                    case INCONSISTENT_TOPIC:
                    {
                        EPROSIMA_LOG_ERROR(MONITOR_SERVICE, "Inconsistent topic status not supported yet");
                        static_cast<void>(local_entity_guid);
                        break;
                    }
                    case LIVELINESS_LOST:
                    {
                        data.liveliness_lost_status(LivelinessLostStatus_s{});
                        status_retrieved = status_queryable_.get_monitoring_status(local_entity_guid, data);
                        break;
                    }
                    case LIVELINESS_CHANGED:
                    {
                        data.liveliness_changed_status(LivelinessChangedStatus_s{});
                        status_retrieved = status_queryable_.get_monitoring_status(local_entity_guid, data);
                        break;
                    }
                    case DEADLINE_MISSED:
                    {
                        data.deadline_missed_status(DeadlineMissedStatus_s{});
                        status_retrieved = status_queryable_.get_monitoring_status(local_entity_guid, data);
                        break;
                    }
                    case SAMPLE_LOST:
                    {
                        data.sample_lost_status(SampleLostStatus_s{});
                        status_retrieved = status_queryable_.get_monitoring_status(local_entity_guid, data);
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
                    status_data.status_kind((StatusKind)i);
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

        status_data.status_kind(PROXY);
        status_data.value().entity_proxy(std::vector<uint8_t>());

        //! Communicate the application what entity was removed
        //! by sending an empty proxy update
        add_change(status_data, false);

        //! Send a dispose for every statuskind of this entity
        for (uint32_t i = PROXY; i < STATUSES_SIZE; i++)
        {
            status_data.status_kind((StatusKind)i);
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
    type_.getKey(&status_data, &handle, false);

    CacheChange_t* change = status_writer_->new_change(
        type_.getSerializedSizeProvider(&status_data),
        (disposed ? fastrtps::rtps::NOT_ALIVE_DISPOSED_UNREGISTERED : fastrtps::rtps::ALIVE),
        handle);

    if (nullptr != change)
    {
        CDRMessage_t aux_msg(change->serializedPayload);

        if (!type_.serialize(&status_data, &change->serializedPayload))
        {
            EPROSIMA_LOG_ERROR(MONITOR_SERVICE, "Serialization failed");
            status_writer_->release_change(change);
            return false;
        }

        WriteParams wp;
        auto datawriter_history = static_cast<eprosima::fastdds::dds::DataWriterHistory*>(status_writer_history_.get());

        std::unique_lock<RecursiveTimedMutex> lock(status_writer_->getMutex());
        auto max_blocking_time = std::chrono::steady_clock::now() +
                std::chrono::microseconds(::TimeConv::Time_t2MicroSecondsInt64(Duration_t()));
        datawriter_history->add_pub_change(change, wp, lock, max_blocking_time);
    }
    else
    {
        EPROSIMA_LOG_ERROR(MONITOR_SERVICE, "Could not request a valid CacheChange for " << status_data.status_kind() <<
                " of " << to_fastdds_type(status_data.local_entity()));
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
    watts.endpoint.endpointKind = fastrtps::rtps::WRITER;
    watts.endpoint.durabilityKind = TRANSIENT_LOCAL;
    watts.endpoint.reliabilityKind = RELIABLE;
    watts.endpoint.topicKind = WITH_KEY;

    Property property;
    property.name("topic_name");
    property.value(MONITOR_SERVICE_TOPIC);
    watts.endpoint.properties.properties().push_back(std::move(property));

    HistoryAttributes hatt;
    hatt.payloadMaxSize = type_.m_typeSize;
    hatt.memoryPolicy = MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    hatt.initialReservedCaches = 25;
    hatt.maximumReservedCaches = 0;

    TopicAttributes tatt;
    tatt.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    tatt.historyQos.depth = 1;
    tatt.topicKind = WITH_KEY;
    tatt.topicName = MONITOR_SERVICE_TOPIC;
    tatt.resourceLimitsQos.max_instances = 0;
    tatt.resourceLimitsQos.max_samples_per_instance = 1;

    status_writer_history_.reset(new eprosima::fastdds::dds::DataWriterHistory(tatt, type_.m_typeSize,
            MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE,
            [](
                const InstanceHandle_t& ) -> void
            {
            }));

    PoolConfig writer_pool_cfg = PoolConfig::from_history_attributes(hatt);
    status_writer_payload_pool_ = TopicPayloadPoolRegistry::get(MONITOR_SERVICE_TOPIC, writer_pool_cfg);
    status_writer_payload_pool_->reserve_history(writer_pool_cfg, false);

    listener_ = new MonitorServiceListener(this);

    created = endpoint_creator_(&tmp_writer,
                    watts,
                    status_writer_payload_pool_,
                    status_writer_history_.get(),
                    listener_,
                    monitor_service_status_writer,
                    false);

    if (created)
    {
        status_writer_ = dynamic_cast<StatefulWriter*>(tmp_writer);

        //! Register the writer in the participant
        WriterQos wqos;

        wqos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        wqos.m_durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;

        TopicAttributes tatts;
        tatts.topicName = MONITOR_SERVICE_TOPIC;
        tatts.topicDataType = type_.getName();
        tatts.topicKind = WITH_KEY;

        endpoint_registrator_(status_writer_, tatts, wqos);
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
    std::bitset<STATUSES_SIZE> changed_statuses;
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

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima
