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
 * @file DiscoveryParticipantInfo.hpp
 *
 */

#ifndef _FASTDDS_RTPS_DISCOVERY_PARTICIPANT_INFO_H_
#define _FASTDDS_RTPS_DISCOVERY_PARTICIPANT_INFO_H_

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

/**
 * Class to join the main info required from a Participant in the Discovery Data Base
 *@ingroup DISCOVERY_MODULE
 */
class DiscoveryParticipantInfo
{

public:

    DiscoveryParticipantInfo(
            eprosima::fastrtps::rtps::CacheChange_t* cache_);

    ~DiscoveryParticipantInfo();

    // populate functions
    void add_reader(
            eprosima::fastrtps::rtps::GUID_t guid);

    void remove_reader(
            eprosima::fastrtps::rtps::GUID_t guid);

    void add_writer(
            eprosima::fastrtps::rtps::GUID_t guid);

    void remove_writer(
            eprosima::fastrtps::rtps::GUID_t guid);

    void add_participant(
            eprosima::fastrtps::rtps::GuidPrefix_t guid_p);

    void match_participant(
            eprosima::fastrtps::rtps::GuidPrefix_t guid_p);

    void remove_participant(
            eprosima::fastrtps::rtps::GuidPrefix_t guid_p);

    void set_disposal(
            eprosima::fastrtps::rtps::CacheChange_t* cache_);

    // get functions
    bool is_matched(
            eprosima::fastrtps::rtps::GuidPrefix_t guid_p);


};

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_PARTICIPANT_INFO_H_ */
