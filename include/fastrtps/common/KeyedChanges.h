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
 * @file KeyedChanges.h
 *
 */

#ifndef KEYEDCHANGES_H_
#define KEYEDCHANGES_H_

#include <fastdds/rtps/common/CacheChange.h>
#include <chrono>

namespace eprosima{
namespace fastrtps{

/**
 * @brief A struct storing a vector of cache changes and the next deadline in the group
 * @ingroup FASTRTPS_MODULE
 */
struct KeyedChanges
{
    //! Default constructor
    KeyedChanges()
        : cache_changes()
        , next_deadline_us()
    {
    }

    //! Copy constructor
    KeyedChanges(const KeyedChanges& other)
        : cache_changes(other.cache_changes)
        , next_deadline_us(other.next_deadline_us)
    {
    }

    //! Destructor
    ~KeyedChanges()
    {
    }

    //! A vector of cache changes
    std::vector<rtps::CacheChange_t*> cache_changes;
    //! The time when the group will miss the deadline
    std::chrono::steady_clock::time_point next_deadline_us;
};

} /* namespace  */
} /* namespace eprosima */

#endif /* KEYEDCHANGES_H_ */
