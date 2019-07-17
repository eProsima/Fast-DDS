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
 * @file SampleRejectedStatus.hpp
*/

#ifndef _FASTRTPS_SAMPLE_REJECTED_STATUS_HPP_
#define _FASTRTPS_SAMPLE_REJECTED_STATUS_HPP_

#include <cstdint>
#include <fastrtps/rtps/common/InstanceHandle.h>

namespace eprosima {
namespace fastrtps {

enum SampleRejectedStatusKind {
    NOT_REJECTED,
    REJECTED_BY_INSTANCES_LIMIT,
    REJECTED_BY_SAMPLES_LIMIT,
    REJECTED_BY_SAMPELS_PER_INSTANCE_LIMIT
};

//! @brief A struct storing the sample lost status
struct SampleRejectedStatus
{
    /**
     * Total cumulative count of samples rejected by the DataReader.
     */
    uint32_t total_count = 0;

    /**
     * The incremental number of samples rejected since the last time the listener was called or the status was read.
     */
    uint32_t total_count_change = 0;

    /**
     * Reason for rejecting the last sample rejected.
     * If no samples have been rejected, the reason is the special value NOT_REJECTED.
     */
    SampleRejectedStatusKind last_reason;

    /**
     * Handle to the instance being updated by the last sample that was rejected.
     */
    rtps::InstanceHandle_t last_instance_handle;
};

} //end of namespace fastrtps
} //end of namespace eprosima

#endif // _FASTRTPS_SAMPLE_REJECTED_STATUS_HPP_
