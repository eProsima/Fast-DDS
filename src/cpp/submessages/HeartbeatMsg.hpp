/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * HeartbeatMsg.hpp
 *
 */


namespace eprosima{
namespace rtps{

bool RTPSMessageCreator::addMessageHeartbeat(CDRMessage_t* msg,const GuidPrefix_t& guidprefix,const EntityId_t& readerId,const EntityId_t& writerId,
		SequenceNumber_t& firstSN,SequenceNumber_t& lastSN, Count_t count,bool isFinal,bool livelinessFlag)
{
	const char* const METHOD_NAME = "addSubmessageData";
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
	const char* const METHOD_NAME = "addSubmessageData";
	CDRMessage_t& submsgElem = g_pool_submsg.reserve_CDRMsg();
	CDRMessage::initCDRMsg(&submsgElem);

	octet flags = 0x0;
	if(EPROSIMA_ENDIAN == LITTLEEND)
	{
		flags = flags | BIT(0);
		submsgElem.msg_endian  = LITTLEEND;
	}
	else
	{
		submsgElem.msg_endian  = BIGEND;
	}
	if(isFinal)
		flags = flags | BIT(1);
	if(livelinessFlag)
		flags = flags | BIT(2);


	try{
		CDRMessage::addEntityId(&submsgElem,&readerId);
		CDRMessage::addEntityId(&submsgElem,&writerId);
		//Add Sequence Number
		CDRMessage::addSequenceNumber(&submsgElem,&firstSN);
		CDRMessage::addSequenceNumber(&submsgElem,&lastSN);
		CDRMessage::addInt32(&submsgElem,(int32_t)count);
	}
	catch(int e)
	{
		logError(RTPS_CDR_MSG,"MessageCreator fails"<<e<<endl)
		return false;
	}



	//Once the submessage elements are added, the header is created
	RTPSMessageCreator::addSubmessageHeader(msg, HEARTBEAT,flags,submsgElem.length);
	//Append Submessage elements to msg
	//Append Submessage elements to msg
	CDRMessage::appendMsg(msg, &submsgElem);
	g_pool_submsg.release_CDRMsg(submsgElem);
	return true;
}


}
}

