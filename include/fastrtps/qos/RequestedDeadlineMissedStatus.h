// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RequestedDeadlineMissedStatus.h
*/

#ifndef _REQUESTED_DEADLINE_MISSED_STATUS_H_
#define _REQUESTED_DEADLINE_MISSED_STATUS_H_

#include <fastrtps/rtps/common/InstanceHandle.h>

namespace eprosima {
namespace fastrtps {

//! @brief A struct storing the status of the requested deadline
//! @ingroup DEADLINE_MODULE
struct RequestedDeadlineMissedStatus
{
    //! @brief Constructor
    RequestedDeadlineMissedStatus()
        : total_count()
        , total_count_change()
        , last_instance_handle()
    {}

    //! @brief Destructor
    ~RequestedDeadlineMissedStatus()
    {}

    //! @brief Total cumulative number of missed deadlines detected for any instance
    //! @details Missed deadlines accumulate, that is, each deadline period the total_count will be incremented by 1 for each instance for which data was not received
    uint32_t total_count;

    //! @brief The change in total_count since the last time the listener was called or the status was read
    uint32_t total_count_change;

    //! @brief Handle to the last instance missing the deadline
    rtps::InstanceHandle_t last_instance_handle;
};
} //end of namespace
} //end of namespace eprosima

#endif /* _REQUESTED_DEADLINE_MISSED_STATUS_H_ */
