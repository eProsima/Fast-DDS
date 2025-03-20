// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ReaderResourceLimitsQos.hpp
 */

#ifndef FASTDDS_DDS_CORE_POLICY__READERRESOURCELIMITSQOS_HPP
#define FASTDDS_DDS_CORE_POLICY__READERRESOURCELIMITSQOS_HPP


#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

//! Qos Policy to configure the limit of the reader resources
class ReaderResourceLimitsQos
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API ReaderResourceLimitsQos() = default;

    /**
     * @brief Destructor
     */
    virtual FASTDDS_EXPORTED_API ~ReaderResourceLimitsQos() = default;

    bool operator ==(
            const ReaderResourceLimitsQos& b) const
    {
        return
            (matched_publisher_allocation == b.matched_publisher_allocation) &&
            (sample_infos_allocation == b.sample_infos_allocation) &&
            (outstanding_reads_allocation == b.outstanding_reads_allocation) &&
            (max_samples_per_read == b.max_samples_per_read);
    }

    inline void clear()
    {
        ReaderResourceLimitsQos reset = ReaderResourceLimitsQos();
        std::swap(*this, reset);
    }

    //! Matched publishers allocation limits.
    fastdds::ResourceLimitedContainerConfig matched_publisher_allocation;
    //! SampleInfo allocation limits.
    fastdds::ResourceLimitedContainerConfig sample_infos_allocation{ 32u };
    //! Loaned collections allocation limits.
    fastdds::ResourceLimitedContainerConfig outstanding_reads_allocation{ 2u };

    /**
     * Maximum number of samples to return on a single call to read / take.
     *
     * This attribute is a signed integer to be consistent with the @c max_samples argument of
     * @ref DataReader methods, but should always have a strict positive value. Bear in mind that
     * a big number here may cause the creation of the DataReader to fail due to pre-allocation of
     * internal resources.
     *
     * Default value: 32.
     */
    int32_t max_samples_per_read = 32;
};

} //namespace dds
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_DDS_CORE_POLICY__READERRESOURCELIMITSQOS_HPP
