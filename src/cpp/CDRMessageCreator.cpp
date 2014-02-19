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

#include "submessages/DataSubMessage.hpp"


namespace eprosima {
namespace rtps{


#if defined(__LITTLE_ENDIAN__)
	const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
	const Endianness_t DEFAULT_ENDIAN = BIGEND;
#else
	const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#endif

CDRMessageCreator::CDRMessageCreator() {
	// TODO Auto-generated constructor stub


}

CDRMessageCreator::~CDRMessageCreator() {
	// TODO Auto-generated destructor stub
}


bool CDRMessageCreator::createHeader(CDRMessage_t*msg, Header_t* header) {
	initCDRMsg(msg,RTPSMESSAGE_HEADER_SIZE);
	try{
		addOctet(msg,'R');
		addOctet(msg,'T');
		addOctet(msg,'P');
		addOctet(msg,'S');

		addOctet(msg,header->version.major);
		addOctet(msg,header->version.minor);

		addOctet(msg,header->vendorId[0]);
		addOctet(msg,header->vendorId[1]);

		for (uint i = 0;i<12;i++){
			addOctet(msg,header->guidPrefix[i]);
		}
	}
	catch(int e){
		return false;
	}

	return true;
}

bool CDRMessageCreator::createSubmessageHeader(CDRMessage_t* msg,
		SubmessageHeader_t* SubMH, unsigned short submsgsize) {
	initCDRMsg(msg,RTPSMESSAGE_SUBMESSAGEHEADER_SIZE);
	try{
		addOctet(msg,SubMH->submessageId);
		octet flags = 0x0;
		for(uint i= 7;i>=0;i--){
			if(SubMH->flags[i])
				flags = flags | BIT(i);
		}
		addOctet(msg,flags);
		addUshort(msg, submsgsize);
	}
	catch(int e){
		return false;
	}

	return true;
}

bool CDRMessageCreator::initCDRMsg(CDRMessage_t* msg, uint size) {
	free(msg->buffer);
	msg->buffer = (octet*)malloc(size);
	msg->max_size = size;
	msg->w_pos = 0;
	return true;
}

bool CDRMessageCreator::initCDRMsg(CDRMessage_t*msg) {
	free(msg->buffer);
	msg->buffer = (octet*)malloc(RTPSMESSAGE_MAX_SIZE);
	msg->max_size = RTPSMESSAGE_MAX_SIZE;
	msg->w_pos = 0;
	return true;
}

bool CDRMessageCreator::appendMsg(CDRMessage_t*first, CDRMessage_t*second) {
	addData(first,second->buffer,second->w_pos);
	return true;
}

bool CDRMessageCreator::addData(CDRMessage_t*msg, const void* data, uint length) {
	if(msg->w_pos + length > msg->max_size)
		throw ERR_CDRMESSAGE_BUFFER_FULL;

	memcpy(&msg->buffer[msg->w_pos],data,length);
	msg->w_pos +=length;
	return true;
}

bool CDRMessageCreator::addDataReversed(CDRMessage_t*msg, const void* data, uint length) {
	if(msg->w_pos + length > msg->max_size)
		throw ERR_CDRMESSAGE_BUFFER_FULL;
	for(uint i = 0;i<length;i++)
		msg->buffer[msg->w_pos+i] = *((octet*)data+length-1-i);
	msg->w_pos +=length;
	return true;
}

bool CDRMessageCreator::addOctet(CDRMessage_t*msg, octet O) {
	const void* d = (void*)&O;
	addData(msg,d,1);
	return true;
}

bool CDRMessageCreator::addUshort(CDRMessage_t*msg,
		unsigned short U) {
	if(msg->msg_endian == DEFAULT_ENDIAN){
		const void* d = (void*)&U;
		addData(msg,d,2);
	}
	else{
		octet* o= (octet*)&U;
		addData(msg,&o[1],1);
		addData(msg,o,1);
	}
	return true;
}

bool CDRMessageCreator::addEntityId(CDRMessage_t* msg, EntityId_t*ID) {
	if(msg->msg_endian == BIGEND){ //Big ENDIAN
		addOctet(msg,ID->entityKey[0]);
		addOctet(msg,ID->entityKey[1]);
		addOctet(msg,ID->entityKey[2]);
		addOctet(msg,ID->entityKind);
	}
	else{
		addOctet(msg,ID->entityKind);
		addOctet(msg,ID->entityKey[2]);
		addOctet(msg,ID->entityKey[1]);
		addOctet(msg,ID->entityKey[0]);
	}
	return true;
}

bool CDRMessageCreator::addParameter(CDRMessage_t*msg, Parameter_t*param) {
	addUshort(msg,param->parameterId);
	addUshort(msg,param->length);
	if(msg->msg_endian ==DEFAULT_ENDIAN)
		addData(msg,param->value,param->length);
	else
		addDataReversed(msg,param->value,(uint)param->length);

	return true;
}

} /* namespace rtps */
} /* namespace eprosima */
