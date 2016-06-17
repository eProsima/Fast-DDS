/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldTopic.cpp
 *
 */

#include "fastcdr/FastBuffer.h"
#include "fastcdr/Cdr.h"
#include "HelloWorld.h"
#include "HelloWorldType.h"

HelloWorldType::HelloWorldType() {
	setName("HelloWorldType");
	m_typeSize = (uint32_t)HelloWorld::getMaxCdrSerializedSize();
	m_isGetKeyDefined = false;

}

HelloWorldType::~HelloWorldType() {
	// TODO Auto-generated destructor stub
}

bool HelloWorldType::serialize(void* data, SerializedPayload_t* payload)
{
	HelloWorld* hw = (HelloWorld*) data;
	// Object that manages the raw buffer.
	eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);
	// Object that serializes the data.
	eprosima::fastcdr::Cdr ser(fastbuffer);
	//serialize the object:
	hw->serialize(ser);
	payload->length = (uint16_t)ser.getSerializedDataLength();
	return true;
}

bool HelloWorldType::deserialize(SerializedPayload_t* payload, void* data)
{
	HelloWorld* hw = (HelloWorld*) data;
	// Object that manages the raw buffer.
	eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length);
	// Object that serializes the data.
	eprosima::fastcdr::Cdr deser(fastbuffer);
	//serialize the object:
	hw->deserialize(deser);
	return true;
}

void* HelloWorldType::createData()
{
	return (void*)new HelloWorld();
}
void HelloWorldType::deleteData(void* data)
{
	delete((HelloWorld*)data);
}

bool HelloWorldType::getKey(void* /*data*/, InstanceHandle_t* /*ihandle*/)
{
	return false;
}
