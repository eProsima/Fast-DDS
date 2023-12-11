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
 * @file MonitorService.hpp
 */

#ifndef _STATISTICS_RTPS_MONITOR_SERVICE_MONITORSERVICE_HPP_
#define _STATISTICS_RTPS_MONITOR_SERVICE_MONITORSERVICE_HPP_


#include <bitset>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/resources/TimedEvent.h>
#include <fastdds/rtps/writer/StatefulWriter.h>
#include <fastdds/statistics/rtps/monitor_service/Interfaces.hpp>
#include <fastrtps/qos/WriterQos.h>

#include <rtps/history/ITopicPayloadPool.h>
#include <statistics/rtps/monitor-service/MonitorServiceListener.hpp>
#include <statistics/types/monitorservice_types.h>
#include <statistics/types/monitorservice_typesPubSubTypes.h>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

#ifdef FASTDDS_STATISTICS

//! Dummy implementation
class SimpleQueryable : public IStatusQueryable
{

public:

    virtual ~SimpleQueryable()
    {

    }

    inline bool get_monitoring_status(
            const fastrtps::rtps::GUID_t&,
            const uint32_t&,
            rtps::DDSEntityStatus*&) override
    {
        return true;
    }

};

class MonitorService
{
    static constexpr int MIN_TIME_BETWEEN_PUBS_MS = 500;

public:

    using endpoint_creator_t = std::function<bool (fastrtps::rtps::RTPSWriter**,
                    fastrtps::rtps::WriterAttributes&,
                    const std::shared_ptr<fastrtps::rtps::IPayloadPool>&,
                    fastrtps::rtps::WriterHistory*,
                    fastrtps::rtps::WriterListener*,
                    const fastrtps::rtps::EntityId_t&,
                    bool)>;

    using endpoint_registrator_t = std::function<bool (
                        fastrtps::rtps::RTPSWriter*,
                        const fastrtps::TopicAttributes&,
                        const fastrtps::WriterQos&)>;

    MonitorService(
            const fastrtps::rtps::GUID_t& guid,
            IProxyQueryable* proxy_q,
            IConnectionsQueryable* conns_q,
            IStatusQueryable& status_q,
            endpoint_creator_t endpoint_creator,
            endpoint_registrator_t endpoint_registrator,
            fastrtps::rtps::ResourceEvent& event_service);

    ~MonitorService();

    /**
     * @brief Enables the Monitor Service in a Participant.
     *
     * @return Whether the service could be correctly enabled.
     * Returns false if the service was already enabled.
     */
    bool enable_monitor_service();

    /**
     * @brief Disables the Monitor Service in a Participant.
     *
     * @return Whether the service could be correctly enabled.
     * Returns false if the service was already disabled.
     */
    bool disable_monitor_service();

    /**
     * @brief Checks whether the Monitor Service is enabled
     * or not.
     *
     * @return True if the service is enabled.
     */
    inline bool is_enabled()
    {
        return enabled_.load();
    }

    /**
     * @brief Getter for the Monitor Service Listener.
     *
     * @return Pointer to the MonitorServiceListener instance.
     */
    inline const MonitorServiceListener* get_listener()
    {
        return listener_;
    }

    /**
     * @brief Removes a local previously known entity
     * from the guid collection.
     *
     * @param entity_id Identifier of the entity.
     *
     * @return True if the operation succeeds.
     */
    bool remove_local_entity(
            const fastrtps::rtps::EntityId_t& entity_id);

    /**
     * @brief Adds a new entity status update to the queue
     *
     * @param entity_id Identifier of the entity.
     * @param status_id Identifier of the status.
     *
     * The status_id can be [PROXY, CONNECTION_LIST, INCOMPATIBLE_QOS,
     * INCONSISTENT_TOPIC, LIVELINESS_LOST, LIVELINESS_CHANGED,
     * DEADLINE_MISSED, SAMPLE_LOST_STATUS]
     *
     * @return True if the operation succeeds.
     */
    bool push_entity_update(
            const fastrtps::rtps::EntityId_t& entity_id,
            const uint32_t& status_id);

private:

    /**
     * @brief This operation publishes all the enabled statuses
     * of the input entity_id.
     *
     * @return True if the operation succeeds.
     */
    bool write_status(
            const fastrtps::rtps::EntityId_t& entity_id,
            const std::bitset<STATUSES_SIZE>& changed_statuses,
            const bool& entity_disposed);

    /**
     * @brief Adds a new change to writer history
     *
     * @return True if the operation succeeds.
     */
    bool add_change(
            MonitorServiceStatusData& status_data,
            const bool& disposed);

    /**
     * @brief Creates and initializes the Monitor Service
     * Status writer
     *
     * @return True if the operation succeeds.
     */
    bool create_endpoint();

    /**
     * @brief Method that is timerly called and
     * proccesses the next element in the queue.
     *
     * @return True if the operation succeeds.
     */
    bool spin_queue();

    /**
     * @brief Initialized a new local entity
     * and requests the proxy and connection_list update
     *
     * @param entity_id Identifier of the entity.
     *
     * @return true if the entity was correctly initialized
     */
    bool initialize_entity(
            const fastrtps::rtps::EntityId_t& entity_id);

    /**
     * @brief Frees the Payload Pool
     *
     */
    void release_payload_pool();

    std::atomic<bool> enabled_;

    std::atomic<bool> initialized_;

    std::atomic<bool> timer_active_;

    const fastrtps::rtps::GUID_t local_participant_guid_;

    IProxyQueryable* proxy_queryable_;

    IConnectionsQueryable* conns_queryable_;

    IStatusQueryable& status_queryable_;

    //! Stores the local entities.
    //! For each entitiy, a bitset indicating the
    //! status id that needs to be updated, alongside
    //! with a bool that prevents the entity_id from being
    //! inserted twice.
    std::map<fastrtps::rtps::EntityId_t,
            std::pair<
                std::bitset<statistics::STATUSES_SIZE>, bool>> local_entities_;

    std::unique_ptr<fastrtps::rtps::TimedEvent> event_;

    std::vector<fastrtps::rtps::EntityId_t> changed_entities_;

    std::mutex mtx_;

    MonitorServiceListener* listener_;

    fastrtps::rtps::StatefulWriter* status_writer_;

    std::unique_ptr<fastrtps::rtps::WriterHistory> status_writer_history_;

    std::shared_ptr<fastrtps::rtps::ITopicPayloadPool> status_writer_payload_pool_;

    endpoint_creator_t endpoint_creator_;

    endpoint_registrator_t endpoint_registrator_;

    MonitorServiceStatusDataPubSubType type_;
};

#endif // FASTDDS_STATISTICS

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif // _STATISTICS_RTPS_MONITOR_SERVICE_MONITORSERVICE_HPP_

