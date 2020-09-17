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

#ifndef RTPS_HISTORY_BASICPAYLOADPOOL_HPP
#define RTPS_HISTORY_BASICPAYLOADPOOL_HPP

#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/history/IPayloadPool.h>
#include <fastdds/rtps/history/PoolConfig.h>

#include <memory>

namespace eprosima {
namespace fastrtps {
namespace rtps {

namespace BasicPayloadPool {

#include "./BasicPayloadPool_impl/Base.hpp"

#include "./BasicPayloadPool_impl/Dynamic.hpp"
#include "./BasicPayloadPool_impl/DynamicReusable.hpp"
#include "./BasicPayloadPool_impl/Preallocated.hpp"
#include "./BasicPayloadPool_impl/PreallocatedWithRealloc.hpp"

namespace {

struct DefaultSizeGrowCalculator
{
    uint32_t operator () (
            uint32_t current_size,
            uint32_t /*max_size*/) const
    {
        return (current_size / 10u) + 10u;
    }

};

} // namespace

template <class SizeGrowCalculator = DefaultSizeGrowCalculator>
std::shared_ptr<IPayloadPool> get(
        PoolConfig config)
{
    switch (config.memory_policy)
    {
        case PREALLOCATED_MEMORY_MODE:
            return std::make_shared<Impl<SizeGrowCalculator, PREALLOCATED_MEMORY_MODE> >(config.payload_initial_size);
        case PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
            return std::make_shared<Impl<SizeGrowCalculator, PREALLOCATED_WITH_REALLOC_MEMORY_MODE> >(config.payload_initial_size);
        case DYNAMIC_RESERVE_MEMORY_MODE:
            return std::make_shared<Impl<SizeGrowCalculator, DYNAMIC_RESERVE_MEMORY_MODE> >();
        case DYNAMIC_REUSABLE_MEMORY_MODE:
            return std::make_shared<Impl<SizeGrowCalculator, DYNAMIC_REUSABLE_MEMORY_MODE> >();
    }

    return nullptr;
}

}  // namespace BasicPayloadPool

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif  // RTPS_HISTORY_BASICPAYLOADPOOL_HPP
