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

#include <fastdds/rtps/common/Guid.h>

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>
#include <foonathan/memory/segregator.hpp>

#include <fastrtps/utils/collections/foonathan_memory_helpers.hpp>

#include <unordered_map>

namespace eprosima {
namespace fastrtps {
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

    node_segregator(
            std::size_t nodes_to_allocate,
            const bool& flag,
            std::size_t padding = foonathan::memory::detail::memory_block_stack::implementation_offset)
        : block_size_(nodes_to_allocate * node_size + padding)
        , node_allocator_(new allocator_type(node_size, nodes_to_allocate * node_size + padding))
        , initialization_is_done_(flag)
    {}

    node_segregator(
            node_segregator&& s)
        : node_allocator_(s.node_allocator_)
        , block_size_(s.block_size_)
        , initialization_is_done_(s.initialization_is_done_)
    {
        s.node_allocator_ = nullptr;
    }

    node_segregator(
            const node_segregator& s)
        : block_size_(s.block_size_)
        , node_allocator_(new allocator_type(node_size, s.block_size_))
        , initialization_is_done_(s.initialization_is_done_)
    {}

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

    allocator_type* node_allocator_ = nullptr;
    std::size_t block_size_ = 0;
    const bool& initialization_is_done_;
};

struct ProxyCollectionInitizalizer
{
    bool initialized_;

    ProxyCollectionInitizalizer()
        : initialized_(false)
    {}

};

} // namespace detail

template<class Proxy>
class ProxyHashTable
    : public detail::ProxyCollectionInitizalizer
    , public foonathan::memory::binary_segregator<
        detail::node_segregator<foonathan::memory::unordered_map_node_size<std::pair<const EntityId_t,
        Proxy*> >::value>,
        foonathan::memory::new_allocator >
    , public foonathan::memory::unordered_map<
        EntityId_t,
        Proxy*,
        foonathan::memory::binary_segregator <
            detail::node_segregator<foonathan::memory::unordered_map_node_size<std::pair<const EntityId_t,
            Proxy*> >::value>,
            foonathan::memory::new_allocator >
        >
{
public:

    using segregator = detail::node_segregator<
        foonathan::memory::unordered_map_node_size<typename ProxyHashTable<Proxy>::value_type>::value>;
    using allocator_type = foonathan::memory::binary_segregator<segregator, foonathan::memory::new_allocator>;
    using base_class = foonathan::memory::unordered_map<EntityId_t, Proxy*, allocator_type>;

    explicit ProxyHashTable(
            const ResourceLimitedContainerConfig& r)
        : ProxyCollectionInitizalizer() // force to be initialize first
        , allocator_type(               // pool must be initialized before the unordered_map
            foonathan::memory::make_segregator(
                segregator(
                    r.initial ? r.initial : 1u,
                    initialized_),
                foonathan::memory::new_allocator()
                )
            )
        , base_class(r.initial ? r.initial : 1u, *static_cast<allocator_type*>(this))
    {
        // notify the pool that fixed allocations may start
        initialized_ = true;
    }
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_BUILTIN_DATA_PROXYHASHTABLES_HPP_