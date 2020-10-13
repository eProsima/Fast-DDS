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

    ~TopicPayloadPoolRegistry() = default;

    /// @return reference to singleton instance
    static TopicPayloadPoolRegistry& instance()
    {
        static TopicPayloadPoolRegistry pool_registry_instance;
        return pool_registry_instance;
    }

    std::shared_ptr<TopicPayloadPoolProxy> get(
            const std::string& topic_name,
            const PoolConfig& config)
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
        pool.reset();
    }

private:

    std::shared_ptr<TopicPayloadPoolProxy> do_get(
            std::shared_ptr<TopicPayloadPoolProxy>& ptr,
            const std::string& topic_name,
            const PoolConfig& config)
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
