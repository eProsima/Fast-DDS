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
 * GapMsg.hpp
 *
 */

namespace eprosima{
namespace fastrtps{
namespace rtps{

bool RTPSMessageCreator::addMessageGap(
        CDRMessage_t* msg,
        const GuidPrefix_t& guidprefix,
        const GuidPrefix_t& remoteGuidPrefix,
        const SequenceNumber_t& seqNumFirst,
        const SequenceNumberSet_t& seqNumList,
        const EntityId_t& readerId,
        const EntityId_t& writerId)
{
    try
    {
        RTPSMessageCreator::addHeader(msg, guidprefix);
        RTPSMessageCreator::addSubmessageInfoDST(msg, remoteGuidPrefix);
        RTPSMessageCreator::addSubmessageInfoTS_Now(msg,false);
        RTPSMessageCreator::addSubmessageGap(msg,seqNumFirst,seqNumList,readerId, writerId);
    }
    catch(int e)
    {
        logError(RTPS_CDR_MSG,"Gap message error"<<e<<endl)
            return false;
    }
    return true;
}

bool RTPSMessageCreator::addSubmessageGap(
        CDRMessage_t* msg, 
        const SequenceNumber_t& seqNumFirst,
        const SequenceNumberSet_t& seqNumList,
        const EntityId_t& readerId,
        const EntityId_t& writerId)
{
    octet flags = 0x0;
    Endianness_t old_endianess = msg->msg_endian;
#if __BIG_ENDIAN__
    msg->msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    msg->msg_endian   = LITTLEEND;
#endif

    // Submessage header.
    CDRMessage::addOctet(msg, GAP);
    CDRMessage::addOctet(msg, flags);
    uint32_t submessage_size_pos = msg->pos;
    uint16_t submessage_size = 0;
    CDRMessage::addUInt16(msg, submessage_size);
    uint32_t position_size_count_size = msg->pos;

    CDRMessage::addEntityId(msg, &readerId);
    CDRMessage::addEntityId(msg, &writerId);
    //Add Sequence Number
    CDRMessage::addSequenceNumber(msg, &seqNumFirst);
    CDRMessage::addSequenceNumberSet(msg, &seqNumList);

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
