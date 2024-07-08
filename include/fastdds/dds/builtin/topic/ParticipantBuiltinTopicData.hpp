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
 * @file ParticipantBuiltinTopicData.hpp
 *
 */

#ifndef FASTDDS_DDS_BUILTIN_TOPIC__PARTICIPANTBUILTINTOPICDATA_HPP
#define FASTDDS_DDS_BUILTIN_TOPIC__PARTICIPANTBUILTINTOPICDATA_HPP

#include <fastdds/dds/builtin/topic/BuiltinTopicKey.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct ParticipantBuiltinTopicData
{
    //! Builtin topic Key
    dds::builtin::BuiltinTopicKey_t key;

    //! UserData QoS
    dds::UserDataQosPolicy user_data;

    //! Participant GUID
    GUID_t guid;

    //! Properties
    dds::PropertyPolicyQos properties;

    //!Participant name
    fastcdr::string_255 participant_name;

    //!Default unicast locators
    LocatorList default_unicast_locator_list;

    //!Default multicast locators
    LocatorList default_multicast_locator_list;

    //!Metatraffic unicast locators
    LocatorList metatraffic_unicast_locator_list;

    //!Metatraffic multicast locators
    LocatorList metatraffic_multicast_locator_list;

    //! Lease Duration
    Duration_t lease_duration;

    // Vendor id
    VendorId_t vendor_id;

    // Participant domain id
    dds::DomainId_t domain_id;
};

} // rtps
} // fastdds
} // eprosima

#endif // FASTDDS_DDS_BUILTIN_TOPIC__PARTICIPANTBUILTINTOPICDATA_HPP
