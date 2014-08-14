/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputTypes.cpp
 *
 */

#include "ThroughputTypes.h"


//Funciones de serializacion y deserializacion para el ejemplo
bool LatencyDataType::serialize(void*data,SerializedPayload_t* payload)
{
	LatencyType* lt = (LatencyType*)data;
	*(uint32_t*)payload->data = lt->seqnum;
	*(uint32_t*)(payload->data+4) = (uint32_t)lt->data.size();
	std::copy(lt->data.begin(),lt->data.end(),payload->data+8);
	payload->length = 8+lt->data.size();
	return true;
}

bool LatencyDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	if(payload->length > 0)
	{
		LatencyType* lt = (LatencyType*)data;
		lt->seqnum = *(uint32_t*)payload->data;
		uint32_t siz = *(uint32_t*)(payload->data+4);
		std::copy(payload->data+8,payload->data+8+siz,lt->data.begin());
		//		lt->data.clear();
		//		lt->data.insert(lt->data.end(),payload->data+8,payload->data+8+siz);
	}
	return true;
}


bool ThroughputCommandDataType::serialize(void*data,SerializedPayload_t* payload)
{
	ThroughputCommandType* t = (ThroughputCommandType*)data;
	uint32_t n = (uint32_t)t->m_command;
	Payload::addUInt32(payload,n);
	Payload::addUInt32(payload,t->m_size);
	Payload::addUInt32(payload,t->m_demand);
	Payload::addDouble(payload,t->m_mbits);
	Payload::addUInt32(payload,t->m_lostsamples);
	Payload::addUInt64(payload,t->m_recsamples);
	Payload::addUInt64(payload,t->m_totaltime);
	return true;
}
bool ThroughputCommandDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	ThroughputCommandType* t = (ThroughputCommandType*)data;
	Payload::readUInt32(payload,(uint32_t*)&t->m_command);
	Payload::readUInt32(payload,&t->m_size);
	Payload::readUInt32(payload,&t->m_demand);
	Payload::readDouble(payload,&t->m_mbits);
	Payload::readUInt32(payload,&t->m_lostsamples);
	Payload::readUInt64(payload,&t->m_recsamples);
	Payload::readUInt64(payload,&t->m_totaltime);
	return true;
}


