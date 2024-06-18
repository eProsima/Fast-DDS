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
 * DataMsg.hpp
 *
 */

#include <fastdds/dds/core/policy/ParameterTypes.hpp>

#include <fastdds/core/policy/ParameterSerializer.hpp>
#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/rtps/transport/NetworkBuffer.hpp>

using NetworkBuffer = eprosima::fastdds::rtps::NetworkBuffer;

namespace eprosima {
namespace fastdds {
namespace rtps {

namespace {

struct DataMsgUtils
{
    static void prepare_submessage_flags(
            const CacheChange_t* change,
            TopicKind_t topicKind,
            bool expectsInlineQos,
            InlineQosWriter* inlineQos,
            bool& dataFlag,
            bool& keyFlag,
            bool& inlineQosFlag,
            octet& status)
    {
        inlineQosFlag =
                (nullptr != inlineQos) ||
                ((WITH_KEY == topicKind) &&
                (!change->writerGUID.is_builtin() || expectsInlineQos || change->kind != ALIVE)) ||
                (change->write_params.related_sample_identity() != SampleIdentity::unknown());

        dataFlag = ALIVE == change->kind &&
                change->serializedPayload.length > 0 && nullptr != change->serializedPayload.data;
        keyFlag = !dataFlag && !inlineQosFlag && (WITH_KEY == topicKind);

        status = 0;
        if (change->kind == NOT_ALIVE_DISPOSED)
        {
            status = status | BIT(0);
        }
        if (change->kind == NOT_ALIVE_UNREGISTERED)
        {
            status = status | BIT(1);
        }
        if (change->kind == NOT_ALIVE_DISPOSED_UNREGISTERED)
        {
            status = status | BIT(0);
            status = status | BIT(1);
        }
    }

    static bool serialize_header(
            uint32_t fragment_number,
            CDRMessage_t* msg,
            const CacheChange_t* change,
            const EntityId_t& readerId,
            octet flags,
            uint32_t& submessage_size_pos,
            uint32_t& position_size_count_size)
    {
        bool ok = true;
        bool is_fragment = fragment_number > 0;

        CDRMessage::addOctet(msg, is_fragment ? DATA_FRAG : DATA);
        CDRMessage::addOctet(msg, flags);
        submessage_size_pos = msg->pos;
        CDRMessage::addUInt16(msg, 0);
        position_size_count_size = msg->pos;

        //extra flags. not in this version.
        ok &= CDRMessage::addUInt16(msg, 0);
        //octet to inline Qos is 12 or 28, may change in future versions
        ok &= is_fragment ?
                CDRMessage::addUInt16(msg, RTPSMESSAGE_OCTETSTOINLINEQOS_DATAFRAGSUBMSG) :
                CDRMessage::addUInt16(msg, RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
        //Entity ids
        ok &= CDRMessage::addEntityId(msg, &readerId);
        ok &= CDRMessage::addEntityId(msg, &change->writerGUID.entityId);
        //Add Sequence Number
        ok &= CDRMessage::addSequenceNumber(msg, &change->sequenceNumber);

        if (is_fragment)
        {
            // Add fragment starting number
            ok &= CDRMessage::addUInt32(msg, fragment_number); // fragments start in 1

            // Add fragments in submessage
            ok &= CDRMessage::addUInt16(msg, 1); // we are sending one fragment

            // Add fragment size
            ok &= CDRMessage::addUInt16(msg, change->getFragmentSize());

            // Add total sample size
            ok &= CDRMessage::addUInt32(msg, change->serializedPayload.length);
        }

        return ok;
    }

    static void serialize_inline_qos(
            CDRMessage_t* msg,
            const CacheChange_t* change,
            TopicKind_t topicKind,
            bool expectsInlineQos,
            InlineQosWriter* inlineQos,
            octet status)
    {
        if (change->write_params.related_sample_identity() != SampleIdentity::unknown())
        {
            fastdds::dds::ParameterSerializer<fastdds::dds::Parameter_t>::add_parameter_sample_identity(msg,
                    change->write_params.related_sample_identity());
            fastdds::dds::ParameterSerializer<fastdds::dds::Parameter_t>::add_parameter_custom_related_sample_identity(
                msg,
                change->write_params.related_sample_identity());
        }

        if (WITH_KEY == topicKind && (!change->writerGUID.is_builtin() || expectsInlineQos || ALIVE != change->kind))
        {
            fastdds::dds::ParameterSerializer<fastdds::dds::Parameter_t>::add_parameter_key(msg,
                    change->instanceHandle);

            if (ALIVE != change->kind)
            {
                fastdds::dds::ParameterSerializer<fastdds::dds::Parameter_t>::add_parameter_status(msg, status);
            }
        }

        if (inlineQos != nullptr)
        {
            inlineQos->writeQosToCDRMessage(msg);
        }

        fastdds::dds::ParameterSerializer<fastdds::dds::Parameter_t>::add_parameter_sentinel(msg);
    }

};

}  // empty namespace

bool RTPSMessageCreator::addMessageData(
        CDRMessage_t* msg,
        GuidPrefix_t& guidprefix,
        const CacheChange_t* change,
        TopicKind_t topicKind,
        const EntityId_t& readerId,
        bool expectsInlineQos,
        InlineQosWriter* inlineQos)
{

    RTPSMessageCreator::addHeader(msg, guidprefix);

    RTPSMessageCreator::addSubmessageInfoTS_Now(msg, false);

    NetworkBuffer pending_buffer;
    uint8_t pending_padding = 0;

    bool is_big_submessage;
    RTPSMessageCreator::addSubmessageData(msg, change, topicKind, readerId, expectsInlineQos, inlineQos,
            is_big_submessage, true, pending_buffer, pending_padding);

    msg->length = msg->pos;

    return true;
}

bool RTPSMessageCreator::addSubmessageData(
        CDRMessage_t* msg,
        const CacheChange_t* change,
        TopicKind_t topicKind,
        const EntityId_t& readerId,
        bool expectsInlineQos,
        InlineQosWriter* inlineQos,
        bool& is_big_submessage,
        bool copy_data,
        NetworkBuffer& pending_buffer,
        uint8_t& pending_padding)
{
    octet status = 0;
    octet flags = 0;

    // Initialize output parameters
    is_big_submessage = false;
    pending_buffer = NetworkBuffer();
    pending_padding = 0;

    //Find out flags
    bool dataFlag = false;
    bool keyFlag = false;
    bool inlineQosFlag = false;

    Endianness_t old_endianess = msg->msg_endian;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian = LITTLEEND;
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

    DataMsgUtils::prepare_submessage_flags(change, topicKind, expectsInlineQos, inlineQos,
            dataFlag, keyFlag, inlineQosFlag, status);

    if (inlineQosFlag)
    {
        flags = flags | BIT(1);
    }

    if (dataFlag)
    {
        flags = flags | BIT(2);
    }

    if (keyFlag)
    {
        flags = flags | BIT(3);
    }

    // Submessage header.
    uint32_t submessage_size_pos = 0;
    uint16_t submessage_size = 0;
    uint32_t position_size_count_size = 0;
    bool added_no_error = DataMsgUtils::serialize_header(0, msg, change, readerId, flags,
                    submessage_size_pos, position_size_count_size);

    //Add INLINE QOS AND SERIALIZED PAYLOAD DEPENDING ON FLAGS:
    if (inlineQosFlag) //inlineQoS
    {
        DataMsgUtils::serialize_inline_qos(msg, change, topicKind, expectsInlineQos, inlineQos, status);
    }

    //Add Serialized Payload
    if (dataFlag)
    {
        if (copy_data)
        {
            added_no_error &=
                    CDRMessage::addData(msg, change->serializedPayload.data, change->serializedPayload.length);
        }
        else if (msg->pos + change->serializedPayload.length > msg->max_size)
        {
            return false;
        }
        else
        {
            pending_buffer = NetworkBuffer(change->serializedPayload.data, change->serializedPayload.length);
            msg->pos += pending_buffer.size;
        }
    }

    if (keyFlag)
    {
        added_no_error &= CDRMessage::addOctet(msg, 0); //ENCAPSULATION
        if (msg->msg_endian == BIGEND)
        {
            added_no_error &= CDRMessage::addOctet(msg, PL_CDR_BE); //ENCAPSULATION
        }
        else
        {
            added_no_error &= CDRMessage::addOctet(msg, PL_CDR_LE); //ENCAPSULATION
        }

        added_no_error &= CDRMessage::addUInt16(msg, 0); //ENCAPSULATION OPTIONS
        added_no_error &=
                fastdds::dds::ParameterSerializer<fastdds::dds::Parameter_t>::add_parameter_key(msg,
                        change->instanceHandle);
        added_no_error &=
                fastdds::dds::ParameterSerializer<fastdds::dds::Parameter_t>::add_parameter_status(msg,
                        status);
        added_no_error &= fastdds::dds::ParameterSerializer<fastdds::dds::Parameter_t>::add_parameter_sentinel(msg);
    }

    // Align submessage to rtps alignment (4).
    uint8_t align = (4 - msg->pos % 4) & 3;
    if (copy_data)
    {
        for (uint32_t count = 0; count < align; ++count)
        {
            added_no_error &= CDRMessage::addOctet(msg, 0);
        }
    }
    else
    {
        pending_padding = align;
        msg->pos += align;
    }

    uint32_t size32 = msg->pos - position_size_count_size;
    if (size32 <= std::numeric_limits<uint16_t>::max())
    {
        submessage_size = static_cast<uint16_t>(size32);
        octet* o = reinterpret_cast<octet*>(&submessage_size);
        if (msg->msg_endian == DEFAULT_ENDIAN)
        {
            msg->buffer[submessage_size_pos] = *(o);
            msg->buffer[submessage_size_pos + 1] = *(o + 1);
        }
        else
        {
            msg->buffer[submessage_size_pos] = *(o + 1);
            msg->buffer[submessage_size_pos + 1] = *(o);
        }
    }
    else
    {
        // Submessage > 64 KB
        is_big_submessage = true;
    }

    // Rewind position when not copying data. Needed for size checks.
    if (!copy_data)
    {
        msg->pos -= pending_padding;
        msg->pos -= pending_buffer.size;
    }

    msg->msg_endian = old_endianess;

    return added_no_error;
}

bool RTPSMessageCreator::addMessageDataFrag(
        CDRMessage_t* msg,
        GuidPrefix_t& guidprefix,
        const CacheChange_t* change,
        uint32_t fragment_number,
        TopicKind_t topicKind,
        const EntityId_t& readerId,
        bool expectsInlineQos,
        InlineQosWriter* inlineQos)
{
    RTPSMessageCreator::addHeader(msg, guidprefix);

    RTPSMessageCreator::addSubmessageInfoTS_Now(msg, false);

    // Calculate fragment start
    uint32_t fragment_start = change->getFragmentSize() * (fragment_number - 1);
    // Calculate fragment size. If last fragment, size may be smaller
    uint32_t fragment_size = fragment_number < change->getFragmentCount() ? change->getFragmentSize() :
            change->serializedPayload.length - fragment_start;

    // TODO (Ricardo). Check to create special wrapper.
    SerializedPayload_t payload;
    payload.data = change->serializedPayload.data + fragment_start;
    payload.length = fragment_size;

    NetworkBuffer pending_buffer;
    uint8_t pending_padding = 0;

    RTPSMessageCreator::addSubmessageDataFrag(msg, change, fragment_number, payload,
            topicKind, readerId, expectsInlineQos, inlineQos, true, pending_buffer, pending_padding);

    payload.data = NULL;

    msg->length = msg->pos;
    return true;
}

bool RTPSMessageCreator::addSubmessageDataFrag(
        CDRMessage_t* msg,
        const CacheChange_t* change,
        uint32_t fragment_number,
        const SerializedPayload_t& payload,
        TopicKind_t topicKind,
        const EntityId_t& readerId,
        bool expectsInlineQos,
        InlineQosWriter* inlineQos,
        bool copy_data,
        NetworkBuffer& pending_buffer,
        uint8_t& pending_padding)
{
    octet status = 0;
    octet flags = 0;
    //Find out flags
    bool dataFlag = false;
    bool keyFlag = false;
    bool inlineQosFlag = false;

    // Initialize output parameters
    pending_buffer = NetworkBuffer();
    pending_padding = 0;

    Endianness_t old_endianess = msg->msg_endian;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian = LITTLEEND;
#endif // if FASTDDS_IS_BIG_ENDIAN_TARGET

    DataMsgUtils::prepare_submessage_flags(change, topicKind, expectsInlineQos, inlineQos,
            dataFlag, keyFlag, inlineQosFlag, status);

    if (inlineQosFlag)
    {
        flags = flags | BIT(1);
    }

    if (keyFlag)
    {
        flags = flags | BIT(2);
    }

    // Submessage header.
    uint32_t submessage_size_pos = 0;
    uint16_t submessage_size = 0;
    uint32_t position_size_count_size = 0;
    bool added_no_error = DataMsgUtils::serialize_header(fragment_number, msg, change, readerId, flags,
                    submessage_size_pos, position_size_count_size);

    //Add INLINE QOS AND SERIALIZED PAYLOAD DEPENDING ON FLAGS:
    if (inlineQosFlag) //inlineQoS
    {
        DataMsgUtils::serialize_inline_qos(msg, change, topicKind, expectsInlineQos, inlineQos, status);
    }

    //Add Serialized Payload XXX TODO
    if (!keyFlag) // keyflag = 0 means that the serializedPayload SubmessageElement contains the serialized Data
    {
        if (copy_data)
        {
            added_no_error &= CDRMessage::addData(msg, payload.data, payload.length);
        }
        else if (msg->pos + payload.length > msg->max_size)
        {
            return false;
        }
        else
        {
            pending_buffer = NetworkBuffer(payload.data, payload.length);
            msg->pos += pending_buffer.size;
        }
    }
    else
    {
        // keyflag = 1 means that the serializedPayload SubmessageElement contains the serialized Key
        /*
            added_no_error &= CDRMessage::addOctet(&submsgElem, 0); //ENCAPSULATION
            if (submsgElem.msg_endian == BIGEND)
            added_no_error &= CDRMessage::addOctet(&submsgElem, PL_CDR_BE); //ENCAPSULATION
            else
            added_no_error &= CDRMessage::addOctet(&submsgElem, PL_CDR_LE); //ENCAPSULATION

            added_no_error &= CDRMessage::addUInt16(&submsgElem, 0); //ENCAPSULATION OPTIONS
            added_no_error &= Parameter_t::addParameterKey(&submsgElem, &change->instanceHandle);
            added_no_error &= Parameter_t::addParameterStatus(&submsgElem, status);
            added_no_error &= Parameter_t::addParameterSentinel(&submsgElem);
         */
        msg->msg_endian = old_endianess;
        return false;
    }

    // TODO(Ricardo) This should be on cachechange.
    // Align submessage to rtps alignment (4).
    submessage_size = uint16_t(msg->pos - position_size_count_size);
    for (; 0 != (submessage_size & 3); ++submessage_size)
    {
        if (copy_data)
        {
            added_no_error &= CDRMessage::addOctet(msg, 0);
        }
        else
        {
            ++pending_padding;
        }
    }

    if (!copy_data)
    {
        msg->pos -= pending_buffer.size;
    }

    //TODO(Ricardo) Improve.
    octet* o = reinterpret_cast<octet*>(&submessage_size);
    if (msg->msg_endian == DEFAULT_ENDIAN)
    {
        msg->buffer[submessage_size_pos] = *(o);
        msg->buffer[submessage_size_pos + 1] = *(o + 1);
    }
    else
    {
        msg->buffer[submessage_size_pos] = *(o + 1);
        msg->buffer[submessage_size_pos + 1] = *(o);
    }

    msg->msg_endian = old_endianess;

    return added_no_error;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
