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
 * @file TopicQos.cpp
 *
 */

#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastdds::dds;

const TopicQos eprosima::fastdds::dds::TOPIC_QOS_DEFAULT;

TopicQos::TopicQos()
{
    this->reliability.kind = RELIABLE_RELIABILITY_QOS;
    this->durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
}

void TopicQos::setQos(
        const TopicQos& qos,
        bool first_time)
{
    if (first_time)
    {
        durability = qos.durability;
        durability.hasChanged = true;
    }
    if (first_time)
    {
        durability_service = qos.durability_service;
        durability_service.hasChanged = true;
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
        liveliness.hasChanged = true;
    }
    if (first_time)
    {
        reliability = qos.reliability;
        reliability.hasChanged = true;
    }
    if (first_time)
    {
        destination_order = qos.destination_order;
        destination_order.hasChanged = true;
    }
    if (first_time)
    {
        history = qos.history;
        history.hasChanged = true;
    }
    if (first_time)
    {
        resource_limits = qos.resource_limits;
        resource_limits.hasChanged = true;
    }
    if (transport_priority.value != qos.transport_priority.value)
    {
        transport_priority = qos.transport_priority;
        transport_priority.hasChanged = true;
    }
    if (lifespan.duration != qos.lifespan.duration)
    {
        lifespan = qos.lifespan;
        lifespan.hasChanged = true;
    }
    if (first_time)
    {
        ownership = qos.ownership;
        ownership.hasChanged = true;
    }
    if (topic_data.getValue() != qos.topic_data.getValue())
    {
        topic_data = qos.topic_data;
        topic_data.hasChanged = true;
    }
    if (auto_fill_type_information != qos.auto_fill_type_information)
    {
        auto_fill_type_information = qos.auto_fill_type_information;
    }
    if (auto_fill_type_object != qos.auto_fill_type_object)
    {
        auto_fill_type_object = qos.auto_fill_type_object;
    }
}

bool TopicQos::checkQos() const
{
    if (durability.kind == PERSISTENT_DURABILITY_QOS)
    {
        logError(DDS_QOS_CHECK, "PERSISTENT Durability not supported");
        return false;
    }
    if (destination_order.kind == BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
    {
        logError(DDS_QOS_CHECK, "BY SOURCE TIMESTAMP DestinationOrder not supported");
        return false;
    }
    if (reliability.kind == BEST_EFFORT_RELIABILITY_QOS && ownership.kind == EXCLUSIVE_OWNERSHIP_QOS)
    {
        logError(DDS_QOS_CHECK, "BEST_EFFORT incompatible with EXCLUSIVE ownership");
        return false;
    }
    if (liveliness.kind == AUTOMATIC_LIVELINESS_QOS || liveliness.kind == MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        if (liveliness.lease_duration < eprosima::fastrtps::c_TimeInfinite &&
                liveliness.lease_duration <= liveliness.announcement_period)
        {
            logError(DDS_QOS_CHECK, "DATAWRITERQOS: LeaseDuration <= announcement period.");
            return false;
        }
    }
    return true;
}

bool TopicQos::canQosBeUpdated(
        const TopicQos& qos) const
{
    bool updatable = true;
    if (durability.kind != qos.durability.kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Durability kind cannot be changed after the creation of a publisher.");
    }

    if (liveliness.kind !=  qos.liveliness.kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Liveliness Kind cannot be changed after the creation of a publisher.");
    }

    if (liveliness.lease_duration != qos.liveliness.lease_duration)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Liveliness lease duration cannot be changed after the creation of a publisher.");
    }

    if (liveliness.announcement_period != qos.liveliness.announcement_period)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Liveliness announcement cannot be changed after the creation of a publisher.");
    }

    if (reliability.kind != qos.reliability.kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Reliability Kind cannot be changed after the creation of a publisher.");
    }
    if (ownership.kind != qos.ownership.kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Ownership Kind cannot be changed after the creation of a publisher.");
    }
    if (destination_order.kind != qos.destination_order.kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Destination order Kind cannot be changed after the creation of a publisher.");
    }
    return updatable;
}
