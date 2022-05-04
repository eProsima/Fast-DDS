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
 * @file WriteParams.h
 */
#ifndef _FASTDDS_RTPS_COMMON_WRITEPARAMS_H_
#define _FASTDDS_RTPS_COMMON_WRITEPARAMS_H_

#include <fastdds/rtps/common/SampleIdentity.h>
#include <fastdds/rtps/common/Time_t.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/*!
 * @brief This class contains additional information of a CacheChange.
 * @ingroup COMMON_MODULE
 */
class RTPS_DllAPI WriteParams
{
public:

    WriteParams& sample_identity(
            const SampleIdentity& sample_id)
    {
        sample_identity_ = sample_id;
        return *this;
    }

    WriteParams& sample_identity(
            SampleIdentity&& sample_id)
    {
        sample_identity_ = std::move(sample_id);
        return *this;
    }

    const SampleIdentity& sample_identity() const
    {
        return sample_identity_;
    }

    SampleIdentity& sample_identity()
    {
        return sample_identity_;
    }

    WriteParams& related_sample_identity(
            const SampleIdentity& sample_id)
    {
        related_sample_identity_ = sample_id;
        return *this;
    }

    WriteParams& related_sample_identity(
            SampleIdentity&& sample_id)
    {
        related_sample_identity_ = std::move(sample_id);
        return *this;
    }

    const SampleIdentity& related_sample_identity() const
    {
        return related_sample_identity_;
    }

    SampleIdentity& related_sample_identity()
    {
        return related_sample_identity_;
    }

    Time_t source_timestamp() const
    {
        return source_timestamp_;
    }

    Time_t& source_timestamp()
    {
        return source_timestamp_;
    }

    WriteParams& source_timestamp(
            const Time_t& timestamp)
    {
        source_timestamp_ = timestamp;
        return *this;
    }

    WriteParams& source_timestamp(
            Time_t&& timestamp)
    {
        source_timestamp_ = std::move(timestamp);
        return *this;
    }

    static WriteParams WRITE_PARAM_DEFAULT;

private:

    SampleIdentity sample_identity_;

    SampleIdentity related_sample_identity_;

    Time_t source_timestamp_{ -1, TIME_T_INFINITE_NANOSECONDS };
};

}  // namespace rtps
}  // namespace fastrtps
}  // namespace eprosima

#endif //_FASTDDS_RTPS_COMMON_WRITEPARAMS_H_
