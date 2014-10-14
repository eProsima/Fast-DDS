/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * AckNackMsg.hpp
 *
 */


namespace eprosima{
namespace rtps{

bool RTPSMessageCreator::addMessageAcknack(CDRMessage_t* msg,const GuidPrefix_t& guidprefix,
		const EntityId_t& readerId,const EntityId_t& writerId,
		SequenceNumberSet_t& SNSet,int32_t count,bool finalFlag){
	const char* const METHOD_NAME = "addMessageAcknack";
	try
	{
		RTPSMessageCreator::addHeader(msg,guidprefix);
		RTPSMessageCreator::addSubmessageAcknack(msg,readerId, writerId,SNSet,count,finalFlag);
		msg->length = msg->pos;
	}
	catch(int e)
	{
		logError(LOG_CATEGORY::RTPS_CDR_MSG,"Data message not created"<<e<<endl);
		return false;
	}
	return true;
}

bool RTPSMessageCreator::addSubmessageAcknack(CDRMessage_t* msg,
		const EntityId_t& readerId,const EntityId_t& writerId,
		SequenceNumberSet_t& SNSet,int32_t count,bool finalFlag)
{
	const char* const METHOD_NAME = "addSubmessageAcknack";
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
		submsgElem.msg_endian =  BIGEND;
	}
	if(finalFlag)
		flags = flags | BIT(1);


	try{
		CDRMessage::addEntityId(&submsgElem,&readerId);
		CDRMessage::addEntityId(&submsgElem,&writerId);
		//Add Sequence Number
		CDRMessage::addSequenceNumberSet(&submsgElem,&SNSet);
		CDRMessage::addInt32(&submsgElem,count);
	}
	catch(int e)
	{
		logError(LOG_CATEGORY::RTPS_CDR_MSG,"Message creator fails"<<e<<endl)
		return false;
	}

	//Once the submessage elements are added, the header is created
	RTPSMessageCreator::addSubmessageHeader(msg,ACKNACK,flags,submsgElem.length);
	//Append Submessage elements to msg

	CDRMessage::appendMsg(msg, &submsgElem);

	g_pool_submsg.release_CDRMsg(submsgElem);
	msg->length = msg->pos;
	return true;
}


}
}




