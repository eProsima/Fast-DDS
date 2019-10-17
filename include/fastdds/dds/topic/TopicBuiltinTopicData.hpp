/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

/**
 * @file TopicBuiltinTopicData.hpp
*/

#ifndef _FASTDDS_TOPIC_BUILTIN_TOPIC_DATA_HPP_
#define _FASTDDS_TOPIC_BUILTIN_TOPIC_DATA_HPP_

#include <fastdds/dds/topic/BuiltinTopicKey.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class TopicBuiltinTopicData
{
public:
    TopicBuiltinTopicData() {}

    ~TopicBuiltinTopicData() {}

    const BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(
            const BuiltinTopicKey& key)
    {
        key_ = key;
    }

    const std::string& name() const
    {
        return name_;
    }

    void name(
            const std::string& name)
    {
        name_ = name;
    }


    const std::string& type_name() const
    {
        return type_name_;
    }

    void type_name(
            const std::string& type_name)
    {
        type_name_ = type_name;
    }

    const DurabilityQosPolicy& durability() const
    {
        return durability_;
    }

    void durability(
            const DurabilityQosPolicy& durability)
    {
        durability_ = durability;
    }

    const DurabilityServiceQosPolicy& durability_service() const
    {
        return durability_service_;
    }

    void durability_service(
            const DurabilityServiceQosPolicy& durability_service)
    {
        durability_service_ = durability_service;
    }

    const DeadlineQosPolicy& deadline() const
    {
        return deadline_;
    }

    void deadline(
            const DeadlineQosPolicy& deadline)
    {
        deadline_ = deadline;
    }

    const LatencyBudgetQosPolicy& latency_budget() const
    {
        return latency_budget_;
    }

    void latency_budget(
            const LatencyBudgetQosPolicy& latency_budget)
    {
        latency_budget_ = latency_budget;
    }

    const LivelinessQosPolicy& liveliness() const
    {
        return liveliness_;
    }

    void liveliness(
            const LivelinessQosPolicy& liveliness)
    {
        liveliness_ = liveliness;
    }

    const ReliabilityQosPolicy& reliability() const
    {
        return reliability_;
    }

    void reliability(
            const ReliabilityQosPolicy& reliability)
    {
        reliability_ = reliability;
    }

    const TransportPriorityQosPolicy& transport_priority() const
    {
        return transport_priority_;
    }

    void transport_priority(
            const TransportPriorityQosPolicy& transport_priority)
    {
        transport_priority_ = transport_priority;
    }

    const LifespanQosPolicy& lifespan() const
    {
        return lifespan_;
    }

    void lifespan(
            const LifespanQosPolicy& lifespan)
    {
        lifespan_ = lifespan;
    }

    const DestinationOrderQosPolicy& destination_order() const
    {
        return destination_order_;
    }

    void destination_order(
            const DestinationOrderQosPolicy& destination_order)
    {
        destination_order_ = destination_order;
    }

    const HistoryQosPolicy& history() const
    {
        return history_;
    }

    void history(
            const HistoryQosPolicy& history)
    {
        history_ = history;
    }

    const ResourceLimitsQosPolicy& resource_limits() const
    {
        return resource_limits_;
    }

    void resource_limits(
            const ResourceLimitsQosPolicy& resource_limits)
    {
        resource_limits_ = resource_limits;
    }

    const OwnershipQosPolicy& ownership() const
    {
        return ownership_;
    }

    void ownership(
            const OwnershipQosPolicy& ownership)
    {
        ownership_ = ownership;
    }

    const TopicDataQosPolicy& topic_data() const
    {
        return topic_data_;
    }

    void topic_data(
            const TopicDataQosPolicy& topic_data)
    {
        topic_data_ = topic_data;
    }

    bool operator ==(
            const TopicBuiltinTopicData& other)
    {
        return (key_ == other.key() &&
                name_.compare(other.name()) &&
                type_name_.compare(other.type_name()) &&
                durability_ == other.durability() &&
                durability_service_ == other.durability_service() &&
                deadline_ == other.deadline() &&
                latency_budget_ == other.latency_budget() &&
                liveliness_ == other.liveliness() &&
                reliability_ == other.reliability() &&
                transport_priority_ == other.transport_priority() &&
                lifespan_ == other.lifespan() &&
                destination_order_ == other.destination_order() &&
                history_ == other.history() &&
                resource_limits_ == other.resource_limits() &&
                ownership_ == other.ownership() &&
                topic_data_ == other.topic_data());
    }

private:

    BuiltinTopicKey key_;

    std::string name_;

    std::string type_name_;

    DurabilityQosPolicy durability_;

    DurabilityServiceQosPolicy durability_service_;

    DeadlineQosPolicy deadline_;

    LatencyBudgetQosPolicy latency_budget_;

    LivelinessQosPolicy liveliness_;

    ReliabilityQosPolicy reliability_;

    TransportPriorityQosPolicy transport_priority_;

    LifespanQosPolicy lifespan_;

    DestinationOrderQosPolicy destination_order_;

    HistoryQosPolicy history_;

    ResourceLimitsQosPolicy resource_limits_;

    OwnershipQosPolicy ownership_;

    TopicDataQosPolicy topic_data_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_PARTICIPANT_BUILTIN_TOPIC_DATA_HPP_
