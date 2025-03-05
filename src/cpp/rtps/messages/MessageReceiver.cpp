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
 * @file MessageReceiver.cpp
 *
 */

#include <cassert>
#include <limits>
#include <thread>

#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/common/Guid.hpp>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <rtps/messages/MessageReceiver.h>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/BaseReader.hpp>
#include <rtps/writer/BaseWriter.hpp>
#include <statistics/rtps/messages/RTPSStatisticsMessages.hpp>
#include <statistics/rtps/StatisticsBase.hpp>
#include <utils/shared_mutex.hpp>

#define INFO_SRC_SUBMSG_LENGTH 20

#define IDSTRING "(ID:" << std::this_thread::get_id() << ") " <<

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastdds {
namespace rtps {

MessageReceiver::MessageReceiver(
        RTPSParticipantImpl* participant,
        uint32_t rec_buffer_size)
    : mtx_()
    , associated_writers_()
    , associated_readers_()
#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    , participant_(participant)
#endif // if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    , source_version_(c_ProtocolVersion)
    , source_vendor_id_(c_VendorId_Unknown)
    , source_guid_prefix_(c_GuidPrefix_Unknown)
    , dest_guid_prefix_(c_GuidPrefix_Unknown)
    , have_timestamp_(false)
    , timestamp_(dds::c_TimeInvalid)
#if HAVE_SECURITY
    , crypto_msg_(participant->is_secure() ? rec_buffer_size : 0)
    , crypto_submsg_(participant->is_secure() ? rec_buffer_size : 0)
    , crypto_payload_(participant->is_secure() ? rec_buffer_size : 0)
#endif // if HAVE_SECURITY
{
    static_cast<void>(participant);
    (void)rec_buffer_size;
    EPROSIMA_LOG_INFO(RTPS_MSG_IN, "Created with CDRMessage of size: " << rec_buffer_size);

#if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    if (participant->is_secure())
    {
        process_data_message_function_ = std::bind(
            &MessageReceiver::process_data_message_with_security,
            this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3);

        process_data_fragment_message_function_ = std::bind(
            &MessageReceiver::process_data_fragment_message_with_security,
            this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5,
            std::placeholders::_6);
    }
    else
    {
#endif // if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    process_data_message_function_ = std::bind(
        &MessageReceiver::process_data_message_without_security,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);

    process_data_fragment_message_function_ = std::bind(
        &MessageReceiver::process_data_fragment_message_without_security,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4,
        std::placeholders::_5,
        std::placeholders::_6);
#if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
}

#endif // if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
}

MessageReceiver::~MessageReceiver()
{
    EPROSIMA_LOG_INFO(RTPS_MSG_IN, "");
    assert(associated_writers_.empty());
    assert(associated_readers_.empty());
}

 #if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
void MessageReceiver::process_data_message_with_security(
        const EntityId_t& reader_id,
        CacheChange_t& change,
        bool was_decoded)
{
    auto process_message = [was_decoded, &change, this](BaseReader* reader)
            {
                if (!was_decoded && reader->getAttributes().security_attributes().is_submessage_protected)
                {
                    return;
                }

                if (!reader->getAttributes().security_attributes().is_payload_protected)
                {
                    reader->process_data_msg(&change);
                    return;
                }

                if (!reader->matched_writer_is_matched(change.writerGUID))
                {
                    return;
                }

                if (!participant_->security_manager().decode_serialized_payload(change.serializedPayload,
                        crypto_payload_, reader->getGuid(), change.writerGUID))
                {
                    return;
                }

                std::swap(change.serializedPayload.data, crypto_payload_.data);
                std::swap(change.serializedPayload.length, crypto_payload_.length);

                octet* original_payload_data = change.serializedPayload.data;
                uint32_t original_payload_length = change.serializedPayload.length;
                reader->process_data_msg(&change);
                IPayloadPool* payload_pool = change.serializedPayload.payload_owner;
                if (payload_pool)
                {
                    payload_pool->release_payload(change.serializedPayload);
                    change.serializedPayload.data = original_payload_data;
                    change.serializedPayload.length = original_payload_length;
                }
                std::swap(change.serializedPayload.data, crypto_payload_.data);
                std::swap(change.serializedPayload.length, crypto_payload_.length);
            };

    findAllReaders(reader_id, process_message);
}

void MessageReceiver::process_data_fragment_message_with_security(
        const EntityId_t& reader_id,
        CacheChange_t& change,
        uint32_t sample_size,
        uint32_t fragment_starting_num,
        uint16_t fragments_in_submessage,
        bool was_decoded)
{
    auto process_message = [was_decoded, &change, sample_size, fragment_starting_num, fragments_in_submessage, this](
        BaseReader* reader)
            {
                if (!was_decoded && reader->getAttributes().security_attributes().is_submessage_protected)
                {
                    return;
                }

                if (!reader->getAttributes().security_attributes().is_payload_protected)
                {
                    reader->process_data_frag_msg(&change, sample_size, fragment_starting_num, fragments_in_submessage);
                    return;
                }

                if (!reader->matched_writer_is_matched(change.writerGUID))
                {
                    return;
                }

                if (!participant_->security_manager().decode_serialized_payload(change.serializedPayload,
                        crypto_payload_, reader->getGuid(), change.writerGUID))
                {
                    return;
                }

                std::swap(change.serializedPayload.data, crypto_payload_.data);
                std::swap(change.serializedPayload.length, crypto_payload_.length);
                reader->process_data_frag_msg(&change, sample_size, fragment_starting_num, fragments_in_submessage);
                std::swap(change.serializedPayload.data, crypto_payload_.data);
                std::swap(change.serializedPayload.length, crypto_payload_.length);
            };

    findAllReaders(reader_id, process_message);
}

#endif // if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)

void MessageReceiver::process_data_message_without_security(
        const EntityId_t& reader_id,
        CacheChange_t& change,
        bool /*was_decoded*/)
{
    auto process_message = [&change](BaseReader* reader)
            {
                reader->process_data_msg(&change);
            };

    findAllReaders(reader_id, process_message);
}

void MessageReceiver::process_data_fragment_message_without_security(
        const EntityId_t& reader_id,
        CacheChange_t& change,
        uint32_t sample_size,
        uint32_t fragment_starting_num,
        uint16_t fragments_in_submessage,
        bool /*was_decoded*/)
{
    auto process_message = [&change, sample_size, fragment_starting_num, fragments_in_submessage](
        BaseReader* reader)
            {
                reader->process_data_frag_msg(&change, sample_size, fragment_starting_num, fragments_in_submessage);
            };

    findAllReaders(reader_id, process_message);
}

void MessageReceiver::associateEndpoint(
        Endpoint* to_add)
{
    std::lock_guard<eprosima::shared_mutex> guard(mtx_);
    if (to_add->getAttributes().endpointKind == WRITER)
    {
        const auto writer = BaseWriter::downcast(to_add);
        for (const auto& it : associated_writers_)
        {
            if (it == writer)
            {
                return;
            }
        }

        associated_writers_.push_back(writer);
    }
    else
    {
        const auto reader = BaseReader::downcast(to_add);
        const auto entityId = reader->getGuid().entityId;
        // search for set of readers by entity ID
        const auto readers = associated_readers_.find(entityId);
        if (readers == associated_readers_.end())
        {
            auto vec = std::vector<BaseReader*>();
            vec.push_back(reader);
            associated_readers_.emplace(entityId, vec);
        }
        else
        {
            for (const auto& it : readers->second)
            {
                if (it == reader)
                {
                    return;
                }
            }

            readers->second.push_back(reader);
        }
    }
}

void MessageReceiver::removeEndpoint(
        Endpoint* to_remove)
{
    std::lock_guard<eprosima::shared_mutex> guard(mtx_);

    if (to_remove->getAttributes().endpointKind == WRITER)
    {
        auto* var = dynamic_cast<BaseWriter*>(to_remove);
        for (auto it = associated_writers_.begin(); it != associated_writers_.end(); ++it)
        {
            if (*it == var)
            {
                associated_writers_.erase(it);
                break;
            }
        }
    }
    else
    {
        auto readers = associated_readers_.find(to_remove->getGuid().entityId);
        if (readers != associated_readers_.end())
        {
            auto* var = BaseReader::downcast(to_remove);
            for (auto it = readers->second.begin(); it != readers->second.end(); ++it)
            {
                if (*it == var)
                {
                    readers->second.erase(it);
                    if (readers->second.empty())
                    {
                        associated_readers_.erase(readers);
                    }
                    break;
                }
            }
        }
    }
}

void MessageReceiver::reset()
{
    source_version_ = c_ProtocolVersion;
    source_vendor_id_ = c_VendorId_Unknown;
    source_guid_prefix_ = c_GuidPrefix_Unknown;
    dest_guid_prefix_ = c_GuidPrefix_Unknown;
    have_timestamp_ = false;
    timestamp_ = dds::c_TimeInvalid;
}

void MessageReceiver::processCDRMsg(
        const Locator_t& source_locator,
        const Locator_t& reception_locator,
        CDRMessage_t* msg)
{
    if (msg->length < RTPSMESSAGE_HEADER_SIZE)
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Received message too short, ignoring");
        return;
    }

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    GuidPrefix_t participantGuidPrefix;
#else
    GuidPrefix_t participantGuidPrefix = participant_->getGuid().guidPrefix;
#endif // ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION

#if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    security::SecurityManager& security = participant_->security_manager();
    CDRMessage_t* auxiliary_buffer = &crypto_msg_;
    int decode_ret = 0;
#endif // if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)

    bool ignore_submessages = false;

    {
        std::lock_guard<eprosima::shared_mutex> guard(mtx_);

        reset();

        dest_guid_prefix_ = participantGuidPrefix;

        msg->pos = 0; //Start reading at 0

        //Once everything is set, the reading begins:
        if (!checkRTPSHeader(msg))
        {
            return;
        }

#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
        ignore_submessages = participant_->is_participant_ignored(source_guid_prefix_);
#endif  // if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)

        if (!ignore_submessages)
        {
            notify_network_statistics(source_locator, reception_locator, msg);
        }

#if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
        decode_ret = security.decode_rtps_message(*msg, *auxiliary_buffer, source_guid_prefix_);

        if (decode_ret < 0)
        {
            return;
        }

        if (decode_ret == 0)
        {
            // The original CDRMessage buffer (msg) now points to the proprietary temporary buffer crypto_msg_.
            // The auxiliary buffer now points to the propietary temporary buffer crypto_submsg_.
            // This way each decoded sub-message will be processed using the crypto_submsg_ buffer.
            msg = auxiliary_buffer;
            auxiliary_buffer = &crypto_submsg_;
        }
#endif // if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    }

    // Loop until there are no more submessages
    // Each submessage processing method choses the lock kind required
    bool valid;
    SubmessageHeader_t submsgh; //Current submessage header

    while (msg->pos < msg->length)// end of the message
    {
        CDRMessage_t* submessage = msg;

        bool current_message_was_decoded = false;

#if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
        decode_ret = security.decode_rtps_submessage(*msg, *auxiliary_buffer, source_guid_prefix_);

        if (decode_ret < 0)
        {
            return;
        }

        if (decode_ret == 0)
        {
            current_message_was_decoded = true;
            submessage = auxiliary_buffer;
        }
#endif // if HAVE_SECURITY && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)

        //First 4 bytes must contain: ID | flags | octets to next header
        if (!readSubmessageHeader(submessage, &submsgh))
        {
            return;
        }

        valid = true;
        uint32_t next_msg_pos = submessage->pos;
        next_msg_pos += (submsgh.submessageLength + 3u) & ~3u;

        // We ignore submessage if the source participant is to be ignored, unless the submessage king is INFO_SRC
        // which triggers a reevaluation of the flag.
        bool ignore_current_submessage = ignore_submessages && submsgh.submessageId != INFO_SRC;

        if (!ignore_current_submessage)
        {
            switch (submsgh.submessageId)
            {
                case DATA:
                {
                    if (dest_guid_prefix_ != participantGuidPrefix)
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Data Submsg ignored, DST is another RTPSParticipant");
                    }
                    else
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Data Submsg received, processing.");
                        EntityId_t writerId = c_EntityId_Unknown;
                        valid = proc_Submsg_Data(submessage, &submsgh, writerId, current_message_was_decoded);
#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
                        if (valid && writerId == c_EntityId_SPDPWriter)
                        {
                            ignore_submessages = participant_->is_participant_ignored(source_guid_prefix_);
                        }
#endif  // if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)

                    }
                    break;
                }
                case DATA_FRAG:
                    if (dest_guid_prefix_ != participantGuidPrefix)
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN,
                                IDSTRING "DataFrag Submsg ignored, DST is another RTPSParticipant");
                    }
                    else
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "DataFrag Submsg received, processing.");
                        valid = proc_Submsg_DataFrag(submessage, &submsgh, current_message_was_decoded);
                    }
                    break;
                case GAP:
                {
                    if (dest_guid_prefix_ != participantGuidPrefix)
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN,
                                IDSTRING "Gap Submsg ignored, DST is another RTPSParticipant...");
                    }
                    else
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Gap Submsg received, processing...");
                        valid = proc_Submsg_Gap(submessage, &submsgh, current_message_was_decoded);
                    }
                    break;
                }
                case ACKNACK:
                {
                    if (dest_guid_prefix_ != participantGuidPrefix)
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN,
                                IDSTRING "Acknack Submsg ignored, DST is another RTPSParticipant...");
                    }
                    else
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Acknack Submsg received, processing...");
                        valid = proc_Submsg_Acknack(submessage, &submsgh, current_message_was_decoded);
                    }
                    break;
                }
                case NACK_FRAG:
                {
                    if (dest_guid_prefix_ != participantGuidPrefix)
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN,
                                IDSTRING "NackFrag Submsg ignored, DST is another RTPSParticipant...");
                    }
                    else
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "NackFrag Submsg received, processing...");
                        valid = proc_Submsg_NackFrag(submessage, &submsgh, current_message_was_decoded);
                    }
                    break;
                }
                case HEARTBEAT:
                {
                    if (dest_guid_prefix_ != participantGuidPrefix)
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "HB Submsg ignored, DST is another RTPSParticipant...");
                    }
                    else
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Heartbeat Submsg received, processing...");
                        valid = proc_Submsg_Heartbeat(submessage, &submsgh, current_message_was_decoded);
                    }
                    break;
                }
                case HEARTBEAT_FRAG:
                {
                    if (dest_guid_prefix_ != participantGuidPrefix)
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN,
                                IDSTRING "HBFrag Submsg ignored, DST is another RTPSParticipant...");
                    }
                    else
                    {
                        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "HeartbeatFrag Submsg received, processing...");
                        valid = proc_Submsg_HeartbeatFrag(submessage, &submsgh, current_message_was_decoded);
                    }
                    break;
                }
                case PAD:
                    EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "PAD messages not yet implemented, ignoring");
                    break;
                case INFO_DST:
                    EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "InfoDST message received, processing...");
                    valid = proc_Submsg_InfoDST(submessage, &submsgh);
                    break;
                case INFO_SRC:
                    EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "InfoSRC message received, processing...");
                    valid = proc_Submsg_InfoSRC(submessage, &submsgh);
#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
                    ignore_submessages = participant_->is_participant_ignored(source_guid_prefix_);
#endif  // if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
                    break;
                case INFO_TS:
                {
                    EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "InfoTS Submsg received, processing...");
                    valid = proc_Submsg_InfoTS(submessage, &submsgh);
                    break;
                }
                case INFO_REPLY:
                    break;
                case INFO_REPLY_IP4:
                    break;
                default:
                    break;
            }
        }
        if (!valid || submsgh.is_last)
        {
            break;
        }

        submessage->pos = next_msg_pos;
    }

#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    participant_->assert_remote_participant_liveliness(source_guid_prefix_);
#endif // if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
}

bool MessageReceiver::checkRTPSHeader(
        CDRMessage_t* msg)
{
    //check and proccess the RTPS Header
    if (msg->buffer[0] != 'R' ||  msg->buffer[1] != 'T' ||
            msg->buffer[2] != 'P' ||  msg->buffer[3] != 'S')
    {
        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Msg received with no RTPS in header, ignoring...");
        return false;
    }

    msg->pos += 4;

    //CHECK AND SET protocol version
    if (msg->buffer[msg->pos] == c_ProtocolVersion.m_major)
    {
        source_version_.m_major = msg->buffer[msg->pos];
        msg->pos++;
        source_version_.m_minor = msg->buffer[msg->pos];
        msg->pos++;
    }
    else
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Major RTPS Version not supported");
        return false;
    }

    //Set source vendor id
    source_vendor_id_[0] = msg->buffer[msg->pos];
    msg->pos++;
    source_vendor_id_[1] = msg->buffer[msg->pos];
    msg->pos++;
    //set source guid prefix
    CDRMessage::readData(msg, source_guid_prefix_.value, GuidPrefix_t::size);
    have_timestamp_ = false;
    return true;
}

bool MessageReceiver::readSubmessageHeader(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh) const
{
    if (msg->length - msg->pos < 4)
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "SubmessageHeader too short");
        return false;
    }

    smh->submessageId = msg->buffer[msg->pos];
    msg->pos++;
    smh->flags = msg->buffer[msg->pos];
    msg->pos++;

    //Set endianness of message
    msg->msg_endian = (smh->flags & BIT(0)) != 0 ? LITTLEEND : BIGEND;
    uint16_t length = 0;
    CDRMessage::readUInt16(msg, &length);
    if (msg->pos + length > msg->length)
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "SubMsg of invalid length (" << length <<
                ") with current msg position/length (" << msg->pos << "/" << msg->length << ")");
        return false;
    }

    if ((length == 0) && (smh->submessageId != INFO_TS) && (smh->submessageId != PAD))
    {
        // THIS IS THE LAST SUBMESSAGE
        smh->submessageLength = msg->length - msg->pos;
        smh->is_last = true;
    }
    else
    {
        smh->submessageLength = length;
        smh->is_last = false;
    }

    return true;
}

bool MessageReceiver::willAReaderAcceptMsgDirectedTo(
        const EntityId_t& readerID,
        BaseReader*& first_reader) const
{
    first_reader = nullptr;
    if (associated_readers_.empty())
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Data received when NO readers are listening");
        return false;
    }

    if (readerID != c_EntityId_Unknown)
    {
        const auto readers = associated_readers_.find(readerID);
        if (readers != associated_readers_.end())
        {
            first_reader = readers->second.front();
            return true;
        }
    }
    else
    {
        for (const auto& readers : associated_readers_)
        {
            if (0 < readers.second.size())
            {
                first_reader = readers.second.front();
                return true;
            }
        }
    }

    EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "No Reader accepts this message (directed to: " << readerID << ")");
    return false;
}

template<typename Functor>
void MessageReceiver::findAllReaders(
        const EntityId_t& readerID,
        const Functor& callback) const
{
    if (readerID != c_EntityId_Unknown)
    {
        const auto readers = associated_readers_.find(readerID);
        if (readers != associated_readers_.end())
        {
            for (const auto& it : readers->second)
            {
                callback(it);
            }
        }
    }
    else
    {
        for (const auto& readers : associated_readers_)
        {
            for (const auto& it : readers.second)
            {
                callback(it);
            }
        }
    }
}

bool MessageReceiver::proc_Submsg_Data(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh,
        EntityId_t& writerID,
        bool was_decoded) const
{
    eprosima::shared_lock<eprosima::shared_mutex> guard(mtx_);

    //READ and PROCESS
    if (smh->submessageLength < RTPSMESSAGE_DATA_MIN_LENGTH)
    {
        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Too short submessage received, ignoring");
        return false;
    }
    //Fill flags bool values
    bool endiannessFlag = (smh->flags & BIT(0)) != 0;
    bool inlineQosFlag = (smh->flags & BIT(1)) != 0;
    bool dataFlag = (smh->flags & BIT(2)) != 0;
    bool keyFlag = (smh->flags & BIT(3)) != 0;
    if (keyFlag && dataFlag)
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Message received with Data and Key Flag set, ignoring");
        return false;
    }

    //Assign message endianness
    if (endiannessFlag)
    {
        msg->msg_endian = LITTLEEND;
    }
    else
    {
        msg->msg_endian = BIGEND;
    }

    //Extra flags don't matter now. Avoid those bytes
    msg->pos += 2;

    bool valid = true;
    int16_t octetsToInlineQos = 0;
    valid &= CDRMessage::readInt16(msg, &octetsToInlineQos); //it should be 16 in this implementation

    //reader and writer ID
    BaseReader* first_reader = nullptr;
    EntityId_t readerID;
    valid &= CDRMessage::readEntityId(msg, &readerID);

    //WE KNOW THE READER THAT THE MESSAGE IS DIRECTED TO SO WE LOOK FOR IT:
    if (!willAReaderAcceptMsgDirectedTo(readerID, first_reader))
    {
        return false;
    }

    //FOUND THE READER.
    //We ask the reader for a cachechange to store the information.
    CacheChange_t ch;
    ch.kind = ALIVE;
    ch.writerGUID.guidPrefix = source_guid_prefix_;
    valid &= CDRMessage::readEntityId(msg, &ch.writerGUID.entityId);

    writerID = ch.writerGUID.entityId;

    //Get sequence number
    valid &= CDRMessage::readSequenceNumber(msg, &ch.sequenceNumber);

    if (!valid)
    {
        return false;
    }

    if (ch.sequenceNumber <= SequenceNumber_t())
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Invalid message received, bad sequence Number");
        return false;
    }

    // Get the vendor id
    ch.vendor_id = source_vendor_id_;

    //Jump ahead if more parameters are before inlineQos (not in this version, maybe if further minor versions.)
    if (octetsToInlineQos > RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG)
    {
        msg->pos += (octetsToInlineQos - RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
        if (msg->pos > msg->length)
        {
            EPROSIMA_LOG_WARNING(RTPS_MSG_IN,
                    IDSTRING "Invalid jump through msg, msg->pos " << msg->pos << " > msg->length " << msg->length);
            return false;
        }
    }

    uint32_t inlineQosSize = 0;

    if (inlineQosFlag)
    {
        if (!ParameterList::updateCacheChangeFromInlineQos(ch, msg, inlineQosSize))
        {
            EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "SubMessage Data ERROR, Inline Qos ParameterList error");
            return false;
        }
        ch.inline_qos.data = &msg->buffer[msg->pos - inlineQosSize];
        ch.inline_qos.max_size = inlineQosSize;
        ch.inline_qos.length = inlineQosSize;
        ch.inline_qos.encapsulation = endiannessFlag ? PL_CDR_LE : PL_CDR_BE;
        ch.inline_qos.pos = 0;
    }

    if (dataFlag || keyFlag)
    {
        uint32_t payload_size;
        const uint32_t submsg_no_payload_size =
                RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE + octetsToInlineQos + inlineQosSize;

        // Prevent integer overflow of variable payload_size
        if (smh->submessageLength < submsg_no_payload_size)
        {
            EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Serialized Payload avoided overflow "
                    "(" << smh->submessageLength << "/" << submsg_no_payload_size << ")");
            ch.serializedPayload.data = nullptr;
            ch.inline_qos.data = nullptr;
            return false;
        }

        payload_size = smh->submessageLength - submsg_no_payload_size;
        uint32_t next_pos = msg->pos + payload_size;
        if (msg->length >= next_pos && payload_size > 0)
        {
            FASTDDS_TODO_BEFORE(3, 3, "Pass keyFlag in serializedPayload, and always pass input data upwards");
            if (dataFlag)
            {
                ch.serializedPayload.data = &msg->buffer[msg->pos];
                ch.serializedPayload.length = payload_size;
                ch.serializedPayload.max_size = payload_size;
            }
            else // keyFlag would be true since we are inside an if (dataFlag || keyFlag)
            {
                if (payload_size <= PARAMETER_KEY_HASH_LENGTH)
                {
                    if (!ch.instanceHandle.isDefined())
                    {
                        memcpy(ch.instanceHandle.value, &msg->buffer[msg->pos], payload_size);
                    }
                }
                else
                {
                    EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Ignoring Serialized Payload for too large key-only data (" <<
                            payload_size << ")");
                }
            }
            msg->pos = next_pos;
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Serialized Payload value invalid or larger than maximum allowed size"
                    "(" << payload_size << "/" << (msg->length - msg->pos) << ")");
            ch.serializedPayload.data = nullptr;
            ch.inline_qos.data = nullptr;
            return false;
        }
    }

    // Set sourcetimestamp
    if (have_timestamp_)
    {
        ch.sourceTimestamp = timestamp_;
    }

    EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "from Writer " << ch.writerGUID << "; possible Reader entities: " <<
            associated_readers_.size());

    //Look for the correct reader to add the change
    process_data_message_function_(readerID, ch, was_decoded);

    IPayloadPool* payload_pool = ch.serializedPayload.payload_owner;
    if (payload_pool)
    {
        payload_pool->release_payload(ch.serializedPayload);
    }

    //TODO(Ricardo) If an exception is thrown (ex, by fastcdr), these lines are not executed -> segmentation fault
    ch.serializedPayload.data = nullptr;
    ch.inline_qos.data = nullptr;

    EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Sub Message DATA processed");
    return true;
}

bool MessageReceiver::proc_Submsg_DataFrag(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh,
        bool was_decoded) const
{
    eprosima::shared_lock<eprosima::shared_mutex> guard(mtx_);

    //READ and PROCESS
    if (smh->submessageLength < RTPSMESSAGE_DATA_MIN_LENGTH)
    {
        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Too short submessage received, ignoring");
        return false;
    }

    //Fill flags bool values
    bool endiannessFlag = (smh->flags & BIT(0)) != 0;
    bool inlineQosFlag = (smh->flags & BIT(1)) != 0;
    bool keyFlag = (smh->flags & BIT(2)) != 0;

    //Assign message endianness
    if (endiannessFlag)
    {
        msg->msg_endian = LITTLEEND;
    }
    else
    {
        msg->msg_endian = BIGEND;
    }

    //Extra flags don't matter now. Avoid those bytes
    msg->pos += 2;

    bool valid = true;
    int16_t octetsToInlineQos = 0;
    valid &= CDRMessage::readInt16(msg, &octetsToInlineQos); //it should be 16 in this implementation

    //reader and writer ID
    BaseReader* first_reader = nullptr;
    EntityId_t readerID;
    valid &= CDRMessage::readEntityId(msg, &readerID);

    //WE KNOW THE READER THAT THE MESSAGE IS DIRECTED TO SO WE LOOK FOR IT:
    if (!willAReaderAcceptMsgDirectedTo(readerID, first_reader))
    {
        return false;
    }

    //FOUND THE READER.
    //We ask the reader for a cachechange to store the information.
    CacheChange_t ch;
    ch.writerGUID.guidPrefix = source_guid_prefix_;
    valid &= CDRMessage::readEntityId(msg, &ch.writerGUID.entityId);

    //Get sequence number
    valid &= CDRMessage::readSequenceNumber(msg, &ch.sequenceNumber);

    if (ch.sequenceNumber <= SequenceNumber_t())
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Invalid message received, bad sequence Number");
        return false;
    }

    // Get the vendor id
    ch.vendor_id = source_vendor_id_;

    // READ FRAGMENT NUMBER
    uint32_t fragmentStartingNum;
    valid &= CDRMessage::readUInt32(msg, &fragmentStartingNum);

    // READ FRAGMENTSINSUBMESSAGE
    uint16_t fragmentsInSubmessage;
    valid &= CDRMessage::readUInt16(msg, &fragmentsInSubmessage);

    // READ FRAGMENTSIZE
    uint16_t fragmentSize = 0;
    valid &= CDRMessage::readUInt16(msg, &fragmentSize);

    // READ SAMPLESIZE
    uint32_t sampleSize;
    valid &= CDRMessage::readUInt32(msg, &sampleSize);

    if (!valid)
    {
        return false;
    }

    //Jump ahead if more parameters are before inlineQos (not in this version, maybe if further minor versions.)
    if (octetsToInlineQos > RTPSMESSAGE_OCTETSTOINLINEQOS_DATAFRAGSUBMSG)
    {
        msg->pos += (octetsToInlineQos - RTPSMESSAGE_OCTETSTOINLINEQOS_DATAFRAGSUBMSG);
        if (msg->pos > msg->length)
        {
            EPROSIMA_LOG_WARNING(RTPS_MSG_IN,
                    IDSTRING "Invalid jump through msg, msg->pos " << msg->pos << " > msg->length " << msg->length);
            return false;
        }
    }

    uint32_t inlineQosSize = 0;

    if (inlineQosFlag)
    {
        if (!ParameterList::updateCacheChangeFromInlineQos(ch, msg, inlineQosSize))
        {
            EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "SubMessage Data ERROR, Inline Qos ParameterList error");
            return false;
        }
        ch.inline_qos.data = &msg->buffer[msg->pos - inlineQosSize];
        ch.inline_qos.max_size = inlineQosSize;
        ch.inline_qos.length = inlineQosSize;
        ch.inline_qos.encapsulation = endiannessFlag ? PL_CDR_LE : PL_CDR_BE;
        ch.inline_qos.pos = 0;
    }

    uint32_t payload_size;
    payload_size = smh->submessageLength - (RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE + octetsToInlineQos + inlineQosSize);

    // Validations??? XXX TODO

    if (!keyFlag)
    {
        uint32_t next_pos = msg->pos + payload_size;
        if (msg->length >= next_pos && payload_size > 0)
        {
            ch.kind = ALIVE;
            ch.serializedPayload.data = &msg->buffer[msg->pos];
            ch.serializedPayload.length = payload_size;
            ch.serializedPayload.max_size = payload_size;
            ch.setFragmentSize(fragmentSize);

            msg->pos = next_pos;
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Serialized Payload value invalid or larger than maximum allowed size "
                    "(" << payload_size << "/" << (msg->length - msg->pos) << ")");
            ch.serializedPayload.data = nullptr;
            ch.inline_qos.data = nullptr;
            return false;
        }
    }
    else if (keyFlag)
    {
        /* XXX TODO
           Endianness_t previous_endian = msg->msg_endian;
           if (ch->serializedPayload.encapsulation == PL_CDR_BE)
           msg->msg_endian = BIGEND;
           else if (ch->serializedPayload.encapsulation == PL_CDR_LE)
           msg->msg_endian = LITTLEEND;
           else
           {
           EPROSIMA_LOG_ERROR(RTPS_MSG_IN, IDSTRING"Bad encapsulation for KeyHash and status parameter list");
           return false;
           }
           //uint32_t param_size;
           if (ParameterList::readParameterListfromCDRMsg(msg, &m_ParamList, ch, false) <= 0)
           {
           EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING"SubMessage Data ERROR, keyFlag ParameterList");
           return false;
           }
           msg->msg_endian = previous_endian;
         */
    }

    // Set sourcetimestamp
    if (have_timestamp_)
    {
        ch.sourceTimestamp = timestamp_;
    }

    EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "from Writer " << ch.writerGUID << "; possible Reader entities: " <<
            associated_readers_.size());
    process_data_fragment_message_function_(readerID, ch, sampleSize, fragmentStartingNum, fragmentsInSubmessage,
            was_decoded);
    ch.serializedPayload.data = nullptr;
    ch.inline_qos.data = nullptr;

    EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Sub Message DATA_FRAG processed");

    return true;
}

bool MessageReceiver::proc_Submsg_Heartbeat(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh,
        bool was_decoded) const
{
    eprosima::shared_lock<eprosima::shared_mutex> guard(mtx_);

    bool endiannessFlag = (smh->flags & BIT(0)) != 0;
    bool finalFlag = (smh->flags & BIT(1)) != 0;
    bool livelinessFlag = (smh->flags & BIT(2)) != 0;
    //Assign message endianness
    if (endiannessFlag)
    {
        msg->msg_endian = LITTLEEND;
    }
    else
    {
        msg->msg_endian = BIGEND;
    }

    GUID_t readerGUID;
    GUID_t writerGUID;
    readerGUID.guidPrefix = dest_guid_prefix_;
    CDRMessage::readEntityId(msg, &readerGUID.entityId);
    writerGUID.guidPrefix = source_guid_prefix_;
    CDRMessage::readEntityId(msg, &writerGUID.entityId);
    SequenceNumber_t firstSN;
    SequenceNumber_t lastSN;
    CDRMessage::readSequenceNumber(msg, &firstSN);
    CDRMessage::readSequenceNumber(msg, &lastSN);

    SequenceNumber_t zeroSN;
    if (firstSN <= zeroSN)
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Invalid Heartbeat received (" << firstSN << " <= 0), ignoring");
        return false;
    }
    if (lastSN < firstSN && lastSN != firstSN - 1)
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Invalid Heartbeat received (" << firstSN << ") - (" <<
                lastSN << "), ignoring");
        return false;
    }
    uint32_t HBCount;
    if (!CDRMessage::readUInt32(msg, &HBCount))
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Unable to read heartbeat count from heartbeat message");
        return false;
    }

    //Look for the correct reader and writers:
    findAllReaders(readerGUID.entityId,
            [was_decoded, &writerGUID, &HBCount, &firstSN, &lastSN, finalFlag, livelinessFlag, this](
                BaseReader* reader)
            {
                // Only used when HAVE_SECURITY is defined
                static_cast<void>(was_decoded);
#if HAVE_SECURITY
                if (was_decoded || !reader->getAttributes().security_attributes().is_submessage_protected)
#endif  // HAVE_SECURITY
                {
                    reader->process_heartbeat_msg(writerGUID, HBCount, firstSN, lastSN, finalFlag, livelinessFlag,
                    source_vendor_id_);
                }
            });

    return true;
}

bool MessageReceiver::proc_Submsg_Acknack(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh,
        bool was_decoded) const
{
    // Only used when HAVE_SECURITY is defined
    static_cast<void>(was_decoded);

    eprosima::shared_lock<eprosima::shared_mutex> guard(mtx_);

    bool endiannessFlag = (smh->flags & BIT(0)) != 0;
    bool finalFlag = (smh->flags & BIT(1)) != 0;
    //Assign message endianness
    if (endiannessFlag)
    {
        msg->msg_endian = LITTLEEND;
    }
    else
    {
        msg->msg_endian = BIGEND;
    }
    GUID_t readerGUID;
    GUID_t writerGUID;
    readerGUID.guidPrefix = source_guid_prefix_;
    CDRMessage::readEntityId(msg, &readerGUID.entityId);
    writerGUID.guidPrefix = dest_guid_prefix_;
    CDRMessage::readEntityId(msg, &writerGUID.entityId);

    SequenceNumberSet_t SNSet = CDRMessage::readSequenceNumberSet(msg);
    uint32_t Ackcount;
    if (!CDRMessage::readUInt32(msg, &Ackcount))
    {
        EPROSIMA_LOG_WARNING(RTPS_MSG_IN, IDSTRING "Unable to read ackcount from message");
        return false;
    }

    //Look for the correct writer to use the acknack
    for (BaseWriter* it : associated_writers_)
    {
#if HAVE_SECURITY
        if (was_decoded || !it->getAttributes().security_attributes().is_submessage_protected)
#endif  // HAVE_SECURITY
        {
            bool result;
            if (it->process_acknack(writerGUID, readerGUID, Ackcount, SNSet, finalFlag, result, source_vendor_id_))
            {
                if (!result)
                {
                    EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Acknack msg to NOT stateful writer ");
                }
                return result;
            }
        }
    }
    EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Acknack msg to UNKNOWN writer (I looked through "
            << associated_writers_.size() << " writers in this ListenResource)");
    return false;
}

bool MessageReceiver::proc_Submsg_Gap(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh,
        bool was_decoded) const
{
    eprosima::shared_lock<eprosima::shared_mutex> guard(mtx_);

    bool endiannessFlag = (smh->flags & BIT(0)) != 0;
    //Assign message endianness
    if (endiannessFlag)
    {
        msg->msg_endian = LITTLEEND;
    }
    else
    {
        msg->msg_endian = BIGEND;
    }

    GUID_t writerGUID;
    GUID_t readerGUID;
    readerGUID.guidPrefix = dest_guid_prefix_;
    CDRMessage::readEntityId(msg, &readerGUID.entityId);
    writerGUID.guidPrefix = source_guid_prefix_;
    CDRMessage::readEntityId(msg, &writerGUID.entityId);
    SequenceNumber_t gapStart;
    CDRMessage::readSequenceNumber(msg, &gapStart);
    SequenceNumberSet_t gapList = CDRMessage::readSequenceNumberSet(msg);
    if (gapStart <= SequenceNumber_t(0, 0))
    {
        return false;
    }

    findAllReaders(readerGUID.entityId,
            [was_decoded, &writerGUID, &gapStart, &gapList, this](BaseReader* reader)
            {
                // Only used when HAVE_SECURITY is defined
                static_cast<void>(was_decoded);
#if HAVE_SECURITY
                if (was_decoded || !reader->getAttributes().security_attributes().is_submessage_protected)
#endif  // HAVE_SECURITY
                {
                    reader->process_gap_msg(writerGUID, gapStart, gapList, source_vendor_id_);
                }
            });

    return true;
}

bool MessageReceiver::proc_Submsg_InfoTS(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
    std::lock_guard<eprosima::shared_mutex> guard(mtx_);

    bool endiannessFlag = (smh->flags & BIT(0)) != 0;
    bool timeFlag = (smh->flags & BIT(1)) != 0;
    //Assign message endianness
    if (endiannessFlag)
    {
        msg->msg_endian = LITTLEEND;
    }
    else
    {
        msg->msg_endian = BIGEND;
    }
    if (!timeFlag)
    {
        have_timestamp_ = true;
        CDRMessage::readTimestamp(msg, &timestamp_);
    }
    else
    {
        have_timestamp_ = false;
    }

    return true;
}

bool MessageReceiver::proc_Submsg_InfoDST(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
    std::lock_guard<eprosima::shared_mutex> guard(mtx_);

    bool endiannessFlag = (smh->flags & BIT(0)) != 0u;
    //bool timeFlag = smh->flags & BIT(1) ? true : false;
    //Assign message endianness
    if (endiannessFlag)
    {
        msg->msg_endian = LITTLEEND;
    }
    else
    {
        msg->msg_endian = BIGEND;
    }
    GuidPrefix_t guidP;
    CDRMessage::readData(msg, guidP.value, GuidPrefix_t::size);
    if (guidP != c_GuidPrefix_Unknown)
    {
        dest_guid_prefix_ = guidP;
        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "DST RTPSParticipant is now: " << dest_guid_prefix_);
    }
    return true;
}

bool MessageReceiver::proc_Submsg_InfoSRC(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
    std::lock_guard<eprosima::shared_mutex> guard(mtx_);

    bool endiannessFlag = (smh->flags & BIT(0)) != 0;
    //bool timeFlag = smh->flags & BIT(1) ? true : false;
    //Assign message endianness
    if (endiannessFlag)
    {
        msg->msg_endian = LITTLEEND;
    }
    else
    {
        msg->msg_endian = BIGEND;
    }
    if (smh->submessageLength == INFO_SRC_SUBMSG_LENGTH)
    {
        //AVOID FIRST 4 BYTES:
        msg->pos += 4;
        CDRMessage::readOctet(msg, &source_version_.m_major);
        CDRMessage::readOctet(msg, &source_version_.m_minor);
        CDRMessage::readData(msg, &source_vendor_id_[0], 2);
        CDRMessage::readData(msg, source_guid_prefix_.value, GuidPrefix_t::size);
        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "SRC RTPSParticipant is now: " << source_guid_prefix_);
        return true;
    }
    return false;
}

bool MessageReceiver::proc_Submsg_NackFrag(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh,
        bool was_decoded) const
{
    // Only used when HAVE_SECURITY is defined
    static_cast<void>(was_decoded);

    eprosima::shared_lock<eprosima::shared_mutex> guard(mtx_);

    bool endiannessFlag = (smh->flags & BIT(0)) != 0;
    //Assign message endianness
    if (endiannessFlag)
    {
        msg->msg_endian = LITTLEEND;
    }
    else
    {
        msg->msg_endian = BIGEND;
    }

    GUID_t readerGUID;
    GUID_t writerGUID;
    readerGUID.guidPrefix = source_guid_prefix_;
    CDRMessage::readEntityId(msg, &readerGUID.entityId);
    writerGUID.guidPrefix = dest_guid_prefix_;
    CDRMessage::readEntityId(msg, &writerGUID.entityId);

    SequenceNumber_t writerSN;
    CDRMessage::readSequenceNumber(msg, &writerSN);

    FragmentNumberSet_t fnState;
    CDRMessage::readFragmentNumberSet(msg, &fnState);

    uint32_t Ackcount;
    if (!CDRMessage::readUInt32(msg, &Ackcount))
    {
        EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Unable to read ackcount from message");
        return false;
    }

    //Look for the correct writer to use the acknack
    for (BaseWriter* it : associated_writers_)
    {
#if HAVE_SECURITY
        if (was_decoded || !it->getAttributes().security_attributes().is_submessage_protected)
#endif  // HAVE_SECURITY
        {
            bool result;
            if (it->process_nack_frag(writerGUID, readerGUID, Ackcount, writerSN, fnState, result, source_vendor_id_))
            {
                if (!result)
                {
                    EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Acknack msg to NOT stateful writer ");
                }
                return result;
            }
        }
    }
    EPROSIMA_LOG_INFO(RTPS_MSG_IN, IDSTRING "Acknack msg to UNKNOWN writer (I looked through "
            << associated_writers_.size() << " writers in this ListenResource)");
    return false;
}

bool MessageReceiver::proc_Submsg_HeartbeatFrag(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh,
        bool was_decoded) const
{
    // TODO: Add support for HEARTBEAT_FRAG submessage
    static_cast<void>(msg);
    static_cast<void>(smh);
    static_cast<void>(was_decoded);

    return true;
}

void MessageReceiver::notify_network_statistics(
        const Locator_t& source_locator,
        const Locator_t& reception_locator,
        CDRMessage_t* msg) const
{
    static_cast<void>(source_locator);
    static_cast<void>(reception_locator);
    static_cast<void>(msg);

#ifdef FASTDDS_STATISTICS
    using namespace eprosima::fastdds::statistics;
    using namespace eprosima::fastdds::statistics::rtps;

    if ((c_VendorId_eProsima != source_vendor_id_) ||
            (LOCATOR_KIND_SHM == source_locator.kind))
    {
        return;
    }

    // Keep track of current position, so we can restore it later.
    auto initial_pos = msg->pos;
    auto msg_length = msg->length;
    while (msg->pos < msg_length)
    {
        SubmessageHeader_t header;
        if (!readSubmessageHeader(msg, &header))
        {
            break;
        }

        if (FASTDDS_STATISTICS_NETWORK_SUBMESSAGE == header.submessageId)
        {
            // Check submessage validity
            if ((statistics_submessage_data_length != header.submessageLength) ||
                    ((msg->pos + header.submessageLength) > msg_length))
            {
                break;
            }

            StatisticsSubmessageData data;
            read_statistics_submessage(msg, data);
#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
            participant_->on_network_statistics(
                source_guid_prefix_, source_locator, reception_locator, data, msg_length);
#endif // if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
            break;
        }

        if (header.is_last)
        {
            break;
        }
        msg->pos += (header.submessageLength + 3u) & ~3u;
    }

    msg->pos = initial_pos;
#endif // FASTDDS_STATISTICS
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
