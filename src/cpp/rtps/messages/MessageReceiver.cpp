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

#include <fastdds/rtps/messages/MessageReceiver.h>

#include <cassert>
#include <limits>
#include <mutex>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/dds/log/Log.hpp>

#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/writer/RTPSWriter.h>

#include <rtps/participant/RTPSParticipantImpl.h>
#include <statistics/rtps/StatisticsBase.hpp>
#include <statistics/rtps/messages/RTPSStatisticsMessages.hpp>

#define INFO_SRC_SUBMSG_LENGTH 20

#define IDSTRING "(ID:" << std::this_thread::get_id() << ") " <<

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastrtps {
namespace rtps {

MessageReceiver::MessageReceiver(
        RTPSParticipantImpl* participant,
        uint32_t rec_buffer_size)
    : participant_(participant)
    , source_version_(c_ProtocolVersion)
    , source_vendor_id_(c_VendorId_Unknown)
    , source_guid_prefix_(c_GuidPrefix_Unknown)
    , dest_guid_prefix_(c_GuidPrefix_Unknown)
    , have_timestamp_(false)
    , timestamp_(c_TimeInvalid)
#if HAVE_SECURITY
    , crypto_msg_(participant->is_secure() ? rec_buffer_size : 0)
    , crypto_submsg_(participant->is_secure() ? rec_buffer_size : 0)
    , crypto_payload_(participant->is_secure() ? rec_buffer_size : 0)
#endif // if HAVE_SECURITY
{
    (void)rec_buffer_size;
    logInfo(RTPS_MSG_IN, "Created with CDRMessage of size: " << rec_buffer_size);

#if HAVE_SECURITY
    if (participant->is_secure())
    {
        process_data_message_function_ = std::bind(
            &MessageReceiver::process_data_message_with_security,
            this,
            std::placeholders::_1,
            std::placeholders::_2);

        process_data_fragment_message_function_ = std::bind(
            &MessageReceiver::process_data_fragment_message_with_security,
            this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3,
            std::placeholders::_4,
            std::placeholders::_5);
    }
    else
    {
#endif // if HAVE SECURITY
    process_data_message_function_ = std::bind(
        &MessageReceiver::process_data_message_without_security,
        this,
        std::placeholders::_1,
        std::placeholders::_2);

    process_data_fragment_message_function_ = std::bind(
        &MessageReceiver::process_data_fragment_message_without_security,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4,
        std::placeholders::_5);
#if HAVE_SECURITY
}

#endif // if HAVE SECURITY
}

MessageReceiver::~MessageReceiver()
{
    logInfo(RTPS_MSG_IN, "");
    assert(associated_writers_.empty());
    assert(associated_readers_.empty());
}

 #if HAVE_SECURITY
void MessageReceiver::process_data_message_with_security(
        const EntityId_t& reader_id,
        CacheChange_t& change)
{
    auto process_message = [&change, this](RTPSReader* reader)
            {
                if (!reader->getAttributes().security_attributes().is_payload_protected)
                {
                    reader->processDataMsg(&change);
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

                SerializedPayload_t original_payload = change.serializedPayload;
                reader->processDataMsg(&change);
                IPayloadPool* payload_pool = change.payload_owner();
                if (payload_pool)
                {
                    payload_pool->release_payload(change);
                    change.serializedPayload = original_payload;
                }
                original_payload.data = nullptr;
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
        uint16_t fragments_in_submessage)
{
    auto process_message =
            [&change, sample_size, fragment_starting_num, fragments_in_submessage, this](RTPSReader* reader)
            {
                if (!reader->getAttributes().security_attributes().is_payload_protected)
                {
                    reader->processDataFragMsg(&change, sample_size, fragment_starting_num, fragments_in_submessage);
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
                reader->processDataFragMsg(&change, sample_size, fragment_starting_num, fragments_in_submessage);
                std::swap(change.serializedPayload.data, crypto_payload_.data);
                std::swap(change.serializedPayload.length, crypto_payload_.length);
            };

    findAllReaders(reader_id, process_message);
}

#endif // if HAVE SECURITY

void MessageReceiver::process_data_message_without_security(
        const EntityId_t& reader_id,
        CacheChange_t& change)
{
    auto process_message = [&change](RTPSReader* reader)
            {
                reader->processDataMsg(&change);
            };

    findAllReaders(reader_id, process_message);
}

void MessageReceiver::process_data_fragment_message_without_security(
        const EntityId_t& reader_id,
        CacheChange_t& change,
        uint32_t sample_size,
        uint32_t fragment_starting_num,
        uint16_t fragments_in_submessage)
{
    auto process_message = [&change, sample_size, fragment_starting_num, fragments_in_submessage](RTPSReader* reader)
            {
                reader->processDataFragMsg(&change, sample_size, fragment_starting_num, fragments_in_submessage);
            };

    findAllReaders(reader_id, process_message);
}

void MessageReceiver::associateEndpoint(
        Endpoint* to_add)
{
    std::lock_guard<std::mutex> guard(mtx_);
    if (to_add->getAttributes().endpointKind == WRITER)
    {
        const auto writer = dynamic_cast<RTPSWriter*>(to_add);
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
        const auto reader = dynamic_cast<RTPSReader*>(to_add);
        const auto entityId = reader->getGuid().entityId;
        // search for set of readers by entity ID
        const auto readers = associated_readers_.find(entityId);
        if (readers == associated_readers_.end())
        {
            auto vec = std::vector<RTPSReader*>();
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
    std::lock_guard<std::mutex> guard(mtx_);

    if (to_remove->getAttributes().endpointKind == WRITER)
    {
        auto* var = dynamic_cast<RTPSWriter*>(to_remove);
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
            auto* var = dynamic_cast<RTPSReader*>(to_remove);
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
    timestamp_ = c_TimeInvalid;
}

void MessageReceiver::processCDRMsg(
        const Locator_t& source_locator,
        const Locator_t& reception_locator,
        CDRMessage_t* msg)
{
    if (msg->length < RTPSMESSAGE_HEADER_SIZE)
    {
        logWarning(RTPS_MSG_IN, IDSTRING "Received message too short, ignoring");
        return;
    }

    reset();

#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    GuidPrefix_t participantGuidPrefix;
#else
    GuidPrefix_t participantGuidPrefix = participant_->getGuid().guidPrefix;
#endif // ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    dest_guid_prefix_ = participantGuidPrefix;

    msg->pos = 0; //Start reading at 0

    //Once everything is set, the reading begins:
    if (!checkRTPSHeader(msg))
    {
        return;
    }

    notify_network_statistics(source_locator, reception_locator, msg);

#if HAVE_SECURITY
    security::SecurityManager& security = participant_->security_manager();
    CDRMessage_t* auxiliary_buffer = &crypto_msg_;

    int decode_ret = security.decode_rtps_message(*msg, *auxiliary_buffer, source_guid_prefix_);

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
#endif // if HAVE_SECURITY

    // Loop until there are no more submessages
    bool valid;
    SubmessageHeader_t submsgh; //Current submessage header

    while (msg->pos < msg->length)// end of the message
    {
        CDRMessage_t* submessage = msg;

#if HAVE_SECURITY
        decode_ret = security.decode_rtps_submessage(*msg, *auxiliary_buffer, source_guid_prefix_);

        if (decode_ret < 0)
        {
            return;
        }

        if (decode_ret == 0)
        {
            submessage = auxiliary_buffer;
        }
#endif // if HAVE_SECURITY

        //First 4 bytes must contain: ID | flags | octets to next header
        if (!readSubmessageHeader(submessage, &submsgh))
        {
            return;
        }

        valid = true;
        uint32_t next_msg_pos = submessage->pos;
        next_msg_pos += (submsgh.submessageLength + 3u) & ~3u;
        switch (submsgh.submessageId)
        {
            case DATA:
            {
                if (dest_guid_prefix_ != participantGuidPrefix)
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "Data Submsg ignored, DST is another RTPSParticipant");
                }
                else
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "Data Submsg received, processing.");
                    valid = proc_Submsg_Data(submessage, &submsgh);
                }
                break;
            }
            case DATA_FRAG:
                if (dest_guid_prefix_ != participantGuidPrefix)
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "DataFrag Submsg ignored, DST is another RTPSParticipant");
                }
                else
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "DataFrag Submsg received, processing.");
                    valid = proc_Submsg_DataFrag(submessage, &submsgh);
                }
                break;
            case GAP:
            {
                if (dest_guid_prefix_ != participantGuidPrefix)
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "Gap Submsg ignored, DST is another RTPSParticipant...");
                }
                else
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "Gap Submsg received, processing...");
                    valid = proc_Submsg_Gap(submessage, &submsgh);
                }
                break;
            }
            case ACKNACK:
            {
                if (dest_guid_prefix_ != participantGuidPrefix)
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "Acknack Submsg ignored, DST is another RTPSParticipant...");
                }
                else
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "Acknack Submsg received, processing...");
                    valid = proc_Submsg_Acknack(submessage, &submsgh);
                }
                break;
            }
            case NACK_FRAG:
            {
                if (dest_guid_prefix_ != participantGuidPrefix)
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "NackFrag Submsg ignored, DST is another RTPSParticipant...");
                }
                else
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "NackFrag Submsg received, processing...");
                    valid = proc_Submsg_NackFrag(submessage, &submsgh);
                }
                break;
            }
            case HEARTBEAT:
            {
                if (dest_guid_prefix_ != participantGuidPrefix)
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "HB Submsg ignored, DST is another RTPSParticipant...");
                }
                else
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "Heartbeat Submsg received, processing...");
                    valid = proc_Submsg_Heartbeat(submessage, &submsgh);
                }
                break;
            }
            case HEARTBEAT_FRAG:
            {
                if (dest_guid_prefix_ != participantGuidPrefix)
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "HBFrag Submsg ignored, DST is another RTPSParticipant...");
                }
                else
                {
                    logInfo(RTPS_MSG_IN, IDSTRING "HeartbeatFrag Submsg received, processing...");
                    valid = proc_Submsg_HeartbeatFrag(submessage, &submsgh);
                }
                break;
            }
            case PAD:
                logWarning(RTPS_MSG_IN, IDSTRING "PAD messages not yet implemented, ignoring");
                break;
            case INFO_DST:
                logInfo(RTPS_MSG_IN, IDSTRING "InfoDST message received, processing...");
                valid = proc_Submsg_InfoDST(submessage, &submsgh);
                break;
            case INFO_SRC:
                logInfo(RTPS_MSG_IN, IDSTRING "InfoSRC message received, processing...");
                valid = proc_Submsg_InfoSRC(submessage, &submsgh);
                break;
            case INFO_TS:
            {
                logInfo(RTPS_MSG_IN, IDSTRING "InfoTS Submsg received, processing...");
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

        if (!valid || submsgh.is_last)
        {
            break;
        }

        submessage->pos = next_msg_pos;
    }

#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    participant_->assert_remote_participant_liveliness(source_guid_prefix_);
#endif // ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
}

bool MessageReceiver::checkRTPSHeader(
        CDRMessage_t* msg)
{
    //check and proccess the RTPS Header
    if (msg->buffer[0] != 'R' ||  msg->buffer[1] != 'T' ||
            msg->buffer[2] != 'P' ||  msg->buffer[3] != 'S')
    {
        logInfo(RTPS_MSG_IN, IDSTRING "Msg received with no RTPS in header, ignoring...");
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
        logWarning(RTPS_MSG_IN, IDSTRING "Major RTPS Version not supported");
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
        SubmessageHeader_t* smh)
{
    if (msg->length - msg->pos < 4)
    {
        logWarning(RTPS_MSG_IN, IDSTRING "SubmessageHeader too short");
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
        logWarning(RTPS_MSG_IN, IDSTRING "SubMsg of invalid length (" << length <<
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
        RTPSReader*& first_reader)
{
    first_reader = nullptr;
    if (associated_readers_.empty())
    {
        logWarning(RTPS_MSG_IN, IDSTRING "Data received when NO readers are listening");
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
            for (const auto& it : readers.second)
            {
                if (it->m_acceptMessagesToUnknownReaders)
                {
                    first_reader = it;
                    return true;
                }
            }
        }
    }

    logWarning(RTPS_MSG_IN, IDSTRING "No Reader accepts this message (directed to: " << readerID << ")");
    return false;
}

template<typename Functor>
void MessageReceiver::findAllReaders(
        const EntityId_t& readerID,
        const Functor& callback)
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
                if (it->m_acceptMessagesToUnknownReaders)
                {
                    callback(it);
                }
            }
        }
    }
}

bool MessageReceiver::proc_Submsg_Data(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
    std::lock_guard<std::mutex> guard(mtx_);

    //READ and PROCESS
    if (smh->submessageLength < RTPSMESSAGE_DATA_MIN_LENGTH)
    {
        logInfo(RTPS_MSG_IN, IDSTRING "Too short submessage received, ignoring");
        return false;
    }
    //Fill flags bool values
    bool endiannessFlag = (smh->flags & BIT(0)) != 0;
    bool inlineQosFlag = (smh->flags & BIT(1)) != 0;
    bool dataFlag = (smh->flags & BIT(2)) != 0;
    bool keyFlag = (smh->flags & BIT(3)) != 0;
    if (keyFlag && dataFlag)
    {
        logWarning(RTPS_MSG_IN, IDSTRING "Message received with Data and Key Flag set, ignoring");
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
    RTPSReader* first_reader = nullptr;
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

    //Get sequence number
    valid &= CDRMessage::readSequenceNumber(msg, &ch.sequenceNumber);

    if (!valid)
    {
        return false;
    }

    if (ch.sequenceNumber <= SequenceNumber_t())
    {
        logWarning(RTPS_MSG_IN, IDSTRING "Invalid message received, bad sequence Number");
        return false;
    }

    //Jump ahead if more parameters are before inlineQos (not in this version, maybe if further minor versions.)
    if (octetsToInlineQos > RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG)
    {
        msg->pos += (octetsToInlineQos - RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
        if (msg->pos > msg->length)
        {
            logWarning(RTPS_MSG_IN,
                    IDSTRING "Invalid jump through msg, msg->pos " << msg->pos << " > msg->length " << msg->length);
            return false;
        }
    }

    uint32_t inlineQosSize = 0;

    if (inlineQosFlag)
    {
        if (!ParameterList::updateCacheChangeFromInlineQos(ch, msg, inlineQosSize))
        {
            logInfo(RTPS_MSG_IN, IDSTRING "SubMessage Data ERROR, Inline Qos ParameterList error");
            return false;
        }
    }

    if (dataFlag || keyFlag)
    {
        uint32_t payload_size;
        payload_size = smh->submessageLength -
                (RTPSMESSAGE_DATA_EXTRA_INLINEQOS_SIZE + octetsToInlineQos + inlineQosSize);

        if (dataFlag)
        {
            uint32_t next_pos = msg->pos + payload_size;
            if (msg->length >= next_pos && payload_size > 0)
            {
                ch.serializedPayload.data = &msg->buffer[msg->pos];
                ch.serializedPayload.length = payload_size;
                ch.serializedPayload.max_size = payload_size;
                msg->pos = next_pos;
            }
            else
            {
                logWarning(RTPS_MSG_IN, IDSTRING "Serialized Payload value invalid or larger than maximum allowed size"
                        "(" << payload_size << "/" << (msg->length - msg->pos) << ")");
                return false;
            }
        }
        else if (keyFlag)
        {
            if (payload_size <= 0)
            {
                logWarning(RTPS_MSG_IN, IDSTRING "Serialized Payload value invalid (" << payload_size << ")");
                return false;
            }

            if (payload_size <= PARAMETER_KEY_HASH_LENGTH)
            {
                memcpy(ch.instanceHandle.value, &msg->buffer[msg->pos], payload_size);
            }
            else
            {
                logWarning(RTPS_MSG_IN, IDSTRING "Ignoring Serialized Payload for too large key-only data (" <<
                        payload_size << ")");
            }
            msg->pos += payload_size;
        }
    }

    // Set sourcetimestamp
    if (have_timestamp_)
    {
        ch.sourceTimestamp = timestamp_;
    }

    logInfo(RTPS_MSG_IN, IDSTRING "from Writer " << ch.writerGUID << "; possible RTPSReader entities: " <<
            associated_readers_.size());

    //Look for the correct reader to add the change
    process_data_message_function_(readerID, ch);

    IPayloadPool* payload_pool = ch.payload_owner();
    if (payload_pool)
    {
        payload_pool->release_payload(ch);
    }

    //TODO(Ricardo) If a exception is thrown (ex, by fastcdr), this line is not executed -> segmentation fault
    ch.serializedPayload.data = nullptr;

    logInfo(RTPS_MSG_IN, IDSTRING "Sub Message DATA processed");
    return true;
}

bool MessageReceiver::proc_Submsg_DataFrag(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
    std::lock_guard<std::mutex> guard(mtx_);

    //READ and PROCESS
    if (smh->submessageLength < RTPSMESSAGE_DATA_MIN_LENGTH)
    {
        logInfo(RTPS_MSG_IN, IDSTRING "Too short submessage received, ignoring");
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
    RTPSReader* first_reader = nullptr;
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
        logWarning(RTPS_MSG_IN, IDSTRING "Invalid message received, bad sequence Number");
        return false;
    }

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
            logWarning(RTPS_MSG_IN,
                    IDSTRING "Invalid jump through msg, msg->pos " << msg->pos << " > msg->length " << msg->length);
            return false;
        }
    }

    uint32_t inlineQosSize = 0;

    if (inlineQosFlag)
    {
        if (!ParameterList::updateCacheChangeFromInlineQos(ch, msg, inlineQosSize))
        {
            logInfo(RTPS_MSG_IN, IDSTRING "SubMessage Data ERROR, Inline Qos ParameterList error");
            return false;
        }
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
            logWarning(RTPS_MSG_IN, IDSTRING "Serialized Payload value invalid or larger than maximum allowed size "
                    "(" << payload_size << "/" << (msg->length - msg->pos) << ")");
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
           logError(RTPS_MSG_IN, IDSTRING"Bad encapsulation for KeyHash and status parameter list");
           return false;
           }
           //uint32_t param_size;
           if (ParameterList::readParameterListfromCDRMsg(msg, &m_ParamList, ch, false) <= 0)
           {
           logInfo(RTPS_MSG_IN, IDSTRING"SubMessage Data ERROR, keyFlag ParameterList");
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

    logInfo(RTPS_MSG_IN, IDSTRING "from Writer " << ch.writerGUID << "; possible RTPSReader entities: " <<
            associated_readers_.size());
    process_data_fragment_message_function_(readerID, ch, sampleSize, fragmentStartingNum, fragmentsInSubmessage);
    ch.serializedPayload.data = nullptr;

    logInfo(RTPS_MSG_IN, IDSTRING "Sub Message DATA_FRAG processed");

    return true;
}

bool MessageReceiver::proc_Submsg_Heartbeat(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
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
    if (lastSN < firstSN && lastSN != firstSN - 1)
    {
        logWarning(RTPS_MSG_IN, IDSTRING "Invalid Heartbeat received (" << firstSN << ") - (" <<
                lastSN << "), ignoring");
        return false;
    }
    uint32_t HBCount;
    if (!CDRMessage::readUInt32(msg, &HBCount))
    {
        logWarning(RTPS_MSG_IN, IDSTRING "Unable to read heartbeat count from heartbeat message");
        return false;
    }

    std::lock_guard<std::mutex> guard(mtx_);
    //Look for the correct reader and writers:
    findAllReaders(readerGUID.entityId,
            [&writerGUID, &HBCount, &firstSN, &lastSN, finalFlag, livelinessFlag](RTPSReader* reader)
            {
                reader->processHeartbeatMsg(writerGUID, HBCount, firstSN, lastSN, finalFlag, livelinessFlag);
            });

    return true;
}

bool MessageReceiver::proc_Submsg_Acknack(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
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
        logWarning(RTPS_MSG_IN, IDSTRING "Unable to read ackcount from message");
        return false;
    }

    std::lock_guard<std::mutex> guard(mtx_);
    //Look for the correct writer to use the acknack
    for (RTPSWriter* it : associated_writers_)
    {
        bool result;
        if (it->process_acknack(writerGUID, readerGUID, Ackcount, SNSet, finalFlag, result))
        {
            if (!result)
            {
                logInfo(RTPS_MSG_IN, IDSTRING "Acknack msg to NOT stateful writer ");
            }
            return result;
        }
    }
    logInfo(RTPS_MSG_IN, IDSTRING "Acknack msg to UNKNOWN writer (I looked through "
            << associated_writers_.size() << " writers in this ListenResource)");
    return false;
}

bool MessageReceiver::proc_Submsg_Gap(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
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

    std::lock_guard<std::mutex> guard(mtx_);
    findAllReaders(readerGUID.entityId,
            [&writerGUID, &gapStart, &gapList](RTPSReader* reader)
            {
                reader->processGapMsg(writerGUID, gapStart, gapList);
            });

    return true;
}

bool MessageReceiver::proc_Submsg_InfoTS(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
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
        logInfo(RTPS_MSG_IN, IDSTRING "DST RTPSParticipant is now: " << dest_guid_prefix_);
    }
    return true;
}

bool MessageReceiver::proc_Submsg_InfoSRC(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
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
        logInfo(RTPS_MSG_IN, IDSTRING "SRC RTPSParticipant is now: " << source_guid_prefix_);
        return true;
    }
    return false;
}

bool MessageReceiver::proc_Submsg_NackFrag(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
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
        logInfo(RTPS_MSG_IN, IDSTRING "Unable to read ackcount from message");
        return false;
    }

    std::lock_guard<std::mutex> guard(mtx_);
    //Look for the correct writer to use the acknack
    for (RTPSWriter* it : associated_writers_)
    {
        bool result;
        if (it->process_nack_frag(writerGUID, readerGUID, Ackcount, writerSN, fnState, result))
        {
            if (!result)
            {
                logInfo(RTPS_MSG_IN, IDSTRING "Acknack msg to NOT stateful writer ");
            }
            return result;
        }
    }
    logInfo(RTPS_MSG_IN, IDSTRING "Acknack msg to UNKNOWN writer (I looked through "
            << associated_writers_.size() << " writers in this ListenResource)");
    return false;
}

bool MessageReceiver::proc_Submsg_HeartbeatFrag(
        CDRMessage_t* msg,
        SubmessageHeader_t* smh)
{
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
    readerGUID.guidPrefix = dest_guid_prefix_;
    CDRMessage::readEntityId(msg, &readerGUID.entityId);
    writerGUID.guidPrefix = source_guid_prefix_;
    CDRMessage::readEntityId(msg, &writerGUID.entityId);

    SequenceNumber_t writerSN;
    CDRMessage::readSequenceNumber(msg, &writerSN);

    FragmentNumber_t lastFN;
    CDRMessage::readUInt32(msg, static_cast<uint32_t*>(&lastFN));

    uint32_t HBCount;
    CDRMessage::readUInt32(msg, &HBCount);

    // XXX TODO VALIDATE DATA?

    //Look for the correct reader and writers:
    /* XXX TODO
       std::lock_guard<std::mutex> guard(mtx_);
       for (std::vector<RTPSReader*>::iterator it = associated_readers_.begin();
            it != associated_readers_.end(); ++it)
       {
           if ((*it)->acceptMsgDirectedTo(readerGUID.entityId))
           {
           (*it)->processHeartbeatMsg(writerGUID, HBCount, firstSN, lastSN, finalFlag, livelinessFlag);
           }
       }
     */
    return true;
}

void MessageReceiver::notify_network_statistics(
        const Locator_t& source_locator,
        const Locator_t& reception_locator,
        CDRMessage_t* msg)
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
            participant_->on_network_statistics(
                source_guid_prefix_, source_locator, reception_locator, data, msg_length);
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
} /* namespace fastrtps */
} /* namespace eprosima */
