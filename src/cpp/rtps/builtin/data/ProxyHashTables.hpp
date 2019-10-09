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

#include <fastrtps/utils/collections/foonathan_memory_helpers.hpp>

#include <unordered_map>

namespace eprosima {
namespace fastrtps {
namespace rtps {

// Static allocation ancillary for proxies
using pool_allocator_t =
    foonathan::memory::memory_pool<foonathan::memory::node_pool, foonathan::memory::heap_allocator>;

template<class Proxy>
constexpr size_t guid_proxy_map_node_size =
    foonathan::memory::unordered_map_node_size<typename ProxyHashTable<Proxy>::value_type>::value;

template<class Proxy>
class ProxyHashTable : public pool_allocator_t, public foonathan::memory::unordered_map<EntityId_t, Proxy*, pool_allocator_t>
{
public:
    using allocator_type = pool_allocator_t;
    using base_class = foonathan::memory::unordered_map<EntityId_t, Proxy*, allocator_type >;

    explicit ProxyHashTable(const ResourceLimitedContainerConfig &r)
        : pool_allocator_t(guid_proxy_map_node_size<Proxy>, memory_pool_block_size<pool_allocator_t>(guid_proxy_map_node_size<Proxy>, r))
        , base_class(*static_cast<pool_allocator_t*>(this))
    {
    }
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_BUILTIN_DATA_PROXYHASHTABLES_HPP_