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
 * @file LivelinessLostStatus.h
*/

#ifndef _LIVELINESS_LOST_STATUS_H_
#define _LIVELINESS_LOST_STATUS_H_

#include <cstdint>

namespace eprosima {
namespace fastrtps {

//! @brief A struct storing the liveliness lost status
struct LivelinessLostStatus
{
    //! @brief Total cumulative number of times that a previously-alive publisher became not alive due to a
    //! failure to actively signal its liveliness within its offered liveliness period. This count does not
    //! change when an already not alive publisher remains not alive for another liveliness period
    uint32_t total_count = 0;

    //! @brief The change in total_count since the last time the listener was called or the status was read
    uint32_t total_count_change = 0;
};

} //end of namespace
} //end of namespace eprosima

#endif /* _LIVELINESS_LOST_STATUS_H_ */
