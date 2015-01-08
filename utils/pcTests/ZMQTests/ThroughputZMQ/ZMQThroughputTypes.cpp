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

#include "ZMQThroughputTypes.h"


//Funciones de serializacion y deserializacion para el ejemplo
bool ZMQLatencyDataType::serialize(void*data,zmq::message_t* payload)
{
	LatencyType* lt = (LatencyType*)data;
	*(uint32_t*)payload->data() = lt->seqnum;
	*(uint32_t*)((uint8_t*)payload->data()+4) = (uint32_t)lt->data.size();
	//	std::copy(lt->data.begin(),lt->data.end(),payload->data+8);
	memcpy((uint8_t*)payload->data() + 8, lt->data.data(), lt->data.size());
	return true;
}

bool ZMQLatencyDataType::deserialize(zmq::message_t* payload,void * data)
{
	LatencyType* lt = (LatencyType*)data;
	lt->seqnum = *(uint32_t*)payload->data();
	uint32_t siz = *(uint32_t*)((uint8_t*)payload->data()+4);
	std::copy((uint8_t*)payload->data() + 8, (uint8_t*)payload->data() + 8 + siz, lt->data.begin());
	return true;
}


bool ZMQThroughputCommandDataType::serialize(void*data,zmq::message_t* p)
{
	ThroughputCommandType* t = (ThroughputCommandType*)data;
	uint16_t length = 0;
	*(uint32_t*)p->data() = (uint32_t)t->m_command;length+=4;
	*(uint32_t*)&(((uint8_t*)p->data())[length]) = (uint32_t)t->m_size;length+=4;
	*(uint32_t*)&(((uint8_t*)p->data())[length]) = (uint32_t)t->m_demand;length+=4;
	*(uint32_t*)&(((uint8_t*)p->data())[length]) = (uint32_t)t->m_lostsamples;length+=4;
	*(uint64_t*)&(((uint8_t*)p->data())[length]) = (uint32_t)t->m_lastrecsample;length+=8;
	*(uint64_t*)&(((uint8_t*)p->data())[length]) = (uint32_t)t->m_totaltime;length+=8;
	return true;
}
bool ZMQThroughputCommandDataType::deserialize(zmq::message_t* p,void * data)
{
	ThroughputCommandType* t = (ThroughputCommandType*)data;
	uint16_t pos = 0;
	t->m_command = (e_Command)*(uint32_t*)p->data();pos+=4;
	t->m_size = *(uint32_t*)&((uint8_t*)p->data())[pos];pos+=4;
	t->m_demand = *(uint32_t*)&((uint8_t*)p->data())[pos];pos+=4;
	t->m_lostsamples = *(uint32_t*)&((uint8_t*)p->data())[pos];pos+=4;
	t->m_lastrecsample = *(uint64_t*)&((uint8_t*)p->data())[pos];pos+=8;
	t->m_totaltime = *(uint64_t*)&((uint8_t*)p->data())[pos];pos+=8;
	return true;
}

