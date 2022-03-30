// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ReaderFilterCollection.hpp
 */

#ifndef _FASTDDS_PUBLISHER_FILTERING_READERFILTERCOLLECTION_HPP_
#define _FASTDDS_PUBLISHER_FILTERING_READERFILTERCOLLECTION_HPP_

#include <fastdds/rtps/common/Guid.h>

#include <fastrtps/utils/collections/ResourceLimitedContainerConfig.hpp>

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>

#include <fastdds/publisher/filtering/ReaderFilterInformation.hpp>

#include <utils/collections/node_size_helpers.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class ReaderFilterCollection
{
    using reader_filter_map_helper =
            utilities::collections::map_size_helper<fastrtps::rtps::GUID_t, ReaderFilterInformation>;

public:

    explicit ReaderFilterCollection(
            const fastrtps::ResourceLimitedContainerConfig& allocation)
        : reader_filter_allocator_(
            reader_filter_map_helper::node_size,
            reader_filter_map_helper::min_pool_size<pool_allocator_t>(allocation.initial))
        , reader_filters_(reader_filter_allocator_)
    {
    }

    void remove_reader(
            const fastrtps::rtps::GUID_t& guid)
    {
        reader_filters_.erase(guid);
    }

private:

    using pool_allocator_t =
            foonathan::memory::memory_pool<foonathan::memory::node_pool, foonathan::memory::heap_allocator>;

    pool_allocator_t reader_filter_allocator_;

    foonathan::memory::map<fastrtps::rtps::GUID_t, ReaderFilterInformation, pool_allocator_t> reader_filters_;

};

}  // namespace dds
}  // namespace fastdds
}  // namespace eprosima

#endif  //_FASTDDS_PUBLISHER_FILTERING_READERFILTERCOLLECTION_HPP_
