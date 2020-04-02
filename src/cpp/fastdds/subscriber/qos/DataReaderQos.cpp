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

#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastdds::dds;

const DataReaderQos eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;

void DataReaderQos::set_qos(
        const DataReaderQos& qos,
        bool first_time)
{
    //TODO: Implement this function
    (void)qos;
    (void)first_time;
}

bool DataReaderQos::check_qos() const
{
    //TODO: Implement this function
    return true;
}


bool DataReaderQos::can_qos_be_updated(
        const DataReaderQos& qos) const
{
    //TODO: Implement this function
    (void)qos;
    return true;
}

ReaderQos DataReaderQos::get_readerqos(
        const SubscriberQos& pqos) const
{
    ReaderQos qos;
    qos.m_durability = durability;
    qos.m_deadline = deadline;
    qos.m_latencyBudget = latency_budget;
    qos.m_liveliness = liveliness;
    qos.m_reliability = reliability;
    qos.m_destinationOrder = destination_order;
    qos.m_presentation = pqos.presentation();
    qos.m_partition = pqos.partition();
    qos.m_groupData = pqos.group_data();
    qos.m_userData = user_data;
    qos.m_ownership = ownership;
    qos.m_timeBasedFilter = time_based_filter;
    qos.m_lifespan = lifespan;
    qos.m_topicData = topicData;
    qos.m_durabilityService = durabilityService;
    qos.m_disablePositiveACKs = disablePositiveACKs;
    qos.type_consistency = type_consistency;
    qos.representation = representation;
    return qos;
}

void DataReaderQos::to_datareaderqos(
        const ReaderQos& rqos,
        const SubscriberQos& sqos)
{
    durability = rqos.m_durability;
    deadline = rqos.m_deadline;
    latency_budget = rqos.m_latencyBudget;
    liveliness = rqos.m_liveliness;
    reliability = rqos.m_reliability;
    destination_order = rqos.m_destinationOrder;
    //history = sqos.subscriber_attr.topic.historyQos;
    //resource_limits = sqos.subscriber_attr.topic.resourceLimitsQos;
    user_data = rqos.m_userData;
    ownership = rqos.m_ownership;
    time_based_filter = rqos.m_timeBasedFilter;
    lifespan = rqos.m_lifespan;
    topicData = rqos.m_topicData;
    durabilityService = rqos.m_durabilityService;
    disablePositiveACKs = rqos.m_disablePositiveACKs;
    type_consistency = rqos.type_consistency;
    representation = rqos.representation;
}

