// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file foonathan_memory_helpers.hpp
 *
 */

#ifndef FASTRTPS_UTILS_COLLECTIONS_FOONATHAN_MEMORY_HELPERS_HPP_
#define FASTRTPS_UTILS_COLLECTIONS_FOONATHAN_MEMORY_HELPERS_HPP_

#include <foonathan/memory/memory_pool.hpp>
#include <foonathan/memory/detail/debug_helpers.hpp>

#include "ResourceLimitedContainerConfig.hpp"
#include "fastrtps/config.h"

namespace eprosima {
namespace fastrtps {

/**
 * A helper to calculate the block size of a memory pool given the size of the node and
 * a resource limits configuration.
 *
 * @tparam MemoryPool memory_pool specialization
 *
 * @param node_size   Size of the node for the memory pool
 * @param limits      Resource limits configuration of the container
 *
 * @return the block size to pass to the memory pool constructor
 */
template <typename MemoryPool>
std::size_t memory_pool_block_size(
        std::size_t node_size,
        const ResourceLimitedContainerConfig& limits)
{
    FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastrtps::memory_pool_block_size", "You should not use this method")
    namespace fm = foonathan::memory;

    size_t num_elems = limits.increment > 0 ? limits.initial : limits.maximum;
    if (num_elems < 1u)
    {
        num_elems = 1u;
    }

    return num_elems
           * ((node_size > MemoryPool::min_node_size ? node_size : MemoryPool::min_node_size) // Room for elements
#if FOONATHAN_MEMORY_DEBUG_DOUBLE_DEALLOC_CHECK
           * (fm::detail::debug_fence_size ? 3 : 1))                                          // Room for debug info
#else
           + (fm::detail::debug_fence_size ? 2 * fm::detail::max_alignment : 0))              // Room for debug info
#endif // if FOONATHAN_MEMORY_DEBUG_DOUBLE_DEALLOC_CHECK
#ifdef FOONATHAN_MEMORY_MEMORY_POOL_HAS_MIN_BLOCK_SIZE
           + fm::detail::memory_block_stack::implementation_offset();                         // Room for padding
#else
           + fm::detail::memory_block_stack::implementation_offset;                           // Room for padding
#endif  // FOONATHAN_MEMORY_MEMORY_POOL_HAS_MIN_BLOCK_SIZE
}

}  // namespace fastrtps
}  // namespace eprosima

#endif /* FASTRTPS_UTILS_COLLECTIONS_FOONATHAN_MEMORY_HELPERS_HPP_ */
