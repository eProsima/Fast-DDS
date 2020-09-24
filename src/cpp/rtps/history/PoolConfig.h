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
 * @file PoolConfig.h
 */

#ifndef RTPS_HISTORY_POOLCONFIG_H_
#define RTPS_HISTORY_POOLCONFIG_H_

#include <fastdds/rtps/attributes/HistoryAttributes.h>
#include <fastdds/rtps/resources/ResourceManagement.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

struct PoolConfig
{
    //! Memory management policy.
    MemoryManagementPolicy_t memory_policy;

    //! Payload size when preallocating data.
    uint32_t payload_initial_size;

    //! Initial number of elements when preallocating data.
    uint32_t initial_size;

    //! Maximum number of elements in the pool. Default value is 0, indicating to make allocations until they fail.
    uint32_t maximum_size;

    /**
     * Transform a HistoryAttributes object into a PoolConfig
     *
     * @param [in] history_attr HistoryAttributes to be transformed
     *
     * @return equivalent PoolConfig object
     */
    static constexpr PoolConfig from_history_attributes(
            const HistoryAttributes& history_attr) noexcept
    {
        int32_t max_caches = std::max(history_attr.maximumReservedCaches, 0);
        int32_t initial_caches = std::max(history_attr.initialReservedCaches, 0);

        return
            {
                history_attr.memoryPolicy,
                history_attr.payloadMaxSize,
                static_cast<uint32_t>(initial_caches),
                static_cast<uint32_t>(max_caches)
            };
    }

};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* RTPS_HISTORY_POOLCONFIG_H_ */
