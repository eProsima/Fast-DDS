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

#ifndef RTPS_HISTORY_TOPICPAYLOADPOOLREGISTRY_HPP
#define RTPS_HISTORY_TOPICPAYLOADPOOLREGISTRY_HPP

#include <rtps/history/ITopicPayloadPool.h>

#include <rtps/history/PoolConfig.h>

#include <memory>
#include <string>

namespace eprosima {
namespace fastdds {
namespace rtps {

namespace detail {
class TopicPayloadPoolRegistry;
} // namespace detail

class TopicPayloadPoolRegistry
{

public:

    using reference = std::shared_ptr<detail::TopicPayloadPoolRegistry>;

    static const reference& instance();

    static std::shared_ptr<ITopicPayloadPool> get(
            const std::string& topic_name,
            const BasicPoolConfig& config);
};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOLREGISTRY_HPP
