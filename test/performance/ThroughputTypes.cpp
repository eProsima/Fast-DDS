/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputTypes.cpp
 *
 */

#include "ThroughputTypes.h"

//Funciones de serializacion y deserializacion para el ejemplo
bool ThroughputDataType::serialize(void*data,SerializedPayload_t* payload)
{
	ThroughputType* lt = (ThroughputType*)data;
	*(uint32_t*)payload->data = lt->seqnum;
	*(uint32_t*)(payload->data+4) = (uint32_t)lt->data.size();
//	std::copy(lt->data.begin(),lt->data.end(),payload->data+8);
	memcpy(payload->data + 8, lt->data.data(), lt->data.size());
	payload->length = 8+(uint16_t)lt->data.size();
	return true;
}

bool ThroughputDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	if(payload->length > 0)
	{
		ThroughputType* lt = (ThroughputType*)data;
		lt->seqnum = *(uint32_t*)payload->data;
		uint32_t siz = *(uint32_t*)(payload->data+4);
		//std::copy(payload->data+8,payload->data+8+siz,lt->data.begin());
		std::copy(payload->data + 8, payload->data + 8 + siz, lt->data.begin());
		//		lt->data.clear();
		//		lt->data.insert(lt->data.end(),payload->data+8,payload->data+8+siz);
	}
	return true;
}

void* ThroughputDataType::createData()
{
	return (void*)new ThroughputType(this->m_typeSize);
}
void ThroughputDataType::deleteData(void* data)
{
	delete((ThroughputType*)data);
}



bool ThroughputCommandDataType::serialize(void*data,SerializedPayload_t* p)
{
	ThroughputCommandType* t = (ThroughputCommandType*)data;
	p->length = 0;
	*(uint32_t*)p->data = (uint32_t)t->m_command;p->length+=4;
	*(uint32_t*)&(p->data[p->length]) = (uint32_t)t->m_size;p->length+=4;
	*(uint32_t*)&(p->data[p->length]) = (uint32_t)t->m_demand;p->length+=4;
	*(uint32_t*)&(p->data[p->length]) = (uint32_t)t->m_lostsamples;p->length+=4;
	*(uint64_t*)&(p->data[p->length]) = (uint32_t)t->m_lastrecsample;p->length+=8;
	*(uint64_t*)&(p->data[p->length]) = (uint32_t)t->m_totaltime;p->length+=8;
	return true;
}
bool ThroughputCommandDataType::deserialize(SerializedPayload_t* p,void * data)
{
	ThroughputCommandType* t = (ThroughputCommandType*)data;
	p->pos = 0;
	t->m_command = (e_Command)*(uint32_t*)p->data;p->pos+=4;
	t->m_size = *(uint32_t*)&p->data[p->pos];p->pos+=4;
	t->m_demand = *(uint32_t*)&p->data[p->pos];p->pos+=4;
	t->m_lostsamples = *(uint32_t*)&p->data[p->pos];p->pos+=4;
	t->m_lastrecsample = *(uint64_t*)&p->data[p->pos];p->pos+=8;
	t->m_totaltime = *(uint64_t*)&p->data[p->pos];p->pos+=8;
	return true;
}

void* ThroughputCommandDataType::createData()
{
	return (void*)new ThroughputCommandType();
}
void ThroughputCommandDataType::deleteData(void* data)
{
	delete((ThroughputCommandType*)data);
}


