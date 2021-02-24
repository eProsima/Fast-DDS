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
 * @file DeadlineMissedStatus.hpp
 */

#ifndef _FASTDDS_DDS_QOS_DEADLINEMISSEDSTATUS_HPP_
#define _FASTDDS_DDS_QOS_DEADLINEMISSEDSTATUS_HPP_

#include <fastdds/dds/topic/TypeSupport.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

//! @brief A struct storing the deadline status
//! @ingroup DEADLINE_MODULE
struct DeadlineMissedStatus
{
    //! @brief Constructor
    DeadlineMissedStatus()
        : total_count()
        , total_count_change()
        , last_instance_handle()
    {
    }

    //! @brief Destructor
    ~DeadlineMissedStatus()
    {
    }

    //! @brief Total cumulative number of offered deadline periods elapsed during which a writer failed to provide data
    //! @details Missed deadlines accumulate, that is, each deadline period the total_count will be incremented by 1
    uint32_t total_count;

    //! @brief The change in total_count since the last time the listener was called or the status was read
    uint32_t total_count_change;

    //! @brief Handle to the last instance missing the deadline
    InstanceHandle_t last_instance_handle;
};

//! Typedef of DeadlineMissedStatus
typedef DeadlineMissedStatus OfferedDeadlineMissedStatus;
//! Typedef of DeadlineMissedStatus
typedef DeadlineMissedStatus RequestedDeadlineMissedStatus;

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // _FASTDDS_DDS_QOS_DEADLINEMISSEDSTATUS_HPP_
