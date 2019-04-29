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
#ifndef LIVELINESS_DATA_H_
#define LIVELINESS_DATA_H_

#include "../../qos/QosPolicies.h"
#include "../common/Time_t.h"

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
        : writer_guid(guid_in)
        , kind(kind_in)
        , lease_duration(lease_duration_in)
    {}

    ~LivelinessData()
    {}

    /**
     * @brief Equality operator
     * @param other Liveliness data to compare to
     * @return True if equal
     */
    bool operator==(const LivelinessData& other) const
    {
        return ((writer_guid == other.writer_guid) &&
                (kind == other.kind) &&
                (lease_duration == other.lease_duration));
    }

    /**
     * @brief Inequality operator
     * @param other Liveliness data to compare to
     * @return True if different
     */
    bool operator!=(const LivelinessData& other) const
    {
        return (!operator==(other));
    }

    //! GUID of the writer
    GUID_t writer_guid;

    //! Writer liveliness kind
    LivelinessQosPolicyKind kind;

    //! True if the writer is alive, false otherwise
    bool alive = false;

    //! The time when the writer will lose liveliness
    std::chrono::steady_clock::time_point time;

    //! The lease duration
    Duration_t lease_duration;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
#endif /* LIVELINESS_DATA_H_ */
