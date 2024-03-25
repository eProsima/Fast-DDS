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

#ifndef _FASTDDS_PUBLISHER_DATAWRITERHISTORY_HPP_
#define _FASTDDS_PUBLISHER_DATAWRITERHISTORY_HPP_

#include <chrono>
#include <functional>
#include <mutex>

#include <gmock/gmock.h>

#include <fastdds/rtps/history/WriterHistory.h>
#include <fastdds/rtps/resources/ResourceManagement.h>
#include <fastrtps/utils/TimedMutex.hpp>

namespace eprosima {
namespace fastrtps {

class TopicAttributes;

namespace rtps {

class WriteParams;

} // namespace rtps
} // namespace fastrtps


namespace fastdds {
namespace dds {

class DomainParticipant;
class PublisherListener;
class DataWriter;
class DataWriterListener;
class Topic;


static fastrtps::rtps::HistoryAttributes to_history_attributes(
        const fastrtps::TopicAttributes&,
        uint32_t,
        fastrtps::rtps::MemoryManagementPolicy_t)
{

    return fastrtps::rtps::HistoryAttributes();
}

class DataWriterHistory : public fastrtps::rtps::WriterHistory
{
public:

    DataWriterHistory(
            const fastrtps::TopicAttributes& topic_att,
            uint32_t payloadMaxSize,
            fastrtps::rtps::MemoryManagementPolicy_t mempolicy,
            std::function<void (const fastrtps::rtps::InstanceHandle_t&)>)
        : WriterHistory(to_history_attributes(topic_att, payloadMaxSize, mempolicy))
    {
    }

    MOCK_METHOD4(add_pub_change, bool(
                fastrtps::rtps::CacheChange_t*,
                fastrtps::rtps::WriteParams&,
                std::unique_lock<fastrtps::RecursiveTimedMutex>&,
                const std::chrono::time_point<std::chrono::steady_clock>&));

};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // _FASTDDS_PUBLISHER_DATAWRITERHISTORY_HPP_
