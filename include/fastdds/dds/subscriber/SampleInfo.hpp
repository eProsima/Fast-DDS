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
 * @file SampleInfo.hpp
 */

#ifndef _FASTDDS_SAMPLEINFO_HPP_
#define _FASTDDS_SAMPLEINFO_HPP_

#include <cstdint>

#include <fastrtps/fastrtps_dll.h>

#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/rtps/common/InstanceHandle.h>

#include <dds/sub/status/DataState.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Class SampleInfo_t with information that is provided along a sample when reading data from a Subscriber.
 * @ingroup FASTDDS_MODULE
 */
class RTPS_DllAPI SampleInfo_t
{
public:

    SampleInfo_t()
    {
    }

    virtual ~SampleInfo_t()
    {
    }

    //!READ/NOT_READ.
    ::dds::sub::status::SampleState sample_state;

    //!NEW/NOT_NEW.
    ::dds::sub::status::ViewState view_state;

    //!ALIVE/NOT_ALIVE_DISPOSED/NOT_ALIVE_NO_WRITERS.
    ::dds::sub::status::InstanceState instance_state;

    //!Times sample become ALIVE when it was received.
    int32_t disposed_generation_count = 0;

    //!Times sample become ALIVE when it was received.
    int32_t no_writers_generation_count = 0;

    //!Preview of the samples that follow within the sequence returned by the read or take operations.
    int32_t sample_rank = 0;

    //!Preview of the samples that follow within the sequence returned by the read or take operations.
    int32_t generation_rank = 0;

    //!Preview of what is available within DataReader.
    int32_t absolute_generation_rank = 0;

    //!Timestamp provided by the DataWriter at the time the sample was produced.
    fastrtps::Time_t source_timestamp;

    fastrtps::rtps::InstanceHandle_t instance_handle;

    fastrtps::rtps::InstanceHandle_t publication_handle;

    // The sample has data associated.
    bool valid_data = false;
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_SAMPLEINFO_HPP_ */
