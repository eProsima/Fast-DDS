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
 * DataSubMessage.hpp
 *
 */

namespace eprosima{
namespace fastrtps{
namespace rtps{

bool RTPSMessageCreator::addMessageData(
        CDRMessage_t* msg,
        GuidPrefix_t& guidprefix,
        const CacheChange_t* change,
        TopicKind_t topicKind,
        const EntityId_t& readerId,
        bool expectsInlineQos,
        InlineQosWriter* inlineQos)
{
    try
    {

        RTPSMessageCreator::addHeader(msg,guidprefix);

        RTPSMessageCreator::addSubmessageInfoTS_Now(msg,false);

        RTPSMessageCreator::addSubmessageData(msg, change,topicKind,readerId,expectsInlineQos,inlineQos);

        msg->length = msg->pos;
    }
    catch(int e)
    {
        logError(RTPS_CDR_MSG, "Data message error" << e << endl)

            return false;
    }

    return true;
}

bool RTPSMessageCreator::addSubmessageData(
        CDRMessage_t* msg,
        const CacheChange_t* change,
        TopicKind_t topicKind,
        const EntityId_t& readerId,
        bool expectsInlineQos,
        InlineQosWriter* inlineQos)
{
    octet flags = 0x0;
    //Find out flags
    bool dataFlag = false;
    bool keyFlag = false;
    bool inlineQosFlag = false;

    Endianness_t old_endianess = msg->msg_endian;
#if __BIG_ENDIAN__
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian = LITTLEEND;
#endif

    if(change->kind == ALIVE && change->serializedPayload.length>0 && change->serializedPayload.data!=NULL)
    {
        dataFlag = true;
        keyFlag = false;
    }
    else
    {
        dataFlag = false;
        keyFlag = true;
    }
    if(topicKind == NO_KEY)
        keyFlag = false;
    inlineQosFlag = false;
    // cout << "expects inline qos: " << expectsInlineQos << " topic KIND: " << (topicKind == WITH_KEY) << endl;
    if(inlineQos != NULL || expectsInlineQos || change->kind != ALIVE) //expects inline qos
    {
        if(topicKind == WITH_KEY)
        {
            flags = flags | BIT(1);
            inlineQosFlag = true;
            //cout << "INLINE QOS FLAG TO 1 " << endl;
            keyFlag = false;
        }
    }
    // Maybe the inline QoS because a WriteParam.
    else if(change->write_params.related_sample_identity() != SampleIdentity::unknown())
    {
        inlineQosFlag = true;
        flags = flags | BIT(1);
    }

    if(dataFlag)
        flags = flags | BIT(2);
    if(keyFlag)
        flags = flags | BIT(3);

    octet status = 0;
    if(change->kind == NOT_ALIVE_DISPOSED)
        status = status | BIT(0);
    if(change->kind == NOT_ALIVE_UNREGISTERED)
        status = status | BIT(1);
    if(change->kind == NOT_ALIVE_DISPOSED_UNREGISTERED)
    {
        status = status | BIT(0);
        status = status | BIT(1);
    }

    bool added_no_error = true;
    try
    {
        // Submessage header.
        CDRMessage::addOctet(msg, DATA);
        CDRMessage::addOctet(msg, flags);
        uint32_t submessage_size_pos = msg->pos;
        uint16_t submessage_size = 0;
        CDRMessage::addUInt16(msg, submessage_size);
        uint32_t position_size_count_size = msg->pos;

        //extra flags. not in this version.
        added_no_error &= CDRMessage::addUInt16(msg, 0);
        //octet to inline Qos is 12, may change in future versions
        added_no_error &= CDRMessage::addUInt16(msg, RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
        //Entity ids
        added_no_error &= CDRMessage::addEntityId(msg, &readerId);
        added_no_error &= CDRMessage::addEntityId(msg, &change->writerGUID.entityId);
        //Add Sequence Number
        added_no_error &= CDRMessage::addSequenceNumber(msg, &change->sequenceNumber);
        //Add INLINE QOS AND SERIALIZED PAYLOAD DEPENDING ON FLAGS:


        if(inlineQosFlag) //inlineQoS
        {
            if(change->write_params.related_sample_identity() != SampleIdentity::unknown())
            {
                CDRMessage::addParameterSampleIdentity(msg, change->write_params.related_sample_identity());
            }

            if(topicKind == WITH_KEY)
            {
                //cout << "ADDDING PARAMETER KEY " << endl;
                CDRMessage::addParameterKey(msg, &change->instanceHandle);
            }

            if(change->kind != ALIVE)
            {
                CDRMessage::addParameterStatus(msg, status);
            }

            if (inlineQos != nullptr)
            {
                inlineQos->writeQosToCDRMessage(msg);
            }

            CDRMessage::addParameterSentinel(msg);
        }

        //Add Serialized Payload
        if(dataFlag)
            added_no_error &= CDRMessage::addData(msg, change->serializedPayload.data, change->serializedPayload.length);

        if(keyFlag)
        {
            added_no_error &= CDRMessage::addOctet(msg, 0); //ENCAPSULATION
            if(msg->msg_endian == BIGEND)
            {
                added_no_error &= CDRMessage::addOctet(msg, PL_CDR_BE); //ENCAPSULATION
            }
            else
            {
                added_no_error &= CDRMessage::addOctet(msg, PL_CDR_LE); //ENCAPSULATION
            }

            added_no_error &= CDRMessage::addUInt16(msg, 0); //ENCAPSULATION OPTIONS
            added_no_error &= CDRMessage::addParameterKey(msg, &change->instanceHandle);
            added_no_error &= CDRMessage::addParameterStatus(msg, status);
            added_no_error &= CDRMessage::addParameterSentinel(msg);
        }

        // Align submessage to rtps alignment (4).
        uint32_t align = (4 - msg->pos % 4) & 3;
        for(uint32_t count = 0; count < align; ++count)
            added_no_error &= CDRMessage::addOctet(msg, 0);

        //if(align > 0)
        {
            //submsgElem.pos += align;
            //submsgElem.length += align;
        }


        //TODO(Ricardo) Improve.
        submessage_size = msg->pos - position_size_count_size;
        octet* o= (octet*)&submessage_size;
        if(msg->msg_endian == DEFAULT_ENDIAN)
        {
            msg->buffer[submessage_size_pos] = *(o);
            msg->buffer[submessage_size_pos+1] = *(o+1);
        }
        else
        {
            msg->buffer[submessage_size_pos] = *(o+1);
            msg->buffer[submessage_size_pos+1] = *(o);
        }

        msg->msg_endian = old_endianess;
    }
    catch(int t){
        logError(RTPS_CDR_MSG,"Data SUBmessage not created"<<t<<endl)

            return false;
    }
    return added_no_error;
}


bool RTPSMessageCreator::addMessageDataFrag(CDRMessage_t* msg, GuidPrefix_t& guidprefix,
        const CacheChange_t* change, uint32_t fragment_number, TopicKind_t topicKind, const EntityId_t& readerId,
        bool expectsInlineQos, InlineQosWriter* inlineQos)
{

    try{

        RTPSMessageCreator::addHeader(msg, guidprefix);

        RTPSMessageCreator::addSubmessageInfoTS_Now(msg, false);

        // Calculate fragment start
        uint32_t fragment_start = change->getFragmentSize() * (fragment_number - 1);
        // Calculate fragment size. If last fragment, size may be smaller
        uint32_t fragment_size = fragment_number < change->getFragmentCount() ? change->getFragmentSize() :
            change->serializedPayload.length - fragment_start;

        // TODO (Ricardo). Check to create special wrapper.
        CacheChange_t change_to_add;
        change_to_add.copy_not_memcpy(change);
        change_to_add.serializedPayload.data = change->serializedPayload.data + fragment_start;
        change_to_add.serializedPayload.length = fragment_size;

        RTPSMessageCreator::addSubmessageDataFrag(msg, &change_to_add, fragment_number, change->serializedPayload.length,
                topicKind, readerId, expectsInlineQos, inlineQos);

        change_to_add.serializedPayload.data = NULL;

        msg->length = msg->pos;
    }
    catch (int e)
    {
        logError(RTPS_CDR_MSG, "Data message error" << e << endl)
            return false;
    }
    return true;
}



bool RTPSMessageCreator::addSubmessageDataFrag(
        CDRMessage_t* msg,
        const CacheChange_t* change,
        uint32_t fragment_number,
        uint32_t sample_size,
        TopicKind_t topicKind,
        const EntityId_t& readerId,
        bool expectsInlineQos,
        InlineQosWriter* inlineQos)
{
    octet flags = 0x0;
    //Find out flags
    bool keyFlag = false;
    bool inlineQosFlag = false;

    Endianness_t old_endianess = msg->msg_endian;
#if __BIG_ENDIAN__
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian = LITTLEEND;
#endif

    if (change->kind == ALIVE && change->serializedPayload.length>0 && change->serializedPayload.data != NULL)
    {
        keyFlag = false;
    }
    else
    {
        keyFlag = true;
    }

    if (topicKind == NO_KEY)
        keyFlag = false;

    // cout << "expects inline qos: " << expectsInlineQos << " topic KIND: " << (topicKind == WITH_KEY) << endl;
    if (inlineQos != NULL || expectsInlineQos || change->kind != ALIVE) //expects inline qos
    {
        if (topicKind == WITH_KEY)
        {
            flags = flags | BIT(1);
            inlineQosFlag = true;
            //cout << "INLINE QOS FLAG TO 1 " << endl;
            keyFlag = false;
        }
    }
    // Maybe the inline QoS because a WriteParam.
    else if (change->write_params.related_sample_identity() != SampleIdentity::unknown())
    {
        inlineQosFlag = true;
        flags = flags | BIT(1);
    }

    if (keyFlag)
        flags = flags | BIT(2);

    octet status = 0;
    if (change->kind == NOT_ALIVE_DISPOSED)
        status = status | BIT(0);
    if (change->kind == NOT_ALIVE_UNREGISTERED)
        status = status | BIT(1);

    if (change->kind == NOT_ALIVE_DISPOSED_UNREGISTERED)
    {
        status = status | BIT(0);
        status = status | BIT(1);
    }

    bool added_no_error = true;

    try
    {
        // Submessage header.
        CDRMessage::addOctet(msg, DATA_FRAG);
        CDRMessage::addOctet(msg, flags);
        uint32_t submessage_size_pos = msg->pos;
        uint16_t submessage_size = 0;
        CDRMessage::addUInt16(msg, submessage_size);
        uint32_t position_size_count_size = msg->pos;

        //extra flags. not in this version.
        added_no_error &= CDRMessage::addUInt16(msg, 0);

        //octet to inline Qos is 28, may change in future versions
        added_no_error &= CDRMessage::addUInt16(msg, RTPSMESSAGE_OCTETSTOINLINEQOS_DATAFRAGSUBMSG);

        //Entity ids
        added_no_error &= CDRMessage::addEntityId(msg, &readerId);
        added_no_error &= CDRMessage::addEntityId(msg, &change->writerGUID.entityId);

        //Add Sequence Number
        added_no_error &= CDRMessage::addSequenceNumber(msg, &change->sequenceNumber);

        // Add fragment starting number
        added_no_error &= CDRMessage::addUInt32(msg, fragment_number); // fragments start in 1

        // Add fragments in submessage
        added_no_error &= CDRMessage::addUInt16(msg, 1); // we are sending one fragment

        // Add fragment size
        added_no_error &= CDRMessage::addUInt16(msg, change->getFragmentSize());

        // Add total sample size
        added_no_error &= CDRMessage::addUInt32(msg, sample_size); //TODO(Ricardo) Sample size in CacheChange

        //Add INLINE QOS AND SERIALIZED PAYLOAD DEPENDING ON FLAGS:
        if (inlineQosFlag) //inlineQoS
        {
            if(change->write_params.related_sample_identity() != SampleIdentity::unknown())
            {
                CDRMessage::addParameterSampleIdentity(msg, change->write_params.related_sample_identity());
            }

            if(topicKind == WITH_KEY)
            {
                CDRMessage::addParameterKey(msg,&change->instanceHandle);
            }

            if(change->kind != ALIVE)
            {
                CDRMessage::addParameterStatus(msg,status);
            }

            if (inlineQos != nullptr)
            {
                inlineQos->writeQosToCDRMessage(msg);
            }

            CDRMessage::addParameterSentinel(msg);
        }

        //Add Serialized Payload XXX TODO
        if (!keyFlag) // keyflag = 0 means that the serializedPayload SubmessageElement contains the serialized Data 
        {
            added_no_error &= CDRMessage::addData(msg, change->serializedPayload.data,
                    change->serializedPayload.length);
        }
        else
        {   // keyflag = 1 means that the serializedPayload SubmessageElement contains the serialized Key 
            /*
               added_no_error &= CDRMessage::addOctet(&submsgElem, 0); //ENCAPSULATION
               if (submsgElem.msg_endian == BIGEND)
               added_no_error &= CDRMessage::addOctet(&submsgElem, PL_CDR_BE); //ENCAPSULATION
               else
               added_no_error &= CDRMessage::addOctet(&submsgElem, PL_CDR_LE); //ENCAPSULATION

               added_no_error &= CDRMessage::addUInt16(&submsgElem, 0); //ENCAPSULATION OPTIONS
               added_no_error &= CDRMessage::addParameterKey(&submsgElem, &change->instanceHandle);
               added_no_error &= CDRMessage::addParameterStatus(&submsgElem, status);
               added_no_error &= CDRMessage::addParameterSentinel(&submsgElem);
               */
            msg->msg_endian = old_endianess;
            return false;
        }

        // TODO(Ricardo) This should be on cachechange.
        // Align submessage to rtps alignment (4).
        uint32_t align = (4 - msg->pos % 4) & 3;
        for (uint32_t count = 0; count < align; ++count)
            added_no_error &= CDRMessage::addOctet(msg, 0);

        //TODO(Ricardo) Improve.
        submessage_size = msg->pos - position_size_count_size;
        octet* o= (octet*)&submessage_size;
        if(msg->msg_endian == DEFAULT_ENDIAN)
        {
            msg->buffer[submessage_size_pos] = *(o);
            msg->buffer[submessage_size_pos+1] = *(o+1);
        }
        else
        {
            msg->buffer[submessage_size_pos] = *(o+1);
            msg->buffer[submessage_size_pos+1] = *(o);
        }

        msg->msg_endian = old_endianess;
    }
    catch (int t){
        logError(RTPS_CDR_MSG, "Data SUBmessage not created" << t << endl)
            return false;
    }

    return added_no_error;
}


}
}
}
