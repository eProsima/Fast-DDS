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
 * @file DynamicReusable.hpp
 */

#ifndef RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_DYNAMIC_REUSABLE_HPP
#define RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_DYNAMIC_REUSABLE_HPP

#include <rtps/history/TopicPayloadPool.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class DynamicReusableTopicPayloadPool : public TopicPayloadPool
{
    bool get_payload(
            uint32_t size,
            CacheChange_t& cache_change) override
    {
        if (!TopicPayloadPool::get_payload(size, cache_change))
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

    bool release_history(
            const PoolConfig& config,
            bool is_reader) override
    {
        if (!TopicPayloadPool::release_history(config, is_reader))
        {
            return false;
        }

        return shrink(max_pool_size_);
    }

protected:

    MemoryManagementPolicy_t memory_policy() const
    {
        return DYNAMIC_REUSABLE_MEMORY_MODE;
    }

};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_DYNAMIC_REUSABLE_HPP
