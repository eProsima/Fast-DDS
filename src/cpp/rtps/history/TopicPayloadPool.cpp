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
    PayloadNode* payload = nullptr;

    std::lock_guard<std::mutex> lock(mutex_);
    if (free_payloads_.empty())
    {
        payload = allocate(size); //Allocates a single payload
        if (payload == nullptr)
        {
            cache_change.serializedPayload.data = nullptr;
            cache_change.serializedPayload.max_size = 0;
            cache_change.payload_owner(nullptr);
            return false;
        }
    }
    else
    {
        payload = free_payloads_.back();
        free_payloads_.pop_back();
    }

    payload->reference();
    cache_change.serializedPayload.data = payload->data();
    cache_change.serializedPayload.max_size = payload->data_size();
    cache_change.payload_owner(this);
    return true;
}

bool TopicPayloadPool::get_payload(
        SerializedPayload_t& data,
        IPayloadPool*& data_owner,
        CacheChange_t& cache_change)
{
    assert(cache_change.writerGUID != GUID_t::unknown());
    assert(cache_change.sequenceNumber != SequenceNumber_t::unknown());

    if (data_owner == this)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        PayloadNode* payload = all_payloads_.at(PayloadNode::data_index(data.data));
        payload->reference();

        cache_change.serializedPayload.data = payload->data();
        cache_change.serializedPayload.max_size = payload->data_size();
        cache_change.payload_owner(this);
        return true;
    }

    else
    {
        if (get_payload(data.length, cache_change))
        {
            if (!cache_change.serializedPayload.copy(&data, true))
            {
                release_payload(cache_change);
                return false;
            }

            data_owner = this;
            return true;
        }
    }

    return false;
}

bool TopicPayloadPool::release_payload(
        CacheChange_t& cache_change)
{
    assert(cache_change.payload_owner() == this);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        PayloadNode* payload = all_payloads_.at(PayloadNode::data_index(cache_change.serializedPayload.data));
        if (!payload->dereference())
        {
            free_payloads_.push_back(payload);
        }
    }

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

    std::lock_guard<std::mutex> lock(mutex_);
    update_maximum_size(config, true);
    return true;
}

bool TopicPayloadPool::release_history(
        const PoolConfig& config,
        bool /*is_reader*/)
{
    assert(config.memory_policy == memory_policy());

    std::lock_guard<std::mutex> lock(mutex_);
    update_maximum_size(config, false);

    return shrink(max_pool_size_);
}

TopicPayloadPool::PayloadNode* TopicPayloadPool::allocate(
        uint32_t size)
{
    PayloadNode* payload = nullptr;

    if (all_payloads_.size() >= max_pool_size_)
    {
        logWarning(RTPS_HISTORY, "Maximum number of allowed reserved payloads reached");
        return nullptr;
    }

    payload = new PayloadNode(size);
    all_payloads_.push_back(payload);
    payload->data_index(all_payloads_.size() - 1);
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

void TopicPayloadPool::reserve (
        uint32_t min_num_payloads,
        uint32_t size)
{
    assert (min_num_payloads <= max_pool_size_);

    for (uint32_t i = all_payloads_.size(); i < min_num_payloads; ++i)
    {
        PayloadNode* payload = new PayloadNode(size);
        all_payloads_.push_back(payload);
        free_payloads_.push_back(payload);
        payload->data_index(all_payloads_.size() - 1);
    }
}

bool TopicPayloadPool::shrink (
        uint32_t max_num_payloads)
{
    assert(get_allPayloadsSize() - get_freePayloadsSize() <= max_num_payloads);

    while (max_num_payloads < all_payloads_.size())
    {
        PayloadNode* payload = free_payloads_.back();
        free_payloads_.pop_back();

        // Find data in allPayloads, remove element, then delete it
        all_payloads_.at(payload->data_index()) = all_payloads_.back();
        all_payloads_.back()->data_index(payload->data_index());
        all_payloads_.pop_back();
        delete payload;
    }

    return true;
}

bool TopicPayloadPool::resize_payload (
        octet*& data,
        uint32_t& size,
        uint32_t new_size)
{
    std::lock_guard<std::mutex> lock(mutex_);
    PayloadNode* payload = all_payloads_.at(PayloadNode::data_index(data));
    if (payload->resize(new_size))
    {
        data = payload->data();
        size = payload->data_size();
        return true;
    }
    return false;
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
