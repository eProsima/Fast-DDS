// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file QosPolicies.h
 *
 */

#ifndef QOS_POLICIES_H_
#define QOS_POLICIES_H_

#include <fastdds/dds/core/policy/QosPolicies.hpp>

#include <vector>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/common/Time_t.h>
#include <fastrtps/qos/ParameterTypes.h>

namespace eprosima {
namespace fastrtps {

using QosPolicy = fastdds::dds::QosPolicy;
using DurabilityQosPolicyKind = fastdds::dds::DurabilityQosPolicyKind;
constexpr DurabilityQosPolicyKind VOLATILE_DURABILITY_QOS = DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS;
constexpr DurabilityQosPolicyKind TRANSIENT_LOCAL_DURABILITY_QOS =
        DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
constexpr DurabilityQosPolicyKind TRANSIENT_DURABILITY_QOS = DurabilityQosPolicyKind::TRANSIENT_DURABILITY_QOS;
constexpr DurabilityQosPolicyKind PERSISTENT_DURABILITY_QOS = DurabilityQosPolicyKind::PERSISTENT_DURABILITY_QOS;
using DurabilityQosPolicy = fastdds::dds::DurabilityQosPolicy;
using DeadlineQosPolicy = fastdds::dds::DeadlineQosPolicy;
using LatencyBudgetQosPolicy  = fastdds::dds::LatencyBudgetQosPolicy;
using LivelinessQosPolicyKind = fastdds::dds::LivelinessQosPolicyKind;
constexpr LivelinessQosPolicyKind AUTOMATIC_LIVELINESS_QOS = LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
constexpr LivelinessQosPolicyKind MANUAL_BY_PARTICIPANT_LIVELINESS_QOS =
        LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
constexpr LivelinessQosPolicyKind MANUAL_BY_TOPIC_LIVELINESS_QOS =
        LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS;
using LivelinessQosPolicy = fastdds::dds::LivelinessQosPolicy;
using ReliabilityQosPolicyKind = fastdds::dds::ReliabilityQosPolicyKind;
constexpr ReliabilityQosPolicyKind BEST_EFFORT_RELIABILITY_QOS = ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
constexpr ReliabilityQosPolicyKind RELIABLE_RELIABILITY_QOS = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
using ReliabilityQosPolicy = fastdds::dds::ReliabilityQosPolicy;
using OwnershipQosPolicyKind =  fastdds::dds::OwnershipQosPolicyKind;
constexpr OwnershipQosPolicyKind SHARED_OWNERSHIP_QOS = OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS;
constexpr OwnershipQosPolicyKind EXCLUSIVE_OWNERSHIP_QOS = OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS;
using OwnershipQosPolicy = fastdds::dds::OwnershipQosPolicy;
using DestinationOrderQosPolicyKind = fastdds::dds::DestinationOrderQosPolicyKind;
constexpr DestinationOrderQosPolicyKind BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS =
        DestinationOrderQosPolicyKind::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
constexpr DestinationOrderQosPolicyKind BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS =
        DestinationOrderQosPolicyKind::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
using DestinationOrderQosPolicy = fastdds::dds::DestinationOrderQosPolicy;
using TimeBasedFilterQosPolicy = fastdds::dds::TimeBasedFilterQosPolicy;
using PresentationQosPolicyAccessScopeKind = fastdds::dds::PresentationQosPolicyAccessScopeKind;
constexpr PresentationQosPolicyAccessScopeKind INSTANCE_PRESENTATION_QOS =
        PresentationQosPolicyAccessScopeKind::INSTANCE_PRESENTATION_QOS;
constexpr PresentationQosPolicyAccessScopeKind TOPIC_PRESENTATION_QOS =
        PresentationQosPolicyAccessScopeKind::TOPIC_PRESENTATION_QOS;
constexpr PresentationQosPolicyAccessScopeKind GROUP_PRESENTATION_QOS =
        PresentationQosPolicyAccessScopeKind::GROUP_PRESENTATION_QOS;
using PresentationQosPolicy = fastdds::dds::PresentationQosPolicy;
using PartitionQosPolicy = fastdds::dds::PartitionQosPolicy;
using UserDataQosPolicy = fastdds::dds::UserDataQosPolicy;
using TopicDataQosPolicy = fastdds::dds::TopicDataQosPolicy;
using GroupDataQosPolicy = fastdds::dds::GroupDataQosPolicy;
using HistoryQosPolicyKind = fastdds::dds::HistoryQosPolicyKind;
constexpr HistoryQosPolicyKind KEEP_LAST_HISTORY_QOS = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
constexpr HistoryQosPolicyKind KEEP_ALL_HISTORY_QOS = HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
using HistoryQosPolicy = fastdds::dds::HistoryQosPolicy;
using ResourceLimitsQosPolicy = fastdds::dds::ResourceLimitsQosPolicy;
using DurabilityServiceQosPolicy = fastdds::dds::DurabilityServiceQosPolicy;
using LifespanQosPolicy = fastdds::dds::LifespanQosPolicy;
using OwnershipStrengthQosPolicy = fastdds::dds::OwnershipStrengthQosPolicy;
using TransportPriorityQosPolicy = fastdds::dds::TransportPriorityQosPolicy;
using PublishModeQosPolicyKind = fastdds::dds::PublishModeQosPolicyKind;
constexpr PublishModeQosPolicyKind SYNCHRONOUS_PUBLISH_MODE = PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE;
constexpr PublishModeQosPolicyKind ASYNCHRONOUS_PUBLISH_MODE = PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE;
using PublishModeQosPolicy = fastdds::dds::PublishModeQosPolicy;
using DataRepresentationId = fastdds::dds::DataRepresentationId;
using DataRepresentationQosPolicy = fastdds::dds::DataRepresentationQosPolicy;
using TypeConsistencyKind = fastdds::dds::TypeConsistencyKind;
constexpr TypeConsistencyKind DISALLOW_TYPE_COERCION = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
constexpr TypeConsistencyKind ALLOW_TYPE_COERCION = TypeConsistencyKind::ALLOW_TYPE_COERCION;
using TypeConsistencyEnforcementQosPolicy = fastdds::dds::TypeConsistencyEnforcementQosPolicy;
using DisablePositiveACKsQosPolicy = fastdds::dds::DisablePositiveACKsQosPolicy;
using TypeIdV1 = fastdds::dds::TypeIdV1;
using TypeObjectV1 = fastdds::dds::TypeObjectV1;

namespace xtypes {
using TypeInformation = fastdds::dds::xtypes::TypeInformation;
} //namespace xtypes

}
}

#endif /* QOS_POLICIES_H_ */
