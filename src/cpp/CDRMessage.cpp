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

#include <algorithm>

#include "eprosimartps/CDRMessage.h"
#include "eprosimartps/dds/ParameterTypes.h"

using namespace eprosima::dds;
using eprosima::dds::ParameterId_t;

namespace eprosima {
namespace rtps {


CDRMessage::CDRMessage() {
	// TODO Auto-generated constructor stub

}

CDRMessage::~CDRMessage() {
	// TODO Auto-generated destructor stub
}


//bool CDRMessage::initCDRMsg(CDRMessage_t* msg, uint size) {
//	if(msg->buffer!=NULL)
//		free(msg->buffer);
//	msg->buffer = (octet*)malloc(size);
//	msg->max_size = size;
//	msg->pos = 0;
//	msg->length = 0;
//	return true;
//}

bool CDRMessage::initCDRMsg(CDRMessage_t*msg)
{
	if(msg->buffer==NULL)
	{
		msg->buffer = (octet*)malloc(RTPSMESSAGE_MAX_SIZE);
		msg->max_size = RTPSMESSAGE_MAX_SIZE;
	}
	msg->pos = 0;
	msg->length = 0;
	msg->msg_endian = EPROSIMA_ENDIAN;
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
		for(uint8_t i =0;i<4;i++)
			dest[i] = msg->buffer[msg->pos+i];
		msg->pos+=4;
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
		for(uint8_t i =0;i<4;i++)
			dest[i] = msg->buffer[msg->pos+i];
		msg->pos+=4;
	}
	else{
		readDataReversed(msg,dest,4);
	}
	return true;
}

bool CDRMessage::readSequenceNumber(CDRMessage_t* msg,SequenceNumber_t* sn) {
	if(msg->pos+8>msg->length)
		return false;
	bool valid=readInt32(msg,&sn->high);
	valid&=readUInt32(msg,&sn->low);
	return true;
}

bool CDRMessage::readSequenceNumberSet(CDRMessage_t* msg,SequenceNumberSet_t* sns)
{
	bool valid = true;
	valid &=CDRMessage::readSequenceNumber(msg,&sns->base);
	uint32_t numBits;
	valid &=CDRMessage::readUInt32(msg,&numBits);
	uint32_t n_octets = 4*((numBits+31)/32);
	SequenceNumber_t auxSN;
	for(uint8_t i=0;i<n_octets;i++)
	{
		octet o;
		valid &=CDRMessage::readOctet(msg,&o);
		if(8*i<numBits)
		{
			for(uint8_t bit=0;bit<8;++bit)
			{
				if((bit+i*8)>numBits) //no more bits to analyze
					break;
				bool addSN = o & BIT(bit) ? true : false;
				if(addSN)
				{
					auxSN = sns->base;
					auxSN += (i*8+bit);
					sns->set.push_back(auxSN);
				}
			}
		}
	}
	return valid;
}

bool CDRMessage::readTimestamp(CDRMessage_t* msg, Time_t* ts)
{
	bool valid = true;
	valid &=CDRMessage::readInt32(msg,&ts->seconds);
	valid &=CDRMessage::readUInt32(msg,&ts->fraction);
	return valid;
}


bool CDRMessage::readLocator(CDRMessage_t* msg,Locator_t* loc) {
	if(msg->pos+24>msg->length)
		return false;
	bool valid = readInt32(msg,&loc->kind);
	valid&=readUInt32(msg,&loc->port);

	valid&=readData(msg,loc->address,16);

	return valid;
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
		pError( "Message size not enough "<<endl);

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
		pError( "Message size not enough "<<endl);
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

bool CDRMessage::addOctet(CDRMessage_t*msg, octet O)
{
	if(msg->pos + 1 > msg->max_size)
	{
		pError( "Message size not enough "<<endl);
		return false;
	}
	//const void* d = (void*)&O;
	msg->buffer[msg->pos] = O;
	msg->pos++;
	msg->length++;
	return true;
}

bool CDRMessage::addUInt16(CDRMessage_t*msg,unsigned short U)
{
	if(msg->pos + 2 > msg->max_size)
	{
		pError( "Message size not enough "<<endl);
		return false;
	}
	octet* o= (octet*)&U;
	if(msg->msg_endian == DEFAULT_ENDIAN)
	{
		msg->buffer[msg->pos] = *(o);
		msg->buffer[msg->pos+1] = *(o+1);
	}
	else
	{
		msg->buffer[msg->pos] = *(o+1);
		msg->buffer[msg->pos+1] = *(o);
	}
	msg->pos+=2;
	msg->length+=2;
	return true;
}


bool CDRMessage::addParameterId(CDRMessage_t*msg,ParameterId_t p)
{
	return CDRMessage::addUInt16(msg,(uint16_t)p);
}



bool CDRMessage::addInt32(CDRMessage_t* msg, int32_t lo) {
	octet* o= (octet*)&lo;
	if(msg->pos + 4 > msg->max_size)
	{
		pError( "Message size not enough "<<endl);
		return false;
	}
	if(msg->msg_endian == DEFAULT_ENDIAN)
	{
		for(uint8_t i=0;i<4;i++)
		{
			msg->buffer[msg->pos+i] = *(o+i);
		}
	}
	else
	{
		for(uint8_t i=0;i<4;i++)
		{
			msg->buffer[msg->pos+i] = *(o+3-i);
		}
	}
	msg->pos+=4;
	msg->length+=4;
	return true;
}



bool CDRMessage::addUInt32(CDRMessage_t* msg, uint32_t ulo) {
	octet* o= (octet*)&ulo;
	if(msg->pos + 4 > msg->max_size)
	{
		pError( "Message size not enough "<<endl);
		return false;
	}
	if(msg->msg_endian == DEFAULT_ENDIAN)
	{
		for(uint8_t i=0;i<4;i++)
		{
			msg->buffer[msg->pos+i] = *(o+i);
		}
	}
	else
	{
		for(uint8_t i=0;i<4;i++)
		{
			msg->buffer[msg->pos+i] = *(o+3-i);
		}
	}
	msg->pos+=4;
	msg->length+=4;
	return true;
}

bool CDRMessage::addInt64(CDRMessage_t* msg, int64_t lolo) {
	octet* o= (octet*)&lolo;
	if(msg->pos + 8 > msg->max_size)
	{
		pError( "Message size not enough "<<endl);
		return false;
	}
	if(msg->msg_endian == DEFAULT_ENDIAN)
	{
		for(uint8_t i=0;i<8;i++)
		{
			msg->buffer[msg->pos+i] = *(o+i);
		}
	}
	else
	{
		for(uint8_t i=0;i<8;i++)
		{
			msg->buffer[msg->pos+i] = *(o+7-i);
		}
	}
	msg->pos+=8;
	msg->length+=8;
	return true;
}


bool CDRMessage::addEntityId(CDRMessage_t* msg, EntityId_t*ID) {
	if(msg->pos+4>=msg->max_size)
	{
		pError( "Message size not enough "<<endl);
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

bool sort_seqNum (SequenceNumber_t s1,SequenceNumber_t s2)
{
	return(s1.to64long() < s2.to64long());
}

bool CDRMessage::addSequenceNumberSet(CDRMessage_t* msg,
		SequenceNumberSet_t* sns) {
	addSequenceNumber(msg, &sns->base);
	//Add set
	std::sort(sns->set.begin(),sns->set.end(),sort_seqNum);
	SequenceNumber_t maxseqNum = *std::max_element(sns->set.begin(),sns->set.end(),sort_seqNum);
	uint32_t numBits = (uint32_t)(maxseqNum.to64long() - sns->base.to64long());
	addUInt32(msg,numBits);
	std::vector<SequenceNumber_t>::iterator it;
	uint32_t n_octet = 0;
	uint8_t bit_n = 0;
	octet o = 0;
	//Compute the bitmap in terms of octets:
	for(it=sns->set.begin();it!=sns->set.end();++it)
	{
		bit_n = it->to64long()-sns->base.to64long()-n_octet*8;
		switch(bit_n)
		{
			case 0: o= o| BIT(7); break;
			case 1: o= o| BIT(6); break;
			case 2: o= o| BIT(5); break;
			case 3: o= o| BIT(4); break;
			case 4: o= o| BIT(3); break;
			case 5: o= o| BIT(2); break;
			case 6: o= o| BIT(1); break;
			case 7: o= o| BIT(0); break;
		}
		if(bit_n>7)
		{
			addOctet(msg,o);
			o = 0x0;
			for(int i=0;i<(floor(bit_n/8)-1);i++)
				addOctet(msg,o);
			n_octet += floor(bit_n/8);
			it--;
		}
	}
	//add enough octets as gap
	o=0;
	while((n_octet+1)%4 != 0)
	{
		addOctet(msg,o);
		n_octet++;
	}
	return true;
}

bool CDRMessage::addLocator(CDRMessage_t* msg, Locator_t* loc) {
	addInt32(msg,loc->kind);
	addUInt32(msg,loc->port);

	addData(msg,loc->address,16);

	return true;
}

bool CDRMessage::addParameterStatus(CDRMessage_t* msg, octet status)
{
	if(msg->pos+8>=msg->max_size)
	{
		pError( "Message size not enough "<<endl);
		return false;
	}
	CDRMessage::addUInt16(msg,PID_STATUS_INFO);
	CDRMessage::addUInt16(msg,4);
	CDRMessage::addOctet(msg,0);
	CDRMessage::addOctet(msg,0);
	CDRMessage::addOctet(msg,0);
	CDRMessage::addOctet(msg,status);
	return true;
}


bool CDRMessage::addParameterKey(CDRMessage_t* msg, InstanceHandle_t* iHandle)
{
	if(msg->pos+20>=msg->max_size)
	{
		pError( "Message size not enough "<<endl);
		return false;
	}
	CDRMessage::addUInt16(msg,PID_KEY_HASH);
	CDRMessage::addUInt16(msg,16);
	for(uint8_t i=0;i<16;i++)
		msg->buffer[msg->pos+i] = iHandle->value[i];
	msg->pos+=16;
	msg->length+=16;
	return true;
}

bool CDRMessage::addParameterSentinel(CDRMessage_t* msg)
{
	if(msg->pos+4>=msg->max_size)
	{
		pError( "Message size not enough "<<endl);
		return false;
	}
	CDRMessage::addUInt16(msg,PID_SENTINEL);
	CDRMessage::addUInt16(msg,0);

	return true;
}











} /* namespace rtps */
} /* namespace eprosima */


