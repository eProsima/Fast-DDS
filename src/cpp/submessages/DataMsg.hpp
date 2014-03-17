/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * DataSubMessage.hpp
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

namespace eprosima{
namespace rtps{




bool CDRMessageCreator::createMessageData(CDRMessage_t* msg,
		GuidPrefix_t guidprefix,SubmsgData_t* DataSubM,RTPSWriter* W){

	CDRMessage::initCDRMsg(msg, RTPSMESSAGE_MAX_SIZE);
	try{
		Header_t H = Header_t();
		H.guidPrefix = guidprefix;
		CDRMessage_t header;
		createHeader(&header,&H);
		CDRMessage::appendMsg(msg, &header);
		//cout << "Header created and added to message" << endl;
		CDRMessage_t submsgdata;
		createSubmessageData(&submsgdata,DataSubM,W);
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



bool CDRMessageCreator::createSubmessageData(CDRMessage_t* submessage,SubmsgData_t* DataSubM,RTPSWriter* W) {

	CDRMessage::initCDRMsg(submessage,RTPSMESSAGE_MAX_SIZE);
	//Create the two CDR msgs
	CDRMessage_t submsgHeader,submsgElem;
	CDRMessage::initCDRMsg(&submsgHeader,RTPSMESSAGE_SUBMESSAGEHEADER_SIZE);
	CDRMessage::initCDRMsg(&submsgElem,RTPSMESSAGE_MAX_SIZE);

	//COMPLETE FLAG OCTET IN SIBMESSAGEHEADER
	DataSubM->SubmessageHeader.flags = 0x0;
	if(EPROSIMA_ENDIAN == LITTLEEND)
	{
		DataSubM->SubmessageHeader.flags = DataSubM->SubmessageHeader.flags | BIT(0);
		submsgHeader.msg_endian = submsgElem.msg_endian = LITTLEEND;
	}
	else
	{
		submsgHeader.msg_endian = submsgElem.msg_endian = BIGEND;
	}
	//Find out flags
	bool dataFlag,keyFlag;
	if(DataSubM->changeKind == ALIVE)
	{
		dataFlag = true;
		keyFlag = false;
	}
	else
	{
		dataFlag = false;
		keyFlag = true;
	}
	 if(W->topicKind == NO_KEY)
		 keyFlag = false;
	if(DataSubM->expectsInlineQos)
	{
		DataSubM->SubmessageHeader.flags = DataSubM->SubmessageHeader.flags | BIT(1);
		keyFlag = false;
	}
	if(dataFlag)
		DataSubM->SubmessageHeader.flags = DataSubM->SubmessageHeader.flags | BIT(2);
	if(keyFlag)
		DataSubM->SubmessageHeader.flags = DataSubM->SubmessageHeader.flags | BIT(3);


	//cout << "CDR MEnsaje endianness: " << submsgHeader.msg_endian << " vs default endian: " << DEFAULT_ENDIAN << endl;
	try{
		//First we create the submsgElements:
		//extra flags. not in this version.
		CDRMessage::addUInt16(&submsgElem,0);
		//octet to inline Qos is 12, may change in future versions
		CDRMessage::addUInt16(&submsgElem,RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
		//Entity ids
		CDRMessage::addEntityId(&submsgElem,&DataSubM->readerId);
		CDRMessage::addEntityId(&submsgElem,&DataSubM->writerId);
		//Add Sequence Number
		CDRMessage::addSequenceNumber(&submsgElem,&DataSubM->writerSN);
		//Add INLINE QOS AND SERIALIZED PAYLOAD DEPENDING ON FLAGS:


		if(DataSubM->expectsInlineQos) //inlineQoS
		{
			if(DataSubM->inlineQos.has_changed)
			{
				ParameterListCreator::updateMsg(&DataSubM->inlineQos);
			}
			CDRMessage::appendMsg(&submsgElem,&DataSubM->inlineQos.ParamsMsg);
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
			CDRMessage::addOctet(&submsgElem,DataSubM->serializedPayload.encapsulation); //ENCAPSULATION
			CDRMessage::addUInt16(&submsgElem,0); //OPTIONS
			CDRMessage::addData(&submsgElem,DataSubM->serializedPayload.data,DataSubM->serializedPayload.length);
		}
		if(keyFlag)
		{
			octet status = 0;
			if(DataSubM->changeKind == NOT_ALIVE_DISPOSED)
				status = status | BIT(0);
			if(DataSubM->changeKind == NOT_ALIVE_UNREGISTERED)
				status = status | BIT(1);

			ParameterList_t p;
			ParameterListCreator::addParameterKey(&p,PID_KEY_HASH,DataSubM->instanceHandle);
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
		DataSubM->SubmessageHeader.submessageLength = submsgElem.pos;
		DataSubM->SubmessageHeader.submessageId = DATA;
		this->createSubmessageHeader(&submsgHeader, &DataSubM->SubmessageHeader,submsgElem.pos);
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















//
//
//
//
//bool CDRMessageCreator::createMessageData(CDRMessage_t* msg,
//		GuidPrefix_t guidprefix,SubmsgData_t* DataSubM){
//
//		initCDRMsg(msg, RTPSMESSAGE_MAX_SIZE);
//	try{
//		Header_t H = Header_t();
//		memcpy(H.guidPrefix,guidprefix,12);
//		CDRMessage_t header;
//		createHeader(&header,&H);
//		appendMsg(msg, &header);
//		header.~CDRMessage_t();
//		cout << "Header created and added to message" << endl;
//		CDRMessage_t submsgdata;
//		createSubmessageData(&submsgdata,DataSubM);
//		appendMsg(msg, &submsgdata);
//		submsgdata.~CDRMessage_t();
//		cout << "SubMEssage created and added to message" << endl;
//	}
//	catch(int e){
//		cout << B_RED<<"FALLO EN CREACION DE MESSAGEDATA" << DEF << endl;
//		return false;
//	}
//	return true;
//}
//
//
//
//bool CDRMessageCreator::createSubmessageData(CDRMessage_t* submessage,SubmsgData_t* DataSubM) {
//
//	initCDRMsg(submessage,RTPSMESSAGE_MAX_SIZE);
//
//	CDRMessage_t submsgHeader,submsgElem;
//	initCDRMsg(&submsgHeader,RTPSMESSAGE_SUBMESSAGEHEADER_SIZE);
//	initCDRMsg(&submsgElem,RTPSMESSAGE_MAX_SIZE);
//
//	submsgHeader.msg_endian = DataSubM->SubmessageHeader.flags[0] ? BIGEND : LITTLEEND;
//	submsgElem.msg_endian = DataSubM->SubmessageHeader.flags[0] ? BIGEND : LITTLEEND;
//
//	try{
//		//First we create the submsgElements:
//		//extra flags
//		addUshort(&submsgElem,0);
//
//		//octet to inline Qos is 12, may change in future versions
//		addUshort(&submsgElem,RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
//		//Entity ids
//		addEntityId(&submsgElem,&DataSubM->readerId);
//		addEntityId(&submsgElem,&DataSubM->writerId);
//		//Add Sequence Number
//		addSequenceNumber(&submsgElem,&DataSubM->writerSN);
//
//		//Add Parameter List
//		if(DataSubM->SubmessageHeader.flags[1]) //inlineQoS
//		{
//			std::vector<Parameter_t>::iterator it;
//			for(it=DataSubM->inlineQos.begin();it!=DataSubM->inlineQos.end();it++){
//				addParameter(&submsgElem,&(*it));
//			}
//			addUshort(&submsgElem,PID_SENTINEL);
//			addUshort(&submsgElem,0);
//		}
//
//		//Add Serialized Payload
//		if(DataSubM->SubmessageHeader.flags[2] || DataSubM->SubmessageHeader.flags[3]){
//			if(submsgElem.msg_endian ==DEFAULT_ENDIAN)
//				addData(&submsgElem,DataSubM->serializedPayload.data,DataSubM->serializedPayload.length);
//			else
//				addDataReversed(&submsgElem,DataSubM->serializedPayload.data,DataSubM->serializedPayload.length);
//		}
//		//COMPLETE FLAG VECTOR IN SIBMESSAGEHEADER
//		DataSubM->SubmessageHeader.flags[0] = DataSubM->endiannessFlag;
//		DataSubM->SubmessageHeader.flags[1] = DataSubM->inlineQosFlag;
//		DataSubM->SubmessageHeader.flags[2] = DataSubM->dataFlag;
//		DataSubM->SubmessageHeader.flags[3] = DataSubM->keyFlag;
//		for (uint i =7;i>=4;i--)
//			DataSubM->SubmessageHeader.flags[i] = false;
//
//		//Once the submessage elements are added, the header is created
//		DataSubM->SubmessageHeader.submessageLength = submsgElem.w_pos;
//		cout << "Tamaño submensaje: " << submsgElem.w_pos << endl;
//		createSubmessageHeader(&submsgHeader, &DataSubM->SubmessageHeader,submsgElem.w_pos);
//		//Append Submessage elements to msg
//		appendMsg(submessage, &submsgHeader);
//		appendMsg(submessage, &submsgElem);
//		cout << "Fin Creacion SubMensaje" << endl;
//	}
//	catch(int t){
//		cout << B_RED << "FALLO MORTAL SUBMSGHEADER"<< endl;
//		return false;
//	}
//	return true;
//}

}
}


