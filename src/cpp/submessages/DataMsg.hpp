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

using namespace eprosima::dds;

namespace eprosima{
namespace rtps{




bool RTPSMessageCreator::addMessageData(CDRMessage_t* msg,
		GuidPrefix_t& guidprefix,CacheChange_t* change,TopicKind_t topicKind,const EntityId_t& readerId,ParameterList_t* inlineQos){


	try{

		RTPSMessageCreator::addHeader(msg,guidprefix);

		RTPSMessageCreator::addSubmessageInfoTS_Now(msg,false);

		RTPSMessageCreator::addSubmessageData(msg,change,topicKind,readerId,inlineQos);

		//cout << "SubMEssage created and added to message" << endl;
		msg->length = msg->pos;
	}
	catch(int e)
	{
		pError("Data message error"<<endl)

		return false;
	}
	return true;
}



bool RTPSMessageCreator::addSubmessageData(CDRMessage_t* msg,CacheChange_t* change,
		TopicKind_t topicKind,const EntityId_t& readerId,ParameterList_t* inlineQos) {

	CDRMessage_t& submsgElem = pool_submsg.reserve_Object();
	CDRMessage::initCDRMsg(&submsgElem);
	//Create the two CDR msgs
	//CDRMessage_t submsgElem;
	octet flags = 0x0;
	if(EPROSIMA_ENDIAN == LITTLEEND)
	{
		flags = flags | BIT(0);
		 submsgElem.msg_endian = LITTLEEND;
	}
	else
	{
		 submsgElem.msg_endian = BIGEND;
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

	octet status = 0;
	if(change->kind == NOT_ALIVE_DISPOSED)
		status = status | BIT(0);
	if(change->kind == NOT_ALIVE_UNREGISTERED)
		status = status | BIT(1);

	CDRMessage::initCDRMsg(&submsgElem);
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
			if(inlineQos->has_changed_inlineQos || ParameterList::get_inlineQos_endian(inlineQos)!=submsgElem.msg_endian)
			{
			//	cout << "Updating endian message" << endl;
				ParameterList::updateInlineQosMsg(inlineQos,submsgElem.msg_endian);
			}
			CDRMessage::addParameterKey(&submsgElem,&change->instanceHandle);
			if(change->kind != ALIVE)
				CDRMessage::addParameterStatus(&submsgElem,status);
			//cout << "Adding message of length: " << inlineQos->inlineQosMsg.length <<" and endian: " << inlineQos->inlineQosMsg.msg_endian<< endl;
			CDRMessage::appendMsg(&submsgElem,&inlineQos->inlineQosMsg);
		}

		//Add Serialized Payload
		if(dataFlag)
		{
			CDRMessage::addOctet(&submsgElem,0); //ENCAPSULATION
			CDRMessage::addOctet(&submsgElem,change->serializedPayload.encapsulation); //ENCAPSULATION
			CDRMessage::addUInt16(&submsgElem,0); //OPTIONS
			CDRMessage::addData(&submsgElem,change->serializedPayload.data,change->serializedPayload.length);
		}
		if(keyFlag)
		{
			CDRMessage::addOctet(&submsgElem,0); //ENCAPSULATION
			if(submsgElem.msg_endian == BIGEND)
				CDRMessage::addOctet(&submsgElem,PL_CDR_BE); //ENCAPSULATION
			else
				CDRMessage::addOctet(&submsgElem,PL_CDR_LE); //ENCAPSULATION

			CDRMessage::addUInt16(&submsgElem,0); //ENCAPSULATION OPTIONS
			CDRMessage::addParameterKey(&submsgElem,&change->instanceHandle);
			CDRMessage::addParameterStatus(&submsgElem,status);
			CDRMessage::addParameterSentinel(&submsgElem);
		}

		//Once the submessage elements are added, the submessage header is created, assigning the correct size.
		RTPSMessageCreator::addSubmessageHeader(msg, DATA,flags,submsgElem.length);
		//Append Submessage elements to msg

		CDRMessage::appendMsg(msg, &submsgElem);
		pool_submsg.release_Object(submsgElem);
	}
	catch(int t){
		pError("Data SUBmessage not created"<<endl)

		return false;
	}
	return true;
}


}
}











