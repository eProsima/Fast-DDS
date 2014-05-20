/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * @file CDRMessage.hpp
 *
 *  Created on: Feb 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */



namespace eprosima {
namespace rtps {

#include <algorithm>



inline bool CDRMessage::initCDRMsg(CDRMessage_t*msg)
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

inline bool CDRMessage::appendMsg(CDRMessage_t*first, CDRMessage_t*second) {
	return(CDRMessage::addData(first,second->buffer,second->length));
}


inline bool CDRMessage::readEntityId(CDRMessage_t* msg,const EntityId_t* id) {
	if(msg->pos+4>msg->length)
		return false;
	uint32_t* aux1 = (uint32_t*) id->value;
	uint32_t* aux2 = (uint32_t*) &msg->buffer[msg->pos];
	*aux1 = *aux2;
	msg->pos+=4;
	return true;
}

inline bool CDRMessage::readData(CDRMessage_t* msg,octet* o,uint16_t length) {
	memcpy(o,&msg->buffer[msg->pos],length);
	msg->pos+=length;
	return true;
}

inline bool CDRMessage::readDataReversed(CDRMessage_t* msg,octet* o,uint16_t length) {
	for(uint i=0;i<length;i++)
	{
		*(o+i)=*(msg->buffer+msg->pos+length-1-i);
	}
	msg->pos+=length;
	return true;
}

inline bool CDRMessage::readInt32(CDRMessage_t* msg,int32_t* lo) {
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

inline bool CDRMessage::readUInt32(CDRMessage_t* msg,uint32_t* ulo) {
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

inline bool CDRMessage::readSequenceNumber(CDRMessage_t* msg,SequenceNumber_t* sn) {
	if(msg->pos+8>msg->length)
		return false;
	bool valid=readInt32(msg,&sn->high);
	valid&=readUInt32(msg,&sn->low);
	return true;
}

inline bool CDRMessage::readSequenceNumberSet(CDRMessage_t* msg,SequenceNumberSet_t* sns)
{
	bool valid = true;
	valid &=CDRMessage::readSequenceNumber(msg,&sns->base);
	uint32_t numBits;
	valid &=CDRMessage::readUInt32(msg,&numBits);
	int32_t bitmap;
	SequenceNumber_t seqNum;
	for(uint32_t i=0;i<(numBits+31)/32;++i)
	{
		valid &= CDRMessage::readInt32(msg,&bitmap);
		for(uint8_t bit=0;bit<32;++bit)
		{
			if((bitmap & (1<<(31-bit%32)))==(1<<(31-bit%32)))
			{
				seqNum = sns->base+i*32+bit;
				if(!sns->add(seqNum))
				{
					pWarning("CDRMessage:readSequenceNumberSet:malformed seqNumSet"<<endl;);
					return false;
				}
			}
		}
	}
	return valid;
}

inline bool CDRMessage::readTimestamp(CDRMessage_t* msg, Time_t* ts)
{
	bool valid = true;
	valid &=CDRMessage::readInt32(msg,&ts->seconds);
	valid &=CDRMessage::readUInt32(msg,&ts->nanoseconds);
	return valid;
}


inline bool CDRMessage::readLocator(CDRMessage_t* msg,Locator_t* loc)
{
	if(msg->pos+24>msg->length)
		return false;
	bool valid = readInt32(msg,&loc->kind);
	valid&=readUInt32(msg,&loc->port);

	valid&=readData(msg,loc->address,16);

	return valid;
}

inline bool CDRMessage::readInt16(CDRMessage_t* msg,int16_t* i16)
{
	if(msg->pos+2>msg->length)
		return false;
	octet* o = (octet*)i16;
	if(msg->msg_endian == DEFAULT_ENDIAN)
	{
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

inline bool CDRMessage::readUInt16(CDRMessage_t* msg,uint16_t* i16)
{
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



inline bool CDRMessage::readOctet(CDRMessage_t* msg, octet* o) {
	if(msg->pos+1>msg->length)
		return false;
	*o = msg->buffer[msg->pos];
	msg->pos++;
	return true;
}

inline bool CDRMessage::readOctetVector(CDRMessage_t*msg,std::vector<octet>* ocvec)
{
	if(msg->pos+4>msg->length)
		return false;
	uint32_t vecsize;
	bool valid = CDRMessage::readUInt32(msg,&vecsize);
	ocvec->resize(vecsize);
	valid &= CDRMessage::readData(msg,ocvec->data(),vecsize);
	return valid;
}


inline bool CDRMessage::readString(CDRMessage_t*msg, std::string* stri)
{
	uint32_t str_size = 1;
	bool valid = true;
	valid&=CDRMessage::readUInt32(msg,&str_size);

	*stri = std::string();stri->resize(str_size);
	octet* oc1 = new octet[str_size];
	valid &= CDRMessage::readData(msg,oc1,str_size);
	for(uint32_t i =0;i<str_size;i++)
		stri->at(i) = oc1[i];
	uint32_t rest = (uint32_t)(str_size-4*floor((float)str_size/4));
	rest = rest==0 ? 0 : 4-rest;
	msg->pos+=rest;
	return valid;
}


inline bool CDRMessage::addData(CDRMessage_t*msg, octet* data, uint length) {
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

inline bool CDRMessage::addDataReversed(CDRMessage_t*msg, octet* data, uint length) {
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

inline bool CDRMessage::addOctet(CDRMessage_t*msg, octet O)
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

inline bool CDRMessage::addUInt16(CDRMessage_t*msg,uint16_t us)
{
	if(msg->pos + 2 > msg->max_size)
	{
		pError( "Message size not enough "<<endl);
		return false;
	}
	octet* o= (octet*)&us;
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


inline bool CDRMessage::addParameterId(CDRMessage_t* msg, ParameterId_t pid)
{
	return CDRMessage::addUInt16(msg,(uint16_t)pid);
}



inline bool CDRMessage::addInt32(CDRMessage_t* msg, int32_t lo) {
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



inline bool CDRMessage::addUInt32(CDRMessage_t* msg, uint32_t ulo) {
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

inline bool CDRMessage::addInt64(CDRMessage_t* msg, int64_t lolo) {
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

inline bool addOctetVector(CDRMessage_t*msg,std::vector<octet>* ocvec)
{
	if(msg->pos+4+ocvec->size()>=msg->max_size)
	{
		pError( "Message size not enough "<<endl);
				return false;
	}
	bool valid = CDRMessage::addUInt32(msg,ocvec->size());
	valid &= CDRMessage::addData(msg,(octet*)ocvec->data(),ocvec->size());

	int rest = ocvec->size()% 4;
	if (rest != 0)
		rest = 4 - rest; //how many you have to add
	msg->pos+=rest;
	msg->length+=rest;
	return valid;
}


inline bool CDRMessage::addEntityId(CDRMessage_t* msg, const EntityId_t*ID) {
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





inline bool CDRMessage::addSequenceNumber(CDRMessage_t* msg,
		SequenceNumber_t* sn) {
	addInt32(msg,sn->high);
	addUInt32(msg,sn->low);

	return true;
}



inline bool CDRMessage::addSequenceNumberSet(CDRMessage_t* msg,
		SequenceNumberSet_t* sns) {
	if(sns->base.to64long()== 0)
		return false;
	CDRMessage::addSequenceNumber(msg, &sns->base);
	//Add set
	if(sns->isSetEmpty())
	{
		addUInt32(msg,0); //numbits 0
		return true;
	}

	SequenceNumber_t maxseqNum = sns->get_maxSeqNum();

	uint32_t numBits = (uint32_t)(maxseqNum.to64long() - sns->base.to64long()+1);

	if(numBits > 256)
	{
		pWarning("CDRMessage:addSequenceNumberSet:seqNum max - base >256"<<std::endl);
		return false;
	}

	addUInt32(msg,numBits);
	uint8_t n_longs = (numBits+31)/32;
	int32_t* bitmap = new int32_t[n_longs];
	for(uint32_t i= 0;i<n_longs;i++)
	{
		bitmap[i] = 0;
	}
	uint32_t deltaN = 0;
	for(std::vector<SequenceNumber_t>::iterator it=sns->get_begin();
			it!=sns->get_end();++it)
	{
		deltaN =(uint32_t)( it->to64long() - sns->base.to64long());
		bitmap[(uint32_t)(deltaN/32)] = (bitmap[(uint32_t)(deltaN/32)] | (1<<(31-deltaN%32)));
	}
	for(uint32_t i= 0;i<n_longs;i++)
	{
		addInt32(msg,bitmap[i]);
	}
	delete(bitmap);
	return true;
}

inline bool CDRMessage::addLocator(CDRMessage_t* msg, Locator_t* loc) {
	addInt32(msg,loc->kind);
	addUInt32(msg,loc->port);

	addData(msg,loc->address,16);

	return true;
}

inline bool CDRMessage::addParameterStatus(CDRMessage_t* msg, octet status)
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


inline bool CDRMessage::addParameterKey(CDRMessage_t* msg, InstanceHandle_t* iHandle)
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

inline bool CDRMessage::addParameterSentinel(CDRMessage_t* msg)
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

inline bool CDRMessage::addString(CDRMessage_t*msg,std::string& in_str)
{
	uint32_t str_siz = in_str.size();
	int rest = str_siz % 4;
	if (rest != 0)
		rest = 4 - rest; //how many you have to add

	bool valid = CDRMessage::addUInt32(msg, str_siz);
	valid &= CDRMessage::addData(msg,
			(unsigned char*) in_str.c_str(), str_siz);
	if (rest != 0) {
		octet oc = '\0';
		for (int i = 0; i < rest; i++) {
			valid &= CDRMessage::addOctet(msg, oc);
		}
	}
	return valid;
}











} /* namespace rtps */
} /* namespace eprosima */

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
