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
 * @file TopicPayloadPoolRegistry.hpp
 */

#ifndef RTPS_HISTORY_TOPICPAYLOADPOOLREGISTRY_IMPL_TOPICPAYLOADPOOLREGISTRY_HPP
#define RTPS_HISTORY_TOPICPAYLOADPOOLREGISTRY_IMPL_TOPICPAYLOADPOOLREGISTRY_HPP

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace eprosima {
namespace fastrtps {
namespace rtps {
namespace detail {

class TopicPayloadPoolRegistry
{
private:

    TopicPayloadPoolRegistry() = default;

public:

    /// @return reference to singleton instance
    static const std::shared_ptr<TopicPayloadPoolRegistry>& instance()
    {
        static std::shared_ptr<TopicPayloadPoolRegistry> pool_registry_instance(new TopicPayloadPoolRegistry());
        return pool_registry_instance;
    }

    std::shared_ptr<TopicPayloadPoolProxy> get(
            const std::string& topic_name,
            const BasicPoolConfig& config)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = pool_map_.find(topic_name);
        if (it == pool_map_.end())
        {
            it = pool_map_.emplace(topic_name, TopicPayloadPoolRegistryEntry()).first;
        }

        switch (config.memory_policy)
        {
            case PREALLOCATED_MEMORY_MODE:
                return do_get(it->second.pool_for_preallocated, topic_name, config);
            case PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
                return do_get(it->second.pool_for_preallocated_realloc, topic_name, config);
            case DYNAMIC_RESERVE_MEMORY_MODE:
                return do_get(it->second.pool_for_dynamic, topic_name, config);
            case DYNAMIC_REUSABLE_MEMORY_MODE:
                return do_get(it->second.pool_for_dynamic_reusable, topic_name, config);
        }

        return nullptr;
    }

    void release(
            std::shared_ptr<TopicPayloadPoolProxy>& pool)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // A reference count of 2 means the only ones referencing the pointer are the caller and the registry.
        // This means we can release the pointer on the registry also.
        if (pool.use_count() == 2)
        {
            auto it = pool_map_.find(pool->topic_name());
            assert(it != pool_map_.end());
            switch (pool->memory_policy())
            {
                case PREALLOCATED_MEMORY_MODE:
                    it->second.pool_for_preallocated.reset();
                    break;
                case PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
                    it->second.pool_for_preallocated_realloc.reset();
                    break;
                case DYNAMIC_RESERVE_MEMORY_MODE:
                    it->second.pool_for_dynamic.reset();
                    break;
                case DYNAMIC_REUSABLE_MEMORY_MODE:
                    it->second.pool_for_dynamic_reusable.reset();
                    break;
            }
        }
    }

private:

    std::shared_ptr<TopicPayloadPoolProxy> do_get(
            std::shared_ptr<TopicPayloadPoolProxy>& ptr,
            const std::string& topic_name,
            const BasicPoolConfig& config)
    {
        if (!ptr)
        {
            ptr = std::make_shared<TopicPayloadPoolProxy>(topic_name, config);
        }

        return ptr;
    }

    std::mutex mutex_;
    std::unordered_map<std::string, TopicPayloadPoolRegistryEntry> pool_map_;

};

}  // namespace detail
}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOLREGISTRY_IMPL_TOPICPAYLOADPOOLREGISTRY_HPP
