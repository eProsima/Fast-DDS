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
 * @file ParticipantBuiltinTopicData.hpp
 *
 */

#ifndef FASTDDS_RTPS_BUILTIN_DATA__PARTICIPANTBUILTINTOPICDATA_HPP
#define FASTDDS_RTPS_BUILTIN_DATA__PARTICIPANTBUILTINTOPICDATA_HPP

#include <fastcdr/xcdr/optional.hpp>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/builtin/data/BuiltinTopicKey.hpp>
#include <fastdds/rtps/common/ProductVersion_t.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct ParticipantBuiltinTopicData
{
    //! Default constructor
    FASTDDS_EXPORTED_API ParticipantBuiltinTopicData() = default;

    //! Constructor with allocation attributes
    FASTDDS_EXPORTED_API ParticipantBuiltinTopicData(
            const VendorId_t vendor_id,
            const dds::DomainId_t domain_id,
            const RTPSParticipantAllocationAttributes& allocation);

    /// Builtin topic Key
    BuiltinTopicKey_t key;

    /// UserData QoS
    dds::UserDataQosPolicy user_data;

    /// Participant GUID
    GUID_t guid;

    /// Properties
    dds::ParameterPropertyList_t properties;

    /// Participant name
    fastcdr::string_255 participant_name;

    /// Metatraffic locators
    RemoteLocatorList metatraffic_locators;

    /// Default locators
    RemoteLocatorList default_locators;

    /// Lease Duration
    dds::Duration_t lease_duration;

    /// Vendor id
    VendorId_t vendor_id;

    /// Product version
    ProductVersion_t product_version;

    /// Participant domain id
    dds::DomainId_t domain_id;

    /// Wire Protocol Qos
    fastcdr::optional<dds::WireProtocolConfigQos> wire_protocol;
};

} // rtps
} // fastdds
} // eprosima

#endif // FASTDDS_RTPS_BUILTIN_DATA__PARTICIPANTBUILTINTOPICDATA_HPP
