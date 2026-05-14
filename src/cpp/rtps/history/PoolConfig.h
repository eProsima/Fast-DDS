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

#include <cstdint>
#include <limits>

#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/attributes/ResourceManagement.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct BasicPoolConfig
{
    //! Memory management policy.
    MemoryManagementPolicy_t memory_policy;

    //! Payload size when preallocating data.
    uint32_t payload_initial_size;
};

struct PoolConfig : public BasicPoolConfig
{
    PoolConfig() = default;

    constexpr PoolConfig(
            MemoryManagementPolicy_t policy,
            uint32_t payload_size,
            uint32_t ini_size,
            uint32_t max_size) noexcept
        : BasicPoolConfig {policy, payload_size}
        , initial_size(ini_size)
        , maximum_size(max_size)
    {
    }

    //! Initial number of elements when preallocating data.
    uint32_t initial_size;

    //! Maximum number of elements in the pool. Default value is 0, indicating to make allocations until they fail.
    uint32_t maximum_size;

    /**
     * Compute reserved + extra in int64_t and clamp to UINT32_MAX, avoiding the signed overflow
     * A reserved value <= 0 means "no preallocation" (initial) or "infinite maximum" (max)
     *
     * @param reserved initial or maximum reserved caches
     * @param extra extra caches to be added to the reserved caches
     * @return the sum of reserved and extra, clamped to UINT32_MAX, or 0 if reserved is <= 0
     */
    static constexpr uint32_t clamped_reserved_plus_extra(
            int32_t reserved,
            int32_t extra) noexcept
    {
        return reserved <= 0 ? 0u :
               (static_cast<int64_t>(reserved) + extra >=
               static_cast<int64_t>((std::numeric_limits<uint32_t>::max)()) ? (std::numeric_limits<uint32_t>::max)() :
               static_cast<uint32_t>(static_cast<int64_t>(reserved) + extra));
    }

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
        return
            {
                history_attr.memoryPolicy,
                history_attr.payloadMaxSize,
                clamped_reserved_plus_extra(
                    history_attr.initialReservedCaches, history_attr.extraReservedCaches),
                clamped_reserved_plus_extra(
                    history_attr.maximumReservedCaches, history_attr.extraReservedCaches)
            };
    }

};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* RTPS_HISTORY_POOLCONFIG_H_ */
