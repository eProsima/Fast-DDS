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

bool RTPSMessageCreator::addMessageGap(CDRMessage_t* msg, const GuidPrefix_t& guidprefix,
        const GuidPrefix_t& remoteGuidPrefix,
        const SequenceNumber_t& seqNumFirst, const SequenceNumberSet_t& seqNumList,
        const EntityId_t& readerId,const EntityId_t& writerId)
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

bool RTPSMessageCreator::addSubmessageGap(CDRMessage_t* msg, const SequenceNumber_t& seqNumFirst,
        const SequenceNumberSet_t& seqNumList,const EntityId_t& readerId,const EntityId_t& writerId)
{
    CDRMessage_t& submsgElem = g_pool_submsg.reserve_CDRMsg();
    CDRMessage::initCDRMsg(&submsgElem);
    octet flags = 0x0;
#if __BIG_ENDIAN__
    submsgElem.msg_endian = BIGEND;
#else
    flags = flags | BIT(0);
    submsgElem.msg_endian   = LITTLEEND;
#endif

    try{
        CDRMessage::addEntityId(&submsgElem,&readerId);
        CDRMessage::addEntityId(&submsgElem,&writerId);
        //Add Sequence Number
        CDRMessage::addSequenceNumber(&submsgElem,&seqNumFirst);
        CDRMessage::addSequenceNumberSet(&submsgElem,&seqNumList);
    }
    catch(int e)
    {
        logError(RTPS_CDR_MSG,"Gap submessage error"<<e<<endl)
            return false;
    }


    //Once the submessage elements are added, the header is created
    RTPSMessageCreator::addSubmessageHeader(msg, GAP, flags, (uint16_t)submsgElem.length);
    //Append Submessage elements to msg
    CDRMessage::appendMsg(msg, &submsgElem);
    g_pool_submsg.release_CDRMsg(submsgElem);

    return true;
}

}
}
}
