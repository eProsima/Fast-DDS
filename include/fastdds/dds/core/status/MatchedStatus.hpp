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
 * @file MatchedStatus.hpp
 */

#ifndef FASTDDS_DDS_CORE_STATUS__MATCHEDSTATUS_HPP
#define FASTDDS_DDS_CORE_STATUS__MATCHEDSTATUS_HPP

#include <cstdint>

namespace eprosima {
namespace fastdds {
namespace dds {

//! @brief A structure storing a matching status
struct MatchedStatus
{
    //! @brief Constructor
    MatchedStatus() = default;

    //! @brief Destructor
    ~MatchedStatus() = default;

    //! @brief Total cumulative count the concerned reader discovered a match with a writer
    //! @details It found a writer for the same topic with a requested QoS that is compatible with that offered by the reader
    int32_t total_count = 0;

    //! @brief The change in total_count since the last time the listener was called or the status was read
    int32_t total_count_change = 0;

    //! @brief The number of writers currently matched to the concerned reader
    int32_t current_count = 0;

    //! @brief The change in current_count since the last time the listener was called or the status was read
    int32_t current_count_change = 0;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif //FASTDDS_DDS_CORE_STATUS__MATCHEDSTATUS_HPP
