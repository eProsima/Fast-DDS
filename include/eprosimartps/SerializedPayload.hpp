/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * @file CDRMessage.hpp
 *
 */



namespace eprosima {
namespace rtps {


#include <cstring>
#include "eprosimartps/common/types/SerializedPayload.h"

namespace Payload
{
inline bool addUInt32(SerializedPayload_t* payload,uint32_t& n)
{
	if(payload->length+4>payload->max_size)
		return false;
	*(uint32_t*)(payload->data+payload->length) = n;
	payload->length+=4;
	return true;
}
inline bool readUInt32(SerializedPayload_t* payload,uint32_t* n)
{
	if(payload->pos+4>payload->length)
		return false;
	*n = *(uint32_t*)(payload->data+payload->pos);
	payload->pos+=4;
	return true;
}
inline bool addDouble(SerializedPayload_t*payload,double& n)
{
	if(payload->length+sizeof(double)>payload->max_size)
		return false;
	*(double*)(payload->data+payload->length) = n;
	payload->length+=sizeof(double);
	return true;
}
inline bool readDouble(SerializedPayload_t* payload,double* n)
{
	if(payload->pos+sizeof(double)>payload->length)
		return false;
	*n = *(double*)(payload->data+payload->pos);
	payload->pos+=sizeof(double);
	return true;
}
inline bool addUInt64(SerializedPayload_t*payload,uint64_t& n)
{
	if(payload->length+8>payload->max_size)
		return false;
	*(uint64_t*)(payload->data+payload->length) = n;
	payload->length+=8;
	return true;
}
inline bool readUInt64(SerializedPayload_t* payload,uint64_t* n)
{
	if(payload->pos+8>payload->length)
		return false;
	*n = *(uint64_t*)(payload->data+payload->pos);
	payload->pos+=8;
	return true;
}
inline bool addUInt16(SerializedPayload_t*payload,uint16_t& n)
{
	if(payload->length+2>payload->max_size)
		return false;
	*(uint16_t*)(payload->data+payload->length) = n;
	payload->length+=2;
	return true;
}
inline bool readUInt16(SerializedPayload_t* payload,uint16_t* n)
{
	if(payload->pos+2>payload->length)
		return false;
	*n = *(uint16_t*)(payload->data+payload->pos);
	payload->pos+=2;
	return true;
}
inline bool addInt(SerializedPayload_t*payload,int& n)
{
	if(payload->length+sizeof(int)>payload->max_size)
		return false;
	*(int*)(payload->data+payload->length) = n;
	payload->length+=sizeof(int);
	return true;
}
inline bool readInt(SerializedPayload_t* payload,int* n)
{
	if(payload->pos+sizeof(int)>payload->length)
		return false;
	*n = *(int*)(payload->data+payload->pos);
	payload->pos+=sizeof(int);
	return true;
}
inline bool addFloat(SerializedPayload_t*payload,float& n)
{
	if(payload->length+sizeof(float)>payload->max_size)
		return false;
	*(float*)(payload->data+payload->length) = n;
	payload->length+=sizeof(float);
	return true;
}
inline bool readFloat(SerializedPayload_t* payload,float* n)
{
	if(payload->pos+sizeof(float)>payload->length)
		return false;
	*n = *(float*)(payload->data+payload->pos);
	payload->pos+=sizeof(float);
	return true;
}
inline bool addData(SerializedPayload_t*payload,void* data,uint32_t len)
{
	if(payload->length+len>payload->max_size)
		return false;
	memcpy(payload->data+payload->length,data,len);
	payload->length+=len;
	return true;
}
inline bool readData(SerializedPayload_t* payload,void* data,uint32_t len)
{
	if(payload->pos+len>payload->length)
		return false;
	memcpy(data,payload->data+payload->pos,len);
	payload->pos+=len;
	return true;
}
inline bool addString(SerializedPayload_t* payload,std::string& in_str)
{
	if(payload->length+4+in_str.size()+1>payload->max_size)
		return false;
	uint32_t str_siz = (uint32_t)in_str.size();
	int rest = (str_siz+1) % 4;
	if (rest != 0)
		rest = 4 - rest; //how many you have to add
	*(uint32_t*)(payload->data+payload->length) = str_siz+1;
	payload->length+=4;
	memcpy(payload->data+payload->length,(void*) in_str.c_str(),str_siz+1);
	payload->length+=str_siz+1;
	if (rest != 0) {
		octet oc = '\0';
		for (int i = 0; i < rest; i++)
		{
			*(octet*)(payload->data+payload->length) = oc;
			++payload->length;
		}
	}
	return true;
}
inline bool readString(SerializedPayload_t*payload, std::string* stri)
{
	uint32_t str_size = 1;
	bool valid = true;
	valid&=Payload::readUInt32(payload,&str_size);
	if(str_size>1)
	{
		*stri = std::string(str_size-1,'\0');
		stri->copy((char*)(payload->data+payload->pos),str_size,0);
		payload->pos+=str_size;
	}
	else
	{
		payload->pos+=str_size;
		valid = false;
	}
	uint32_t rest = (uint32_t)(str_size-4*(str_size/4));
	rest = rest==0 ? 0 : 4-rest;
	payload->pos+=rest;

	return valid;
}

}


} /* namespace rtps */
} /* namespace eprosima */

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
