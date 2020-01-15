// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PublisherQos.cpp
 *
 */

#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/utils/TimeConversion.h>

using namespace eprosima::fastdds::dds;

RTPS_DllAPI const PublisherQos eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT;

PublisherQos::PublisherQos()
{
    this->reliability.kind = RELIABLE_RELIABILITY_QOS;
    this->durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
}

PublisherQos::~PublisherQos()
{

}

void PublisherQos::set_qos(
        const PublisherQos& qos,
        bool first_time)
{
    entity_factory = qos.entity_factory;
    if (first_time)
    {
        durability = qos.durability;
        durability.hasChanged = true;
    }
    if (first_time || deadline.period != qos.deadline.period)
    {
        deadline = qos.deadline;
        deadline.hasChanged = true;
    }
    if (latency_budget.duration != qos.latency_budget.duration)
    {
        latency_budget = qos.latency_budget;
        latency_budget.hasChanged = true;
    }
    if (first_time)
    {
        liveliness = qos.liveliness;
        if (liveliness.lease_duration < fastrtps::c_TimeInfinite &&
                liveliness.lease_duration <= liveliness.announcement_period &&
                liveliness.announcement_period == fastrtps::c_TimeInfinite)
        {
            liveliness.announcement_period = fastrtps::rtps::TimeConv::Duration_t2MilliSecondsDouble(
                liveliness.lease_duration) * 0.25;
            logInfo(RTPS_LIVELINESS,
                    "Setting liveliness announcement period to " << liveliness.announcement_period << " ms");
        }
        liveliness.hasChanged = true;
    }
    if (first_time)
    {
        reliability = qos.reliability;
        reliability.hasChanged = true;
    }
    if (first_time)
    {
        ownership = qos.ownership;
        ownership.hasChanged = true;
    }
    if (destination_order.kind != qos.destination_order.kind )
    {
        destination_order = qos.destination_order;
        destination_order.hasChanged = true;
    }
    if (user_data.data_vec() != qos.user_data.data_vec())
    {
        user_data = qos.user_data;
        user_data.hasChanged = true;
    }
    if (first_time || time_based_filter.minimum_separation != qos.time_based_filter.minimum_separation )
    {
        time_based_filter = qos.time_based_filter;
        time_based_filter.hasChanged = true;
    }
    if (first_time || presentation.access_scope != qos.presentation.access_scope ||
            presentation.coherent_access != qos.presentation.coherent_access ||
            presentation.ordered_access != qos.presentation.ordered_access)
    {
        presentation = qos.presentation;
        presentation.hasChanged = true;
    }
    if (qos.partition.names().size() > 0)
    {
        partition = qos.partition;
        partition.hasChanged = true;
    }

    if (topic_data.getValue() != qos.topic_data.getValue())
    {
        topic_data = qos.topic_data;
        topic_data.hasChanged = true;
    }
    if (group_data.getValue() != qos.group_data.getValue())
    {
        group_data = qos.group_data;
        group_data.hasChanged = true;
    }
    if (first_time || durability_service.history_kind != qos.durability_service.history_kind ||
            durability_service.history_depth != qos.durability_service.history_depth ||
            durability_service.max_instances != qos.durability_service.max_instances ||
            durability_service.max_samples != qos.durability_service.max_samples ||
            durability_service.max_samples_per_instance != qos.durability_service.max_samples_per_instance ||
            durability_service.service_cleanup_delay != qos.durability_service.service_cleanup_delay
            )
    {
        durability_service = qos.durability_service;
        durability_service.hasChanged = true;
    }
    if (lifespan.duration != qos.lifespan.duration )
    {
        lifespan = qos.lifespan;
        lifespan.hasChanged = true;
    }
    if (qos.ownership_strength.value != ownership_strength.value)
    {
        ownership_strength = qos.ownership_strength;
        ownership_strength.hasChanged = true;
    }
    if (first_time)
    {
        disable_positive_acks = qos.disable_positive_acks;
        disable_positive_acks.hasChanged = true;
    }
}

bool PublisherQos::check_qos() const
{
    using namespace fastrtps;
    if (durability.kind == PERSISTENT_DURABILITY_QOS)
    {
        logError(RTPS_QOS_CHECK, "PERSISTENT Durability not supported");
        return false;
    }
    if (destination_order.kind == BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
    {
        logError(RTPS_QOS_CHECK, "BY SOURCE TIMESTAMP DestinationOrder not supported");
        return false;
    }
    if (reliability.kind == BEST_EFFORT_RELIABILITY_QOS && ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
    {
        logError(RTPS_QOS_CHECK, "BEST_EFFORT incompatible with EXCLUSIVE ownership");
        return false;
    }
    if (liveliness.kind == AUTOMATIC_LIVELINESS_QOS || liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        if (liveliness.lease_duration < fastrtps::c_TimeInfinite &&
                liveliness.announcement_period < fastrtps::c_TimeInfinite &&
                liveliness.lease_duration <= liveliness.announcement_period)
        {
            logError(DDS_QOS_CHECK, "DATAWRITERQOS: LeaseDuration <= announcement period.");
            return false;
        }
    }
    return true;
}

bool PublisherQos::can_qos_be_updated(
        const PublisherQos& qos) const
{
    using namespace fastrtps;
    bool updatable = true;
    if (durability.kind != qos.durability.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Durability kind cannot be changed after the creation of a subscriber.");
    }

    if (liveliness.kind !=  qos.liveliness.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Liveliness Kind cannot be changed after the creation of a subscriber.");
    }

    if (reliability.kind != qos.reliability.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Reliability Kind cannot be changed after the creation of a subscriber.");
    }
    if (ownership.kind != qos.ownership.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Ownership Kind cannot be changed after the creation of a subscriber.");
    }
    if (destination_order.kind != qos.destination_order.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Destination order Kind cannot be changed after the creation of a subscriber.");
    }
    return updatable;

}
