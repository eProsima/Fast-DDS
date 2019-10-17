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
 * @file SubscriptionBuiltinTopicData.hpp
*/

#ifndef _FASTDDS_SUBCRIPTION_BUILTIN_TOPIC_DATA_HPP_
#define _FASTDDS_SUBCRIPTION_BUILTIN_TOPIC_DATA_HPP_

#include <fastdds/dds/topic/BuiltinTopicKey.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class SubscriptionBuiltinTopicData
{
public:
    SubscriptionBuiltinTopicData() {}

    ~SubscriptionBuiltinTopicData() {}

    const BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(
            const BuiltinTopicKey& key)
    {
        key_ = key;
    }

    const BuiltinTopicKey& participant_key() const
    {
        return participant_key_;
    }

    void participant_key(
            const BuiltinTopicKey& participant_key)
    {
        participant_key_ = participant_key;
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

    const OwnershipQosPolicy& ownership() const
    {
        return ownership_;
    }

    void ownership(
            const OwnershipQosPolicy& ownership)
    {
        ownership_ = ownership;
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

    const UserDataQosPolicy& user_data() const
    {
        return user_data_;
    }

    void user_data(
            const UserDataQosPolicy& user_data)
    {
        user_data_ = user_data;
    }

    const TimeBasedFilterQosPolicy& time_based_filter() const
    {
        return time_based_filter_;
    }

    void time_based_filter(
            const TimeBasedFilterQosPolicy& time_based_filter)
    {
        time_based_filter_ = time_based_filter;
    }

    const PresentationQosPolicy& presentation() const
    {
        return presentation_;
    }

    void presentation(
            const PresentationQosPolicy& presentation)
    {
        presentation_ = presentation;
    }

    const PartitionQosPolicy& partition() const
    {
        return partition_;
    }

    void partition(
            const PartitionQosPolicy& partition)
    {
        partition_ = partition;
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

    const GroupDataQosPolicy& group_data() const
    {
        return group_data_;
    }

    void group_data(
            const GroupDataQosPolicy& group_data)
    {
        group_data_ = group_data;
    }

    bool operator ==(
            const SubscriptionBuiltinTopicData& other)
    {
        return (key_ == other.key() &&
                participant_key_ == other.participant_key() &&
                name_.compare(other.name()) &&
                type_name_.compare(other.type_name()) &&
                durability_ == other.durability() &&
                deadline_ == other.deadline() &&
                latency_budget_ == other.latency_budget() &&
                liveliness_ == other.liveliness() &&
                reliability_ == other.reliability() &&
                ownership_ == other.ownership() &&
                destination_order_ == other.destination_order() &&
                user_data_ == other.user_data() &&
                time_based_filter_ == other.time_based_filter() &&
                presentation_ == other.presentation() &&
                partition_ == other.partition() &&
                topic_data_ == other.topic_data() &&
                group_data_ == other.group_data());
    }

private:

    BuiltinTopicKey key_;

    BuiltinTopicKey participant_key_;

    std::string name_;

    std::string type_name_;

    DurabilityQosPolicy durability_;

    DeadlineQosPolicy deadline_;

    LatencyBudgetQosPolicy latency_budget_;

    LivelinessQosPolicy liveliness_;

    ReliabilityQosPolicy reliability_;

    OwnershipQosPolicy ownership_;

    DestinationOrderQosPolicy destination_order_;

    UserDataQosPolicy user_data_;

    TimeBasedFilterQosPolicy time_based_filter_;

    PresentationQosPolicy presentation_;

    PartitionQosPolicy partition_;

    TopicDataQosPolicy topic_data_;

    GroupDataQosPolicy group_data_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_PARTICIPANT_BUILTIN_TOPIC_DATA_HPP_
