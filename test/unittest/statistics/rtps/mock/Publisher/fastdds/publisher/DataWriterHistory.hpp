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

#ifndef FASTDDS_PUBLISHER__DATAWRITERHISTORY_HPP
#define FASTDDS_PUBLISHER__DATAWRITERHISTORY_HPP

#include <chrono>
#include <functional>
#include <mutex>

#include <gmock/gmock.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/ResourceManagement.hpp>
#include <fastdds/rtps/history/IChangePool.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>
#include <fastdds/utils/TimedMutex.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class WriteParams;

} // namespace rtps

namespace dds {

class DomainParticipant;
class PublisherListener;
class DataWriter;
class DataWriterListener;
class Topic;


static fastdds::rtps::HistoryAttributes to_history_attributes(
        const HistoryQosPolicy&,
        const ResourceLimitsQosPolicy&,
        const rtps::TopicKind_t&,
        uint32_t,
        rtps::MemoryManagementPolicy_t)
{

    return fastdds::rtps::HistoryAttributes();
}

class DataWriterHistory : public fastdds::rtps::WriterHistory
{
public:

    DataWriterHistory(
            const std::shared_ptr<rtps::IPayloadPool>& payload_pool,
            const std::shared_ptr<rtps::IChangePool>& change_pool,
            const HistoryQosPolicy& history_qos,
            const ResourceLimitsQosPolicy& resource_limits_qos,
            const rtps::TopicKind_t& topic_kind,
            uint32_t payloadMaxSize,
            rtps::MemoryManagementPolicy_t mempolicy,
            std::function<void (const fastdds::rtps::InstanceHandle_t&)>)
        : WriterHistory(to_history_attributes(history_qos, resource_limits_qos, topic_kind, payloadMaxSize,
                mempolicy), payload_pool, change_pool)
    {
    }

    MOCK_METHOD4(add_pub_change, bool(
                fastdds::rtps::CacheChange_t*,
                fastdds::rtps::WriteParams&,
                std::unique_lock<fastdds::RecursiveTimedMutex>&,
                const std::chrono::time_point<std::chrono::steady_clock>&));

};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS_PUBLISHER__DATAWRITERHISTORY_HPP
