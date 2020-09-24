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
 * @file TopicPayloadPool.cpp
 */

#include <rtps/history/ITopicPayloadPool.h>

#include "./TopicPayloadPool.hpp"
#include "./TopicPayloadPool_impl/Preallocated.hpp"
#include "./TopicPayloadPool_impl/PreallocatedWithRealloc.hpp"
#include "./TopicPayloadPool_impl/Dynamic.hpp"
#include "./TopicPayloadPool_impl/DynamicReusable.hpp"

#include <memory>

namespace eprosima {
namespace fastrtps {
namespace rtps {

bool TopicPayloadPool::get_payload(
        uint32_t size,
        CacheChange_t& cache_change)
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

bool TopicPayloadPool::get_payload(
        SerializedPayload_t& data,
        IPayloadPool*& /* data_owner */,
        CacheChange_t& cache_change)
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

bool TopicPayloadPool::release_payload(
        CacheChange_t& cache_change)
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

bool TopicPayloadPool::reserve_history(
        const PoolConfig& config,
        bool /*is_reader*/)
{
    assert(config.memory_policy == memory_policy());

    update_maximum_size(config, true);
    return true;
}

bool TopicPayloadPool::release_history(
        const PoolConfig& config,
        bool /*is_reader*/)
{
    assert(config.memory_policy == memory_policy());

    update_maximum_size(config, false);
    return true;
}

octet* TopicPayloadPool::allocate(
        uint32_t size)
{
    octet* payload = nullptr;

    if (all_payloads_.size() >= max_pool_size_)
    {
        logWarning(RTPS_HISTORY, "Maximum number of allowed reserved payloads reached");
        return nullptr;
    }

    payload = (octet*)calloc(size, sizeof(octet));
    all_payloads_.push_back(payload);
    return payload;
}

void TopicPayloadPool::update_maximum_size(
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

void TopicPayloadPool::reserve (uint32_t min_num_payloads, uint32_t size)
{
    assert (min_num_payloads <= max_pool_size_);

    for (uint32_t i = all_payloads_.size(); i < min_num_payloads; ++i)
    {
        octet* data = (octet*)calloc(size, sizeof(octet));
        all_payloads_.push_back(data);

        FreePayload payload;
        payload.data = data;
        payload.max_size = size;
        free_payloads_.push_back(payload);
    }
}

bool TopicPayloadPool::shrink (uint32_t max_num_payloads)
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

bool TopicPayloadPool::resize_payload (
        octet*& payload,
        uint32_t& size,
        uint32_t new_size)
{
    octet* old_payload = payload;
    payload = (octet*)realloc(payload, new_size);
    if (!payload)
    {
        // Nothing changed on the buffers, so nothing to undo
        payload = old_payload;
        return false;
    }
    memset(payload + size, 0, (new_size - size) * sizeof(octet));
    size = new_size;

    // Find data in known payloads to update the pointer
    std::vector<octet*>::iterator target =
            std::find(all_payloads_.begin(), all_payloads_.end(), old_payload);
    assert(target != all_payloads_.end());
    *target = payload;
    return true;
}

std::shared_ptr<ITopicPayloadPool> TopicPayloadPool::get(
        PoolConfig config)
{
    switch (config.memory_policy)
    {
        case PREALLOCATED_MEMORY_MODE:
            return std::make_shared<PreallocatedTopicPayloadPool>(config.payload_initial_size);
        case PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
            return std::make_shared<PreallocatedReallocTopicPayloadPool>(config.payload_initial_size);
        case DYNAMIC_RESERVE_MEMORY_MODE:
            return std::make_shared<DynamicTopicPayloadPool>();
        case DYNAMIC_REUSABLE_MEMORY_MODE:
            return std::make_shared<DynamicReusableTopicPayloadPool>();
    }

    return nullptr;
}

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima
