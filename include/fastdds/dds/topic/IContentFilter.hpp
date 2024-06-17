// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file IContentFilter.hpp
 */

#ifndef FASTDDS_DDS_TOPIC__ICONTENTFILTER_HPP
#define FASTDDS_DDS_TOPIC__ICONTENTFILTER_HPP

#include <fastdds/fastdds_dll.hpp>

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/SampleIdentity.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * The interface that content filter objects should implement.
 */
struct IContentFilter
{
    using SerializedPayload = eprosima::fastdds::rtps::SerializedPayload_t;
    using GUID_t = fastdds::rtps::GUID_t;

    /**
     * Selected information from the cache change that is passed to the content filter object on
     * payload evaluation.
     */
    struct FilterSampleInfo
    {
        using SampleIdentity = eprosima::fastdds::rtps::SampleIdentity;

        /// Identity of the sample being filtered.
        SampleIdentity sample_identity;
        /// Identity of a sample related to the one being filtered.
        SampleIdentity related_sample_identity;
    };

    /**
     * Evaluate if a serialized payload should be accepted by certain reader.
     *
     * @param [in]  payload      The serialized payload of the sample being evaluated.
     * @param [in]  sample_info  The accompanying sample information.
     * @param [in]  reader_guid  The GUID of the reader for which the filter is being evaluated.
     *
     * @return whether the sample should be accepted for the specified reader.
     */
    virtual bool evaluate(
            const SerializedPayload& payload,
            const FilterSampleInfo& sample_info,
            const GUID_t& reader_guid) const = 0;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif  // FASTDDS_DDS_TOPIC__ICONTENTFILTER_HPP
