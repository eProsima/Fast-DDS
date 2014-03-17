/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * DataSubMessage.hpp
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

namespace eprosima{
namespace rtps{




bool CDRMessageCreator2::createMessageData(CDRMessage_t* msg,
		GuidPrefix_t guidprefix,CacheChange_t* change,TopicKind_t topicKind,EntityId_t readerId,ParameterList_t* inlineQos){

	CDRMessage::initCDRMsg(msg, RTPSMESSAGE_MAX_SIZE);
	try{
		CDRMessage_t header;
		VendorId_t vendor;
		VENDORID_EPROSIMA(vendor);
		ProtocolVersion_t version;
		PROTOCOLVERSION(version);
		CDRMessageCreator2::createHeader(&header,guidprefix,version,vendor);
		CDRMessage::appendMsg(msg, &header);

		CDRMessage_t submsgdata;
		CDRMessageCreator2::createSubmessageData(&submsgdata,change,topicKind,readerId,inlineQos);
		CDRMessage::appendMsg(msg, &submsgdata);
		//cout << "SubMEssage created and added to message" << endl;
		msg->length = msg->pos;
	}
	catch(int e){

		RTPSLog::Error << "Data message not created: " << e << DEF<< endl;
		RTPSLog::printError();
		return false;
	}
	return true;
}



bool CDRMessageCreator2::createSubmessageData(CDRMessage_t* submessage,CacheChange_t* change,
		TopicKind_t topicKind,EntityId_t readerId,ParameterList_t* inlineQos) {

	CDRMessage::initCDRMsg(submessage,RTPSMESSAGE_MAX_SIZE);
	//Create the two CDR msgs
	CDRMessage_t submsgHeader,submsgElem;
	CDRMessage::initCDRMsg(&submsgHeader,RTPSMESSAGE_SUBMESSAGEHEADER_SIZE);
	CDRMessage::initCDRMsg(&submsgElem,RTPSMESSAGE_MAX_SIZE);

	octet flags = 0x0;
	if(EPROSIMA_ENDIAN == LITTLEEND)
	{
		flags = flags | BIT(0);
		submsgHeader.msg_endian = submsgElem.msg_endian = LITTLEEND;
	}
	else
	{
		submsgHeader.msg_endian = submsgElem.msg_endian = BIGEND;
	}
	//Find out flags
	bool dataFlag,keyFlag;
	if(change->kind == ALIVE)
	{
		dataFlag = true;
		keyFlag = false;
	}
	else
	{
		dataFlag = false;
		keyFlag = true;
	}
	 if(topicKind == NO_KEY)
		 keyFlag = false;
	if(inlineQos != NULL) //expects inline qos
	{
		flags = flags | BIT(1);
		keyFlag = false;
	}
	if(dataFlag)
		flags = flags | BIT(2);
	if(keyFlag)
		flags = flags | BIT(3);

	try{
		//First we create the submsgElements:
		//extra flags. not in this version.
		CDRMessage::addUInt16(&submsgElem,0);
		//octet to inline Qos is 12, may change in future versions
		CDRMessage::addUInt16(&submsgElem,RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
		//Entity ids
		CDRMessage::addEntityId(&submsgElem,&readerId);
		CDRMessage::addEntityId(&submsgElem,&change->writerGUID.entityId);
		//Add Sequence Number
		CDRMessage::addSequenceNumber(&submsgElem,&change->sequenceNumber);
		//Add INLINE QOS AND SERIALIZED PAYLOAD DEPENDING ON FLAGS:


		if(inlineQos != NULL) //inlineQoS
		{
			if(inlineQos->has_changed)
			{
				ParameterListCreator::updateMsg(inlineQos);
			}
			CDRMessage::appendMsg(&submsgElem,&inlineQos->ParamsMsg);
		}

		//Add Serialized Payload
		if(dataFlag && keyFlag)
		{
			RTPSLog::Error << "Data and keyFlag set simultaneously" << endl;
			RTPSLog::printError();
			return false;
		}
		if(dataFlag){
			CDRMessage::addOctet(&submsgElem,0); //ENCAPSULATION
			CDRMessage::addOctet(&submsgElem,change->serializedPayload.encapsulation); //ENCAPSULATION
			CDRMessage::addUInt16(&submsgElem,0); //OPTIONS
			CDRMessage::addData(&submsgElem,change->serializedPayload.data,change->serializedPayload.length);
		}
		if(keyFlag)
		{
			octet status = 0;
			if(change->kind == NOT_ALIVE_DISPOSED)
				status = status | BIT(0);
			if(change->kind == NOT_ALIVE_UNREGISTERED)
				status = status | BIT(1);

			ParameterList_t p;
			ParameterListCreator::addParameterKey(&p,PID_KEY_HASH,change->instanceHandle);
			ParameterListCreator::addParameterStatus(&p,PID_STATUS_INFO,status);
			p.ParamsMsg.msg_endian = submsgElem.msg_endian;
			ParameterListCreator::updateMsg(&p);

			CDRMessage::addOctet(&submsgElem,0); //ENCAPSULATION
			if(submsgElem.msg_endian == BIGEND)
				CDRMessage::addOctet(&submsgElem,PL_CDR_BE); //ENCAPSULATION
			else
				CDRMessage::addOctet(&submsgElem,PL_CDR_LE); //ENCAPSULATION

			CDRMessage::addUInt16(&submsgElem,0); //ENCAPSULATION OPTIONS
			CDRMessage::appendMsg(&submsgElem,&p.ParamsMsg); //Parameters

		}

		//Once the submessage elements are added, the submessage header is created, assigning the correct size.
		CDRMessageCreator2::createSubmessageHeader(&submsgHeader, DATA,flags,submsgElem.length);
		//Append Submessage elements to msg
		CDRMessage::appendMsg(submessage, &submsgHeader);
		CDRMessage::appendMsg(submessage, &submsgElem);
		submessage->length = submessage->pos;
	}
	catch(int t){
		RTPSLog::Error << "Data SUBmessage not created: " << t << DEF<< endl;
				RTPSLog::printError();
		return false;
	}
	return true;
}


}
}











