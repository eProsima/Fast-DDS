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

#include <fastcdr/cdr/fixed_size_string.hpp>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/builtin/data/BuiltinTopicKey.hpp>
#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/rtps/common/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/// Structure SubscriptionBuiltinTopicData, contains the information on a discovered subscription.
struct SubscriptionBuiltinTopicData
{
    /// Builtin topic Key
    BuiltinTopicKey_t key{{0, 0, 0}};

    /// Builtin participant topic Key
    BuiltinTopicKey_t participant_key{{0, 0, 0}};

    /// Topic name
    fastcdr::string_255 topic_name;

    /// Type name
    fastcdr::string_255 type_name;

    /// Topic kind
    TopicKind_t topic_kind = TopicKind_t::NO_KEY;

    // DataReader Qos

    /// Durability Qos, implemented in the library.
    dds::DurabilityQosPolicy durability;

    /// Deadline Qos, implemented in the library.
    dds::DeadlineQosPolicy deadline;

    /// Latency Budget Qos, NOT implemented in the library.
    dds::LatencyBudgetQosPolicy latency_budget;

    /// Lifespan Qos, implemented in the library.
    dds::LifespanQosPolicy lifespan;

    /// Liveliness Qos, implemented in the library.
    dds::LivelinessQosPolicy liveliness;

    /// Reliability Qos, implemented in the library.
    dds::ReliabilityQosPolicy reliability;

    /// Ownership Qos, implemented in the library.
    dds::OwnershipQosPolicy ownership;

    /// Destination Order Qos, NOT implemented in the library.
    dds::DestinationOrderQosPolicy destination_order;

    /// User Data Qos, implemented in the library.
    dds::UserDataQosPolicy user_data;

    /// Time Based Filter Qos, NOT implemented in the library.
    dds::TimeBasedFilterQosPolicy time_based_filter;

    // Subscriber Qos

    /// Presentation Qos, NOT implemented in the library.
    dds::PresentationQosPolicy presentation;

    /// Partition Qos, implemented in the library.
    dds::PartitionQosPolicy partition;

    /// Topic Data Qos, NOT implemented in the library.
    dds::TopicDataQosPolicy topic_data;

    /// Group Data Qos, implemented in the library.
    dds::GroupDataQosPolicy group_data;

    // X-Types 1.3

    /// Type information
    dds::xtypes::TypeInformationParameter type_information;

    /// Data representation
    dds::DataRepresentationQosPolicy representation;

    /// Type consistency enforcement Qos, NOT implemented in the library.
    dds::TypeConsistencyEnforcementQosPolicy type_consistency;

    // eProsima Extensions

    /// Content filter configuration
    ContentFilterProperty content_filter{ ContentFilterProperty::AllocationConfiguration{} };

    /// Disable positive acks, implemented in the library.
    dds::DisablePositiveACKsQosPolicy disable_positive_acks;

    /// Information for data sharing compatibility check.
    dds::DataSharingQosPolicy data_sharing;

    /// GUID
    GUID_t guid;

    /// Participant GUID
    GUID_t participant_guid;

    /// Remote locators
    RemoteLocatorList remote_locators;

    /// Network configuration
    NetworkConfigSet_t loopback_transformation{};

    /// Expects Inline Qos
    bool expects_inline_qos = false;
};

}   // namespace rtps
}   // namespace fastdds
}   // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DATA__SUBSCRIPTIONBUILTINTOPICDATA_HPP
