// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ParticipantBuiltinTopicData.cpp
 */

#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>


namespace eprosima {
namespace fastdds {
namespace rtps {

ParticipantBuiltinTopicData::ParticipantBuiltinTopicData(
        const VendorId_t vendor_id,
        const dds::DomainId_t domain_id,
        const RTPSParticipantAllocationAttributes& allocation)
    : user_data(static_cast<uint16_t>(allocation.data_limits.max_user_data))
    , properties(static_cast<uint32_t>(allocation.data_limits.max_properties))
    , metatraffic_locators(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
    , default_locators(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
    , vendor_id(vendor_id)
    , domain_id(domain_id)
{

}

}   // namespace rtps
}   // namespace fastdds
}   // namespace eprosima
