// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file SubscriptionBuiltinTopicData.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DATA__SUBSCRIPTIONBUILTINTOPICDATA_HPP
#define FASTDDS_RTPS_BUILTIN_DATA__SUBSCRIPTIONBUILTINTOPICDATA_HPP

#include <string>

#include <fastdds/dds/builtin/topic/BuiltinTopicKey.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/// Structure SubscriptionBuiltinTopicData, contains the information on a discovered subscription.
struct SubscriptionBuiltinTopicData
{
    //! Builtin topic Key
    BuiltinTopicKey_t key;

    //! Builtin participant topic Key
    BuiltinTopicKey_t participant_key;

    //! Topic name
    std::string topic_name;

    //! Type name
    std::string type_name;

    // DataReader Qos

    //!Durability Qos, implemented in the library.
    dds::DurabilityQosPolicy durability;

    //!Deadline Qos, implemented in the library.
    dds::DeadlineQosPolicy deadline;

    //!Latency Budget Qos, NOT implemented in the library.
    dds::LatencyBudgetQosPolicy latency_budget;

    //!Liveliness Qos, implemented in the library.
    dds::LivelinessQosPolicy liveliness;

    //!Reliability Qos, implemented in the library.
    dds::ReliabilityQosPolicy reliability;

    //!Ownership Qos, implemented in the library.
    dds::OwnershipQosPolicy ownership;

    //!Destination Order Qos, NOT implemented in the library.
    dds::DestinationOrderQosPolicy destination_order;

    //!User Data Qos, implemented in the library.
    dds::UserDataQosPolicy user_data;

    //!Time Based Filter Qos, NOT implemented in the library.
    dds::TimeBasedFilterQosPolicy m_timeBasedFilter;

    // Subscriber Qos

    //!Presentation Qos, NOT implemented in the library.
    dds::PresentationQosPolicy presentation;

    //!Partition Qos, implemented in the library.
    dds::PartitionQosPolicy partition;

    //!Topic Data Qos, NOT implemented in the library.
    dds::TopicDataQosPolicy topic_data;

    //!Group Data Qos, implemented in the library.
    dds::GroupDataQosPolicy group_data;
};

}   // namespace rtps
}   // namespace fastdds
}   // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DATA__SUBSCRIPTIONBUILTINTOPICDATA_HPP
