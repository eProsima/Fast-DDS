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

bool RTPSMessageCreator::addMessageHeartbeat(CDRMessage_t* msg, const GuidPrefix_t& guidprefix,
        const EntityId_t& readerId, const EntityId_t& writerId,
        const SequenceNumber_t& firstSN, const SequenceNumber_t& lastSN,
        Count_t count, bool isFinal, bool livelinessFlag)
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

bool RTPSMessageCreator::addMessageHeartbeat(CDRMessage_t* msg,const GuidPrefix_t& guidprefix,
        const GuidPrefix_t& remoteGuidprefix, const EntityId_t& readerId,const EntityId_t& writerId,
        const SequenceNumber_t& firstSN, const SequenceNumber_t& lastSN,
        Count_t count, bool isFinal, bool livelinessFlag)
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

bool RTPSMessageCreator::addSubmessageHeartbeat(CDRMessage_t* msg, const EntityId_t& readerId,
        const EntityId_t& writerId, const SequenceNumber_t& firstSN, const SequenceNumber_t& lastSN,
        Count_t count, bool isFinal, bool livelinessFlag)
{
    CDRMessage_t& submsgElem = g_pool_submsg.reserve_CDRMsg();
    CDRMessage::initCDRMsg(&submsgElem);

    octet flags = 0x0;
#if __BIG_ENDIAN__
    submsgElem.msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    submsgElem.msg_endian  = LITTLEEND;
#endif

    if(isFinal)
        flags = flags | BIT(1);
    if(livelinessFlag)
        flags = flags | BIT(2);


    CDRMessage::addEntityId(&submsgElem,&readerId);
    CDRMessage::addEntityId(&submsgElem,&writerId);
    //Add Sequence Number
    CDRMessage::addSequenceNumber(&submsgElem,&firstSN);
    CDRMessage::addSequenceNumber(&submsgElem,&lastSN);
    CDRMessage::addInt32(&submsgElem,(int32_t)count);

    //Once the submessage elements are added, the header is created
    RTPSMessageCreator::addSubmessageHeader(msg, HEARTBEAT, flags, (uint16_t)submsgElem.length);
    //Append Submessage elements to msg
    //Append Submessage elements to msg
    CDRMessage::appendMsg(msg, &submsgElem);
    g_pool_submsg.release_CDRMsg(submsgElem);
    return true;
}

bool RTPSMessageCreator::addMessageHeartbeatFrag(CDRMessage_t* msg, const GuidPrefix_t& guidprefix, const EntityId_t& readerId, const EntityId_t& writerId,
        SequenceNumber_t& firstSN, FragmentNumber_t& lastFN, Count_t count)
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

bool RTPSMessageCreator::addSubmessageHeartbeatFrag(CDRMessage_t* msg, const EntityId_t& readerId,
        const EntityId_t& writerId, SequenceNumber_t& firstSN, FragmentNumber_t& lastFN, Count_t count)
{
    CDRMessage_t& submsgElem = g_pool_submsg.reserve_CDRMsg();
    CDRMessage::initCDRMsg(&submsgElem);

    octet flags = 0x0;
#if __BIG_ENDIAN__
    submsgElem.msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    submsgElem.msg_endian = LITTLEEND;
#endif

    CDRMessage::addEntityId(&submsgElem, &readerId);
    CDRMessage::addEntityId(&submsgElem, &writerId);
    //Add Sequence Number
    CDRMessage::addSequenceNumber(&submsgElem, &firstSN);
    CDRMessage::addUInt32(&submsgElem, (uint32_t)lastFN);
    CDRMessage::addInt32(&submsgElem, (int32_t)count);

    //Once the submessage elements are added, the header is created
    RTPSMessageCreator::addSubmessageHeader(msg, HEARTBEAT_FRAG, flags, (uint16_t)submsgElem.length);
    //Append Submessage elements to msg
    //Append Submessage elements to msg
    CDRMessage::appendMsg(msg, &submsgElem);
    g_pool_submsg.release_CDRMsg(submsgElem);
    return true;
}

}
}
}
