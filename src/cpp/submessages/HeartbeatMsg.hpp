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
		CDRMessage::appendMsg(msg, &submsg);
		msg->length = msg->pos;
	}
	catch(int e)
	{
		cout << " Message creator fails: " << e << DEF<< endl;
		return false;
	}
	return true;
}

bool CDRMessageCreator::createSubmessageHeartbeat(CDRMessage_t* msg,SubmsgHeartbeat_t* HBSubM){
	CDRMessage_t* submsg = new CDRMessage_t();
	CDRMessage::initCDRMsg(submsg);
	try{
		CDRMessage::addEntityId(submsg,&HBSubM->readerId);
		CDRMessage::addEntityId(submsg,&HBSubM->writerId);
		//Add Sequence Number
		CDRMessage::addSequenceNumber(submsg,&HBSubM->firstSN);
		CDRMessage::addSequenceNumber(submsg,&HBSubM->lastSN);
		CDRMessage::addInt32(submsg,HBSubM->count);
	}
	catch(int t)
	{
		cout << " Message creator fails: " << t << DEF<< endl;
		return false;
	}
	HBSubM->SubmessageHeader.flags = 0x0;
	if(EPROSIMA_ENDIAN == BIGEND)
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

	HBSubM->SubmessageHeader.submessageLength = submsg->pos;
	HBSubM->SubmessageHeader.submessageId = HEARTBEAT;
	//Once the submessage elements are added, the header is created
	createSubmessageHeader(msg, &HBSubM->SubmessageHeader,submsg->pos);
	//Append Submessage elements to msg
	CDRMessage::appendMsg(msg, submsg);
	msg->length = msg->pos;
	return true;
}


}
}

