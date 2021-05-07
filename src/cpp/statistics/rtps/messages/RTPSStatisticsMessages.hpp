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
 * @file RTPSStatisticsMessages.hpp
 */

#ifndef _STATISTICS_RTPS_MESSAGES_RTPSSTATISTICSMESSAGES_HPP_
#define _STATISTICS_RTPS_MESSAGES_RTPSSTATISTICSMESSAGES_HPP_

#include <cstdint>

#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/common/Types.h>
#include <fastdds/rtps/messages/RTPSMessageCreator.h>

#define FASTDDS_STATISTICS_NETWORK_SUBMESSAGE 0x80

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace rtps {

struct StatisticsSubmessageData
{
    struct TimeStamp
    {
        int32_t seconds = 0;
        uint32_t fraction = 0;
    };

    struct Sequence
    {
        uint64_t sequence = 0;
        uint64_t bytes = 0;
        uint16_t bytes_high = 0;

        void add_message(
                uint32_t message_size)
        {
            sequence++;
            auto new_bytes = bytes + message_size;
            if (new_bytes < bytes)
            {
                bytes_high++;
            }
            bytes = new_bytes;
        }
    };

    TimeStamp ts{};
    Sequence seq{};
};

constexpr uint16_t statistics_submessage_data_length = sizeof(StatisticsSubmessageData);
constexpr uint16_t statistics_submessage_length =
        RTPSMESSAGE_SUBMESSAGEHEADER_SIZE + // submessage header
        statistics_submessage_data_length;  // submessage data

/**
 * @brief Adds an empty statistics submessage to a message.
 * @param msg Message where the statistics submessage will be added.
 * @pre There should be room in the message for the statistics submessage.
 */
inline void add_statistics_submessage(
        eprosima::fastrtps::rtps::CDRMessage_t* msg)
{
    static_cast<void>(msg);

#ifdef FASTDDS_STATISTICS
    assert(msg->max_size >= msg->length + statistics_submessage_length);

    using namespace eprosima::fastrtps::rtps;
    RTPSMessageCreator::addSubmessageHeader(
        msg, FASTDDS_STATISTICS_NETWORK_SUBMESSAGE, 0x00, statistics_submessage_data_length);
    memset(msg->buffer + msg->pos, 0, statistics_submessage_data_length);
    msg->length += statistics_submessage_data_length;
    msg->pos += statistics_submessage_data_length;
#endif // FASTDDS_STATISTICS
}

inline void set_statistics_submessage_from_transport(
        const eprosima::fastrtps::rtps::octet* send_buffer,
        uint32_t send_buffer_size,
        StatisticsSubmessageData::Sequence& sequence)
{
    static_cast<void>(send_buffer);
    static_cast<void>(send_buffer_size);
    static_cast<void>(sequence);

#ifdef FASTDDS_STATISTICS
    using namespace eprosima::fastrtps::rtps;

    // Message should contain RTPS header and statistic submessage
    assert(statistics_submessage_length + RTPSMESSAGE_HEADER_SIZE <= send_buffer_size);

    // The last submessage should be the statistics submessage
    auto statistics_pos = send_buffer_size - statistics_submessage_length;
    assert(FASTDDS_STATISTICS_NETWORK_SUBMESSAGE == send_buffer[statistics_pos]);

    // Accumulate bytes on sequence
    sequence.add_message(send_buffer_size);

    // Skip the submessage header
    statistics_pos += RTPSMESSAGE_SUBMESSAGEHEADER_SIZE;

    // Set current timestamp and sequence
    auto submessage = (StatisticsSubmessageData*)(&send_buffer[statistics_pos]);
    Time_t ts;
    Time_t::now(ts);

    submessage->ts.seconds = ts.seconds();
    submessage->ts.fraction = ts.fraction();
    submessage->seq.sequence = sequence.sequence;
    submessage->seq.bytes = sequence.bytes;
    submessage->seq.bytes_high = sequence.bytes_high;
#endif // FASTDDS_STATISTICS
}

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif  // _STATISTICS_RTPS_MESSAGES_RTPSSTATISTICSMESSAGES_HPP_
