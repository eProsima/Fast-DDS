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
 * @file ReaderHistoryState.hpp
 */

#ifndef FASTDDS_RTPS_READER_READERHISTORYSTATE_HPP_
#define FASTDDS_RTPS_READER_READERHISTORYSTATE_HPP_

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>

#include "utils/collections/node_size_helpers.hpp"

#include <map>

namespace eprosima {
namespace fastdds {
namespace rtps {

using guid_map_helper = utilities::collections::map_size_helper<GUID_t, GUID_t>;
using guid_count_helper = utilities::collections::map_size_helper<GUID_t, uint16_t>;
using history_record_helper = utilities::collections::map_size_helper<GUID_t, SequenceNumber_t>;

/**
 * Class RTPSReader, manages the reception of data from its matched writers.
 * @ingroup READER_MODULE
 */
struct ReaderHistoryState
{
    using pool_allocator_t =
            foonathan::memory::memory_pool<foonathan::memory::node_pool, foonathan::memory::heap_allocator>;

    explicit ReaderHistoryState(
            size_t initial_writers_allocation)
        : persistence_guid_map_allocator(
            guid_map_helper::node_size,
            guid_map_helper::min_pool_size<pool_allocator_t>(initial_writers_allocation))
        , persistence_guid_count_allocator(
            guid_count_helper::node_size,
            guid_count_helper::min_pool_size<pool_allocator_t>(initial_writers_allocation))
        , history_record_allocator(
            history_record_helper::node_size,
            history_record_helper::min_pool_size<pool_allocator_t>(initial_writers_allocation))
        , persistence_guid_map(persistence_guid_map_allocator)
        , persistence_guid_count(persistence_guid_count_allocator)
        , history_record(history_record_allocator)
    {
    }

    pool_allocator_t persistence_guid_map_allocator;
    pool_allocator_t persistence_guid_count_allocator;
    pool_allocator_t history_record_allocator;

    //!Physical GUID to persistence GUID map
    foonathan::memory::map<GUID_t, GUID_t, pool_allocator_t> persistence_guid_map;
    //!Persistence GUID count map
    foonathan::memory::map<GUID_t, uint16_t, pool_allocator_t> persistence_guid_count;
    //!Information about max notified change
    foonathan::memory::map<GUID_t, SequenceNumber_t, pool_allocator_t> history_record;
};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* FASTDDS_RTPS_READER_READERHISTORYSTATE_HPP_ */
