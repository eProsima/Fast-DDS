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
 * @file PublicationBuiltinTopicData.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DATA__PUBLICATIONBUILTINTOPICDATA_HPP
#define FASTDDS_RTPS_BUILTIN_DATA__PUBLICATIONBUILTINTOPICDATA_HPP

#include <cstdint>
#include <string>

#include <fastcdr/cdr/fixed_size_string.hpp>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/builtin/data/BuiltinTopicKey.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/rtps/common/Types.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/// Structure PublicationBuiltinTopicData, contains the information on a discovered publication.
struct PublicationBuiltinTopicData
{
    PublicationBuiltinTopicData()
    {
        reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
    }

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

    // DataWriter Qos

    /// Durability Qos, implemented in the library.
    dds::DurabilityQosPolicy durability;

    /// Durability Service Qos, NOT implemented in the library.
    dds::DurabilityServiceQosPolicy durability_service;

    /// Deadline Qos, implemented in the library.
    dds::DeadlineQosPolicy deadline;

    /// Latency Budget Qos, NOT implemented in the library.
    dds::LatencyBudgetQosPolicy latency_budget;

    /// Liveliness Qos, implemented in the library.
    dds::LivelinessQosPolicy liveliness;

    /// Reliability Qos, implemented in the library.
    dds::ReliabilityQosPolicy reliability;

    /// Lifespan Qos, implemented in the library.
    dds::LifespanQosPolicy lifespan;

    /// User Data Qos, implemented in the library.
    dds::UserDataQosPolicy user_data;

    /// Ownership Qos, implemented in the library.
    dds::OwnershipQosPolicy ownership;

    /// Ownership Strength Qos, implemented in the library.
    dds::OwnershipStrengthQosPolicy ownership_strength;

    /// Destination Order Qos, NOT implemented in the library.
    dds::DestinationOrderQosPolicy destination_order;

    // Publisher Qos

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

    // eProsima extensions

    /// Disable positive acks, implemented in the library.
    dds::DisablePositiveACKsQosPolicy disable_positive_acks;

    /// Information for data sharing compatibility check.
    dds::DataSharingQosPolicy data_sharing;

    /// GUID
    GUID_t guid;

    /// Persistence GUID
    GUID_t persistence_guid;

    /// Participant GUID
    GUID_t participant_guid;

    /// Remote locators
    RemoteLocatorList remote_locators;

    /// Maximum serialized size of data type
    uint32_t max_serialized_size = 0;

    /// Network configuration
    NetworkConfigSet_t loopback_transformation{};

};

}   // namespace rtps
}   // namespace fastdds
}   // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DATA__PUBLICATIONBUILTINTOPICDATA_HPP
