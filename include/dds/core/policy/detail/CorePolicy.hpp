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

#ifndef EPROSIMA_DDS_CORE_POLICY_DETAIL_CORE_POLICY_HPP_
#define EPROSIMA_DDS_CORE_POLICY_DETAIL_CORE_POLICY_HPP_

#include <fastdds/dds/core/policy/QosPolicies.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace core {
namespace policy {
namespace detail {

// #ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
using DataRepresentation = eprosima::fastdds::dds::DataRepresentationQosPolicy;
// #endif // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

using Deadline = eprosima::fastdds::dds::DeadlineQosPolicy;

using DestinationOrder = eprosima::fastdds::dds::DestinationOrderQosPolicy;

using Durability = eprosima::fastdds::dds::DurabilityQosPolicy;

// #ifdef  OMG_DDS_PERSISTENCE_SUPPORT
using DurabilityService = eprosima::fastdds::dds::DurabilityServiceQosPolicy;
// #endif  // OMG_DDS_PERSISTENCE_SUPPORT

using EntityFactory = eprosima::fastdds::dds::EntityFactoryQosPolicy;

using GroupData = eprosima::fastdds::dds::GroupDataQosPolicy;

using History = eprosima::fastdds::dds::HistoryQosPolicy;

using LatencyBudget = eprosima::fastdds::dds::LatencyBudgetQosPolicy;

using Lifespan = eprosima::fastdds::dds::LifespanQosPolicy;

using Liveliness = eprosima::fastdds::dds::LivelinessQosPolicy;

using Ownership = eprosima::fastdds::dds::OwnershipQosPolicy;

// #ifdef  OMG_DDS_OWNERSHIP_SUPPORT
using OwnershipStrength = eprosima::fastdds::dds::OwnershipStrengthQosPolicy;
// #endif  //OMG_DDS_OWNERSHIP_SUPPORT

using Partition = eprosima::fastdds::dds::PartitionQosPolicy;

using Presentation = eprosima::fastdds::dds::PresentationQosPolicy;

using ReaderDataLifecycle = eprosima::fastdds::dds::ReaderDataLifecycleQosPolicy;

using Reliability = eprosima::fastdds::dds::ReliabilityQosPolicy;

using ResourceLimits = eprosima::fastdds::dds::ResourceLimitsQosPolicy;

using TimeBasedFilter = eprosima::fastdds::dds::TimeBasedFilterQosPolicy;

using TopicData = eprosima::fastdds::dds::TopicDataQosPolicy;

using TransportPriority = eprosima::fastdds::dds::TransportPriorityQosPolicy;

// #ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
using TypeConsistencyEnforcement = eprosima::fastdds::dds::TypeConsistencyEnforcementQosPolicy;
// #endif //OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

using UserData = eprosima::fastdds::dds::UserDataQosPolicy;

using WriterDataLifecycle = eprosima::fastdds::dds::WriterDataLifecycleQosPolicy;

} //namespace detail
} //namespace policy
} //namespace core
} //namespace dds

/** @endcond */

#endif //EPROSIMA_DDS_CORE_POLICY_DETAIL_CORE_POLICY_HPP_
