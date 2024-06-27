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
 * @file TopicBuiltinTopicData.hpp
 *
 */

#ifndef FASTDDS_DDS_BUILTIN_TOPIC__TOPICBUILTINTOPICDATA_HPP
#define FASTDDS_DDS_BUILTIN_TOPIC__TOPICBUILTINTOPICDATA_HPP

#include <fastdds/dds/builtin/topic/BuiltinTopicKey.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace builtin {

struct TopicBuiltinTopicData
{
    //! Builtin topic Key
    BuiltinTopicKey_t key;

    //! Name
    std::string name;

    //! Type name
    std::string type_name;

    //!Durability Qos, implemented in the library.
    DurabilityQosPolicy durability;

    //!Durability Service Qos, NOT implemented in the library.
    DurabilityServiceQosPolicy durability_service;

    //!Deadline Qos, implemented in the library.
    DeadlineQosPolicy deadline;

    //!Liveliness Qos, implemented in the library.
    LivelinessQosPolicy liveliness;

    //!Reliability Qos, implemented in the library.
    ReliabilityQosPolicy reliability;

    //!Transport Priority Qos, NOT implemented in the library.
    TransportPriorityQosPolicy transport_priority;

    //!Lifespan Qos, implemented in the library.
    LifespanQosPolicy lifespan;

    //!History Qos, implemented in the library.
    HistoryQosPolicy history;

    //!Resource Limits Qos, implemented in the library.
    ResourceLimitsQosPolicy resource_limits;

    //!Ownership Qos, NOT implemented in the library.
    OwnershipQosPolicy ownership;

    //!Topic Data Qos, NOT implemented in the library.
    TopicDataQosPolicy topic_data;
};

} // builtin
} // dds
} // fastdds
} // eprosima

#endif // FASTDDS_DDS_BUILTIN_TOPIC__TOPICBUILTINTOPICDATA_HPP
