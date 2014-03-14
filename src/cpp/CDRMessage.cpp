/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * CDRMessage.cpp
 *
 *  Created on: Feb 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */


#include "eprosimartps/CDRMessage.h"



namespace eprosima {
namespace rtps {

#if defined(__LITTLE_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = LITTLEEND;
#elif defined (__BIG_ENDIAN__)
const Endianness_t DEFAULT_ENDIAN = BIGEND;
#endif


CDRMessage::CDRMessage() {
	// TODO Auto-generated constructor stub

}

CDRMessage::~CDRMessage() {
	// TODO Auto-generated destructor stub
}


bool CDRMessage::initCDRMsg(CDRMessage_t* msg, uint size) {
	if(msg->buffer!=NULL)
		free(msg->buffer);
	msg->buffer = (octet*)malloc(size);
	msg->max_size = size;
	msg->pos = 0;
	msg->length = 0;
	return true;
}

bool CDRMessage::initCDRMsg(CDRMessage_t*msg) {
	if(msg->buffer!=NULL)
		free(msg->buffer);
	msg->buffer = (octet*)malloc(RTPSMESSAGE_MAX_SIZE);
	msg->max_size = RTPSMESSAGE_MAX_SIZE;
	msg->pos = 0;
	msg->length = 0;
	return true;
}

bool CDRMessage::appendMsg(CDRMessage_t*first, CDRMessage_t*second) {
	return(CDRMessage::addData(first,second->buffer,second->length));
}


bool CDRMessage::readEntityId(CDRMessage_t* msg,EntityId_t* id) {
	if(msg->pos+4>msg->length)
		return false;
	uint32_t* aux1 = (uint32_t*) id->value;
	uint32_t* aux2 = (uint32_t*) &msg->buffer[msg->pos];
	*aux1 = *aux2;
	msg->pos+=4;
	return true;
}

bool CDRMessage::readData(CDRMessage_t* msg,octet* o,uint16_t length) {
	memcpy(o,&msg->buffer[msg->pos],length);
	msg->pos+=length;
	return true;
}

bool CDRMessage::readDataReversed(CDRMessage_t* msg,octet* o,uint16_t length) {
	for(uint i=0;i<length;i++)
	{
		*(o+i)=*(msg->buffer+msg->pos+length-1-i);
	}
	msg->pos+=length;
	return true;
}

bool CDRMessage::readInt32(CDRMessage_t* msg,int32_t* lo) {
	if(msg->pos+4>msg->length)
		return false;
	octet* dest = (octet*)lo;
	if(msg->msg_endian == DEFAULT_ENDIAN){
		readData(msg,dest,4);
	}
	else{
		readDataReversed(msg,dest,4);
	}
	return true;
}

bool CDRMessage::readUInt32(CDRMessage_t* msg,uint32_t* ulo) {
	if(msg->pos+4>msg->length)
		return false;
	octet* dest = (octet*)ulo;
	if(msg->msg_endian == DEFAULT_ENDIAN){
		readData(msg,dest,4);
	}
	else{
		readDataReversed(msg,dest,4);
	}
	return true;
}

bool CDRMessage::readSequenceNumber(CDRMessage_t* msg,SequenceNumber_t* sn) {
	if(msg->pos+8>msg->length)
		return false;
	readInt32(msg,&sn->high);
	readUInt32(msg,&sn->low);
	return true;
}


bool CDRMessage::readLocator(CDRMessage_t* msg,Locator_t* loc) {
	if(msg->pos+24>msg->length)
		return false;
	readInt32(msg,&loc->kind);
	readUInt32(msg,&loc->port);

	readData(msg,loc->address,16);

	return true;
}

bool CDRMessage::readInt16(CDRMessage_t* msg,int16_t* i16) {
	if(msg->pos+2>msg->length)
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

bool CDRMessage::readUInt16(CDRMessage_t* msg,uint16_t* i16) {
	if(msg->pos+2>msg->length)
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



bool CDRMessage::readOctet(CDRMessage_t* msg, octet* o) {
	if(msg->pos+1>msg->length)
		return false;
	*o = msg->buffer[msg->pos];
	msg->pos++;
	return true;
}


bool CDRMessage::addData(CDRMessage_t*msg, octet* data, uint length) {
	if(msg->pos + length > msg->max_size)
	{
		RTPSLog::Error << "Message size not enough " << DEF<< endl;
		RTPSLog::printError();
		return false;
	}

	memcpy(&msg->buffer[msg->pos],data,length);
	msg->pos +=length;
	msg->length+=length;
	return true;
}

bool CDRMessage::addDataReversed(CDRMessage_t*msg, octet* data, uint length) {
	if(msg->pos + length > msg->max_size)
	{
		RTPSLog::Error << "Message size not enough " << DEF<< endl;
		RTPSLog::printError();
		return false;
	}
	for(uint i = 0;i<length;i++)
	{
		msg->buffer[msg->pos+i] = *(data+length-1-i);
	}
	msg->pos +=length;
	msg->length+=length;
	return true;
}

bool CDRMessage::addOctet(CDRMessage_t*msg, octet O) {
	//const void* d = (void*)&O;
	addData(msg,&O,1);
	return true;
}

bool CDRMessage::addUInt16(CDRMessage_t*msg,
		unsigned short U) {
	if(msg->msg_endian == DEFAULT_ENDIAN){
		addData(msg,(octet*)&U,2);
	}
	else{
		octet* o= (octet*)&U;
		addData(msg,&o[1],1);
		addData(msg,o,1);
	}
	return true;
}

bool CDRMessage::addInt32(CDRMessage_t* msg, int32_t lo) {
	octet* o= (octet*)&lo;
	if(msg->msg_endian == DEFAULT_ENDIAN){
		addData(msg,o,4);
	}
	else
	{
		addDataReversed(msg,o,4);

	}
	return true;
}



bool CDRMessage::addUInt32(CDRMessage_t* msg, uint32_t ulo) {
	octet* o= (octet*)&ulo;

	if(msg->msg_endian == DEFAULT_ENDIAN)
	{
		addData(msg,o,4);
	}
	else{
		addDataReversed(msg,o,4);
	}
	return true;
}

bool CDRMessage::addInt64(CDRMessage_t* msg, int64_t lolo) {
	octet* o= (octet*)&lolo;
	if(msg->msg_endian == DEFAULT_ENDIAN)
		addData(msg,o,8);
	else
		addDataReversed(msg,o,8);
	return true;
}


bool CDRMessage::addEntityId(CDRMessage_t* msg, EntityId_t*ID) {
	if(msg->pos+4>=msg->max_size)
	{
		RTPSLog::Error << "Message size not enough " << DEF<< endl;
		RTPSLog::printError();
		return false;
	}
	int* aux1;
	int* aux2;
	aux1 = (int*)(&msg->buffer[msg->pos]);
	aux2 = (int*) ID->value;
	*aux1 = *aux2;
	msg->pos +=4;
	msg->length+=4;
	return true;
}





bool CDRMessage::addSequenceNumber(CDRMessage_t* msg,
		SequenceNumber_t* sn) {
	addInt32(msg,sn->high);
	addUInt32(msg,sn->low);

	return true;
}



bool CDRMessage::addSequenceNumberSet(CDRMessage_t* msg,
		SequenceNumberSet_t* sns) {
	int64_t bitmapbase = sns->base.to64long();
	addInt64(msg, bitmapbase);
	//TODOG: Finish addSequenceNumberSet to CDR Message

	return true;
}


bool CDRMessage::addLocator(CDRMessage_t* msg, Locator_t* loc) {
	addInt32(msg,loc->kind);
	addUInt32(msg,loc->port);

	addData(msg,loc->address,16);

	return true;
}



} /* namespace rtps */
} /* namespace eprosima */



