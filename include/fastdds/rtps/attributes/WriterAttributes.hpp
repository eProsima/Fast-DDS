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
 * @file WriterAttributes.hpp
 *
 */
#ifndef FASTDDS_RTPS_ATTRIBUTES__WRITERATTRIBUTES_HPP
#define FASTDDS_RTPS_ATTRIBUTES__WRITERATTRIBUTES_HPP

#include <functional>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/rtps/attributes/EndpointAttributes.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/Time_t.hpp>
#include <fastdds/rtps/flowcontrol/FlowControllerConsts.hpp>
#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

typedef enum RTPSWriterPublishMode : octet
{
    SYNCHRONOUS_WRITER,
    ASYNCHRONOUS_WRITER
} RTPSWriterPublishMode;


/**
 * Struct WriterTimes, defining the times associated with the Reliable Writers events.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
struct WriterTimes
{
    bool operator ==(
            const WriterTimes& b) const
    {
        return (initial_heartbeat_delay == b.initial_heartbeat_delay) &&
               (heartbeat_period == b.heartbeat_period) &&
               (nack_response_delay == b.nack_response_delay) &&
               (nack_supression_duration == b.nack_supression_duration);
    }

    /// Initial heartbeat delay. Default value 12ms.
    dds::Duration_t initial_heartbeat_delay {0, 12 * 1000 * 1000};
    /// Periodic HB period, default value 3s.
    dds::Duration_t heartbeat_period {3, 0};
    /// Delay to apply to the response of a ACKNACK message, default value 5ms.
    dds::Duration_t nack_response_delay {0, 5 * 1000 * 1000};
    /// This time allows the RTPSWriter to ignore nack messages too soon after the data as sent, default value 0s.
    dds::Duration_t nack_supression_duration {0, 0};

};

/**
 * Class WriterAttributes, defining the attributes of a RTPSWriter.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */
class WriterAttributes
{
public:

    WriterAttributes()
        : liveliness_kind(fastdds::dds::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS)
        , liveliness_lease_duration(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
        , liveliness_announcement_period(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
        , mode(SYNCHRONOUS_WRITER)
        , disable_heartbeat_piggyback(false)
        , disable_positive_acks(false)
        , keep_duration(TIME_T_INFINITE_SECONDS, TIME_T_INFINITE_NANOSECONDS)
    {
        endpoint.endpointKind = WRITER;
        endpoint.durabilityKind = TRANSIENT_LOCAL;
        endpoint.reliabilityKind = RELIABLE;
    }

    virtual ~WriterAttributes()
    {
    }

    //!Attributes of the associated endpoint.
    EndpointAttributes endpoint;

    //!Writer Times (only used for RELIABLE).
    WriterTimes times;

    //! Liveliness kind
    fastdds::dds::LivelinessQosPolicyKind liveliness_kind;

    //! Liveliness lease duration
    dds::Duration_t liveliness_lease_duration;

    //! Liveliness announcement period
    dds::Duration_t liveliness_announcement_period;

    //!Indicates if the Writer is synchronous or asynchronous
    RTPSWriterPublishMode mode;

    //! Disable the sending of heartbeat piggybacks.
    bool disable_heartbeat_piggyback;

    //! Define the allocation behaviour for matched-reader-dependent collections.
    ResourceLimitedContainerConfig matched_readers_allocation;

    //! Disable the sending of positive ACKs
    bool disable_positive_acks;

    //! Keep duration to keep a sample before considering it has been acked
    dds::Duration_t keep_duration;

    //! Flow controller name. Default: fastdds::rtps::FASTDDS_FLOW_CONTROLLER_DEFAULT.
    std::string flow_controller_name = fastdds::rtps::FASTDDS_FLOW_CONTROLLER_DEFAULT;

    //! Whether to send data to each matched reader separately.
    bool separate_sending = false;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_ATTRIBUTES__WRITERATTRIBUTES_HPP
