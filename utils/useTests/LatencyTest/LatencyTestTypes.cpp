/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LatencyTestTypes.cpp
 *
 */

#include "LatencyTestTypes.h"


bool LatencyDataType::serialize(void*data,SerializedPayload_t* payload)
{
	LatencyType* lt = (LatencyType*)data;
	*(uint32_t*)payload->data = lt->seqnum;
	*(uint32_t*)(payload->data+4) = (uint32_t)lt->data.size();
	//std::copy(lt->data.begin(),lt->data.end(),payload->data+8);
	memcpy(payload->data + 8, lt->data.data(), lt->data.size());
	payload->length = (uint16_t)(8+lt->data.size());
	return true;
}

bool LatencyDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	LatencyType* lt = (LatencyType*)data;
	lt->seqnum = *(uint32_t*)payload->data;
	uint32_t siz = *(uint32_t*)(payload->data+4);
	std::copy(payload->data+8,payload->data+8+siz,lt->data.begin());
	return true;
}

void* LatencyDataType::createData()
{
	return (void*)new LatencyType();
}
void LatencyDataType::deleteData(void* data)
{
	delete((LatencyType*)data);
}


bool TestCommandDataType::serialize(void*data,SerializedPayload_t* payload)
{
	TestCommandType* t = (TestCommandType*)data;
	*(TESTCOMMAND*)payload->data = t->m_command;
	payload->length = 4;
	return true;
}
bool TestCommandDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	TestCommandType* t = (TestCommandType*)data;
//	cout << "PAYLOAD LENGTH: "<<payload->length << endl;
//	cout << "PAYLOAD FIRST BYTE: "<< (int)payload->data[0] << endl;
	 t->m_command = *(TESTCOMMAND*)payload->data;
//	cout << "COMMAND: "<<t->m_command<< endl;
	return true;
}

void* TestCommandDataType::createData()
{
	return (void*)new TestCommandType();
}
void TestCommandDataType::deleteData(void* data)
{
	delete((TestCommandType*)data);
}
