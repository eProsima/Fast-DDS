/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * MessageReceiver.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/MessageReceiver.h"

namespace eprosima {
namespace rtps {

#if defined(__LITTLE_ENDIAN__)
	const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
	const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif




MessageReceiver::MessageReceiver() {
	// TODO Auto-generated constructor stub
	PROTOCOLVERSION(sourceVersion);
	VENDORID_UNKNOWN(sourceVendorId);
	GUIDPREFIX_UNKNOWN(sourceGuidPrefix);
	GUIDPREFIX_UNKNOWN(sourceGuidPrefix);
	haveTimestamp = false;
	TIME_INVALID(timestamp);
	unicastReplyLocatorList.clear();
	multicastReplyLocatorList.clear();
}

MessageReceiver::~MessageReceiver() {
	// TODO Auto-generated destructor stub
}

void MessageReceiver::reset(){
	MessageReceiver();
	Locator_t defUniL;
	defUniL.kind = LOCATOR_KIND_UDPv4;
	LOCATOR_ADDRESS_INVALID(defUniL.address);
	defUniL.port = LOCATOR_PORT_INVALID;
	unicastReplyLocatorList.push_back(defUniL);
	multicastReplyLocatorList.push_back(defUniL);
}

void MessageReceiver::processMsg(GuidPrefix_t participantguidprefix,
								octet address[16],
								void*data,short length){
	reset();
	destGuidPrefix = participantguidprefix;
	for(uint8_t i = 0;i<16;i++)
		unicastReplyLocatorList[0].address[i] = address[i];
	msg.buffer = (octet*)data;
	msg.max_size = length;
	msg.pos = 0;





}

bool MessageReceiver::extractHeader(CDRMessage_t* msg, Header_t* H) {
	if(msg->buffer[0] != 'R' ||  msg->buffer[1] != 'T' ||
       msg->buffer[2] != 'P' ||  msg->buffer[3] != 'S')
		return false;

	if(true)


	return true;
}

bool MessageReceiver::readEntityId(CDRMessage_t* msg,EntityId_t* id) {
	if(msg->pos+4>=msg->max_size)
			return false;
	uint32_t* aux1 = (uint32_t*) id->value;
	uint32_t* aux2 = (uint32_t*) &msg->buffer[msg->pos];
	*aux1 = *aux2;
	msg->pos+=4;
	return true;
}

bool MessageReceiver::readData(CDRMessage_t* msg,octet* o,uint16_t length) {
	memcpy(o,&msg->buffer[msg->pos],length);
	return true;
}

bool MessageReceiver::readDataReversed(CDRMessage_t* msg,octet* o,uint16_t length) {
	for(uint i=0;i<length;i++)
		*(o+i)=*(msg->buffer+msg->pos+length-1-i);

	return true;
}

bool MessageReceiver::readInt32(CDRMessage_t* msg,int32_t* lo) {
	if(msg->pos+4>=msg->max_size)
			return false;
	octet* dest = (octet*)lo;
	if(msg->msg_endian == DEFAULT_ENDIAN){
		readData(msg,dest,4);
	}
	else{
		readDataReversed(msg,dest,4);
	}
	msg->pos+=4;
	return true;
}

bool MessageReceiver::readUInt32(CDRMessage_t* msg,uint32_t* ulo) {
	if(msg->pos+4>=msg->max_size)
			return false;
	octet* dest = (octet*)ulo;
	if(msg->msg_endian == DEFAULT_ENDIAN){
		readData(msg,dest,4);
	}
	else{
		readDataReversed(msg,dest,4);
	}
	msg->pos+=4;
	return true;
}

bool MessageReceiver::readSequenceNumber(CDRMessage_t* msg,SequenceNumber_t* sn) {
	if(msg->pos+8>=msg->max_size)
			return false;
	readInt32(msg,&sn->high);
	readUInt32(msg,&sn->low);
	return true;
}

bool MessageReceiver::readInt16(CDRMessage_t* msg,int16_t* i16) {
	if(msg->pos+2>=msg->max_size)
			return false;
	octet* o = (octet*)i16;
	if(msg->msg_endian == DEFAULT_ENDIAN){
		 *o = msg->buffer[msg->pos];
		 *(o+1) = msg->buffer[msg->pos+1];
	}
	else{
		*o = msg->buffer[msg->pos+1];
		*(o+1) = msg->buffer[msg->pos];
	}
	msg->pos+=2;
	return true;
}

bool MessageReceiver::readParameterList(CDRMessage_t* msg,ParameterList_t* list) {

	int16_t paramId,paramlength;
	while(1){
		Parameter_t p;
		readInt16(msg,&paramId);
		if(paramId == PID_SENTINEL)
			break;
		readInt16(msg,&paramlength);
		if(p.reset(paramlength))
			memcpy(p.value,&msg->buffer[msg->pos],paramlength);
		else
			return false;
		p.length = paramlength;
		p.parameterId = paramId;
		list->push_back(p);
		msg->pos+=paramlength;
	}
	msg->pos +=2;
	return true;
}

bool MessageReceiver::readOctet(CDRMessage_t* msg, octet* o) {
	if(msg->pos+1>=msg->max_size)
		return false;
	*o = msg->buffer[msg->pos];
	msg->pos++;
	return true;
}

} /* namespace rtps */
} /* namespace eprosima */
