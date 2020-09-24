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
 * @file DiscoverySharedInfo.hpp
 *
 */

#ifndef _FASTDDS_RTPS_DISCOVERY_SHARED_INFO_H_
#define _FASTDDS_RTPS_DISCOVERY_SHARED_INFO_H_

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include "./DiscoveryParticipantsAckStatus.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

/**
 * Class to join the shared info from Participant and Endpoint
 *@ingroup DISCOVERY_MODULE
 */
class DiscoverySharedInfo
{

public:

    DiscoverySharedInfo(
            eprosima::fastrtps::rtps::CacheChange_t* change_);

    ~DiscoverySharedInfo()
    {
    }

    // populate functions
    void add_participant(
            eprosima::fastrtps::rtps::GuidPrefix_t guid_p);

    void match_participant(
            eprosima::fastrtps::rtps::GuidPrefix_t guid_p);

    void remove_participant(
            eprosima::fastrtps::rtps::GuidPrefix_t guid_p);

    void set_disposal(
            eprosima::fastrtps::rtps::CacheChange_t* change_);

    void change_info(
            eprosima::fastrtps::rtps::CacheChange_t* change_);

    // getter functions
    bool is_matched(
            eprosima::fastrtps::rtps::GuidPrefix_t guid_p);

private:

    eprosima::fastrtps::rtps::CacheChange_t* change_;

    eprosima::fastdds::rtps::ddb::DiscoveryParticipantAckStatus
            relevant_participants_builtin_ack_status;


};

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_DISCOVERY_SHARED_INFO_H_ */
