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

/*
 * HeartbeatMsg.hpp
 *
 */

namespace eprosima {
namespace fastrtps {
namespace rtps {

bool RTPSMessageCreator::addMessageHeartbeat(
        CDRMessage_t* msg, 
        const GuidPrefix_t& guidprefix,
        const EntityId_t& readerId,
        const EntityId_t& writerId,
        const SequenceNumber_t& firstSN,
        const SequenceNumber_t& lastSN,
        Count_t count,
        bool isFinal,
        bool livelinessFlag)
{
    try
    {
        RTPSMessageCreator::addHeader(msg,guidprefix);
        RTPSMessageCreator::addSubmessageHeartbeat(msg,readerId, writerId,firstSN,lastSN,count,isFinal,livelinessFlag);
        msg->length = msg->pos;
    }
    catch(int e)
    {
        logError(RTPS_CDR_MSG,"HB message not created"<<e<<endl)
            return false;
    }
    return true;
}

bool RTPSMessageCreator::addMessageHeartbeat(
        CDRMessage_t* msg,
        const GuidPrefix_t& guidprefix,
        const GuidPrefix_t& remoteGuidprefix,
        const EntityId_t& readerId,
        const EntityId_t& writerId,
        const SequenceNumber_t& firstSN,
        const SequenceNumber_t& lastSN,
        Count_t count,
        bool isFinal, 
        bool livelinessFlag)
{
    try
    {
        RTPSMessageCreator::addHeader(msg,guidprefix);
        RTPSMessageCreator::addSubmessageInfoDST(msg, remoteGuidprefix);
        RTPSMessageCreator::addSubmessageHeartbeat(msg,readerId, writerId,firstSN,lastSN,count,isFinal,livelinessFlag);
        msg->length = msg->pos;
    }
    catch(int e)
    {
        logError(RTPS_CDR_MSG,"HB message not created"<<e<<endl)
            return false;
    }
    return true;
}

bool RTPSMessageCreator::addSubmessageHeartbeat(
        CDRMessage_t* msg,
        const EntityId_t& readerId,
        const EntityId_t& writerId,
        const SequenceNumber_t& firstSN,
        const SequenceNumber_t& lastSN,
        Count_t count,
        bool isFinal,
        bool livelinessFlag)
{
    octet flags = 0x0;

    Endianness_t old_endianess = msg->msg_endian;
#if __BIG_ENDIAN__
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian = LITTLEEND;
#endif

    if(isFinal)
        flags = flags | BIT(1);
    if(livelinessFlag)
        flags = flags | BIT(2);

    // Submessage header.
    CDRMessage::addOctet(msg, HEARTBEAT);
    CDRMessage::addOctet(msg, flags);
    uint32_t submessage_size_pos = msg->pos;
    uint16_t submessage_size = 0;
    CDRMessage::addUInt16(msg, submessage_size);
    uint32_t position_size_count_size = msg->pos;

    CDRMessage::addEntityId(msg, &readerId);
    CDRMessage::addEntityId(msg, &writerId);
    //Add Sequence Number
    CDRMessage::addSequenceNumber(msg, &firstSN);
    CDRMessage::addSequenceNumber(msg, &lastSN);
    CDRMessage::addInt32(msg,(int32_t)count);

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

    return true;
}

bool RTPSMessageCreator::addMessageHeartbeatFrag(
        CDRMessage_t* msg,
        const GuidPrefix_t& guidprefix,
        const EntityId_t& readerId,
        const EntityId_t& writerId,
        SequenceNumber_t& firstSN,
        FragmentNumber_t& lastFN,
        Count_t count)
{
    try
    {
        RTPSMessageCreator::addHeader(msg, guidprefix);
        RTPSMessageCreator::addSubmessageHeartbeatFrag(msg, readerId, writerId, firstSN, lastFN, count);
        msg->length = msg->pos;
    }
    catch (int e)
    {
        logError(RTPS_CDR_MSG, "HB message not created" << e << endl)
            return false;
    }
    return true;
}

bool RTPSMessageCreator::addSubmessageHeartbeatFrag(
        CDRMessage_t* msg,
        const EntityId_t& readerId,
        const EntityId_t& writerId,
        SequenceNumber_t& firstSN,
        FragmentNumber_t& lastFN,
        Count_t count)
{
    octet flags = 0x0;
    Endianness_t old_endianess = msg->msg_endian;
#if __BIG_ENDIAN__
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian = LITTLEEND;
#endif

    // Submessage header.
    CDRMessage::addOctet(msg, HEARTBEAT_FRAG);
    CDRMessage::addOctet(msg, flags);
    uint32_t submessage_size_pos = msg->pos;
    uint16_t submessage_size = 0;
    CDRMessage::addUInt16(msg, submessage_size);
    uint32_t position_size_count_size = msg->pos;

    CDRMessage::addEntityId(msg, &readerId);
    CDRMessage::addEntityId(msg, &writerId);
    //Add Sequence Number
    CDRMessage::addSequenceNumber(msg, &firstSN);
    CDRMessage::addUInt32(msg, (uint32_t)lastFN);
    CDRMessage::addInt32(msg, (int32_t)count);

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

    return true;
}

}
}
}
