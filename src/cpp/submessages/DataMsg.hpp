/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * DataSubMessage.hpp
 *
 */

using namespace eprosima::dds;

namespace eprosima{
namespace rtps{




bool RTPSMessageCreator::addMessageData(CDRMessage_t* msg,
		GuidPrefix_t& guidprefix,CacheChange_t* change,TopicKind_t topicKind,const EntityId_t& readerId,bool expectsInlineQos,ParameterList_t* inlineQos){


	try{

		RTPSMessageCreator::addHeader(msg,guidprefix);

		RTPSMessageCreator::addSubmessageInfoTS_Now(msg,false);

		RTPSMessageCreator::addSubmessageData(msg,change,topicKind,readerId,expectsInlineQos,inlineQos);

		msg->length = msg->pos;
	}
	catch(int e)
	{
		pError("Data message error"<<e<<endl)

		return false;
	}
	return true;
}



bool RTPSMessageCreator::addSubmessageData(CDRMessage_t* msg,CacheChange_t* change,
		TopicKind_t topicKind,const EntityId_t& readerId,bool expectsInlineQos,ParameterList_t* inlineQos) {

	CDRMessage_t& submsgElem = g_pool_submsg.reserve_CDRMsg(change->serializedPayload.length);
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
	bool dataFlag = false;
	bool keyFlag = false;
	bool inlineQosFlag = false;
	if(change->kind == ALIVE && change->serializedPayload.length>0 && change->serializedPayload.data!=NULL)
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
	 inlineQosFlag = false;
	if(inlineQos != NULL || expectsInlineQos || change->kind != ALIVE) //expects inline qos
	{
		if(topicKind == WITH_KEY)
		{
			flags = flags | BIT(1);
			inlineQosFlag = true;
			keyFlag = false;
		}
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
	if(change->kind == NOT_ALIVE_DISPOSED_UNREGISTERED)
	{
		status = status | BIT(0);
		status = status | BIT(1);
	}

	CDRMessage::initCDRMsg(&submsgElem);
	bool added_no_error = true;
	try{
		//First we create the submsgElements:
		//extra flags. not in this version.
		added_no_error &= CDRMessage::addUInt16(&submsgElem,0);
		//octet to inline Qos is 12, may change in future versions
		added_no_error &= CDRMessage::addUInt16(&submsgElem,RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
		//Entity ids
		added_no_error &= CDRMessage::addEntityId(&submsgElem,&readerId);
		added_no_error &= CDRMessage::addEntityId(&submsgElem,&change->writerGUID.entityId);
		//Add Sequence Number
		added_no_error &= CDRMessage::addSequenceNumber(&submsgElem,&change->sequenceNumber);
		//Add INLINE QOS AND SERIALIZED PAYLOAD DEPENDING ON FLAGS:


		if(inlineQosFlag) //inlineQoS
		{
			if(inlineQos!=NULL)
			{
				if(inlineQos->m_hasChanged || inlineQos->m_cdrmsg.msg_endian!=submsgElem.msg_endian)
				{
					ParameterList::updateCDRMsg(inlineQos,submsgElem.msg_endian);
				}
			}

			if(topicKind == WITH_KEY)
			{
				CDRMessage::addParameterKey(&submsgElem,&change->instanceHandle);
			}
			if(change->kind != ALIVE)
				CDRMessage::addParameterStatus(&submsgElem,status);
			if(inlineQos!=NULL)
				CDRMessage::appendMsg(&submsgElem,&inlineQos->m_cdrmsg);
			else
				CDRMessage::addParameterSentinel(&submsgElem);
		}
		//Add Serialized Payload
		if(dataFlag)
		{
			added_no_error &= CDRMessage::addOctet(&submsgElem,0); //ENCAPSULATION
			added_no_error &= CDRMessage::addOctet(&submsgElem,(octet)change->serializedPayload.encapsulation); //ENCAPSULATION
			added_no_error &= CDRMessage::addUInt16(&submsgElem,0); //OPTIONS
			//cout << "Adding Data of length: "<<change->serializedPayload.length<<endl;
			//cout << "Msg size: "<<submsgElem.max_size << " length: "<< submsgElem.length<< " pos "<< submsgElem.pos<<endl;
			added_no_error &= CDRMessage::addData(&submsgElem,change->serializedPayload.data,change->serializedPayload.length);
		}
		if(keyFlag)
		{
			added_no_error &= CDRMessage::addOctet(&submsgElem,0); //ENCAPSULATION
			if(submsgElem.msg_endian == BIGEND)
				added_no_error &= CDRMessage::addOctet(&submsgElem,PL_CDR_BE); //ENCAPSULATION
			else
				added_no_error &= CDRMessage::addOctet(&submsgElem,PL_CDR_LE); //ENCAPSULATION

			added_no_error &= CDRMessage::addUInt16(&submsgElem,0); //ENCAPSULATION OPTIONS
			added_no_error &= CDRMessage::addParameterKey(&submsgElem,&change->instanceHandle);
			added_no_error &= CDRMessage::addParameterStatus(&submsgElem,status);
			added_no_error &= CDRMessage::addParameterSentinel(&submsgElem);
		}

		//Once the submessage elements are added, the submessage header is created, assigning the correct size.
		added_no_error &= RTPSMessageCreator::addSubmessageHeader(msg, DATA,flags,submsgElem.length);
		//Append Submessage elements to msg

		added_no_error &= CDRMessage::appendMsg(msg, &submsgElem);
		g_pool_submsg.release_CDRMsg(submsgElem);
	}
	catch(int t){
		pError("Data SUBmessage not created"<<t<<endl)

		return false;
	}
	return added_no_error;
}


}
}











