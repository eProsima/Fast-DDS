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
 * @file Dynamic.hpp
 */

#ifndef RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_DYNAMIC_HPP
#define RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_DYNAMIC_HPP

#include <rtps/history/TopicPayloadPool.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class DynamicTopicPayloadPool : public TopicPayloadPool
{
public:

    bool get_payload(
            uint32_t size,
            SerializedPayload_t& payload) override
    {
        return do_get_payload(size, payload, true);
    }

    bool release_payload(
            SerializedPayload_t& payload) override
    {
        assert(payload.payload_owner == this);

        {
            if (PayloadNode::dereference(payload.data))
            {
                //First remove it from all_payloads
                std::unique_lock<std::mutex> lock(mutex_);
                uint32_t data_index = PayloadNode::data_index(payload.data);
                PayloadNode* payload_node = all_payloads_.at(data_index);
                all_payloads_.at(data_index) = all_payloads_.back();
                all_payloads_.back()->data_index(data_index);
                all_payloads_.pop_back();
                lock.unlock();

                // Now delete the data
                delete(payload_node);
            }
        }

        payload.length = 0;
        payload.pos = 0;
        payload.max_size = 0;
        payload.data = nullptr;
        payload.payload_owner = nullptr;

        return true;
    }

    bool release_history(
            const PoolConfig& config,
            bool /*is_reader*/) override
    {
        assert(config.memory_policy == memory_policy());

        std::lock_guard<std::mutex> lock(mutex_);
        update_maximum_size(config, false);

        return true;
    }

protected:

    MemoryManagementPolicy_t memory_policy() const override
    {
        return DYNAMIC_RESERVE_MEMORY_MODE;
    }

private:

    using TopicPayloadPool::get_payload;

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_DYNAMIC_HPP
