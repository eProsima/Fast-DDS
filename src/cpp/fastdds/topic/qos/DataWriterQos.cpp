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
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastdds::dds;

const DataWriterQos eprosima::fastdds::dds::DDS_DATAWRITER_QOS_DEFAULT;


DataWriterQos::DataWriterQos()
{
    this->reliability.kind = RELIABLE_RELIABILITY_QOS;
    this->durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
}

void DataWriterQos::setQos(
        const DataWriterQos& qos,
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
    if (first_time)
    {
        disable_positive_ACKs = qos.disable_positive_ACKs;
        disable_positive_ACKs.hasChanged = true;
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

bool DataWriterQos::checkQos() const
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

bool DataWriterQos::canQosBeUpdated(
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

//TODO: Remove below methods when the rtps classes use DataWriterQos instead of WriterQos
WriterQos DataWriterQos::changeToWriterQos() const
{
    WriterQos wqos;
    wqos.m_durability = this->durability;
    wqos.m_durabilityService = this->durability_service;
    wqos.m_deadline = this->deadline;
    wqos.m_latencyBudget = this->latency_budget;
    wqos.m_liveliness = this->liveliness;
    wqos.m_reliability = this->reliability;
    wqos.m_destinationOrder = this->destination_order;
    wqos.m_lifespan = this->lifespan;
    wqos.m_userData = this->user_data;
    wqos.m_ownership = this->ownership;
    wqos.m_ownershipStrength = this->ownership_strength;
    wqos.m_publishMode = this->publish_mode;
    wqos.m_disablePositiveACKs = this->disable_positive_ACKs;
    wqos.representation = this->representation;
    return wqos;
}

void DataWriterQos::changeToDataWriterQos(
        const WriterQos& qos)
{
    this->durability = qos.m_durability;
    this->durability_service = qos.m_durabilityService;
    this->deadline = qos.m_deadline;
    this->latency_budget = qos.m_latencyBudget;
    this->liveliness = qos.m_liveliness;
    this->reliability = qos.m_reliability;
    this->destination_order = qos.m_destinationOrder;
    this->lifespan = qos.m_lifespan;
    this->user_data = qos.m_userData;
    this->ownership = qos.m_ownership;
    this->ownership_strength = qos.m_ownershipStrength;
    this->publish_mode = qos.m_publishMode;
    this->disable_positive_ACKs = qos.m_disablePositiveACKs;
    this->representation = qos.representation;
}

void DataWriterQos::copyFromTopicQos(
        const TopicQos& topic_qos)
{
    this->deadline = topic_qos.deadline;
    this->destination_order = topic_qos.destination_order;
    this->durability = topic_qos.durability;
    this->durability_service = topic_qos.durability_service;
    this->history = topic_qos.history;
    this->latency_budget = topic_qos.latency_budget;
    this->lifespan = topic_qos.lifespan;
    this->liveliness = topic_qos.liveliness;
    this->ownership = topic_qos.ownership;
    this->reliability = topic_qos.reliability;
    this->resource_limits = topic_qos.resource_limits;
    this->transport_priority = topic_qos.transport_priority;

}

std::string DataWriterQos::search_qos_by_id(
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
