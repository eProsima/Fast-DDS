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
 * @file DynamicReusable.hpp
 */

#ifndef RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_DYNAMIC_REUSABLE_HPP
#define RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_DYNAMIC_REUSABLE_HPP

#include <rtps/history/TopicPayloadPool.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class DynamicReusableTopicPayloadPool : public TopicPayloadPool
{
    bool get_payload(
            uint32_t size,
            CacheChange_t& cache_change) override
    {
        return (size > 0u) && do_get_payload(size, cache_change, true);
    }

protected:

    MemoryManagementPolicy_t memory_policy() const override
    {
        return DYNAMIC_REUSABLE_MEMORY_MODE;
    }

};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_TOPICPAYLOADPOOLIMPL_DYNAMIC_REUSABLE_HPP
