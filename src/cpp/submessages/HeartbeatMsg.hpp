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

bool RTPSMessageCreator::createMessageHeartbeat(CDRMessage_t* msg,GuidPrefix_t guidprefix,EntityId_t readerId,EntityId_t writerId,
		SequenceNumber_t firstSN,SequenceNumber_t lastSN,int32_t count,bool isFinal,bool livelinessFlag)
{

	try
	{

		VendorId_t vendor;
		VENDORID_EPROSIMA(vendor);
		ProtocolVersion_t version;
		PROTOCOLVERSION(version);
		RTPSMessageCreator::createHeader(msg,guidprefix,version,vendor);



		RTPSMessageCreator::createSubmessageHeartbeat(msg,readerId, writerId,firstSN,lastSN,count,isFinal,livelinessFlag);

		//cout << "SubMEssage created and added to message" << endl;
		msg->length = msg->pos;
	}
	catch(int e)
	{
		pError("HB message not created"<<endl)
		return false;
	}
	return true;
}

bool RTPSMessageCreator::createSubmessageHeartbeat(CDRMessage_t* msg,EntityId_t readerId,
		EntityId_t writerId,SequenceNumber_t firstSN,SequenceNumber_t lastSN,int32_t count,bool isFinal,bool livelinessFlag)
{

	//Create the two CDR msgs
	CDRMessage_t submsgElem;


	octet flags = 0x0;
	if(EPROSIMA_ENDIAN == BIGEND)
	{
		flags = flags | BIT(0);
		submsgElem.msg_endian  = BIGEND;
	}
	else
	{
		submsgElem.msg_endian  = LITTLEEND;
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
		CDRMessage::addInt32(&submsgElem,count);
	}
	catch(int e)
	{
		pError("MessageCreator fails"<<endl)
		return false;
	}



	//Once the submessage elements are added, the header is created
	RTPSMessageCreator::createSubmessageHeader(msg, HEARTBEAT,flags,submsgElem.length);
	//Append Submessage elements to msg
	//Append Submessage elements to msg
	CDRMessage::appendMsg(msg, &submsgElem);

	return true;
}


}
}

