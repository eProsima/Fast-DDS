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
 * @file ProxyHashTables.hpp
 *
 */

#ifndef _FASTDDS_RTPS_BUILTIN_DATA_PROXYHASHTABLES_HPP_
#define _FASTDDS_RTPS_BUILTIN_DATA_PROXYHASHTABLES_HPP_

#include <fastdds/rtps/common/Guid.hpp>

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>
#include <foonathan/memory/segregator.hpp>

#include "utils/collections/node_size_helpers.hpp"

#include <unordered_map>

namespace eprosima {
namespace fastdds {
namespace rtps {

// Static allocation ancillary for proxies

namespace detail {

template<
    std::size_t node_size,
    class RawAllocator = foonathan::memory::default_allocator>
class node_segregator
{
public:

    using allocator_type = foonathan::memory::memory_pool<foonathan::memory::node_pool, RawAllocator>;
    using size_helper = utilities::collections::detail::pool_size_helper<node_size>;

    node_segregator(
            std::size_t nodes_to_allocate,
            const bool& flag)
        : block_size_(size_helper::template min_pool_size<allocator_type>(nodes_to_allocate))
        , node_allocator_(new allocator_type(node_size, block_size_))
        , initialization_is_done_(flag)
    {
    }

    node_segregator(
            node_segregator&& s)
        : block_size_(s.block_size_)
        , node_allocator_(s.node_allocator_)
        , initialization_is_done_(s.initialization_is_done_)
    {
        s.node_allocator_ = nullptr;
    }

    node_segregator(
            const node_segregator& s)
        : block_size_(s.block_size_)
        , node_allocator_(new allocator_type(node_size, s.block_size_))
        , initialization_is_done_(s.initialization_is_done_)
    {
    }

    ~node_segregator()
    {
        if (node_allocator_)
        {
            delete node_allocator_;
        }
    }

    allocator_type& get_allocator() const noexcept
    {
        return *node_allocator_;
    }

    bool use_allocate_node(
            std::size_t size,
            std::size_t) noexcept
    {
        return initialization_is_done_ && size == node_size;
    }

    bool use_allocate_array(
            std::size_t,
            std::size_t,
            std::size_t) noexcept
    {
        return false;
    }

private:

    std::size_t block_size_ = 0;
    allocator_type* node_allocator_ = nullptr;
    const bool& initialization_is_done_;
};

template<
    std::size_t node_size,
    class RawAllocator = foonathan::memory::new_allocator>
class binary_node_segregator
    : public foonathan::memory::binary_segregator<node_segregator<node_size>, RawAllocator>
{
public:

    using segregator = node_segregator<node_size>;
    using base_class = foonathan::memory::binary_segregator<segregator, RawAllocator>;
    using is_stateful = std::true_type;
    using is_shared = std::false_type;

    binary_node_segregator(
            std::size_t nodes_to_allocate)
        : base_class(segregator(nodes_to_allocate, initialized_), RawAllocator())
        , initialized_(false)
    {
    }

    void has_been_initialized()
    {
        initialized_ = true;
    }

    void is_being_destroyed()
    {
        initialized_ = false;
    }

private:

    bool initialized_;
};

} // namespace detail

template<class Proxy>
class ProxyHashTable
    : protected detail::binary_node_segregator<
        utilities::collections::unordered_map_size_helper<EntityId_t, Proxy*>::node_size>
    , public foonathan::memory::unordered_map<
        EntityId_t,
        Proxy*,
        detail::binary_node_segregator<
            utilities::collections::unordered_map_size_helper<EntityId_t, Proxy*>::node_size>
        >
{
public:

    using allocator_type = detail::binary_node_segregator<
        utilities::collections::unordered_map_size_helper<EntityId_t, Proxy*>::node_size>;
    using base_class = foonathan::memory::unordered_map<EntityId_t, Proxy*, allocator_type>;

    explicit ProxyHashTable(
            const ResourceLimitedContainerConfig& r)
        : allocator_type(r.initial ? r.initial : 1u)
        , base_class(
            r.initial ? r.initial : 1u,
            std::hash<EntityId_t>(),
            std::equal_to<EntityId_t>(),
            *static_cast<allocator_type*>(this))
    {
        // notify the pool that fixed allocations may start
        allocator_type::has_been_initialized();
    }

    ~ProxyHashTable()
    {
        base_class::clear();
        allocator_type::is_being_destroyed();
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_RTPS_BUILTIN_DATA_PROXYHASHTABLES_HPP_
