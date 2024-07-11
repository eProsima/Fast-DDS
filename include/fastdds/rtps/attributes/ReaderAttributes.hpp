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
 * @file ReaderAttributes.hpp
 *
 */

#ifndef FASTDDS_RTPS_ATTRIBUTES__READERATTRIBUTES_HPP
#define FASTDDS_RTPS_ATTRIBUTES__READERATTRIBUTES_HPP

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/EndpointAttributes.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * Class ReaderTimes, defining the times associated with the Reliable Readers events.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class ReaderTimes
{
public:

    bool operator ==(
            const ReaderTimes& b) const
    {
        return (initial_acknack_delay == b.initial_acknack_delay)  &&
               (heartbeat_response_delay == b.heartbeat_response_delay);
    }

    //! Initial AckNack delay. Default value 70ms.
    dds::Duration_t initial_acknack_delay {0, 70 * 1000 * 1000};
    //! Delay to be applied when a HEARTBEAT message is received, default value 5ms.
    dds::Duration_t heartbeat_response_delay {0,  5 * 1000 * 1000};
};

/**
 * Class ReaderAttributes, to define the attributes of a RTPSReader.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class ReaderAttributes
{
public:

    ReaderAttributes()
    {
        endpoint.endpointKind = READER;
        endpoint.durabilityKind = VOLATILE;
        endpoint.reliabilityKind = BEST_EFFORT;
    }

    //! Attributes of the associated endpoint.
    EndpointAttributes endpoint {};

    //! Times associated with this reader (only for stateful readers)
    ReaderTimes times {};

    //! Liveliness kind
    fastdds::dds::LivelinessQosPolicyKind liveliness_kind =
            fastdds::dds::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;

    //! Liveliness lease duration
    dds::Duration_t liveliness_lease_duration {TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS};

    //! Indicates if the reader expects Inline qos, default value false.
    bool expects_inline_qos = false;

    //! Disable positive ACKs
    bool disable_positive_acks = false;

    //! Enable or disable the reception of messages from unknown writers.
    bool accept_messages_from_unkown_writers = false;

    //! Define the allocation behaviour for matched-writer-dependent collections.
    ResourceLimitedContainerConfig matched_writers_allocation {};

    //! Thread settings for the data-sharing listener thread
    fastdds::rtps::ThreadSettings data_sharing_listener_thread {};
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_ATTRIBUTES__READERATTRIBUTES_HPP
