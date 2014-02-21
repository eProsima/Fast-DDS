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
	memcpy(H.guidPrefix,guidprefix,12);
	try{
		createHeader(msg,&H);
		CDRMessage_t submsg;
		createSubmessageGap(&submsg,SubM);
		appendMsg(msg, &submsg);
	}
	catch(int e){
		return false;
	}
	return true;
}

bool CDRMessageCreator::createSubmessageGap(CDRMessage_t* msg,SubmsgGap_t* SubM){
	CDRMessage_t* submsg = new CDRMessage_t();
	initCDRMsg(submsg);
	submsg->msg_endian = SubM->SubmessageHeader.flags[0] ? BIGEND : LITTLEEND;
	try{
		addEntityId(submsg,&SubM->readerId);
		addEntityId(submsg,&SubM->writerId);
		//Add Sequence Number
		addSequenceNumber(submsg,&SubM->gapStart);
		addSequenceNumberSet(submsg,&SubM->gapList);
	}
	catch(int t){
		return false;
	}
	SubM->SubmessageHeader.flags[0] = SubM->endiannessFlag;
	for (uint i =7;i>=1;i--)
		SubM->SubmessageHeader.flags[i] = false;
	//Once the submessage elements are added, the header is created
	createSubmessageHeader(msg, &SubM->SubmessageHeader,submsg->w_pos);
	//Append Submessage elements to msg
	appendMsg(msg, submsg);
	return true;
}


}
}
