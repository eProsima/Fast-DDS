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
 * @file DataReaderQos.cpp
 *
 */

#include <fastdds/dds/topic/qos/DataReaderQos.hpp>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastdds::dds;

const DataReaderQos eprosima::fastdds::dds::DDS_DATAREADER_QOS_DEFAULT;


void DataReaderQos::setQos(
        const DataReaderQos& qos,
        bool first_time)
{
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
    if (user_data.data_vec() != qos.user_data.data_vec())
    {
        user_data = qos.user_data;
        user_data.hasChanged = true;
    }
    if (first_time)
    {
        ownership = qos.ownership;
        ownership.hasChanged = true;
    }
    if (time_based_filter.minimum_separation != qos.time_based_filter.minimum_separation)
    {
        time_based_filter = qos.time_based_filter;
        time_based_filter.hasChanged = true;
    }
    if (reader_data_lifecycle != qos.reader_data_lifecycle)
    {
        reader_data_lifecycle = qos.reader_data_lifecycle;
        reader_data_lifecycle.hasChanged = true;
    }
    if (representation.m_value != qos.representation.m_value)
    {
        representation = qos.representation;
        representation.hasChanged = true;
    }
    if (first_time ||
            type_consistency.m_kind != qos.type_consistency.m_kind ||
            type_consistency.m_ignore_member_names != qos.type_consistency.m_ignore_member_names ||
            type_consistency.m_ignore_string_bounds != qos.type_consistency.m_ignore_string_bounds ||
            type_consistency.m_ignore_sequence_bounds != qos.type_consistency.m_ignore_sequence_bounds ||
            type_consistency.m_force_type_validation != qos.type_consistency.m_force_type_validation ||
            type_consistency.m_prevent_type_widening != qos.type_consistency.m_prevent_type_widening)
    {
        type_consistency = qos.type_consistency;
        type_consistency.hasChanged = true;
    }
    if (first_time)
    {
        disable_positive_ACKs = qos.disable_positive_ACKs;
        disable_positive_ACKs.hasChanged = true;
    }
}

bool DataReaderQos::checkQos() const
{
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
    return true;
}

bool DataReaderQos::canQosBeUpdated(
        const DataReaderQos& qos) const
{
    bool updatable = true;
    if ( durability.kind != qos.durability.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Durability kind cannot be changed after the creation of a subscriber.");
    }

    if (liveliness.kind != qos.liveliness.kind)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Liveliness Kind cannot be changed after the creation of a subscriber.");
    }

    if (liveliness.lease_duration != qos.liveliness.lease_duration)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Liveliness lease duration cannot be changed after the creation of a subscriber.");
    }

    if (liveliness.announcement_period != qos.liveliness.announcement_period)
    {
        updatable = false;
        logWarning(RTPS_QOS_CHECK, "Liveliness announcement cannot be changed after the creation of a subscriber.");
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

//TODO: Remove below methods when the rtps classes use DataReaderQos instead of ReaderQos
ReaderQos DataReaderQos::changeToReaderQos() const
{
    ReaderQos rqos;
    rqos.m_durability = this->durability;
    rqos.m_deadline = this->deadline;
    rqos.m_latencyBudget = this->latency_budget;
    rqos.m_liveliness = this->liveliness;
    rqos.m_reliability = this->reliability;
    rqos.m_ownership = this->ownership;
    rqos.m_destinationOrder = this->destination_order;
    rqos.m_userData = this->user_data;
    rqos.m_timeBasedFilter = this->time_based_filter;
    rqos.representation = this->representation;
    rqos.type_consistency = this->type_consistency;
    rqos.m_disablePositiveACKs = this->disable_positive_ACKs;
    return rqos;
}

void DataReaderQos::changeToDataReaderQos(
        const ReaderQos& qos)
{
    this->durability = qos.m_durability;
    this->deadline = qos.m_deadline;
    this->latency_budget = qos.m_latencyBudget;
    this->liveliness = qos.m_liveliness;
    this->reliability = qos.m_reliability;
    this->destination_order = qos.m_destinationOrder;
    this->user_data = qos.m_userData;
    this->ownership = qos.m_ownership;
    this->time_based_filter = qos.m_timeBasedFilter;
    this->representation = qos.representation;
    this->type_consistency = qos.type_consistency;
    this->disable_positive_ACKs = qos.m_disablePositiveACKs;
}

void DataReaderQos::copyFromTopicQos(
        const TopicQos& topic_qos)
{
    this->durability = topic_qos.durability;
    this->deadline = topic_qos.deadline;
    this->destination_order = topic_qos.destination_order;
    this->history = topic_qos.history;
    this->latency_budget = topic_qos.latency_budget;
    this->liveliness = topic_qos.liveliness;
    this->ownership = topic_qos.ownership;
    this->reliability = topic_qos.reliability;
    this->resource_limits = topic_qos.resource_limits;
}

void DataReaderQos::copy_to_topic_attributes(
        eprosima::fastrtps::TopicAttributes* topic_att) const
{
    topic_att->historyQos = this->history;
    topic_att->resourceLimitsQos = this->resource_limits;
}

std::string DataReaderQos::search_qos_by_id(
        QosPolicyId_t id)
{
    switch (id){
        case 1:
            return "UserDataQosPolicy";
        case 2:
            return "DurabilityQosPolicy";
        case 3:
            return "PresentationQosPolicy";
        case 4:
            return "DeadlineQosPolicy";
        case 5:
            return "LatencyBudgetQosPolicy";
        case 6:
            return "OwnershipQosPolicy";
        case 7:
            return "OwnershipStrengthQosPolicy";
        case 8:
            return "LivelinessQosPolicy";
        case 9:
            return "TimeBasedFilterQosPolicy";
        case 10:
            return "PartitionQosPolicy";
        case 11:
            return "ReliabilityQosPolicy";
        case 12:
            return "DestinationOrderQosPolicy";
        case 13:
            return "HistoryQosPolicy";
        case 14:
            return "ResourceLimitsQosPolicy";
        case 15:
            return "EntityFactoryQosPolicy";
        case 16:
            return "WriterDataLifecycleQosPolicy";
        case 17:
            return "ReaderDataLifecycleQosPolicy";
        case 18:
            return "TopicDataQosPolicy";
        case 19:
            return "GroupDataQosPolicy";
        case 20:
            return "TransportPriorityQosPolicy";
        case 21:
            return "LifespanQosPolicy";
        case 22:
            return "DurabilityServiceQosPolicy";
        default:
            return "";
    }
}
