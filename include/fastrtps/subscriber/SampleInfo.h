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
 * @file SampleInfo.h
 */

#ifndef SAMPLEINFO_H_
#define SAMPLEINFO_H_

#include <cstdint>

#include <fastdds/rtps/common/ChangeKind_t.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastdds/rtps/common/SampleIdentity.h>
#include <fastdds/rtps/common/Time_t.h>

#include <fastrtps/fastrtps_dll.h>

namespace eprosima {
namespace fastrtps {

/**
 * Class SampleInfo_t with information that is provided along a sample when reading data from a Subscriber.
 * @ingroup FASTRTPS_MODULE
 */
class RTPS_DllAPI SampleInfo_t
{
public:

    SampleInfo_t()
        : sampleKind(rtps::ALIVE)
        , ownershipStrength(0)
        , sample_identity(rtps::SampleIdentity::unknown())
        , related_sample_identity(rtps::SampleIdentity::unknown())
    {
    }

    virtual ~SampleInfo_t()
    {
    }

    //!Sample kind.
    rtps::ChangeKind_t sampleKind;
    //!Ownership Strength of the writer of the sample (0 if the ownership kind is set to SHARED_OWNERSHIP_QOS).
    uint32_t ownershipStrength;
    //!Source timestamp of the sample.
    rtps::Time_t sourceTimestamp;
    //!Reception timestamp of the sample.
    rtps::Time_t receptionTimestamp;
    //!InstanceHandle of the data
    rtps::InstanceHandle_t iHandle;

    rtps::SampleIdentity sample_identity;

    rtps::SampleIdentity related_sample_identity;
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* SAMPLEINFO_H_ */
