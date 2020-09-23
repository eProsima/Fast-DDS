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

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace TopicPayloadPool {

template <>
class Impl<DYNAMIC_REUSABLE_MEMORY_MODE> : public Base
{
    bool get_payload(
            uint32_t size,
            CacheChange_t& cache_change) override
    {
        octet* payload = nullptr;
        if (free_payloads_.empty())
        {
            payload = allocate(size); //Allocates a single payload
            cache_change.serializedPayload.data = payload;
            if (payload != nullptr)
            {
                cache_change.serializedPayload.max_size = size;
                cache_change.payload_owner(this);
                return true;
            }
            cache_change.serializedPayload.max_size = 0;
            cache_change.payload_owner(nullptr);
            return false;
        }

        octet* data = free_payloads_.back().data;
        uint32_t max_size = free_payloads_.back().max_size;

        // Resize if needed
        if (size > max_size)
        {
            octet* old_data = data;
            data = (octet*)realloc(data, size);
            if (!data)
            {
                // Nothing changed on the buffers, so nothing to undo
                throw std::bad_alloc();
            }
            memset(data + max_size, 0, (size - max_size) * sizeof(octet));
            max_size = size;

            // Find data in allPayloads to update the pointer
            std::vector<octet*>::iterator target =
                    std::find(all_payloads_.begin(), all_payloads_.end(), old_data);
            if (target != all_payloads_.end())
            {
                *target = data;
            }
            else
            {
                logError(RTPS_HISTORY, "Found a free payload that is not logged in the Pool");
                return false;
            }
        }

        free_payloads_.pop_back();
        cache_change.serializedPayload.data = data;
        cache_change.serializedPayload.max_size = max_size;
        cache_change.payload_owner(this);

        return true;
    }

    bool release_history(
            const PoolConfig& config,
            bool /*is_reader*/) override
    {
        assert(config.memory_policy == memory_policy());

        update_maximum_size(config, false);
        return shrink(max_pool_size_);
    }

protected:

    MemoryManagementPolicy_t memory_policy() const
    {
        return DYNAMIC_REUSABLE_MEMORY_MODE;
    }

};

}  // namespace TopicPayloadPool
}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima
