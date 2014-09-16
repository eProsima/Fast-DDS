/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldTopic.cpp
 *
 */

#include "fastcdr/FastBuffer.h"
#include "fastcdr/Cdr.h"

#include "HelloWorldType.h"


SimplePubSubType::SimplePubSubType() {
	m_topicDataTypeName = "SimplePubSubType";

	m_typeSize = 0; //HERE GOES THE MAXIMUM SIZE OF THE TYPE IN BYTES
	m_isGetKeyDefined = false;

}

HelloWorldType::~HelloWorldType() {

}

bool HelloWorldType::serialize(void* data, SerializedPayload_t* payload)
{
	//CONVERT DATA to pointer of your type
	YourType* p_type = (YourType*) data;

	// Object that manages the raw buffer.
	eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);
	// Object that serializes the data.
	eprosima::fastcdr::Cdr ser(fastbuffer);
	//serialize the object:
	p_type->serialize(ser);
	payload->length = ser.getSerializedDataLength();
	return true;
}

bool HelloWorldType::deserialize(SerializedPayload_t* payload, void* data)
{
	//CONVERT DATA to pointer of your type
	YourType* p_type = (YourType*) data;

	// Object that manages the raw buffer.
	eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length);
	// Object that serializes the data.
	eprosima::fastcdr::Cdr deser(fastbuffer);
	//serialize the object:
	p_type->deserialize(deser);
	return true;
}

bool HelloWorldType::getKey(void* data, InstanceHandle_t* ihandle)
{
	return false;
}
