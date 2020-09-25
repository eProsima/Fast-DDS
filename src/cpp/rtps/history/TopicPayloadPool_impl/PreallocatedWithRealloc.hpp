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
        , minimum_pool_size_for_writers(0)
        , minimum_pool_size_for_readers(0)
    {
    }

    bool get_payload(
            uint32_t size,
            CacheChange_t& cache_change) override
    {
        if (!TopicPayloadPool::get_payload(std::max(size, min_payload_size_), cache_change))
        {
            return false;
        }

        // Resize if needed
        if (size > cache_change.serializedPayload.max_size)
        {
            if (!resize_payload(cache_change.serializedPayload.data, cache_change.serializedPayload.max_size, size))
            {
                logError(RTPS_HISTORY, "Failed to resize the payload");
                release_payload(cache_change);
                return false;
            }
        }

        return true;
    }

    bool reserve_history(
            const PoolConfig& config,
            bool is_reader) override
    {
        if (!TopicPayloadPool::reserve_history(config, is_reader))
        {
            return false;
        }

        if (is_reader)
        {
            minimum_pool_size_for_readers =
                    std::max(minimum_pool_size_for_readers, config.initial_size);
        }
        else
        {
            minimum_pool_size_for_writers += config.initial_size;
        }

        reserve(minimum_pool_size_for_writers + minimum_pool_size_for_readers, min_payload_size_);
        return true;
    }

protected:

    MemoryManagementPolicy_t memory_policy() const
    {
        return PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    }

private:

    uint32_t min_payload_size_;
    uint32_t minimum_pool_size_for_writers;    //< Initial pool size due to writers (sum of all writers)
    uint32_t minimum_pool_size_for_readers;    //< Initial pool size due to readers (max of all readers)
};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_PREALLOCATED_REALLOC_HPP
