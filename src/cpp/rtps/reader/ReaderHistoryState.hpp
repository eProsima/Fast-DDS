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

#ifndef FASTRTPS_RTPS_READER_READERHISTORYSTATE_HPP_
#define FASTRTPS_RTPS_READER_READERHISTORYSTATE_HPP_

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/SequenceNumber.h>

#include <foonathan/memory/container.hpp>
#include <foonathan/memory/memory_pool.hpp>

#include <fastrtps/utils/collections/foonathan_memory_helpers.hpp>

#include <map>

namespace eprosima {
namespace fastrtps {
namespace rtps {

constexpr size_t guid_map_node_size =
    foonathan::memory::map_node_size<std::pair<size_t, std::pair<GUID_t, GUID_t>>>::value;
constexpr size_t guid_count_node_size =
    foonathan::memory::map_node_size<std::pair<size_t, std::pair<GUID_t, uint16_t>>>::value;
constexpr size_t history_record_node_size =
    foonathan::memory::map_node_size<std::pair<size_t, std::pair<GUID_t, SequenceNumber_t>>>::value;

/**
 * Class RTPSReader, manages the reception of data from its matched writers.
 * @ingroup READER_MODULE
 */
struct ReaderHistoryState
{
    using pool_allocator_t =
            foonathan::memory::memory_pool<foonathan::memory::node_pool, foonathan::memory::heap_allocator>;

    ReaderHistoryState(
            size_t initial_writers_allocation)
        : persistence_guid_map_allocator(
            guid_map_node_size,
            memory_pool_block_size<pool_allocator_t>(guid_map_node_size, initial_writers_allocation))
        , persistence_guid_count_allocator(
            guid_count_node_size,
            memory_pool_block_size<pool_allocator_t>(guid_count_node_size, initial_writers_allocation))
        , history_record_allocator(
            history_record_node_size,
            memory_pool_block_size<pool_allocator_t>(history_record_node_size, initial_writers_allocation))
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
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* FASTRTPS_RTPS_READER_READERHISTORYSTATE_HPP_ */
