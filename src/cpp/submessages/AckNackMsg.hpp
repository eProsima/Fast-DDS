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
		EntityId_t readerId,EntityId_t writerId,SequenceNumberSet_t SNSet,int32_t count,bool finalFlag){

	try
	{
		VendorId_t vendor;
		VENDORID_EPROSIMA(vendor);
		ProtocolVersion_t version;
		PROTOCOLVERSION(version);
		CDRMessageCreator::createHeader(msg,guidprefix,version,vendor);


		CDRMessageCreator::createSubmessageAcknack(msg,readerId, writerId,SNSet,count,finalFlag);

		//cout << "SubMEssage created and added to message" << endl;
		msg->length = msg->pos;
	}
	catch(int e)
	{
		pError("Data message not created"<<endl);
		return false;
	}
	return true;
}

bool CDRMessageCreator::createSubmessageAcknack(CDRMessage_t* msg,
		EntityId_t readerId,EntityId_t writerId,SequenceNumberSet_t SNSet,int32_t count,bool finalFlag)
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
		submsgElem.msg_endian =  LITTLEEND;
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
		pError("Message creator fails"<<endl)
		return false;
	}

	//Once the submessage elements are added, the header is created
	CDRMessageCreator::createSubmessageHeader(msg,ACKNACK,flags,submsgElem.length);
	//Append Submessage elements to msg
	CDRMessage::appendMsg(msg, &submsgElem);
	msg->length = msg->pos;
	return true;
}


}
}




