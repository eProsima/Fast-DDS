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
	//cout << "1"<<endl;
	if(payload->length > 0)
	{
	//	cout << "2"<<endl;
		LatencyType* lt = (LatencyType*)data;
		//cout << "3"<<endl;
		lt->seqnum = *(uint32_t*)payload->data;
		//cout << "4"<<endl;
		uint32_t siz = *(uint32_t*)(payload->data+4);
		//cout << "5"<<endl;
		//cout << siz << " length: "<<payload->length << endl;
		std::copy(payload->data+8,payload->data+8+siz,lt->data.begin());
//		lt->data.clear();
//		lt->data.insert(lt->data.end(),payload->data+8,payload->data+8+siz);
	}
	return true;
}


bool ThroughputCommandDataType::serialize(void*data,SerializedPayload_t* payload)
{
	ThroughputCommandType* t = (ThroughputCommandType*)data;
	*(e_Command*)payload->data = t->m_command;
	*(uint32_t*)(payload->data+4) = t->m_size;
	*(uint32_t*)(payload->data+4+4) = t->m_demand;
	payload->length = 12;
	return true;
}
bool ThroughputCommandDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	ThroughputCommandType* t = (ThroughputCommandType*)data;
	t->m_command = *(e_Command*)payload->data;
	t->m_size = *(uint32_t*)(payload->data+4);
	t->m_demand = *(uint32_t*)(payload->data+4+4);
	return true;
}


