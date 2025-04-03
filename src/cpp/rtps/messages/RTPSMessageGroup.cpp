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
 * @file RTPSMessageGroup.cpp
 *
 */

#include "RTPSMessageGroup.hpp"

#include <algorithm>

#include <fastdds/dds/log/Log.hpp>
#include <rtps/messages/RTPSMessageCreator.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <rtps/messages/RTPSGapBuilder.hpp>
#include <rtps/messages/RTPSMessageGroup_t.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/writer/BaseWriter.hpp>

#include <statistics/rtps/messages/RTPSStatisticsMessages.hpp>

#ifdef FASTDDS_STATISTICS
const size_t max_boost_buffers = 61; // ... + SubMsg header + SubMsg body + Statistics message
#else
const size_t max_boost_buffers = 62; // ... + SubMsg header + SubMsg body
#endif // ifdef FASTDDS_STATISTICS

namespace eprosima {
namespace fastdds {
namespace rtps {

using BaseReader = fastdds::rtps::BaseReader;

/**
 * An InlineQosWriter that puts the inline_qos of a CacheChange_t into a CDRMessage_t.
 */
class CacheChangeInlineQoSWriter final : public InlineQosWriter
{
    const CacheChange_t& change_;

public:

    explicit CacheChangeInlineQoSWriter(
            const CacheChange_t& change)
        : change_(change)
    {
    }

    bool writeQosToCDRMessage(
            CDRMessage_t* msg) final
    {
        return CDRMessage::addData(msg, change_.inline_qos.data, change_.inline_qos.length);
    }

};

static bool data_exceeds_limitation(
        uint32_t size_to_add,
        uint32_t limitation,
        uint32_t total_sent,
        uint32_t pending_to_send)
{
    return
        // Limitation has been set and
        (0 < limitation) &&
        //   either limitation has already been reached
        ((limitation <= (total_sent + pending_to_send)) ||
        //   or adding size_to_add will exceed limitation
        (size_to_add > (limitation - (total_sent + pending_to_send))));
}

static bool append_message(
        RTPSParticipantImpl* participant,
        CDRMessage_t* full_msg,
        CDRMessage_t* submsg)
{
    static_cast<void>(participant);

    uint32_t extra_size = 0;

#if HAVE_SECURITY
    // Avoid full message growing over estimated extra size for RTPS encryption
    extra_size += participant->calculate_extra_size_for_rtps_message();
#endif  // HAVE_SECURITY

#ifdef FASTDDS_STATISTICS
    // Keep room for the statistics submessage by reducing max_size while appending submessage
    extra_size += eprosima::fastdds::statistics::rtps::statistics_submessage_length;
#endif  // FASTDDS_STATISTICS

    full_msg->max_size -= extra_size;
    bool ret_val = CDRMessage::appendMsg(full_msg, submsg);
    full_msg->max_size += extra_size;

    return ret_val;
}

bool RTPSMessageGroup::append_submessage()
{
    // Three possible cases:
    // - If the RTPS message is protected, append submessages and use 1 buffer --> buffers_to_send_ of size: 1
    //      Final msg Struct: | header_msg_ |
    // - Else:
    //      a. If the submessage contains the payload --> buffers_to_send_ of size: (submessages_added)
    //          Final msg Struct: | header_msg_[RTPS + submsg1] | header_msg_[submsg2] | header_msg_[submsg3] | ...
    //      b. If the submessage does NOT contain the payload --> buffers_to_send_ of size: ((2 + PAD) * submessages_added)
    //          Final msg Struct: | header_msg_[RTPS + submsg1] | payload | padding | header_msg_[submsg2] | payload | padding | ...
    // Note that case 1 and 2 might be intercalated, combining submessages with and without payloads if the RTPSMessageGroup
    // is shared between different writers

    uint32_t pos_header = header_msg_->pos;
    uint32_t length_submsg = submessage_msg_->length;
    if (header_msg_->pos == RTPSMESSAGE_HEADER_SIZE && header_msg_->length == RTPSMESSAGE_HEADER_SIZE)
    {
        // Include the RTPS header into the buffer that will be added to buffers_to_send_ vector
        pos_header = 0;
        length_submsg += RTPSMESSAGE_HEADER_SIZE;
    }

    // Copy the submessage to the header message.
    // The submessage will contain the payload if copy_data is enabled, otherwise gather-send will be used and the
    // submessage will only contain the header. The payload will be added with pending_buffer_, as an extra buffer
    if (!append_message(participant_, header_msg_, submessage_msg_))
    {
        return false;
    }

#if HAVE_SECURITY
    // If the RTPS message is protected, the whole message will be encrypted at once
    // so we need to keep the whole message in a single buffer
    if (participant_->security_attributes().is_rtps_protected && endpoint_->supports_rtps_protection())
    {
        return true;
    }
#endif // if HAVE_SECURITY

    // Add into buffers_to_send_ the submessage added to header_msg_
    buffers_to_send_->emplace_back(&header_msg_->buffer[pos_header], length_submsg);
    buffers_bytes_ += length_submsg;

    if (nullptr != pending_buffer_.buffer)
    {
        // Add pending buffer & padding to buffers_to_send_
        buffers_to_send_->emplace_back(pending_buffer_);
        buffers_bytes_ += pending_buffer_.size;
        pending_buffer_ = NetworkBuffer();
        if (pending_padding_ > 0)
        {
            buffers_to_send_->emplace_back(padding_, pending_padding_);
            buffers_bytes_ += pending_padding_;
            pending_padding_ = 0;
        }
    }
    return true;
}

bool sort_changes_group (
        CacheChange_t* c1,
        CacheChange_t* c2)
{
    return(c1->sequenceNumber < c2->sequenceNumber);
}

bool sort_SeqNum(
        const SequenceNumber_t& s1,
        const SequenceNumber_t& s2)
{
    return(s1 < s2);
}

typedef std::pair<SequenceNumber_t, SequenceNumberSet_t> pair_T;

bool compare_remote_participants(
        const std::vector<GUID_t>& remote_participants1,
        const std::vector<GuidPrefix_t>& remote_participants2)
{
    if (remote_participants1.size() == remote_participants2.size())
    {
        for (auto& participant : remote_participants1)
        {
            if (std::find(remote_participants2.begin(), remote_participants2.end(), participant.guidPrefix) ==
                    remote_participants2.end())
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

void get_participant_from_endpoint(
        const GUID_t& endpoint,
        std::vector<GuidPrefix_t>& participants)
{
    if (std::find(participants.begin(), participants.end(), endpoint.guidPrefix) == participants.end())
    {
        participants.push_back(endpoint.guidPrefix);
    }
}

void get_participants_from_endpoints(
        const std::vector<GUID_t>& endpoints,
        std::vector<GuidPrefix_t>& participants)
{
    participants.clear();

    for (const GUID_t& endpoint : endpoints)
    {
        if (std::find(participants.begin(), participants.end(), endpoint.guidPrefix) == participants.end())
        {
            participants.push_back(endpoint.guidPrefix);
        }
    }
}

const EntityId_t& get_entity_id(
        const std::vector<GUID_t>& endpoints)
{
    if (endpoints.size() == 0)
    {
        return c_EntityId_Unknown;
    }

    const EntityId_t& entityid = endpoints.at(0).entityId;

    for (auto it = endpoints.begin() + 1; it != endpoints.end(); ++it)
    {
        if (entityid != it->entityId)
        {
            return c_EntityId_Unknown;
        }
    }

    return entityid;
}

RTPSMessageGroup::RTPSMessageGroup(
        RTPSParticipantImpl* participant,
        bool internal_buffer,
        std::chrono::steady_clock::time_point max_blocking_time_point)
    : participant_(participant)
    , max_blocking_time_point_(max_blocking_time_point)
    , send_buffer_(!internal_buffer ? participant->get_send_buffer(max_blocking_time_point) : nullptr)
    , internal_buffer_(internal_buffer)
{
    // Avoid warning when neither SECURITY nor DEBUG is used
    (void)participant;

    assert(participant);

    if (internal_buffer)
    {
        const GuidPrefix_t& guid_prefix = participant->getGuid().guidPrefix;
        constexpr size_t align_size = sizeof(octet*) - 1;
        uint32_t payload_size = participant->getMaxMessageSize();
        assert(payload_size > 0u);
        payload_size = (payload_size + align_size) & ~align_size;
        send_buffer_.reset(new RTPSMessageGroup_t(
#if HAVE_SECURITY
                    participant->is_secure(),
#endif // if HAVE_SECURITY
                    payload_size, guid_prefix
                    ));
    }

    header_msg_ = &(send_buffer_->rtpsmsg_fullmsg_);
    submessage_msg_ = &(send_buffer_->rtpsmsg_submessage_);
    buffers_to_send_ = &(send_buffer_->buffers_);
    payloads_to_send_ = &(send_buffer_->payloads_);

    // Init RTPS message.
    reset_to_header();

    CDRMessage::initCDRMsg(submessage_msg_);

#if HAVE_SECURITY
    if (participant->is_secure())
    {
        encrypt_msg_ = &(send_buffer_->rtpsmsg_encrypt_);
        CDRMessage::initCDRMsg(encrypt_msg_);
    }
#endif // if HAVE_SECURITY

}

RTPSMessageGroup::RTPSMessageGroup(
        RTPSParticipantImpl* participant,
        Endpoint* endpoint,
        RTPSMessageSenderInterface* msg_sender,
        std::chrono::steady_clock::time_point max_blocking_time_point)
    : RTPSMessageGroup(participant, false, max_blocking_time_point)
{
    assert(endpoint);

    endpoint_ = endpoint;
    sender_ = msg_sender;
}

RTPSMessageGroup::~RTPSMessageGroup() noexcept(false)
{
    try
    {
        send();
    }
    catch (...)
    {
        if (!internal_buffer_)
        {
            buffers_to_send_->clear();
            payloads_to_send_->clear();
            participant_->return_send_buffer(std::move(send_buffer_));
        }
        throw;
    }

    if (!internal_buffer_)
    {
        buffers_to_send_->clear();
        // Payloads are released in the destructor
        payloads_to_send_->clear();
        participant_->return_send_buffer(std::move(send_buffer_));
    }
}

void RTPSMessageGroup::reset_to_header()
{
    CDRMessage::initCDRMsg(header_msg_);
    header_msg_->pos = RTPSMESSAGE_HEADER_SIZE;
    header_msg_->length = RTPSMESSAGE_HEADER_SIZE;

    buffers_to_send_->clear();
    buffers_bytes_ = 0;
    // Payloads are released in the destructor
    payloads_to_send_->clear();
}

void RTPSMessageGroup::flush()
{
    send();

    reset_to_header();
}

void RTPSMessageGroup::send()
{
    if (endpoint_ && sender_)
    {
        if (header_msg_->length > RTPSMESSAGE_HEADER_SIZE)
        {
            std::lock_guard<RTPSMessageSenderInterface> lock(*sender_);

#if HAVE_SECURITY
            CDRMessage_t* msgToSend = header_msg_;
            // TODO(Ricardo) Control message size if it will be encrypted.
            if (participant_->security_attributes().is_rtps_protected && endpoint_->supports_rtps_protection())
            {
                CDRMessage::initCDRMsg(encrypt_msg_);
                header_msg_->pos = RTPSMESSAGE_HEADER_SIZE;
                encrypt_msg_->pos = RTPSMESSAGE_HEADER_SIZE;
                encrypt_msg_->length = RTPSMESSAGE_HEADER_SIZE;
                memcpy(encrypt_msg_->buffer, header_msg_->buffer, RTPSMESSAGE_HEADER_SIZE);

                if (!participant_->security_manager().encode_rtps_message(*header_msg_, *encrypt_msg_,
                        sender_->remote_participants()))
                {
                    EPROSIMA_LOG_ERROR(RTPS_WRITER, "Error encoding rtps message.");
                    return;
                }

                msgToSend = encrypt_msg_;
                buffers_to_send_->emplace_back(msgToSend->buffer, msgToSend->length);
                buffers_bytes_ += msgToSend->length;
            }
#endif // if HAVE_SECURITY

#ifdef FASTDDS_STATISTICS
            if (buffers_bytes_ <
                    static_cast<uint32_t>(std::numeric_limits<uint16_t>::max() - RTPSMESSAGE_DATA_MIN_LENGTH))
            {
                // Avoid sending the data message for DATA that are not fragmented and exceed the 65 kB limit
                add_stats_submsg();
            }
#endif // FASTDDS_STATISTICS

            if (!sender_->send(*buffers_to_send_,
                    buffers_bytes_,
                    max_blocking_time_point_))
            {
                throw timeout();
            }
            current_sent_bytes_ += buffers_bytes_;
        }
    }
}

void RTPSMessageGroup::flush_and_reset()
{
    // Flush
    flush();

    current_dst_ = c_GuidPrefix_Unknown;
}

void RTPSMessageGroup::sender(
        Endpoint* endpoint,
        RTPSMessageSenderInterface* msg_sender)
{
    assert((endpoint != nullptr && msg_sender != nullptr) || (endpoint == nullptr && msg_sender == nullptr));
    if (endpoint != endpoint_ || msg_sender != sender_)
    {
        flush_and_reset();
    }

    endpoint_ = endpoint;
    sender_ = msg_sender;
}

void RTPSMessageGroup::check_and_maybe_flush(
        const GuidPrefix_t& destination_guid_prefix)
{
    assert(nullptr != sender_);

    CDRMessage::initCDRMsg(submessage_msg_);

    if (sender_->destinations_have_changed())
    {
        flush_and_reset();
    }

    add_info_dst_in_buffer(submessage_msg_, destination_guid_prefix);
}

bool RTPSMessageGroup::insert_submessage(
        const GuidPrefix_t& destination_guid_prefix,
        bool is_big_submessage)
{
    uint32_t total_size = submessage_msg_->length + pending_buffer_.size + buffers_bytes_ + pending_padding_;
    if (!check_space(header_msg_, total_size))
    {
        flush();
        add_info_dst_in_buffer(header_msg_, destination_guid_prefix);
    }

    if (!append_submessage())
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Cannot add RTPS submesage to the CDRMessage. Buffer too small");
        return false;
    }

    // Flush when the submessage is bigger than 64KB OR if the number of buffers to send is 64 (boost limit)
    if (is_big_submessage || max_boost_buffers < buffers_to_send_->size())
    {
        flush();
    }

    return true;
}

bool RTPSMessageGroup::check_space(
        CDRMessage_t* msg,
        const uint32_t length)
{
    uint32_t extra_size = 0;

#if HAVE_SECURITY
    // Avoid full message growing over estimated extra size for RTPS encryption
    extra_size += participant_->calculate_extra_size_for_rtps_message();
#endif  // HAVE_SECURITY

#ifdef FASTDDS_STATISTICS
    // Keep room for the statistics submessage by reducing max_size while appending submessage
    extra_size += eprosima::fastdds::statistics::rtps::statistics_submessage_length;
#endif  // FASTDDS_STATISTICS

    return msg && ((msg->pos + length) <= (msg->max_size - extra_size));
}

bool RTPSMessageGroup::add_info_dst_in_buffer(
        CDRMessage_t* buffer,
        const GuidPrefix_t& destination_guid_prefix)
{
#if HAVE_SECURITY
    // Add INFO_SRC when we are at the beginning of the message and RTPS protection is enabled
    if ((header_msg_->length == RTPSMESSAGE_HEADER_SIZE) &&
            participant_->security_attributes().is_rtps_protected && endpoint_->supports_rtps_protection())
    {
        RTPSMessageCreator::addSubmessageInfoSRC(buffer, c_ProtocolVersion, c_VendorId_eProsima,
                participant_->getGuid().guidPrefix);
    }
#endif // if HAVE_SECURITY

    if (current_dst_ != destination_guid_prefix)
    {
        current_dst_ = destination_guid_prefix;
        RTPSMessageCreator::addSubmessageInfoDST(buffer, current_dst_);
    }

    return true;
}

bool RTPSMessageGroup::add_info_ts_in_buffer(
        const Time_t& timestamp)
{
    assert(nullptr != sender_);

    EPROSIMA_LOG_INFO(RTPS_WRITER, "Sending INFO_TS message");

#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
#endif // if HAVE_SECURITY

    if (!RTPSMessageCreator::addSubmessageInfoTS(submessage_msg_, timestamp, false))
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Cannot add INFO_TS submsg to the CDRMessage. Buffer too small");
        return false;
    }

#if HAVE_SECURITY
    if (endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if (!participant_->security_manager().encode_writer_submessage(*submessage_msg_, *encrypt_msg_,
                endpoint_->getGuid(), sender_->remote_guids()))
        {
            EPROSIMA_LOG_ERROR(RTPS_WRITER, "Cannot encrypt DATA submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if ((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif // if HAVE_SECURITY

    return true;
}

bool RTPSMessageGroup::add_data(
        CacheChange_t& change,
        bool expectsInlineQos)
{
    assert(nullptr != sender_);

    EPROSIMA_LOG_INFO(RTPS_WRITER, "Sending relevant changes as DATA messages");

    // Check limitation
    uint32_t data_size = change.serializedPayload.length;
    if (data_exceeds_limitation(data_size, sent_bytes_limitation_, current_sent_bytes_,
            buffers_bytes_))
    {
        flush_and_reset();
        throw limit_exceeded();
    }

    // Check preconditions. If fail flush and reset.
    check_and_maybe_flush();
    add_info_ts_in_buffer(change.sourceTimestamp);

    CacheChangeInlineQoSWriter qos_writer(change);
    InlineQosWriter* inline_qos;
    inline_qos = (change.inline_qos.length > 0 && nullptr != change.inline_qos.data) ? &qos_writer : nullptr;

    bool copy_data = (nullptr == change.serializedPayload.payload_owner);
#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
    bool protect_payload = endpoint_->getAttributes().security_attributes().is_payload_protected;
    bool protect_submessage = endpoint_->getAttributes().security_attributes().is_submessage_protected;
    bool protect_rtps = participant_->security_attributes().is_rtps_protected;
    copy_data = copy_data || protect_payload || protect_submessage || protect_rtps;
#endif // if HAVE_SECURITY
    const EntityId_t& readerId = get_entity_id(sender_->remote_guids());

    CacheChange_t change_to_add;
    change_to_add.copy_not_memcpy(&change);
    change_to_add.serializedPayload.data = change.serializedPayload.data;
    change_to_add.serializedPayload.length = change.serializedPayload.length;
    change_to_add.writerGUID = endpoint_->getGuid();

#if HAVE_SECURITY
    if (protect_payload)
    {
        SerializedPayload_t encrypt_payload;
        encrypt_payload.data = encrypt_msg_->buffer;
        encrypt_payload.max_size = encrypt_msg_->max_size;

        // If payload protection, encode payload
        if (!participant_->security_manager().encode_serialized_payload(change_to_add.serializedPayload,
                encrypt_payload, endpoint_->getGuid()))
        {
            EPROSIMA_LOG_ERROR(RTPS_WRITER, "Error encoding change " << change.sequenceNumber);
            change_to_add.serializedPayload.data = nullptr;
            encrypt_payload.data = nullptr;
            return false;
        }

        change_to_add.serializedPayload.data = encrypt_msg_->buffer;
        encrypt_payload.data = nullptr;
        change_to_add.serializedPayload.length = encrypt_payload.length;
    }
#endif // if HAVE_SECURITY

    // TODO (Ricardo). Check to create special wrapper.
    bool is_big_submessage;
    if (!RTPSMessageCreator::addSubmessageData(submessage_msg_, &change_to_add, endpoint_->getAttributes().topicKind,
            readerId, expectsInlineQos, inline_qos, is_big_submessage, copy_data, pending_buffer_, pending_padding_))
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Cannot add DATA submsg to the CDRMessage. Buffer too small");
        change_to_add.serializedPayload.data = nullptr;
        return false;
    }
    change_to_add.serializedPayload.data = nullptr;

#if HAVE_SECURITY
    if (protect_submessage)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if (!participant_->security_manager().encode_writer_submessage(*submessage_msg_, *encrypt_msg_,
                endpoint_->getGuid(), sender_->remote_guids()))
        {
            EPROSIMA_LOG_ERROR(RTPS_WRITER, "Cannot encrypt DATA submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if ((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif // if HAVE_SECURITY

    if (insert_submessage(is_big_submessage))
    {
        // If gather-send is possible, get payload
        if (!copy_data)
        {
            get_payload(change);
        }
        return true;
    }

    return false;
}

bool RTPSMessageGroup::add_data_frag(
        CacheChange_t& change,
        const uint32_t fragment_number,
        bool expectsInlineQos)
{
    assert(nullptr != sender_);

    EPROSIMA_LOG_INFO(RTPS_WRITER, "Sending relevant changes as DATA_FRAG messages");

    // Calculate fragment start
    uint32_t fragment_start = change.getFragmentSize() * (fragment_number - 1);
    // Calculate fragment size. If last fragment, size may be smaller
    uint32_t fragment_size = fragment_number < change.getFragmentCount() ? change.getFragmentSize() :
            change.serializedPayload.length - fragment_start;
    // Check limitation
    if (data_exceeds_limitation(fragment_size, sent_bytes_limitation_, current_sent_bytes_,
            buffers_bytes_))
    {
        flush_and_reset();
        throw limit_exceeded();
    }

    // Check preconditions. If fail flush and reset.
    check_and_maybe_flush();
    add_info_ts_in_buffer(change.sourceTimestamp);

    CacheChangeInlineQoSWriter qos_writer(change);
    InlineQosWriter* inline_qos;
    inline_qos = (change.inline_qos.length > 0 && nullptr != change.inline_qos.data) ? &qos_writer : nullptr;

    bool copy_data = (nullptr == change.serializedPayload.payload_owner);
#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
    bool protect_payload = endpoint_->getAttributes().security_attributes().is_payload_protected;
    bool protect_submessage = endpoint_->getAttributes().security_attributes().is_submessage_protected;
    bool protect_rtps = participant_->security_attributes().is_rtps_protected;
    copy_data = copy_data || protect_payload || protect_submessage || protect_rtps;
#endif // if HAVE_SECURITY
    const EntityId_t& readerId = get_entity_id(sender_->remote_guids());

    // TODO (Ricardo). Check to create special wrapper.
    CacheChange_t change_to_add;
    change_to_add.copy_not_memcpy(&change);
    change_to_add.serializedPayload.data = change.serializedPayload.data + fragment_start;
    change_to_add.serializedPayload.length = fragment_size;
    change_to_add.writerGUID = endpoint_->getGuid();

#if HAVE_SECURITY
    if (protect_payload)
    {
        SerializedPayload_t encrypt_payload;
        encrypt_payload.data = encrypt_msg_->buffer;
        encrypt_payload.max_size = encrypt_msg_->max_size;

        // If payload protection, encode payload
        if (!participant_->security_manager().encode_serialized_payload(change_to_add.serializedPayload,
                encrypt_payload, endpoint_->getGuid()))
        {
            EPROSIMA_LOG_ERROR(RTPS_WRITER, "Error encoding change " << change.sequenceNumber);
            change_to_add.serializedPayload.data = nullptr;
            encrypt_payload.data = nullptr;
            return false;
        }

        change_to_add.serializedPayload.data = encrypt_msg_->buffer;
        encrypt_payload.data = nullptr;
        change_to_add.serializedPayload.length = encrypt_payload.length;
    }
#endif // if HAVE_SECURITY

    if (!RTPSMessageCreator::addSubmessageDataFrag(submessage_msg_, &change, fragment_number,
            change_to_add.serializedPayload, endpoint_->getAttributes().topicKind, readerId,
            expectsInlineQos, inline_qos, copy_data, pending_buffer_, pending_padding_))
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Cannot add DATA_FRAG submsg to the CDRMessage. Buffer too small");
        change_to_add.serializedPayload.data = nullptr;
        return false;
    }
    change_to_add.serializedPayload.data = nullptr;

#if HAVE_SECURITY
    if (endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if (!participant_->security_manager().encode_writer_submessage(*submessage_msg_, *encrypt_msg_,
                endpoint_->getGuid(), sender_->remote_guids()))
        {
            EPROSIMA_LOG_ERROR(RTPS_WRITER, "Cannot encrypt DATA submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if ((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif // if HAVE_SECURITY

    if (insert_submessage(false))
    {
        // If gather-send is possible, get payload
        if (!copy_data)
        {
            get_payload(change);
        }
        return true;
    }
    return false;
}

bool RTPSMessageGroup::add_heartbeat(
        const SequenceNumber_t& firstSN,
        const SequenceNumber_t& lastSN,
        const Count_t count,
        bool isFinal,
        bool livelinessFlag)
{
    assert(nullptr != sender_);

    check_and_maybe_flush();

#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
#endif // if HAVE_SECURITY

    const EntityId_t& readerId = get_entity_id(sender_->remote_guids());

    if (!RTPSMessageCreator::addSubmessageHeartbeat(submessage_msg_, readerId, endpoint_->getGuid().entityId,
            firstSN, lastSN, count, isFinal, livelinessFlag))
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Cannot add HEARTBEAT submsg to the CDRMessage. Buffer too small");
        return false;
    }

#if HAVE_SECURITY
    if (endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if (!participant_->security_manager().encode_writer_submessage(*submessage_msg_, *encrypt_msg_,
                endpoint_->getGuid(), sender_->remote_guids()))
        {
            EPROSIMA_LOG_ERROR(RTPS_WRITER, "Cannot encrypt HEARTBEAT submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if ((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif // if HAVE_SECURITY

    return insert_submessage(false);
}

// TODO (Ricardo) Check with standard 8.3.7.4.5
bool RTPSMessageGroup::add_gap(
        std::set<SequenceNumber_t>& changesSeqNum)
{
    RTPSGapBuilder gap_builder(*this);
    for (const SequenceNumber_t& seq : changesSeqNum)
    {
        if (!gap_builder.add(seq))
        {
            return false;
        }
    }

    return gap_builder.flush();
}

bool RTPSMessageGroup::add_gap(
        const SequenceNumber_t& gap_initial_sequence,
        const SequenceNumberSet_t& gap_bitmap)
{
    assert(nullptr != sender_);

    // Check preconditions. If fail flush and reset.
    check_and_maybe_flush();

    const EntityId_t& readerId = get_entity_id(sender_->remote_guids());

    if (!create_gap_submessage(gap_initial_sequence, gap_bitmap, readerId))
    {
        return false;
    }

    return insert_submessage(false);
}

bool RTPSMessageGroup::add_gap(
        const SequenceNumber_t& gap_initial_sequence,
        const SequenceNumberSet_t& gap_bitmap,
        const GUID_t& reader_guid)
{
    // Check preconditions. If fail flush and reset.
    check_and_maybe_flush(reader_guid.guidPrefix);

    const EntityId_t& readerId = reader_guid.entityId;

    if (!create_gap_submessage(gap_initial_sequence, gap_bitmap, readerId))
    {
        return false;
    }

    return insert_submessage(reader_guid.guidPrefix, false);
}

bool RTPSMessageGroup::create_gap_submessage(
        const SequenceNumber_t& gap_initial_sequence,
        const SequenceNumberSet_t& gap_bitmap,
        const EntityId_t& reader_id)
{
    assert(nullptr != sender_);

#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
#endif // if HAVE_SECURITY

    if (!RTPSMessageCreator::addSubmessageGap(submessage_msg_, gap_initial_sequence, gap_bitmap,
            reader_id, endpoint_->getGuid().entityId))
    {
        EPROSIMA_LOG_ERROR(RTPS_WRITER, "Cannot add GAP submsg to the CDRMessage. Buffer too small");
        return false;
    }

#if HAVE_SECURITY
    if (endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if (!participant_->security_manager().encode_writer_submessage(*submessage_msg_, *encrypt_msg_,
                endpoint_->getGuid(), sender_->remote_guids()))
        {
            EPROSIMA_LOG_ERROR(RTPS_WRITER, "Cannot encrypt DATA submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if ((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif // if HAVE_SECURITY

    // Notify the statistics module, note that only writers add gaps
    assert(nullptr != dynamic_cast<BaseWriter*>(endpoint_));
    static_cast<BaseWriter*>(endpoint_)->on_gap();

    return true;
}

void RTPSMessageGroup::get_payload(
        CacheChange_t& change)
{
    payloads_to_send_->emplace_back();
    // Get payload to avoid returning it to the pool before sending
    change.serializedPayload.payload_owner->get_payload(change.serializedPayload, payloads_to_send_->back());
}

#ifdef FASTDDS_STATISTICS
void RTPSMessageGroup::add_stats_submsg()
{
    // Use empty space of header_msg_ buffer to create the msg
    uint32_t stats_pos = header_msg_->pos;

    eprosima::fastdds::statistics::rtps::add_statistics_submessage(header_msg_);

    // Add into buffers_to_send_ the submessage added to header_msg_
    buffers_to_send_->emplace_back(&header_msg_->buffer[stats_pos],
            eprosima::fastdds::statistics::rtps::statistics_submessage_length);
    buffers_bytes_ += eprosima::fastdds::statistics::rtps::statistics_submessage_length;
}

#endif // FASTDDS_STATISTICS

bool RTPSMessageGroup::add_acknack(
        const SequenceNumberSet_t& SNSet,
        int32_t count,
        bool finalFlag)
{
    assert(nullptr != sender_);

    // A vector is used to avoid dynamic allocations, but only first item is used
    size_t n_guids = sender_->remote_guids().size();
    if (n_guids == 0)
    {
        return false;
    }
    assert(n_guids == 1);

    check_and_maybe_flush();

#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
#endif // if HAVE_SECURITY

    if (!RTPSMessageCreator::addSubmessageAcknack(submessage_msg_, endpoint_->getGuid().entityId,
            sender_->remote_guids().front().entityId, SNSet, count, finalFlag))
    {
        EPROSIMA_LOG_ERROR(RTPS_READER, "Cannot add ACKNACK submsg to the CDRMessage. Buffer too small");
        return false;
    }

#if HAVE_SECURITY
    if (endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if (!participant_->security_manager().encode_reader_submessage(*submessage_msg_, *encrypt_msg_,
                endpoint_->getGuid(), sender_->remote_guids()))
        {
            EPROSIMA_LOG_ERROR(RTPS_READER, "Cannot encrypt ACKNACK submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if ((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif // if HAVE_SECURITY

    // Notify the statistics module, note that only readers add acknacks
    BaseReader::downcast(endpoint_)->on_acknack(count);

    return insert_submessage(false);
}

bool RTPSMessageGroup::add_nackfrag(
        const SequenceNumber_t& writerSN,
        FragmentNumberSet_t fnState,
        int32_t count)
{
    assert(nullptr != sender_);

    // A vector is used to avoid dynamic allocations, but only first item is used
    assert(sender_->remote_guids().size() == 1);

    check_and_maybe_flush();

#if HAVE_SECURITY
    uint32_t from_buffer_position = submessage_msg_->pos;
#endif // if HAVE_SECURITY

    if (!RTPSMessageCreator::addSubmessageNackFrag(submessage_msg_, endpoint_->getGuid().entityId,
            sender_->remote_guids().front().entityId, writerSN, fnState, count))
    {
        EPROSIMA_LOG_ERROR(RTPS_READER, "Cannot add ACKNACK submsg to the CDRMessage. Buffer too small");
        return false;
    }

#if HAVE_SECURITY
    if (endpoint_->getAttributes().security_attributes().is_submessage_protected)
    {
        submessage_msg_->pos = from_buffer_position;
        CDRMessage::initCDRMsg(encrypt_msg_);
        if (!participant_->security_manager().encode_reader_submessage(*submessage_msg_, *encrypt_msg_,
                endpoint_->getGuid(), sender_->remote_guids()))
        {
            EPROSIMA_LOG_ERROR(RTPS_READER, "Cannot encrypt ACKNACK submessage for writer " << endpoint_->getGuid());
            return false;
        }

        if ((submessage_msg_->max_size - from_buffer_position) >= encrypt_msg_->length)
        {
            memcpy(&submessage_msg_->buffer[from_buffer_position], encrypt_msg_->buffer, encrypt_msg_->length);
            submessage_msg_->length = from_buffer_position + encrypt_msg_->length;
            submessage_msg_->pos = submessage_msg_->length;
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_OUT, "Not enough memory to copy encrypted data for " << endpoint_->getGuid());
            return false;
        }
    }
#endif // if HAVE_SECURITY

    // Notify the statistics module, note that only readers add NACKFRAGs
    BaseReader::downcast(endpoint_)->on_nackfrag(count);

    return insert_submessage(false);
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
