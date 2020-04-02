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
    if (first_time && durability().kind != qos.durability().kind)
    {
        durability() = qos.durability();
        durability().hasChanged = true;
    }
    if (first_time || deadline().period != qos.deadline().period)
    {
        deadline() = qos.deadline();
        deadline().hasChanged = true;
    }
    if (latency_budget().duration != qos.latency_budget().duration)
    {
        latency_budget() = qos.latency_budget();
        latency_budget().hasChanged = true;
    }
    if (first_time && !(liveliness() == qos.liveliness()))
    {
        liveliness() = qos.liveliness();
        liveliness().hasChanged = true;
    }
    if (first_time && !(reliability() == qos.reliability()))
    {
        reliability() = qos.reliability();
        reliability().hasChanged = true;
    }
    if (first_time && ownership().kind != qos.ownership().kind)
    {
        ownership() = qos.ownership();
        ownership().hasChanged = true;
    }
    if (destination_order().kind != qos.destination_order().kind)
    {
        destination_order() = qos.destination_order();
        destination_order().hasChanged = true;
    }
    if (user_data().data_vec() != qos.user_data().data_vec())
    {
        user_data() = qos.user_data();
        user_data().hasChanged = true;
    }
    if (time_based_filter().minimum_separation != qos.time_based_filter().minimum_separation )
    {
        time_based_filter() = qos.time_based_filter();
        time_based_filter().hasChanged = true;
    }
    if (topicData().getValue() != qos.topicData().getValue())
    {
        topicData() = qos.topicData();
        topicData().hasChanged = true;
    }
    if (first_time || !(durabilityService() == qos.durabilityService()))
    {
        durabilityService() = qos.durabilityService();
        durabilityService().hasChanged = true;
    }
    if (lifespan().duration != qos.lifespan().duration )
    {
        lifespan() = qos.lifespan();
        lifespan().hasChanged = true;
    }
    if (first_time && !(reliable_reader_qos() == qos.reliable_reader_qos()))
    {
        reliable_reader_qos() = qos.reliable_reader_qos();
        reliable_reader_qos().hasChanged = true;
    }
    if (first_time || !(type_consistency() == qos.type_consistency()))
    {
        type_consistency() = qos.type_consistency();
        type_consistency().hasChanged = true;
    }
    if (first_time && (history().kind != qos.history().kind ||
            history().depth != qos.history().depth))
    {
        history() = qos.history();
        history().hasChanged = true;
    }
    if (first_time && !(resource_limits() == qos.resource_limits()))
    {
        resource_limits() = qos.resource_limits();
        resource_limits().hasChanged = true;
    }
    if (!(reader_data_lifecycle() == qos.reader_data_lifecycle()))
    {
        reader_data_lifecycle() = qos.reader_data_lifecycle();
        reader_data_lifecycle().hasChanged = true;
    }

    if (expectsInlineQos() != qos.expectsInlineQos())
    {
        expectsInlineQos(qos.expectsInlineQos());
    }

    if (!(properties() == qos.properties()))
    {
        properties() = qos.properties();
    }

    if (!(enpoint() == qos.enpoint()))
    {
        enpoint() = qos.enpoint();
        enpoint().hasChanged = true;
    }

    if (!(reader_resource_limits() == qos.reader_resource_limits()))
    {
        reader_resource_limits() = qos.reader_resource_limits();
        reader_resource_limits().hasChanged = true;
    }
}

bool DataReaderQos::check_qos() const
{
    if (durability().kind == PERSISTENT_DURABILITY_QOS)
    {
        logError(DDS_QOS_CHECK, "PERSISTENT Durability not supported");
        return false;
    }
    if (destination_order().kind == BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS)
    {
        logError(DDS_QOS_CHECK, "BY SOURCE TIMESTAMP DestinationOrder not supported");
        return false;
    }
    if (reliability().kind == BEST_EFFORT_RELIABILITY_QOS && ownership().kind == EXCLUSIVE_OWNERSHIP_QOS)
    {
        logError(DDS_QOS_CHECK, "BEST_EFFORT incompatible with EXCLUSIVE ownership");
        return false;
    }
    return true;
}


bool DataReaderQos::can_qos_be_updated(
        const DataReaderQos& qos) const
{
    bool updatable = true;
    if (!(resource_limits() == qos.resource_limits()))
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "resource_limits cannot be changed after the creation of a subscriber.");
    }
    if (history().kind != qos.history().kind ||
            history().depth != qos.history().depth)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "History cannot be changed after the creation of a subscriber.");
    }

    if (durability().kind != qos.durability().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Durability kind cannot be changed after the creation of a subscriber.");
    }
    if (liveliness().kind != qos.liveliness().kind ||
            liveliness().lease_duration != qos.liveliness().lease_duration ||
            liveliness().announcement_period != qos.liveliness().announcement_period)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Liveliness cannot be changed after the creation of a subscriber.");
    }
    if (reliability().kind != qos.reliability().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Reliability Kind cannot be changed after the creation of a subscriber.");
    }
    if (ownership().kind != qos.ownership().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Ownership Kind cannot be changed after the creation of a subscriber.");
    }
    if (destination_order().kind != qos.destination_order().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Destination order Kind cannot be changed after the creation of a subscriber.");
    }
    return updatable;
}

ReaderQos DataReaderQos::get_readerqos(
        const SubscriberQos& pqos) const
{
    ReaderQos qos;
    qos.m_durability = durability();
    qos.m_deadline = deadline();
    qos.m_latencyBudget = latency_budget();
    qos.m_liveliness = liveliness();
    qos.m_reliability = reliability();
    qos.m_destinationOrder = destination_order();
    qos.m_presentation = pqos.presentation();
    qos.m_partition = pqos.partition();
    qos.m_groupData = pqos.group_data();
    qos.m_userData = user_data();
    qos.m_ownership = ownership();
    qos.m_timeBasedFilter = time_based_filter();
    qos.m_lifespan = lifespan();
    qos.m_topicData = topicData();
    qos.m_durabilityService = durabilityService();
    qos.m_disablePositiveACKs = reliable_reader_qos().disablePositiveACKs;
    qos.type_consistency = type_consistency().type_consistency;
    qos.representation = type_consistency().representation;
    return qos;
}


