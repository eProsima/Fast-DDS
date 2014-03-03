/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * GapMsg.hpp
 *
 *  Created on: Feb 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */



namespace eprosima{
namespace rtps{

bool CDRMessageCreator::createMessageGap(CDRMessage_t* msg,GuidPrefix_t guidprefix,
									SubmsgGap_t* SubM){
	Header_t H = Header_t();
	H.guidPrefix = guidprefix;
	try{
		createHeader(msg,&H);
		CDRMessage_t submsg;
		createSubmessageGap(&submsg,SubM);
		CDRMessage::appendMsg(msg, &submsg);
		msg->length = msg->pos;
	}
	catch(int e){
		return false;
	}
	return true;
}

bool CDRMessageCreator::createSubmessageGap(CDRMessage_t* msg,SubmsgGap_t* SubM){
	CDRMessage_t* submsg = new CDRMessage_t();
	CDRMessage::initCDRMsg(submsg);
	try{
		CDRMessage::addEntityId(submsg,&SubM->readerId);
		CDRMessage::addEntityId(submsg,&SubM->writerId);
		//Add Sequence Number
		CDRMessage::addSequenceNumber(submsg,&SubM->gapStart);
		CDRMessage::addSequenceNumberSet(submsg,&SubM->gapList);
	}
	catch(int t){
		return false;
	}
	SubM->SubmessageHeader.flags = 0x0;
	if(EPROSIMA_ENDIAN == BIGEND)
	{
		SubM->SubmessageHeader.flags = SubM->SubmessageHeader.flags | BIT(0);
		submsg->msg_endian = BIGEND;
	}
	else
	{
		submsg->msg_endian = LITTLEEND;
	}

	SubM->SubmessageHeader.submessageLength = submsg->pos;
	SubM->SubmessageHeader.submessageId = GAP;
	//Once the submessage elements are added, the header is created
	createSubmessageHeader(msg, &SubM->SubmessageHeader,submsg->pos);
	//Append Submessage elements to msg
	CDRMessage::appendMsg(msg, submsg);
	msg->length = msg->pos;
	return true;
}


}
}
