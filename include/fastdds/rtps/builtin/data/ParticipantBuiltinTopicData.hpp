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

#include <fastdds/dds/builtin/topic/BuiltinTopicKey.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct ParticipantBuiltinTopicData
{
    //! Constructor from ParticipantBuiltinTopicData
    ParticipantBuiltinTopicData(const ParticipantBuiltinTopicData& pdata)
    : key(pdata.key)
    , user_data(pdata.user_data)
    , guid(pdata.guid)
    , properties(pdata.properties)
    , participant_name(pdata.participant_name)
    , metatraffic_locators(pdata.metatraffic_locators)
    , default_locators(pdata.default_locators)
    , lease_duration(pdata.lease_duration)
    , vendor_id(pdata.vendor_id)
    , domain_id(pdata.domain_id)
    {
    }

    //! Default constructor
    ParticipantBuiltinTopicData()
    {}

    //! Builtin topic Key
    BuiltinTopicKey_t key;

    //! UserData QoS
    dds::UserDataQosPolicy user_data;

    //! Participant GUID
    GUID_t guid;

    //! Properties
    dds::ParameterPropertyList_t properties;

    //!Participant name
    fastcdr::string_255 participant_name;

    //!Metatraffic locators
    RemoteLocatorList metatraffic_locators;

    //!Default locators
    RemoteLocatorList default_locators;

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

#endif // FASTDDS_RTPS_BUILTIN_DATA__PARTICIPANTBUILTINTOPICDATA_HPP
