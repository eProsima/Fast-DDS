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

#include "SimplePubSubType.h"


SimplePubSubType::SimplePubSubType() {
	m_topicDataTypeName = "SimplePubSubType";
	YOURTYPE example;
	m_typeSize = YOURTYPE.getMaxCdrSerializedSize(0); //HERE GOES THE MAXIMUM SIZE OF THE TYPE IN BYTES
	m_isGetKeyDefined = false;

}

SimplePubSubType::~SimplePubSubType() {

}

bool SimplePubSubType::serialize(void* data, SerializedPayload_t* payload)
{
	//CONVERT DATA to pointer of your type
	YOURTYPE* p_type = (YOURTYPE*) data;

	// Object that manages the raw buffer.
	eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);
	// Object that serializes the data.
	eprosima::fastcdr::Cdr ser(fastbuffer);
	//serialize the object:
	p_type->serialize(ser);
	payload->length = ser.getSerializedDataLength();
	return true;
}

bool SimplePubSubType::deserialize(SerializedPayload_t* payload, void* data)
{
	//CONVERT DATA to pointer of your type
	YOURTYPE* p_type = (YOURTYPE*) data;

	// Object that manages the raw buffer.
	eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length);
	// Object that serializes the data.
	eprosima::fastcdr::Cdr deser(fastbuffer);
	//serialize the object:
	p_type->deserialize(deser);
	return true;
}

bool SimplePubSubType::getKey(void* data, InstanceHandle_t* ihandle)
{
	return false;
}
