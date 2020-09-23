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
 * @file TopicPayloadPool.hpp
 */

#ifndef RTPS_HISTORY_TOPICPAYLOADPOOL_HPP
#define RTPS_HISTORY_TOPICPAYLOADPOOL_HPP

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/rtps/resources/ResourceManagement.h>
#include <fastdds/dds/log/Log.hpp>
#include <rtps/history/PoolConfig.h>
#include <rtps/history/ITopicPayloadPool.h>

#include <memory>
#include <vector>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace TopicPayloadPool {

class BaseImpl : public ITopicPayloadPool
{

public:

    Base() = default;

    virtual ~Base()
    {
        logInfo(RTPS_UTILS, "PayloadPool destructor");

        for (octet* payload : all_payloads_)
        {
            free(payload);
        }
    }

    virtual bool get_payload(
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

        cache_change.serializedPayload.data = free_payloads_.back().data;
        cache_change.serializedPayload.max_size = free_payloads_.back().max_size;
        cache_change.payload_owner(this);
        free_payloads_.pop_back();
        return true;
    }

    virtual bool get_payload(
            SerializedPayload_t& data,
            IPayloadPool*& /* data_owner */,
            CacheChange_t& cache_change) override
    {
        assert(cache_change.writerGUID != GUID_t::unknown());
        assert(cache_change.sequenceNumber != SequenceNumber_t::unknown());

        if (get_payload(data.length, cache_change))
        {
            if (!cache_change.serializedPayload.copy(&data, false))
            {
                release_payload(cache_change);
                return false;
            }
        }

        return false;
    }

    virtual bool release_payload(
            CacheChange_t& cache_change) override
    {
        assert(cache_change.payload_owner() == this);

        FreePayload payload;
        payload.data = cache_change.serializedPayload.data;
        payload.max_size = cache_change.serializedPayload.max_size;
        free_payloads_.push_back(payload);
        cache_change.serializedPayload.length = 0;
        cache_change.serializedPayload.pos = 0;
        cache_change.serializedPayload.max_size = 0;
        cache_change.serializedPayload.data = nullptr;
        cache_change.payload_owner(nullptr);
        return true;
    }

    virtual bool reserve_history(
            const PoolConfig& config,
            bool /*is_reader*/) override
    {
        assert(config.memory_policy == memory_policy());

        update_maximum_size(config, true);
        return true;
    }

    virtual bool release_history(
            const PoolConfig& config,
            bool /*is_reader*/) override
    {
        assert(config.memory_policy == memory_policy());

        update_maximum_size(config, false);
        return true;
    }

    size_t get_allPayloadsSize() const override
    {
        return all_payloads_.size();
    }

    size_t get_freePayloadsSize() const override
    {
        return free_payloads_.size();
    }

protected:

    struct FreePayload
    {
        uint32_t max_size = 0;
        octet* data = nullptr;
    };

    /**
     * Adds a new payload in the pool, but does not add it to the list of free payloads
     */
    virtual octet* allocate(
            uint32_t size)
    {
        bool added = false;
        octet* payload = nullptr;

        if (all_payloads_.size() < max_pool_size_)
        {
            payload = (octet*)calloc(size, sizeof(octet));
            all_payloads_.push_back(payload);
            added = true;
        }

        if (!added)
        {
            logWarning(RTPS_HISTORY, "Maximum number of allowed reserved payloads reached");
            return nullptr;
        }

        return payload;
    }

    virtual void update_maximum_size(
            const PoolConfig& config,
            bool is_reserve)
    {
        if (is_reserve)
        {
            if (config.maximum_size == 0)
            {
                max_pool_size_ = std::numeric_limits<uint32_t>::max();
                ++infinite_histories_count_;
            }
            else
            {
                finite_max_pool_size_ += std::max(config.initial_size, config.maximum_size);
                if (infinite_histories_count_ == 0)
                {
                    max_pool_size_ = finite_max_pool_size_;
                }
            }
        }
        else
        {
            if (config.maximum_size == 0)
            {
                --infinite_histories_count_;
                if (infinite_histories_count_ == 0)
                {
                    max_pool_size_ = finite_max_pool_size_;
                }
            }
            else
            {
                max_pool_size_ -= (config.initial_size > config.maximum_size ? 
                        config.initial_size : config.maximum_size);
                finite_max_pool_size_ = max_pool_size_;
            }
        }
    }

    /**
     * Ensures the pool has capacity for at least @c num_payloads elements.
     * 
     * @param [IN] min_num_payloads Minimum number of payloads reserved in the pool
     * @param [IN] size             Size to allocate for the payloads that need to be added to the pool
     * 
     * @return @c true on success, @c false otherwise
     * 
     * @post
     *   - On success, get_allPayloadsSize() >= min_num_payloads
     *   - On failure, memory for some payloads may have been reserved, but get_allPayloadsSize() < min_num_payloads
     */
    virtual bool reserve (uint32_t min_num_payloads, uint32_t size)
    {
        for (uint32_t i = all_payloads_.size(); i < min_num_payloads; ++i)
        {
            octet* data = allocate(size);
            if (data == nullptr)
            {
                return false;
            }

            FreePayload payload;
            payload.data = data;
            payload.max_size = size;
            free_payloads_.push_back(payload);
        }

        return true;
    }

    /**
     * Ensures the pool has capacity for at most @c num_payloads elements.
     * 
     * @param [IN] max_num_payloads Maximum number of payloads reserved in the pool
     * 
     * @return @c true on success, @c false otherwise
     * 
     * @post
     *   - On success, get_allPayloadsSize() <= max_num_payloads
     *   - On failure, memory for some payloads may have been released, but get_allPayloadsSize() > min_num_payloads
     */
    bool shrink (uint32_t max_num_payloads)
    {
        assert(get_allPayloadsSize() - get_freePayloadsSize() <= max_num_payloads);

        while (max_num_payloads < all_payloads_.size())
        {
            octet* data = free_payloads_.back().data;
            free_payloads_.pop_back();

            // Find data in allPayloads, remove element, then delete it
            std::vector<octet*>::iterator target =
                    std::find(all_payloads_.begin(), all_payloads_.end(), data);
            if (target != all_payloads_.end())
            {
                // Copy last element into the element being removed
                if (target != --all_payloads_.end())
                {
                    *target = all_payloads_.back();
                }

                // Then drop last element
                all_payloads_.pop_back();
            }
            else
            {
                logError(RTPS_HISTORY, "Found a free payload that is not logged in the Pool");
                return false;
            }

            // Now we can free the memory
            free(data);
        }

        return true;
    }

    virtual MemoryManagementPolicy_t memory_policy() const = 0;

    uint32_t max_pool_size_             = 0;  //< Maximum size of the pool
    uint32_t infinite_histories_count_  = 0;  //< Number of infinite histories reserved
    uint32_t finite_max_pool_size_      = 0;  //< Maximum size of the pool if no infinite histories were reserved

    std::vector<FreePayload> free_payloads_;
    std::vector<octet*> all_payloads_;
};

template <MemoryManagementPolicy_t policy_>
class Impl : public BaseImpl
{
};

}  // namespace TopicPayloadPool
}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#include "./Dynamic.hpp"
#include "./DynamicReusable.hpp"
#include "./Preallocated.hpp"
#include "./PreallocatedWithRealloc.hpp"


namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace TopicPayloadPool {

std::shared_ptr<ITopicPayloadPool> get(
        PoolConfig config)
{
    switch (config.memory_policy)
    {
        case PREALLOCATED_MEMORY_MODE:
            return std::make_shared<Impl<PREALLOCATED_MEMORY_MODE> >(config.payload_initial_size);
        case PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
            return std::make_shared<Impl<PREALLOCATED_WITH_REALLOC_MEMORY_MODE> >(config.payload_initial_size);
        case DYNAMIC_RESERVE_MEMORY_MODE:
            return std::make_shared<Impl<DYNAMIC_RESERVE_MEMORY_MODE> >();
        case DYNAMIC_REUSABLE_MEMORY_MODE:
            return std::make_shared<Impl<DYNAMIC_REUSABLE_MEMORY_MODE> >();
    }

    return nullptr;
}

}  // namespace TopicPayloadPool
}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima


#endif  // RTPS_HISTORY_TOPICPAYLOADPOOL_HPP
