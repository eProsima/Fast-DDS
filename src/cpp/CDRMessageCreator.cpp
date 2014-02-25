/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * CDRMessageCreator.cpp
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/CDRMessageCreator.h"
#include "eprosimartps/CDRMessage.h"

namespace eprosima {
namespace rtps{


#if defined(__LITTLE_ENDIAN__)
	const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
	const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif
};
};


namespace eprosima {
namespace rtps{


CDRMessageCreator::CDRMessageCreator() {
	// TODO Auto-generated constructor stub


}

CDRMessageCreator::~CDRMessageCreator() {
	// TODO Auto-generated destructor stub
}


bool CDRMessageCreator::createHeader(CDRMessage_t*msg, Header_t* header) {
	CDRMessage::initCDRMsg(msg,RTPSMESSAGE_HEADER_SIZE);
	try{
		CDRMessage::addOctet(msg,'R');
		CDRMessage::addOctet(msg,'T');
		CDRMessage::addOctet(msg,'P');
		CDRMessage::addOctet(msg,'S');

		CDRMessage::addOctet(msg,header->version.major);
		CDRMessage::addOctet(msg,header->version.minor);

		CDRMessage::addOctet(msg,header->vendorId[0]);
		CDRMessage::addOctet(msg,header->vendorId[1]);

		for (uint i = 0;i<12;i++){
			CDRMessage::addOctet(msg,header->guidPrefix.value[i]);
		}
		msg->length = msg->pos;
	}
	catch(int e){
		cout << B_RED << "FALLO MORTAL HEADER"<< endl;
						return false;
	}

	return true;
}

bool CDRMessageCreator::createSubmessageHeader(CDRMessage_t* msg,
		SubmessageHeader_t* SubMH, uint16_t submsgsize) {
	CDRMessage::initCDRMsg(msg,RTPSMESSAGE_SUBMESSAGEHEADER_SIZE);
	try{
		CDRMessage::addOctet(msg,SubMH->submessageId);
		CDRMessage::addOctet(msg,SubMH->flags);
		CDRMessage::addUInt16(msg, submsgsize);
		msg->length = msg->pos;
	}
	catch(int e){
		cout << B_RED << "FALLO MORTAL SUBMSGHEADER" << DEF << endl;
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
//		cout << "Header created and added to message" << endl;
//		CDRMessage_t submsgdata;
//		createSubmessageData(&submsgdata,DataSubM);
//		appendMsg(msg, &submsgdata);
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
//
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










}; /* namespace rtps */
}; /* namespace eprosima */


#include "submessages/DataMsg.hpp"
#include "submessages/HeartbeatMsg.hpp"
#include "submessages/AckNackMsg.hpp"
#include "submessages/GapMsg.hpp"
