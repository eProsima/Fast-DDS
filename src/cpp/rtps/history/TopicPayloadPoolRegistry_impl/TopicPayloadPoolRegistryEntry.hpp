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
 * @file TopicPayloadPoolRegistryEntry.hpp
 */

#ifndef RTPS_HISTORY_TOPICPAYLOADPOOLREGISTRY_IMPL_TOPICPAYLOADPOOLREGISTRYENTRY_HPP
#define RTPS_HISTORY_TOPICPAYLOADPOOLREGISTRY_IMPL_TOPICPAYLOADPOOLREGISTRYENTRY_HPP

#include <memory>
#include <string>

namespace eprosima {
namespace fastdds {
namespace rtps {
namespace detail {

struct TopicPayloadPoolRegistryEntry
{
    std::weak_ptr<TopicPayloadPoolProxy> pool_for_preallocated;
    std::weak_ptr<TopicPayloadPoolProxy> pool_for_preallocated_realloc;
    std::weak_ptr<TopicPayloadPoolProxy> pool_for_dynamic;
    std::weak_ptr<TopicPayloadPoolProxy> pool_for_dynamic_reusable;
};

}  // namespace detail
}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOLREGISTRY_IMPL_TOPICPAYLOADPOOLREGISTRYENTRY_HPP
