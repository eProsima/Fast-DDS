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

bool CDRMessageCreator::createMessageAcknack(CDRMessage_t* msg,GuidPrefix_t guidprefix,
									SubmsgAcknack_t* SubM){
	Header_t H = Header_t();
	H.guidPrefix = guidprefix;
	try{
		createHeader(msg,&H);
		CDRMessage_t submsg;
		createSubmessageAcknack(&submsg,SubM);
		CDRMessage::appendMsg(msg, &submsg);
		msg->length = msg->pos;
	}
	catch(int e){
		return false;
	}
	return true;
}

bool CDRMessageCreator::createSubmessageAcknack(CDRMessage_t* msg,SubmsgAcknack_t* SubM){
	CDRMessage_t* submsg = new CDRMessage_t();
	CDRMessage::initCDRMsg(submsg);
	try{
		CDRMessage::addEntityId(submsg,&SubM->readerId);
		CDRMessage::addEntityId(submsg,&SubM->writerId);
		//Add Sequence Number
		CDRMessage::addSequenceNumberSet(submsg,&SubM->readerSNState);
		CDRMessage::addInt32(submsg,SubM->count);
	}
	catch(int t){
		return false;
	}
	SubM->SubmessageHeader.flags = 0x0;
	for(int i=7;i>=0;i--)
	{
		if(SubM->endiannessFlag)
		{
			SubM->SubmessageHeader.flags = SubM->SubmessageHeader.flags | BIT(0);
			submsg->msg_endian = BIGEND;
		}
		else
		{
			submsg->msg_endian = LITTLEEND;
		}
		if(SubM->finalFlag)
			SubM->SubmessageHeader.flags = SubM->SubmessageHeader.flags | BIT(1);
	}

	//Once the submessage elements are added, the header is created
	createSubmessageHeader(msg, &SubM->SubmessageHeader,submsg->pos);
	//Append Submessage elements to msg
	CDRMessage::appendMsg(msg, submsg);
	msg->length = msg->pos;
	return true;
}


}
}




