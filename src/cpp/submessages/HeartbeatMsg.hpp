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
	memcpy(H.guidPrefix,guidprefix,12);
	try{
		createHeader(msg,&H);
		CDRMessage_t submsg;
		createSubmessageHeartbeat(&submsg,HBSubM);
		appendMsg(msg, &submsg);
	}
	catch(int e){
		return false;
	}
	return true;
}

bool CDRMessageCreator::createSubmessageHeartbeat(CDRMessage_t* msg,SubmsgHeartbeat_t* HBSubM){
	CDRMessage_t* submsg = new CDRMessage_t();
	initCDRMsg(submsg);
	submsg->msg_endian = HBSubM->SubmessageHeader.flags[0] ? BIGEND : LITTLEEND;
	try{
		addEntityId(submsg,&HBSubM->readerId);
		addEntityId(submsg,&HBSubM->writerId);
		//Add Sequence Number
		addSequenceNumber(submsg,&HBSubM->firstSN);
		addSequenceNumber(submsg,&HBSubM->lastSN);
		addLong(submsg,HBSubM->count);
	}
	catch(int t){
		return false;
	}
	HBSubM->SubmessageHeader.flags[0] = HBSubM->endiannessFlag;
	HBSubM->SubmessageHeader.flags[1] = HBSubM->finalFlag;
	HBSubM->SubmessageHeader.flags[2] = HBSubM->livelinessFlag;
	for (uint i =7;i>=3;i--)
		HBSubM->SubmessageHeader.flags[i] = false;
	//Once the submessage elements are added, the header is created
	createSubmessageHeader(msg, &HBSubM->SubmessageHeader,submsg->w_pos);
	//Append Submessage elements to msg
	appendMsg(msg, submsg);
	return true;
}


}
}

