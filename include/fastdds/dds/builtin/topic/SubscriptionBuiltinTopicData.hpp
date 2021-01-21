// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 *
 */

#ifndef FASTDDS_DDS_BUILTIN_TOPIC_SUBSCRIPTIONBUILTINTOPICDATA_HPP
#define FASTDDS_DDS_BUILTIN_TOPIC_SUBSCRIPTIONBUILTINTOPICDATA_HPP

#include <fastdds/dds/builtin/topic/BuiltinTopicKey.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

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
    DurabilityQosPolicy durability;

    //!Deadline Qos, implemented in the library.
    DeadlineQosPolicy deadline;

    //!Latency Budget Qos, NOT implemented in the library.
    LatencyBudgetQosPolicy latency_budget;

    //!Liveliness Qos, implemented in the library.
    LivelinessQosPolicy liveliness;

    //!Reliability Qos, implemented in the library.
    ReliabilityQosPolicy reliability;

    //!Ownership Qos, implemented in the library.
    OwnershipQosPolicy ownership;

    //!Destination Order Qos, NOT implemented in the library.
    DestinationOrderQosPolicy destination_order;

    //!User Data Qos, implemented in the library.
    UserDataQosPolicy user_data;

    //!Time Based Filter Qos, NOT implemented in the library.
    TimeBasedFilterQosPolicy m_timeBasedFilter;

    // Subscriber Qos

    //!Presentation Qos, NOT implemented in the library.
    PresentationQosPolicy presentation;

    //!Partition Qos, implemented in the library.
    PartitionQosPolicy partition;

    //!Topic Data Qos, NOT implemented in the library.
    TopicDataQosPolicy topic_data;

    //!Group Data Qos, implemented in the library.
    GroupDataQosPolicy group_data;
};

} // builtin
} // dds
} // fastdds
} // eprosima

#endif // FASTDDS_DDS_BUILTIN_TOPIC_SUBSCRIPTIONBUILTINTOPICDATA_HPP
