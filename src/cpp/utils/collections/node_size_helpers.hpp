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
 * @file node_size_helpers.hpp
 */

#ifndef SRC_CPP_UTILS_COLLECTIONS_NODE_SIZE_HELPERS_HPP_
#define SRC_CPP_UTILS_COLLECTIONS_NODE_SIZE_HELPERS_HPP_

#include <foonathan/memory/memory_pool.hpp>
#include <foonathan/memory/detail/debug_helpers.hpp>

#include <map>
#include <set>
#include <unordered_map>

namespace eprosima {
namespace utilities {
namespace collections {

namespace detail {

namespace fm = foonathan::memory;

// TODO(Miguel C): In the future we should have CMake automatically define these.
// Right now using foonathan debug helper and falling back to custom one if
// FOONATHAN_MEMORY_NO_NODE_SIZE is defined.
#define USE_FOONATHAN_NODE_SIZES
#define USE_CUSTOM_NODE_SIZES

// Include implementations for node size helpers
#include "impl/node-sizes/map_node_size_impl.hpp"
#include "impl/node-sizes/set_node_size_impl.hpp"
#include "impl/node-sizes/unordered_map_node_size_impl.hpp"

/**
 * @brief Common base for all size helpers.
 * Gives all necessary information to use a memory_pool with a node-based collection.
 *
 * @tparam node_size_value   Estimated size of native node
 */
template<size_t node_size_value>
struct pool_size_helper
{
    /**
     * Estimated size of native node.
     * This is the value to be used as first parameter on the memory_pool constructor.
     */
    static constexpr size_t node_size = node_size_value;

    /**
     * Pool size required to store a certain number of nodes.
     * This is the value to be used as first parameter on the memory_pool constructor.
     */
    template<typename Pool>
    static constexpr size_t min_pool_size(
            size_t num_nodes)
    {
        return
            // Book-keeping area for a block in the memory arena
            fm::detail::memory_block_stack::implementation_offset +
            // At least one node
            (num_nodes ? num_nodes : 1) * min_size_per_node<Pool>();
    }

private:

    template<typename Pool>
    static constexpr size_t min_size_per_node()
    {
        // Node size with minimum, plus debug space
        return
            additional_size_per_node +
            ( (node_size > Pool::min_node_size) ? node_size : Pool::min_node_size);
    }

    static constexpr size_t additional_size_per_node =
            fm::detail::debug_fence_size ? 2 * fm::detail::max_alignment : 0;
};

} // namespace detail


template<typename K, typename V>
struct map_size_helper : public detail::pool_size_helper<detail::map_node_size<K, V>::value>
{
};

template<typename K>
struct set_size_helper : public detail::pool_size_helper<detail::set_node_size<K>::value>
{
};

template<typename K, typename V>
struct unordered_map_size_helper : public detail::pool_size_helper<detail::unordered_map_node_size<K, V>::value>
{
};

} // namespace collections
} // namespace utilities
} // namespace eprosima

#endif  /* SRC_CPP_UTILS_COLLECTIONS_NODE_SIZE_HELPERS_HPP_ */