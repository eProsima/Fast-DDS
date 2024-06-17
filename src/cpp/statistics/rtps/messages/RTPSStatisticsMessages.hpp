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
 * @file RTPSStatisticsMessages.hpp
 */

#ifndef _STATISTICS_RTPS_MESSAGES_RTPSSTATISTICSMESSAGES_HPP_
#define _STATISTICS_RTPS_MESSAGES_RTPSSTATISTICSMESSAGES_HPP_

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <rtps/messages/CDRMessage.hpp>
#include <rtps/messages/RTPSMessageCreator.hpp>

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
            bytes_high += (new_bytes < bytes);
            bytes = new_bytes;
        }

        static Sequence distance(
                const Sequence& from,
                const Sequence& to)
        {
            // Check to >= from
            assert(to.sequence >= from.sequence);
            assert(to.bytes_high >= from.bytes_high);
            assert((to.bytes_high > from.bytes_high) || (to.bytes >= from.bytes));

            Sequence ret;
            ret.sequence = to.sequence - from.sequence;
            ret.bytes_high = to.bytes_high - from.bytes_high;
            ret.bytes = to.bytes - from.bytes;
            ret.bytes_high -= (ret.bytes > to.bytes);

            return ret;
        }

    };

    eprosima::fastdds::rtps::Locator_t destination;
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
        eprosima::fastdds::rtps::CDRMessage_t* msg)
{
    static_cast<void>(msg);

#ifdef FASTDDS_STATISTICS
    assert(msg->max_size >= msg->length + statistics_submessage_length);

    using namespace eprosima::fastdds::rtps;
    RTPSMessageCreator::addSubmessageHeader(
        msg, FASTDDS_STATISTICS_NETWORK_SUBMESSAGE, 0x00, statistics_submessage_data_length);
    memset(msg->buffer + msg->pos, 0, statistics_submessage_data_length);
    msg->length += statistics_submessage_data_length;
    msg->pos += statistics_submessage_data_length;
#endif // FASTDDS_STATISTICS
}

/**
 * @brief Read a statistics submessage from a message.
 * @param [in,out] msg Message from where to extract the submessage.
 * @param [out] data Data read from the statistics submessage.
 * @pre msg->pos should point to the beginning of the statistics submessage payload
 * @post msg->length will be decremented by @c statistics_submessage_length (i.e. the
 *       submessage will be consumed, and it won't be available anymore)
 */
inline void read_statistics_submessage(
        eprosima::fastdds::rtps::CDRMessage_t* msg,
        StatisticsSubmessageData& data)
{
    static_cast<void>(msg);
    static_cast<void>(data);

#ifdef FASTDDS_STATISTICS
    // Should be exactly at the end
    assert(msg->pos + statistics_submessage_data_length == msg->length);

    // Read all fields
    using namespace eprosima::fastdds::rtps;
    CDRMessage::readLocator(msg, &data.destination);
    CDRMessage::readInt32(msg, &data.ts.seconds);
    CDRMessage::readUInt32(msg, &data.ts.fraction);
    CDRMessage::readUInt64(msg, &data.seq.sequence);
    CDRMessage::readUInt64(msg, &data.seq.bytes);
    CDRMessage::readUInt16(msg, &data.seq.bytes_high);

    // Consume submessage
    msg->length -= statistics_submessage_length;
    msg->pos = msg->length;
#endif // FASTDDS_STATISTICS
}

#ifdef FASTDDS_STATISTICS
inline uint32_t get_statistics_message_pos(
        const eprosima::fastdds::rtps::octet* send_buffer,
        uint32_t send_buffer_size)
{
    // zero can be use as an error value because the minimum valid value
    // is RTPSMESSAGE_HEADER_SIZE
    uint32_t statistics_pos = 0;

    // Message should contain RTPS header and statistic submessage
    if (statistics_submessage_length + RTPSMESSAGE_HEADER_SIZE <= send_buffer_size)
    {
        // The last submessage should be the statistics submessage
        uint32_t pos = send_buffer_size - statistics_submessage_length;
        if (FASTDDS_STATISTICS_NETWORK_SUBMESSAGE == send_buffer[pos])
        {
            // only succeed if the message is properly formatted
            statistics_pos = pos;
        }
    }

    return statistics_pos;
}

inline bool is_statistics_buffer(
        const eprosima::fastdds::rtps::NetworkBuffer& stat_buffer)
{
    return statistics_submessage_length == stat_buffer.size &&
           FASTDDS_STATISTICS_NETWORK_SUBMESSAGE ==
           static_cast<const eprosima::fastdds::rtps::octet*>(stat_buffer.buffer)[0];
}

#endif // FASTDDS_STATISTICS

inline void set_statistics_submessage_from_transport(
        const eprosima::fastdds::rtps::Locator_t& destination,
        const eprosima::fastdds::rtps::NetworkBuffer& send_buffer,
        const uint32_t& total_bytes,
        StatisticsSubmessageData::Sequence& sequence)
{
    static_cast<void>(destination);
    static_cast<void>(send_buffer);
    static_cast<void>(total_bytes);
    static_cast<void>(sequence);

#ifdef FASTDDS_STATISTICS
    using namespace eprosima::fastdds::rtps;

    if (is_statistics_buffer(send_buffer))
    {
        uint32_t statistics_pos = 0;

        // Accumulate bytes on sequence
        sequence.add_message(total_bytes);

        // Skip the submessage header
        statistics_pos += RTPSMESSAGE_SUBMESSAGEHEADER_SIZE;

        // Set current timestamp and sequence
        auto current_pos = &static_cast<const octet*>(send_buffer.buffer)[statistics_pos];
        eprosima::fastdds::rtps::Time_t ts;
        eprosima::fastdds::rtps::Time_t::now(ts);

        /*
         * This set of memcpy blocks is intended to prevent an undefined behavior caused when casting from an octet* to a StatisticsSubmessageData*
         * since these classes have different alignment.
         */

        memcpy((char*)current_pos + offsetof(StatisticsSubmessageData, destination), &destination, sizeof(destination));
        memcpy((char*)current_pos + offsetof(StatisticsSubmessageData, ts.seconds), &ts.seconds(),
                sizeof(StatisticsSubmessageData::ts.seconds));
        memcpy((char*)current_pos + offsetof(StatisticsSubmessageData, ts.fraction), &ts.fraction(),
                sizeof(StatisticsSubmessageData::ts.fraction));
        memcpy((char*)current_pos + offsetof(StatisticsSubmessageData, seq.sequence), &sequence.sequence,
                sizeof(sequence.sequence));
        memcpy((char*)current_pos + offsetof(StatisticsSubmessageData, seq.bytes), &sequence.bytes,
                sizeof(sequence.bytes));
        memcpy((char*)current_pos + offsetof(StatisticsSubmessageData, seq.bytes_high), &sequence.bytes_high,
                sizeof(sequence.bytes_high));
    }
#endif // FASTDDS_STATISTICS
}

inline bool remove_statistics_buffer(
        const eprosima::fastdds::rtps::NetworkBuffer& stat_buffer,
        uint32_t& total_bytes)
{
    static_cast<void>(stat_buffer);
    static_cast<void>(total_bytes);

#ifdef FASTDDS_STATISTICS
    if (is_statistics_buffer(stat_buffer))
    {
        total_bytes -= statistics_submessage_length;
        return true;
    }
#endif // FASTDDS_STATISTICS
    return false;
}

} // namespace rtps
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif  // _STATISTICS_RTPS_MESSAGES_RTPSSTATISTICSMESSAGES_HPP_
