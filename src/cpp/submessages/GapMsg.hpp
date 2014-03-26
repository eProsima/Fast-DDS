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

bool RTPSMessageCreator::createMessageGap(CDRMessage_t* msg,GuidPrefix_t& guidprefix,
		SequenceNumber_t& seqNumFirst,SequenceNumberSet_t& seqNumList,
		const EntityId_t& readerId,const EntityId_t& writerId)
{

	try
	{

		RTPSMessageCreator::createHeader(msg,guidprefix);
		RTPSMessageCreator::createSubmessageInfoTS_Now(msg,false);
		RTPSMessageCreator::createSubmessageGap(msg,seqNumFirst,seqNumList,readerId, writerId);
		//cout << "SubMEssage created and added to message" << endl;

	}
	catch(int e)
	{
		pError("Gap message error"<<endl)
		return false;
	}
	return true;
}

bool RTPSMessageCreator::createSubmessageGap(CDRMessage_t* msg,SequenceNumber_t& seqNumFirst,SequenceNumberSet_t& seqNumList,const EntityId_t& readerId,const EntityId_t& writerId)
{

	//Create the two CDR msgs
	//CDRMessage_t submsgElem;
	CDRMessage::initCDRMsg(&submsgElem);
	octet flags = 0x0;
	if(EPROSIMA_ENDIAN == BIGEND)
	{
		flags = flags | BIT(0);
		submsgElem.msg_endian   = BIGEND;
	}
	else
	{
		submsgElem.msg_endian  = LITTLEEND;
	}

	try{
		CDRMessage::addEntityId(&submsgElem,&readerId);
		CDRMessage::addEntityId(&submsgElem,&writerId);
		//Add Sequence Number
		CDRMessage::addSequenceNumber(&submsgElem,&seqNumFirst);
		CDRMessage::addSequenceNumberSet(&submsgElem,&seqNumList);
	}
	catch(int e)
	{
		pError("Gap submessage error"<<endl)
		return false;
	}


	//Once the submessage elements are added, the header is created
	RTPSMessageCreator::createSubmessageHeader(msg, GAP,flags,submsgElem.length);
	//Append Submessage elements to msg
	CDRMessage::appendMsg(msg, &submsgElem);

	return true;
}


}
}
