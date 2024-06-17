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

#ifndef FASTDDS_DDS_CORE_STATUS__SAMPLEREJECTEDSTATUS_HPP
#define FASTDDS_DDS_CORE_STATUS__SAMPLEREJECTEDSTATUS_HPP

#include <cstdint>

#include <fastdds/dds/common/InstanceHandle.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

//! An enum with the possible values for the sample rejected reason
enum SampleRejectedStatusKind
{
    //!Default value
    NOT_REJECTED,
    //! Exceeds the max_instance limit
    REJECTED_BY_INSTANCES_LIMIT,
    //! Exceeds the max_samples limit
    REJECTED_BY_SAMPLES_LIMIT,
    //! Exceeds the max_samples_per_instance limit
    REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT
};

//! @brief A struct storing the sample rejected status
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
    SampleRejectedStatusKind last_reason = NOT_REJECTED;

    /**
     * Handle to the instance being updated by the last sample that was rejected.
     */
    InstanceHandle_t last_instance_handle;
};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_DDS_CORE_STATUS__SAMPLEREJECTEDSTATUS_HPP
