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
 * @file TopicPayloadPoolRegistry.cpp
 */

#include <rtps/history/TopicPayloadPoolRegistry.hpp>

#include <rtps/history/TopicPayloadPool.hpp>

#include <rtps/history/TopicPayloadPoolRegistry_impl/TopicPayloadPoolProxy.hpp>
#include <rtps/history/TopicPayloadPoolRegistry_impl/TopicPayloadPoolRegistryEntry.hpp>
#include <rtps/history/TopicPayloadPoolRegistry_impl/TopicPayloadPoolRegistry.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

const TopicPayloadPoolRegistry::reference& TopicPayloadPoolRegistry::instance()
{
    return detail::TopicPayloadPoolRegistry::instance();
}

std::shared_ptr<ITopicPayloadPool> TopicPayloadPoolRegistry::get(
        const std::string& topic_name,
        const BasicPoolConfig& config)
{
    return detail::TopicPayloadPoolRegistry::instance()->get(topic_name, config);
}

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima
