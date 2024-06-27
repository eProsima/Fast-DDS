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
 * @file RTPSMessageGroup.h
 *
 */

#ifndef FASTDDS_RTPS_MESSAGES__RTPSMESSAGEGROUP_H
#define FASTDDS_RTPS_MESSAGES__RTPSMESSAGEGROUP_H
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <vector>
#include <chrono>
#include <cassert>
#include <memory>
#include <list>

#include <fastdds/rtps/common/FragmentNumber.hpp>
#include <fastdds/rtps/messages/RTPSMessageSenderInterface.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>
#include <fastdds/utils/collections/ResourceLimitedVector.hpp>

#include <rtps/messages/RTPSMessageCreator.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantImpl;
class Endpoint;
class RTPSMessageGroup_t;

/**
 * RTPSMessageGroup Class used to construct a RTPS message.
 * @ingroup WRITER_MODULE
 */
class RTPSMessageGroup
{
public:

    /*!
     * Exception thrown when a operation exceeds the maximum blocking time.
     */
    class timeout : public std::runtime_error
    {
    public:

        timeout()
            : std::runtime_error("timeout")
        {
        }

        virtual ~timeout() = default;
    };

    /*!
     * Exception thrown when a operation exceeds the maximum bytes this object can process in the current period of
     * time.
     */
    class limit_exceeded : public std::runtime_error
    {
    public:

        limit_exceeded()
            : std::runtime_error("limit_exceeded")
        {
        }

        virtual ~limit_exceeded() = default;
    };

    /**
     * Basic constructor.
     * Constructs a RTPSMessageGroup allowing to allocate its own buffer.
     * @param participant Pointer to the participant sending data.
     * @param internal_buffer true indicates this object to allocate its own buffer. false indicates to get a buffer
     * @param max_blocking_time_point Future time point where blocking send should end.
     * from the participant.
     */
    RTPSMessageGroup(
            RTPSParticipantImpl* participant,
            bool internal_buffer = false,
            std::chrono::steady_clock::time_point max_blocking_time_point =
            std::chrono::steady_clock::now() + std::chrono::hours(24));

    /**
     * Basic constructor.
     * Constructs a RTPSMessageGroup allowing the destination endpoints to change.
     * @param participant Pointer to the participant sending data.
     * @param endpoint Pointer to the endpoint sending data.
     * @param msg_sender Pointer to message sender interface.
     * @param max_blocking_time_point Future time point where blocking send should end.
     */
    RTPSMessageGroup(
            RTPSParticipantImpl* participant,
            Endpoint* endpoint,
            RTPSMessageSenderInterface* msg_sender,
            std::chrono::steady_clock::time_point max_blocking_time_point =
            std::chrono::steady_clock::now() + std::chrono::hours(24));

    ~RTPSMessageGroup() noexcept(false);

    /**
     * Adds a DATA message to the group.
     * @param change Reference to the cache change to send.
     * @param expects_inline_qos True when one destination is expecting inline QOS.
     * @return True when message was added to the group.
     */
    bool add_data(
            CacheChange_t& change,
            bool expects_inline_qos);

    /**
     * Adds a DATA_FRAG message to the group.
     * @param change Reference to the cache change to send.
     * @param fragment_number Index (1 based) of the fragment to send.
     * @param expects_inline_qos True when one destination is expecting inline QOS.
     * @return True when message was added to the group.
     */
    bool add_data_frag(
            CacheChange_t& change,
            const uint32_t fragment_number,
            bool expects_inline_qos);

    /**
     * Adds a HEARTBEAT message to the group.
     * @param first_seq First available sequence number.
     * @param last_seq Last available sequence number.
     * @param count Counting identifier.
     * @param is_final Should final flag be set?
     * @param liveliness_flag Should liveliness flag be set?
     * @return True when message was added to the group.
     */
    bool add_heartbeat(
            const SequenceNumber_t& first_seq,
            const SequenceNumber_t& last_seq,
            Count_t count,
            bool is_final,
            bool liveliness_flag);

    /**
     * Adds one or more GAP messages to the group.
     * @param changes_seq_numbers Set of missed sequence numbers.
     * @return True when messages were added to the group.
     */
    bool add_gap(
            std::set<SequenceNumber_t>& changes_seq_numbers);

    /**
     * Adds one GAP message to the group.
     * @param gap_initial_sequence Start of consecutive sequence numbers.
     * @param gap_bitmap Bitmap of non-consecutive sequence numbers.
     * @return True when message was added to the group.
     */
    bool add_gap(
            const SequenceNumber_t& gap_initial_sequence,
            const SequenceNumberSet_t& gap_bitmap);

    /**
     * Adds one GAP message to the group.
     * @param gap_initial_sequence Start of consecutive sequence numbers.
     * @param gap_bitmap Bitmap of non-consecutive sequence numbers.
     * @param reader_guid GUID of the destination reader.
     * @return True when message was added to the group.
     */
    bool add_gap(
            const SequenceNumber_t& gap_initial_sequence,
            const SequenceNumberSet_t& gap_bitmap,
            const GUID_t& reader_guid);

    /**
     * Adds a ACKNACK message to the group.
     * @param seq_num_set Set of missing sequence numbers.
     * @param count Counting identifier.
     * @param final_flag Should final flag be set?
     * @return True when message was added to the group.
     */
    bool add_acknack(
            const SequenceNumberSet_t& seq_num_set,
            int32_t count,
            bool final_flag);

    /**
     * Adds a NACKFRAG message to the group.
     * @param seq_number Sequence number being nack'ed.
     * @param fn_state Set of missing fragment numbers.
     * @param count Counting identifier.
     * @return True when message was added to the group.
     */
    bool add_nackfrag(
            const SequenceNumber_t& seq_number,
            FragmentNumberSet_t fn_state,
            int32_t count);

    /**
     * To be used whenever destination locators/guids change between two add_xxx calls.
     * Automatically called inside add_xxx calls if destinations_have_changed() method of
     * RTPSMessageSenderInterface returns true.
     * May become private again with a refactor of RTPSMessageSenderInterface, adding a
     * group_has_been_flushed() method.
     */
    void flush_and_reset();

    /*!
     * Change dynamically the sender of next RTPS submessages.
     *
     * @param endpoint Pointer to next Endpoint sender. nullptr resets object to initial state.
     * @param msg_sender Pointer to the RTPSMessageSenderInterface will be used to send next RTPS messages..
     * nullptr resets object to initial state.
     * @pre (endpoint != nullptr && msg_sender != nullptr) || (endpoint == nullptr && msg_sender == nullptr)
     */
    void sender(
            Endpoint* endpoint,
            RTPSMessageSenderInterface* msg_sender);

    //! Maximum fragment size minus the headers
    static inline constexpr uint32_t get_max_fragment_payload_size()
    {
        // Max fragment is 64KBytes_max - header - inlineqos - 3(for better alignment)
        return std::numeric_limits<uint16_t>::max() - data_frag_header_size_ - max_inline_qos_size_ - 3;
    }

    void set_sent_bytes_limitation(
            uint32_t limit)
    {
        sent_bytes_limitation_ = limit;
    }

    void reset_current_bytes_processed()
    {
        current_sent_bytes_ = 0;
    }

    inline uint32_t get_current_bytes_processed() const
    {
        return current_sent_bytes_ + buffers_bytes_;
    }

private:

    static constexpr uint32_t data_frag_header_size_ = 28;
    static constexpr uint32_t max_inline_qos_size_ = 32;

    void reset_to_header();

    void flush();

    void send();

    void check_and_maybe_flush()
    {
        check_and_maybe_flush(sender_->destination_guid_prefix());
    }

    void check_and_maybe_flush(
            const GuidPrefix_t& destination_guid_prefix);

    bool insert_submessage(
            bool is_big_submessage)
    {
        return insert_submessage(sender_->destination_guid_prefix(), is_big_submessage);
    }

    bool insert_submessage(
            const GuidPrefix_t& destination_guid_prefix,
            bool is_big_submessage);

    /**
     * @brief Checks if there is enough space in the CDRMessage to accommodate the given length.
     *
     * @param msg Pointer to the CDRMessage to be checked.
     * @param length The length to be checked for space availability.
     * @return True if there is enough space, false otherwise.
     */
    bool check_space(
            CDRMessage_t* msg,
            const uint32_t length);

    /**
     * Appends a submessage to the RTPS Message so it can be sent.
     * The submessage is copied into the header_msg_ buffer. The submessage might contain a data payload
     * or not, in case it is possible to avoid the copy of the payload.
     * The payload will be added later to buffers_to_send_ if it exists in pending_buffer_.
     * The copied submessage in added to buffers_to_send_ through a pointer to its position in header_msg_ and
     * its length.
     *
     * In gather-send operation, the submessage appended only contains the header and pending_buffer_
     * points to the data payload.
     *
     * If gather-send operation is not possible (i.e. Security), the submessage received will contain
     * the header AND the data payload. The whole submessage will be copied into header_msg_.
     *
     * @return True if the submessage was successfully appended, false if the copy operation failed.
     */
    bool append_submessage();

    bool add_info_dst_in_buffer(
            CDRMessage_t* buffer,
            const GuidPrefix_t& destination_guid_prefix);

    bool add_info_ts_in_buffer(
            const Time_t& timestamp);

    bool create_gap_submessage(
            const SequenceNumber_t& gap_initial_sequence,
            const SequenceNumberSet_t& gap_bitmap,
            const EntityId_t& reader_id);

    void get_payload(
            CacheChange_t& change);

#ifdef FASTDDS_STATISTICS
    //! Append the Statistics message to the header_msg_ and add the corresponding buffer to buffers_to_send_.
    void add_stats_submsg();
#endif // FASTDDS_STATISTICS

    RTPSMessageSenderInterface* sender_ = nullptr;

    Endpoint* endpoint_ = nullptr;

    CDRMessage_t* header_msg_ = nullptr;

    CDRMessage_t* submessage_msg_ = nullptr;

    GuidPrefix_t current_dst_;

    RTPSParticipantImpl* participant_ = nullptr;

     #if HAVE_SECURITY

    CDRMessage_t* encrypt_msg_ = nullptr;

     #endif // if HAVE_SECURITY

    std::chrono::steady_clock::time_point max_blocking_time_point_;

    std::unique_ptr<RTPSMessageGroup_t> send_buffer_;

    bool internal_buffer_ = false;

    uint32_t sent_bytes_limitation_ = 0;

    uint32_t current_sent_bytes_ = 0;

    // Next buffer that will be sent
    eprosima::fastdds::rtps::NetworkBuffer pending_buffer_;

    // Vector of buffers that will be sent along the header
    ResourceLimitedVector<eprosima::fastdds::rtps::NetworkBuffer>* buffers_to_send_ = nullptr;

    // Vector of payloads of which the RTPSMessageGroup is the owner
    ResourceLimitedVector<eprosima::fastdds::rtps::SerializedPayload_t>* payloads_to_send_ = nullptr;

    // Bytes to send in the next list of buffers
    uint32_t buffers_bytes_ = 0;

    // Size of the pending padding
    uint8_t pending_padding_ = 0;

    // Fixed padding to be used whenever needed
    const octet padding_[3] = {0, 0, 0};
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_RTPS_MESSAGES__RTPSMESSAGEGROUP_H
