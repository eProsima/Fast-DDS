/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldTopic.cpp
 *
 */

#include "fastcdr/FastBuffer.h"
#include "fastcdr/Cdr.h"

#include "HelloWorldType.h"
#include "HelloWorld.h"

HelloWorldType::HelloWorldType() {
	m_topicDataTypeName = "HelloWorldType";
	HelloWorld example;
	m_typeSize = example.getMaxCdrSerializedSize(0);
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
	payload->length = ser.getSerializedDataLength();
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

bool HelloWorldType::getKey(void* data, InstanceHandle_t* ihandle)
{
	return false;
}
