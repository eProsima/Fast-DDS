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
 * @file DataWriterQos.cpp
 *
 */

#include <fastdds/dds/topic/qos/DataWriterQos.hpp>

using namespace eprosima::fastdds::dds;

const DataWriterQos eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT;

void DataWriterQos::set_qos(
        const DataWriterQos& qos,
        bool first_time)
{
    if (first_time && !(durability == qos.durability))
    {
        durability = qos.durability;
        durability.hasChanged = true;
    }
    if (first_time && !(durability_service == qos.durability_service))
    {
        durability_service = qos.durability_service;
        durability_service.hasChanged = true;
    }
    if (deadline.period != qos.deadline.period)
    {
        deadline = qos.deadline;
        deadline.hasChanged = true;
    }
    if (latency_budget.duration != qos.latency_budget.duration)
    {
        latency_budget = qos.latency_budget;
        latency_budget.hasChanged = true;
    }
    if (first_time && !(liveliness == qos.liveliness))
    {
        liveliness = qos.liveliness;
        liveliness.hasChanged = true;
    }
    if (first_time && !(reliability == qos.reliability))
    {
        reliability = qos.reliability;
        reliability.hasChanged = true;
    }
    if (first_time && !(destination_order == qos.destination_order))
    {
        destination_order = qos.destination_order;
        destination_order.hasChanged = true;
    }
    if (first_time && !(history == qos.history))
    {
        history = qos.history;
        history.hasChanged = true;
    }
    if (first_time && !(resource_limits == qos.resource_limits))
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
    if (user_data.data_vec() != qos.user_data.data_vec())
    {
        user_data = qos.user_data;
        user_data.hasChanged = true;
    }
    if (first_time && !(ownership == qos.ownership))
    {
        ownership = qos.ownership;
        ownership.hasChanged = true;
    }
    if (qos.ownership_strength.value != ownership_strength.value)
    {
        ownership_strength = qos.ownership_strength;
        ownership_strength.hasChanged = true;
    }
    if (writer_data_lifecycle.autodispose_unregistered_instances !=
            qos.writer_data_lifecycle.autodispose_unregistered_instances)
    {
        writer_data_lifecycle = qos.writer_data_lifecycle;
        writer_data_lifecycle.hasChanged = true;
    }
    if (first_time && !(disable_positive_acks == qos.disable_positive_acks))
    {
        disable_positive_acks = qos.disable_positive_acks;
        disable_positive_acks.hasChanged = true;
    }
    // Writers only manages the first element in the list of data representations.
    if (qos.representation.m_value.size() != representation.m_value.size() ||
            (qos.representation.m_value.size() > 0 && representation.m_value.size() > 0 &&
            *qos.representation.m_value.begin() != *representation.m_value.begin()))
    {
        representation = qos.representation;
        representation.hasChanged = true;
    }
}

bool DataWriterQos::check_qos() const
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

bool DataWriterQos::can_qos_be_updated(
        const DataWriterQos& qos) const
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

WriterQos DataWriterQos::get_writerqos(
        const PublisherQos& pqos) const
{
    WriterQos qos;
    qos.m_durability = this->durability;
    qos.m_durabilityService = this->durability_service;
    qos.m_deadline = this->deadline;
    qos.m_latencyBudget = this->latency_budget;
    qos.m_liveliness = this->liveliness;
    qos.m_reliability = this->reliability;
    qos.m_lifespan = this->lifespan;
    qos.m_userData = this->user_data;
    qos.m_ownership = this->ownership;
    qos.m_ownershipStrength = this->ownership_strength;
    qos.m_destinationOrder = this->destination_order;
    qos.m_presentation = pqos.presentation;
    qos.m_partition = pqos.partition;
    qos.m_groupData = pqos.group_data;
    qos.m_publishMode = this->publish_mode;
    qos.representation = this->representation;
    qos.m_disablePositiveACKs = this->disable_positive_acks;
    return qos;
}

void DataWriterQos::to_datawriterqos(
        const WriterQos& wqos,
        const PublisherQos& pqos)
{
    this->durability = wqos.m_durability;
    this->durability_service = wqos.m_durabilityService;
    this->deadline = wqos.m_deadline;
    this->latency_budget = wqos.m_latencyBudget;
    this->liveliness = wqos.m_liveliness;
    this->reliability = wqos.m_reliability;
    this->destination_order = wqos.m_destinationOrder;
    this->history = pqos.publisher_attr.topic.historyQos;
    this->resource_limits = pqos.publisher_attr.topic.resourceLimitsQos;
    this->lifespan = wqos.m_lifespan;
    this->user_data = wqos.m_userData;
    this->ownership = wqos.m_ownership;
    this->ownership_strength = wqos.m_ownershipStrength;
    this->publish_mode = wqos.m_publishMode;
    this->representation = wqos.representation;
    this->disable_positive_acks = wqos.m_disablePositiveACKs;
}
