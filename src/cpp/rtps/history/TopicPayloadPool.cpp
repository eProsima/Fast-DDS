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
namespace fastdds {
namespace rtps {

bool TopicPayloadPool::get_payload(
        uint32_t size,
        SerializedPayload_t& payload)
{
    return do_get_payload(size, payload, false);
}

bool TopicPayloadPool::do_get_payload(
        uint32_t size,
        SerializedPayload_t& payload,
        bool resizeable)
{
    PayloadNode* payload_node = nullptr;

    std::unique_lock<std::mutex> lock(mutex_);
    if (free_payloads_.empty())
    {
        payload_node = allocate(size); //Allocates a single payload
        if (payload_node == nullptr)
        {
            lock.unlock();
            payload.data = nullptr;
            payload.max_size = 0;
            payload.payload_owner = nullptr;
            return false;
        }
    }
    else
    {
        payload_node = free_payloads_.back();
        free_payloads_.pop_back();
    }

    // Resize if needed
    if (resizeable && size > payload_node->data_size())
    {
        if (!payload_node->resize(size))
        {
            // Failed to resize, but we can still keep it for later.
            free_payloads_.push_back(payload_node);
            lock.unlock();
            EPROSIMA_LOG_ERROR(RTPS_HISTORY, "Failed to resize the payload");

            payload.data = nullptr;
            payload.max_size = 0;
            payload.payload_owner = nullptr;
            return false;
        }
    }

    lock.unlock();
    payload_node->reference();
    payload.data = payload_node->data();
    payload.max_size = payload_node->data_size();
    payload.payload_owner = this;

    return true;
}

bool TopicPayloadPool::get_payload(
        const SerializedPayload_t& data,
        SerializedPayload_t& payload)
{
    if (data.payload_owner == this)
    {
        PayloadNode::reference(data.data);

        payload.data = data.data;
        payload.length = data.length;
        payload.max_size = PayloadNode::data_size(data.data);
        payload.payload_owner = this;
        return true;
    }
    else
    {
        if (get_payload(data.length, payload))
        {
            if (!payload.copy(&data, true))
            {
                release_payload(payload);
                return false;
            }

            return true;
        }
    }

    return false;
}

bool TopicPayloadPool::release_payload(
        SerializedPayload_t& payload)
{
    assert(payload.payload_owner == this);

    if (PayloadNode::dereference(payload.data))
    {
        std::lock_guard<std::mutex> lock(mutex_);
        PayloadNode* payload_node = all_payloads_.at(PayloadNode::data_index(payload.data));
        free_payloads_.push_back(payload_node);
    }

    payload.length = 0;
    payload.pos = 0;
    payload.max_size = 0;
    payload.data = nullptr;
    payload.payload_owner = nullptr;
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
    if (all_payloads_.size() >= max_pool_size_)
    {
        EPROSIMA_LOG_WARNING(RTPS_HISTORY, "Maximum number of allowed reserved payloads reached");
        return nullptr;
    }

    return do_allocate(size);
}

TopicPayloadPool::PayloadNode* TopicPayloadPool::do_allocate(
        uint32_t size)
{
    PayloadNode* payload = new (std::nothrow) PayloadNode(size);

    if (payload != nullptr)
    {
        payload->data_index(static_cast<uint32_t>(all_payloads_.size()));
        all_payloads_.push_back(payload);
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTPS_HISTORY, "Failure to create a new payload ");
    }

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
            max_pool_size_ = (std::numeric_limits<uint32_t>::max)();
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
        }
        else
        {
            finite_max_pool_size_ -= (config.initial_size > config.maximum_size ?
                    config.initial_size : config.maximum_size);
        }
        if (infinite_histories_count_ == 0)
        {
            max_pool_size_ = finite_max_pool_size_;
        }
    }
}

void TopicPayloadPool::reserve (
        uint32_t min_num_payloads,
        uint32_t size)
{
    assert (min_num_payloads <= max_pool_size_);

    for (size_t i = all_payloads_.size(); i < min_num_payloads; ++i)
    {
        PayloadNode* payload = do_allocate(size);

        if (payload != nullptr)
        {
            free_payloads_.push_back(payload);
        }
    }
}

bool TopicPayloadPool::shrink (
        uint32_t max_num_payloads)
{
    assert(payload_pool_allocated_size() - payload_pool_available_size() <= max_num_payloads);

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

std::unique_ptr<ITopicPayloadPool> TopicPayloadPool::get(
        const BasicPoolConfig& config)
{
    if (config.payload_initial_size == 0u)
    {
        return nullptr;
    }

    ITopicPayloadPool* ret_val = nullptr;

    switch (config.memory_policy)
    {
        case PREALLOCATED_MEMORY_MODE:
            ret_val = new PreallocatedTopicPayloadPool(config.payload_initial_size);
            break;
        case PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
            ret_val = new PreallocatedReallocTopicPayloadPool(config.payload_initial_size);
            break;
        case DYNAMIC_RESERVE_MEMORY_MODE:
            ret_val = new DynamicTopicPayloadPool();
            break;
        case DYNAMIC_REUSABLE_MEMORY_MODE:
            ret_val = new DynamicReusableTopicPayloadPool();
            break;
    }

    return std::unique_ptr<ITopicPayloadPool>(ret_val);
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
