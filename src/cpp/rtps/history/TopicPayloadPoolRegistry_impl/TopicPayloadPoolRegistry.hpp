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

#include <map>
#include <memory>
#include <string>

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
        auto inner_pool = TopicPayloadPool::get(config);
        return std::make_shared<TopicPayloadPoolProxy>(topic_name, inner_pool);
    }

    void release(
            std::shared_ptr<TopicPayloadPoolProxy>& pool)
    {
        pool.reset();
    }

};

}  // namespace detail
}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOLREGISTRY_IMPL_TOPICPAYLOADPOOLREGISTRY_HPP
