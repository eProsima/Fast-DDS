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

#include <fastdds/rtps/common/CacheChange.hpp>
#include <fastdds/rtps/common/ChangeKind_t.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>

#include <utils/constructor_macros.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace detail {

/// Book-keeping information for an instance
struct DataWriterInstance
{
    //! A vector of cache changes
    std::vector<fastdds::rtps::CacheChange_t*> cache_changes;
    //! The time when the group will miss the deadline
    std::chrono::steady_clock::time_point next_deadline_us;
    //! Serialized payload for key holder
    fastdds::rtps::SerializedPayload_t key_payload;

    DataWriterInstance() = default;

    FASTDDS_DELETED_COPY(DataWriterInstance);
    FASTDDS_DEFAULT_MOVE(DataWriterInstance);

    bool is_registered() const
    {
        return cache_changes.empty() ||
               (fastdds::rtps::NOT_ALIVE_UNREGISTERED != cache_changes.back()->kind &&
               fastdds::rtps::NOT_ALIVE_DISPOSED_UNREGISTERED != cache_changes.back()->kind);
    }

};

} /* namespace detail */
} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif  // _FASTDDS_PUBLISHER_HISTORY_DATAWRITERINSTANCE_HPP_
