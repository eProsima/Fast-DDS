// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DiscoveryParticipantChangeData.hpp
 *
 */

#ifndef _FASTDDS_RTPS_DISCOVERY_PARTICIPANT_CHANGE_DATA_H_
#define _FASTDDS_RTPS_DISCOVERY_PARTICIPANT_CHANGE_DATA_H_

#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/dds/core/policy/ParameterTypes.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

/**
 * Class to join the main info required from a Participant in the Discovery Data Base
 *@ingroup DISCOVERY_MODULE
 */
struct DiscoveryParticipantChangeData
{
    DiscoveryParticipantChangeData()
        : metatraffic_locators(fastrtps::rtps::RemoteLocatorList(0, 0))
    {
    }

    DiscoveryParticipantChangeData(
            fastrtps::rtps::RemoteLocatorList metatraffic_locators_,
            bool is_client_,
            bool is_local_)
        : metatraffic_locators(metatraffic_locators_)
        , is_client(is_client_)
        , is_local(is_local_)
    {
    }

    // The metatraffic locators of from the serialized payload
    fastrtps::rtps::RemoteLocatorList metatraffic_locators;
    // Whether this participant is a CLIENT or a SERVER
    bool is_client = false;
    // Whether this participant (CLIENT OR SERVER) is a client of this server
    bool is_local = false;
};

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_PARTICIPANT_INFO_H_ */
