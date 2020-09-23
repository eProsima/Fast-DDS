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

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace TopicPayloadPool {

template <>
class Impl<PREALLOCATED_WITH_REALLOC_MEMORY_MODE> : public BaseImpl
{
public:

    explicit Impl(
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
        octet* payload = nullptr;
        if (free_payloads_.empty())
        {
            payload = allocate(std::max(size, min_payload_size_)); //Allocates a single payload
            cache_change.serializedPayload.data = payload;
            if (payload != nullptr)
            {
                cache_change.serializedPayload.max_size = std::max(size, min_payload_size_);
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
    
    bool reserve_history(
            const PoolConfig& config,
            bool is_reader) override
    {
        assert(config.memory_policy == memory_policy());

        update_maximum_size(config, true);
        if (is_reader)
        {
            minimum_pool_size_for_readers =
                    std::max(minimum_pool_size_for_readers, config.initial_size);
        }
        else
        {
            minimum_pool_size_for_writers += config.initial_size;
        }

        return reserve(minimum_pool_size_for_writers + minimum_pool_size_for_readers, min_payload_size_);
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
        return PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    }

private:

    uint32_t min_payload_size_;
    uint32_t minimum_pool_size_for_writers;    //< Initial pool size due to writers (sum of all writers)
    uint32_t minimum_pool_size_for_readers;    //< Initial pool size due to readers (max of all readers)
};

}  // namespace TopicPayloadPool
}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima
