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
 * @file DataWriterInstance.hpp
 */

#ifndef _FASTDDS_PUBLISHER_HISTORY_DATAWRITERINSTANCE_HPP_
#define _FASTDDS_PUBLISHER_HISTORY_DATAWRITERINSTANCE_HPP_

#include <chrono>

#include <cstddef>
#include <fastdds/rtps/common/CacheChange.h>
#include <fastdds/rtps/common/ChangeKind_t.hpp>
#include <fastdds/rtps/common/SerializedPayload.h>

#include <utils/constructor_macros.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

/// Book-keeping information for an instance
struct DataWriterInstance
{
    //! A vector of cache changes
    std::vector<fastrtps::rtps::CacheChange_t*> cache_changes;
    //! The time when the group will miss the deadline
    std::chrono::steady_clock::time_point next_deadline_us;
    //! Serialized payload for key holder
    fastrtps::rtps::SerializedPayload_t key_payload;

    DataWriterInstance() = default;
    DataWriterInstance(
            size_t preallocated_changes)
    {
        cache_changes.reserve(preallocated_changes);
    }

    FASTDDS_DELETED_COPY(DataWriterInstance);
    FASTDDS_DEFAULT_MOVE(DataWriterInstance);

    bool is_registered() const
    {
        return cache_changes.empty() ||
               (fastrtps::rtps::NOT_ALIVE_UNREGISTERED != cache_changes.back()->kind &&
               fastrtps::rtps::NOT_ALIVE_DISPOSED_UNREGISTERED != cache_changes.back()->kind);
    }

    void clear()
    {
        cache_changes.clear();
        next_deadline_us = std::chrono::steady_clock::time_point();
        key_payload.empty();
    }

};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_PUBLISHER_HISTORY_DATAWRITERINSTANCE_HPP_
