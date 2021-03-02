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

#include <vector>

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>

#include "./DiscoverySharedInfo.hpp"
#include "./DiscoveryParticipantChangeData.hpp"

#include <json.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

/**
 * Class to join the main info required from a Participant in the Discovery Data Base
 *@ingroup DISCOVERY_MODULE
 */
class DiscoveryParticipantInfo : public DiscoverySharedInfo
{

public:

    DiscoveryParticipantInfo(
            eprosima::fastrtps::rtps::CacheChange_t* change,
            const eprosima::fastrtps::rtps::GuidPrefix_t& known_participant,
            DiscoveryParticipantChangeData participant_change_data)
        : DiscoverySharedInfo(change, known_participant)
        , participant_change_data_(participant_change_data)
    {
    }

    ~DiscoveryParticipantInfo()
    {
    }

    eprosima::fastrtps::rtps::CacheChange_t* update(
            eprosima::fastrtps::rtps::CacheChange_t* change,
            DiscoveryParticipantChangeData participant_change_data);

    eprosima::fastrtps::rtps::CacheChange_t* update(
            eprosima::fastrtps::rtps::CacheChange_t* change)
    {
        return DiscoverySharedInfo::update(change);
    }

    eprosima::fastrtps::rtps::CacheChange_t* update_and_unmatch(
            eprosima::fastrtps::rtps::CacheChange_t* change,
            DiscoveryParticipantChangeData participant_change_data);

    eprosima::fastrtps::rtps::CacheChange_t* update_and_unmatch(
            eprosima::fastrtps::rtps::CacheChange_t* change)
    {
        return DiscoverySharedInfo::update_and_unmatch(change);
    }

    // populate functions
    void add_reader(
            const eprosima::fastrtps::rtps::GUID_t& guid);

    void remove_reader(
            const eprosima::fastrtps::rtps::GUID_t& guid);

    void add_writer(
            const eprosima::fastrtps::rtps::GUID_t& guid);

    void remove_writer(
            const eprosima::fastrtps::rtps::GUID_t& guid);

    bool is_client() const
    {
        return participant_change_data_.is_client();
    }

    bool is_local() const
    {
        return participant_change_data_.is_local();
    }

    void participant_change_data(
            const DiscoveryParticipantChangeData& new_participant_change_data)
    {
        participant_change_data_ = new_participant_change_data;
    }

    bool is_external()
    {
        return (!is_local());
    }

    fastrtps::rtps::RemoteLocatorList metatraffic_locators()
    {
        return participant_change_data_.metatraffic_locators();
    }

    std::vector<eprosima::fastrtps::rtps::GUID_t> readers()
    {
        return readers_;
    }

    std::vector<eprosima::fastrtps::rtps::GUID_t> writers()
    {
        return writers_;
    }

    void to_json(
            nlohmann::json& j) const;

private:

    std::vector<eprosima::fastrtps::rtps::GUID_t> readers_;

    std::vector<eprosima::fastrtps::rtps::GUID_t> writers_;

    DiscoveryParticipantChangeData participant_change_data_;

};

} /* namespace ddb */
} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_PARTICIPANT_INFO_H_ */
