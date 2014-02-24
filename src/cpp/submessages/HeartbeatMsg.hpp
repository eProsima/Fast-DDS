/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * HeartbeatMsg.hpp
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */


namespace eprosima{
namespace rtps{

bool CDRMessageCreator::createMessageHeartbeat(CDRMessage_t* msg,GuidPrefix_t guidprefix,
									SubmsgHeartbeat_t* HBSubM){
	Header_t H = Header_t();
	H.guidPrefix = guidprefix;
	try{
		createHeader(msg,&H);
		CDRMessage_t submsg;
		createSubmessageHeartbeat(&submsg,HBSubM);
		appendMsg(msg, &submsg);
		msg->length = msg->pos;
	}
	catch(int e){
		return false;
	}
	return true;
}

bool CDRMessageCreator::createSubmessageHeartbeat(CDRMessage_t* msg,SubmsgHeartbeat_t* HBSubM){
	CDRMessage_t* submsg = new CDRMessage_t();
	initCDRMsg(submsg);
	try{
		addEntityId(submsg,&HBSubM->readerId);
		addEntityId(submsg,&HBSubM->writerId);
		//Add Sequence Number
		addSequenceNumber(submsg,&HBSubM->firstSN);
		addSequenceNumber(submsg,&HBSubM->lastSN);
		addInt32(submsg,HBSubM->count);
	}
	catch(int t){
		return false;
	}
	HBSubM->SubmessageHeader.flags = 0x0;
	for(int i=7;i>=0;i--)
	{
		if(HBSubM->endiannessFlag)
		{
			HBSubM->SubmessageHeader.flags = HBSubM->SubmessageHeader.flags | BIT(0);
			submsg->msg_endian = BIGEND;
		}
		else
		{
			submsg->msg_endian = LITTLEEND;
		}
		if(HBSubM->finalFlag)
			HBSubM->SubmessageHeader.flags = HBSubM->SubmessageHeader.flags | BIT(1);
		if(HBSubM->livelinessFlag)
			HBSubM->SubmessageHeader.flags = HBSubM->SubmessageHeader.flags | BIT(2);
	}

	//Once the submessage elements are added, the header is created
	createSubmessageHeader(msg, &HBSubM->SubmessageHeader,submsg->pos);
	//Append Submessage elements to msg
	appendMsg(msg, submsg);
	msg->length = msg->pos;
	return true;
}


}
}

