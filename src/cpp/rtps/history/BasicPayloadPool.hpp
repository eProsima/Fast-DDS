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
 * @file BasicPayloadPool.hpp
 */

#ifndef RTPS_HISTORY_BASICMEMORYPOOL_HPP
#define RTPS_HISTORY_BASICMEMORYPOOL_HPP

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/rtps/resources/ResourceManagement.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class BasicPayloadPool : public IPayloadPool
{
public:
    explicit BasicPayloadPool(
            MemoryManagementPolicy_t policy)
        : policy_(policy)
    {
    }

    bool get_payload(
            const std::function<uint32_t()>& size_function,
            CacheChange_t& cache_change) override
    {
        if (policy_ != MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE)
        {
            cache_change.serializedPayload.reserve(size_function());
        }

        return true;
    }

    bool get_payload(
            const SerializedPayload_t& data,
            CacheChange_t& cache_change) override
    {
        cache_change.serializedPayload.copy(&data, policy_ == MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE);
        return true;
    }

    bool release_payload(
            CacheChange_t& cache_change) override
    {
        if (policy_ == MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE)
        {
            cache_change.serializedPayload.empty();
        }
        return true;
    }

private:
    MemoryManagementPolicy_t policy_;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif  // RTPS_HISTORY_BASICMEMORYPOOL_HPP
