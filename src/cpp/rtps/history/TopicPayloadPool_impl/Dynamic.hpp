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
namespace fastrtps {
namespace rtps {

class DynamicTopicPayloadPool : public TopicPayloadPool
{
public:

    bool get_payload(
            uint32_t size,
            CacheChange_t& cache_change) override
    {
        return (size > 0u) && do_get_payload(size, cache_change, true);
    }

    bool release_payload(
            CacheChange_t& cache_change) override
    {
        assert(cache_change.payload_owner() == this);

        {
            if (PayloadNode::dereference(cache_change.serializedPayload.data))
            {
                //First remove it from all_payloads
                std::unique_lock<std::mutex> lock(mutex_);
                uint32_t data_index = PayloadNode::data_index(cache_change.serializedPayload.data);
                PayloadNode* payload = all_payloads_.at(data_index);
                all_payloads_.at(data_index) = all_payloads_.back();
                all_payloads_.back()->data_index(data_index);
                all_payloads_.pop_back();
                lock.unlock();

                // Now delete the data
                delete(payload);
            }
        }

        cache_change.serializedPayload.length = 0;
        cache_change.serializedPayload.pos = 0;
        cache_change.serializedPayload.max_size = 0;
        cache_change.serializedPayload.data = nullptr;
        cache_change.payload_owner(nullptr);

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

};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_DYNAMIC_HPP
