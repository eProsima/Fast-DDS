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
 * @file DiscoveryDataQueueInfo.hpp
 *
 */

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/Guid.hpp>

#include <rtps/builtin/discovery/database/DiscoveryParticipantChangeData.hpp>


namespace eprosima {
namespace fastdds {
namespace rtps {
namespace ddb {

class DiscoveryDataQueueInfo
{
public:

    DiscoveryDataQueueInfo(
            CacheChange_t* change)
        : change_(change)
    {
    }

    ~DiscoveryDataQueueInfo()
    {
    }

    CacheChange_t* change()
    {
        return change_;
    }

protected:

    CacheChange_t* change_;

};

class DiscoveryPDPDataQueueInfo : public DiscoveryDataQueueInfo
{
public:

    DiscoveryPDPDataQueueInfo(
            CacheChange_t* change,
            const DiscoveryParticipantChangeData& participant_change_data)
        : DiscoveryDataQueueInfo(change)
        , participant_change_data_(participant_change_data)
    {
    }

    ~DiscoveryPDPDataQueueInfo()
    {
    }

    DiscoveryParticipantChangeData participant_change_data()
    {
        return participant_change_data_;
    }

private:

    const DiscoveryParticipantChangeData participant_change_data_;

};

class DiscoveryEDPDataQueueInfo : public DiscoveryDataQueueInfo
{
public:

    DiscoveryEDPDataQueueInfo(
            CacheChange_t* change,
            const eprosima::fastcdr::string_255& topic)
        : DiscoveryDataQueueInfo(change)
        , topic_(topic)
    {
    }

    ~DiscoveryEDPDataQueueInfo()
    {
    }

    eprosima::fastcdr::string_255 topic()
    {
        return topic_;
    }

private:

    const eprosima::fastcdr::string_255 topic_;

};

} /* namespace ddb */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
