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
 * @file Preallocated.hpp
 */

#ifndef RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_PREALLOCATED_REALLOC_HPP
#define RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_PREALLOCATED_REALLOC_HPP

#include <rtps/history/TopicPayloadPool.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class PreallocatedReallocTopicPayloadPool : public TopicPayloadPool
{
public:

    explicit PreallocatedReallocTopicPayloadPool(
            uint32_t payload_size)
        : min_payload_size_(payload_size)
        , minimum_pool_size_(0)
    {
        assert(min_payload_size_ > 0);
    }

    bool get_payload(
            uint32_t size,
            CacheChange_t& cache_change) override
    {
        return do_get_payload(std::max(size, min_payload_size_), cache_change, true);
    }

    bool reserve_history(
            const PoolConfig& config,
            bool is_reader) override
    {
        if (!TopicPayloadPool::reserve_history(config, is_reader))
        {
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        minimum_pool_size_ += config.initial_size;
        reserve(minimum_pool_size_, min_payload_size_);
        return true;
    }

    bool release_history(
            const PoolConfig& config,
            bool is_reader) override
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            minimum_pool_size_ -= config.initial_size;
        }

        return TopicPayloadPool::release_history(config, is_reader);
    }

protected:

    MemoryManagementPolicy_t memory_policy() const override
    {
        return PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    }

private:

    uint32_t min_payload_size_;
    uint32_t minimum_pool_size_;    //< Minimum initial pool size (sum of all histories)
};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_PREALLOCATED_REALLOC_HPP
