/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * HeartbeatMsg.hpp
 *
 */


namespace eprosima{
namespace fastrtps{
namespace rtps{

bool RTPSMessageCreator::addMessageHeartbeat(CDRMessage_t* msg,const GuidPrefix_t& guidprefix,const EntityId_t& readerId,const EntityId_t& writerId,
		SequenceNumber_t& firstSN,SequenceNumber_t& lastSN, Count_t count,bool isFinal,bool livelinessFlag)
{
	const char* const METHOD_NAME = "addMessageHeartbeat";
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

bool RTPSMessageCreator::addSubmessageHeartbeat(CDRMessage_t* msg,const EntityId_t& readerId,
		const EntityId_t& writerId,SequenceNumber_t& firstSN,SequenceNumber_t& lastSN, Count_t count,bool isFinal,bool livelinessFlag)
{
	CDRMessage_t& submsgElem = g_pool_submsg.reserve_CDRMsg();
	CDRMessage::initCDRMsg(&submsgElem);

	octet flags = 0x0;
#if EPROSIMA_BIG_ENDIAN
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
	const char* const METHOD_NAME = "addMessageHeartbeatFrag";
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
#if EPROSIMA_BIG_ENDIAN
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

