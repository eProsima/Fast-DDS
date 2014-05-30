/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * AckNackMsg.hpp
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */


namespace eprosima{
namespace rtps{

bool RTPSMessageCreator::addMessageAcknack(CDRMessage_t* msg,const GuidPrefix_t& guidprefix,
		const EntityId_t& readerId,const EntityId_t& writerId,
		SequenceNumberSet_t& SNSet,int32_t count,bool finalFlag){

	try
	{
		RTPSMessageCreator::addHeader(msg,guidprefix);
		RTPSMessageCreator::addSubmessageAcknack(msg,readerId, writerId,SNSet,count,finalFlag);
		msg->length = msg->pos;
	}
	catch(int e)
	{
		pError("Data message not created"<<e<<endl);
		return false;
	}
	return true;
}

bool RTPSMessageCreator::addSubmessageAcknack(CDRMessage_t* msg,
		const EntityId_t& readerId,const EntityId_t& writerId,
		SequenceNumberSet_t& SNSet,int32_t count,bool finalFlag)
{
	CDRMessage_t& submsgElem = g_pool_submsg.reserve_Object();
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
		pError("Message creator fails"<<e<<endl)
		return false;
	}

	//Once the submessage elements are added, the header is created
	RTPSMessageCreator::addSubmessageHeader(msg,ACKNACK,flags,submsgElem.length);
	//Append Submessage elements to msg

	CDRMessage::appendMsg(msg, &submsgElem);

	g_pool_submsg.release_Object(submsgElem);
	msg->length = msg->pos;
	return true;
}


}
}




