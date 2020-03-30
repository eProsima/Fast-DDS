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

void DataWriterQos::setQos(
        const DataWriterQos& qos,
        bool first_time)
{
    //TODO: Implement this function
    (void)qos;
    (void)first_time;
}

bool DataWriterQos::checkQos() const
{
    //TODO: Implement this function
    return true;
}

bool DataWriterQos::canQosBeUpdated(
        const DataWriterQos& qos) const
{
    //TODO: Implement this function
    (void)qos;
    return true;
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
