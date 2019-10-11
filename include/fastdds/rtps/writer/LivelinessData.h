// Copyright 2016-2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file LivelinessData.h
 */
#ifndef _FASTDDS_RTPS_LIVELINESS_DATA_H_
#define _FASTDDS_RTPS_LIVELINESS_DATA_H_

#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/rtps/common/Time_t.h>

#include <chrono>

namespace eprosima {
namespace fastrtps {
namespace rtps {

/**
 * @brief A struct keeping relevant liveliness information of a writer
 * @ingroup WRITER_MODULE
 */
struct LivelinessData
{
    enum WriterStatus
    {
        //! Writer is matched but liveliness has not been asserted yet
        NOT_ASSERTED = 0,
        //! Writer is alive
        ALIVE = 1,
        //! Writer is not alive
        NOT_ALIVE = 2
    };

    /**
     * @brief Constructor
     * @param guid_in GUID of the writer
     * @param kind_in Liveliness kind
     * @param lease_duration_in Liveliness lease duration
     */
    LivelinessData(
            GUID_t guid_in,
            LivelinessQosPolicyKind kind_in,
            Duration_t lease_duration_in)
        : guid(guid_in)
        , kind(kind_in)
        , lease_duration(lease_duration_in)
        , status(WriterStatus::NOT_ASSERTED)
    {}

    LivelinessData()
        : guid()
        , kind(LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
        , lease_duration(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
        , status(WriterStatus::NOT_ASSERTED)
    {}

    ~LivelinessData()
    {}

    /**
     * @brief Equality operator
     * @param other Liveliness data to compare to
     * @return True if equal
     */
    bool operator==(
            const LivelinessData& other) const
    {
        return ((guid == other.guid) &&
               (kind == other.kind) &&
               (lease_duration == other.lease_duration));
    }

    /**
     * @brief Inequality operator
     * @param other Liveliness data to compare to
     * @return True if different
     */
    bool operator!=(
            const LivelinessData& other) const
    {
        return (!operator==(other));
    }

    //! GUID of the writer
    GUID_t guid;

    //! Writer liveliness kind
    LivelinessQosPolicyKind kind;

    //! The lease duration
    Duration_t lease_duration;

    //! The number of times the writer is being counted
    unsigned int count = 1;

    //! The writer status
    WriterStatus status;

    //! The time when the writer will lose liveliness
    std::chrono::steady_clock::time_point time;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* _FASTDDS_RTPS_LIVELINESS_DATA_H_ */
