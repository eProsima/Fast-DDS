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

using namespace eprosima::fastdds::dds;

const TopicQos eprosima::fastdds::dds::TOPIC_QOS_DEFAULT;

TopicQos::TopicQos()
{
    reliability_.kind = RELIABLE_RELIABILITY_QOS;
    durability_.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
}

void TopicQos::set_qos(
        const TopicQos& qos,
        bool first_time)
{
    if (first_time && !(durability_ == qos.durability()))
    {
        durability_ = qos.durability();
        durability_.hasChanged = true;
    }
    if (first_time && !(durability_service_ == qos.durability_service()))
    {
        durability_service_ = qos.durability_service();
        durability_service_.hasChanged = true;
    }
    if (deadline_.period != qos.deadline().period)
    {
        deadline_ = qos.deadline();
        deadline_.hasChanged = true;
    }
    if (latency_budget_.duration != qos.latency_budget().duration)
    {
        latency_budget_ = qos.latency_budget();
        latency_budget_.hasChanged = true;
    }
    if (first_time && !(liveliness_ == qos.liveliness()))
    {
        liveliness_ = qos.liveliness();
        liveliness_.hasChanged = true;
    }
    if (first_time && !(reliability_ == qos.reliability()))
    {
        reliability_ = qos.reliability();
        reliability_.hasChanged = true;
    }
    if (first_time && !(destination_order_ == qos.destination_order()))
    {
        destination_order_ = qos.destination_order();
        destination_order_.hasChanged = true;
    }
    if (first_time && !(history_ == qos.history()))
    {
        history_ = qos.history_;
        history_.hasChanged = true;
    }
    if (first_time && !(resource_limits_ == qos.resource_limits()))
    {
        resource_limits_ = qos.resource_limits();
        resource_limits_.hasChanged = true;
    }
    if (transport_priority_.value != qos.transport_priority().value)
    {
        transport_priority_ = qos.transport_priority();
        transport_priority_.hasChanged = true;
    }
    if (lifespan_.duration != qos.lifespan().duration)
    {
        lifespan_ = qos.lifespan();
        lifespan_.hasChanged = true;
    }
    if (first_time && !(ownership_ == qos.ownership()))
    {
        ownership_ = qos.ownership();
        ownership_.hasChanged = true;
    }
    if (topic_data_.getValue() != qos.topic_data().getValue())
    {
        topic_data_ = qos.topic_data();
        topic_data_.hasChanged = true;
    }
}

bool TopicQos::can_qos_be_updated(
        const TopicQos& qos) const
{
    bool updatable = true;
    if (durability_.kind != qos.durability().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Durability kind cannot be changed after the creation of a publisher.");
    }

    if (liveliness_.kind !=  qos.liveliness().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Liveliness Kind cannot be changed after the creation of a publisher.");
    }

    if (liveliness_.lease_duration != qos.liveliness().lease_duration)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Liveliness lease duration cannot be changed after the creation of a publisher.");
    }

    if (liveliness_.announcement_period != qos.liveliness().announcement_period)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Liveliness announcement cannot be changed after the creation of a publisher.");
    }

    if (reliability_.kind != qos.reliability().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Reliability Kind cannot be changed after the creation of a publisher.");
    }
    if (ownership_.kind != qos.ownership().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Ownership Kind cannot be changed after the creation of a publisher.");
    }
    if (destination_order_.kind != qos.destination_order().kind)
    {
        updatable = false;
        logWarning(DDS_QOS_CHECK, "Destination order Kind cannot be changed after the creation of a publisher.");
    }
    return updatable;

}
