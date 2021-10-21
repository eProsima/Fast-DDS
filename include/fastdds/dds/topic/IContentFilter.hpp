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

#ifndef _FASTDDS_DDS_TOPIC_ICONTENTFILTER_HPP_
#define _FASTDDS_DDS_TOPIC_ICONTENTFILTER_HPP_

#include <fastrtps/fastrtps_dll.h>

#include <fastdds/dds/core/LoanableTypedCollection.hpp>

#include <fastdds/rtps/common/SampleIdentity.h>
#include <fastdds/rtps/common/SerializedPayload.h>

#include <fastrtps/types/TypesBase.h>
#include <fastrtps/types/TypeDescriptor.h>

namespace eprosima {
namespace fastdds {
namespace dds {

struct IContentFilter
{
    using SerializedPayload = eprosima::fastrtps::rtps::SerializedPayload_t;

    struct FilterSampleInfo
    {
        using SampleIdentity = eprosima::fastrtps::rtps::SampleIdentity;

        SampleIdentity sample_identity;
        SampleIdentity related_sample_identity;
    };

    virtual ~IContentFilter() = default;

    virtual bool evaluate(
            const SerializedPayload& payload,
            const FilterSampleInfo& sample_info) const = 0;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif  // _FASTDDS_DDS_TOPIC_ICONTENTFILTER_HPP_
