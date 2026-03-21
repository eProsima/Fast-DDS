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

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>

#include <rtps/history/CacheChangePool.h>
#include <rtps/history/PoolConfig.h>

#include <memory>

namespace eprosima {
namespace fastdds {
namespace rtps {

namespace detail {
#include "./BasicPayloadPool_impl/Base.hpp"

#include "./BasicPayloadPool_impl/Dynamic.hpp"
#include "./BasicPayloadPool_impl/DynamicReusable.hpp"
#include "./BasicPayloadPool_impl/Preallocated.hpp"
#include "./BasicPayloadPool_impl/PreallocatedWithRealloc.hpp"
}  // namespace detail

class BasicPayloadPool
{

public:

    static std::shared_ptr<IPayloadPool> get(
            const PoolConfig& config,
            std::shared_ptr<IChangePool>& change_pool)
    {
        auto payload_pool = get(config);
        if (payload_pool)
        {
            if ((PREALLOCATED_MEMORY_MODE == config.memory_policy) ||
                    (PREALLOCATED_WITH_REALLOC_MEMORY_MODE == config.memory_policy))
            {
                change_pool = std::make_shared<CacheChangePool>(config,
                                [&payload_pool, &config](
                                    CacheChange_t* change)
                                {
                                    if (payload_pool->get_payload(config.payload_initial_size,
                                    change->serializedPayload))
                                    {
                                        payload_pool->release_payload(change->serializedPayload);
                                    }
                                });
            }
            else
            {
                change_pool = std::make_shared<CacheChangePool>(config);
            }
        }

        return payload_pool;
    }

private:

    static std::shared_ptr<IPayloadPool> get(
            const BasicPoolConfig& config)
    {
        if (config.payload_initial_size == 0)
        {
            return nullptr;
        }

        switch (config.memory_policy)
        {
            case PREALLOCATED_MEMORY_MODE:
                return std::make_shared<detail::Impl<PREALLOCATED_MEMORY_MODE>>(config.payload_initial_size);
            case PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
                return std::make_shared<detail::Impl<PREALLOCATED_WITH_REALLOC_MEMORY_MODE>>(
                    config.payload_initial_size);
            case DYNAMIC_RESERVE_MEMORY_MODE:
                return std::make_shared<detail::Impl<DYNAMIC_RESERVE_MEMORY_MODE>>();
            case DYNAMIC_REUSABLE_MEMORY_MODE:
                return std::make_shared<detail::Impl<DYNAMIC_REUSABLE_MEMORY_MODE>>();
        }

        return nullptr;
    }

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif  // RTPS_HISTORY_BASICPAYLOADPOOL_HPP
