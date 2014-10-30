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
#include "MyType.h"

using namespace eprosima::fastcdr;

SimplePubSubType::SimplePubSubType():
	DDSTopicDataType()
{
	m_topicDataTypeName = std::string("MyType");
	m_typeSize = (uint32_t)MyType::getMaxCdrSerializedSize(); 
	m_isGetKeyDefined = false;

	m_keyBuffer = (unsigned char*)malloc(MyType::getMaxCdrSerializedSize());
}

SimplePubSubType::~SimplePubSubType() {

}

bool SimplePubSubType::serialize(void* data, SerializedPayload_t* payload)
{
	//CONVERT DATA to pointer of your type
	MyType* p_type = (MyType*) data;

	// Object that manages the raw buffer.
	eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->max_size);
	// Object that serializes the data.
	eprosima::fastcdr::Cdr ser(fastbuffer,Cdr::LITTLE_ENDIANNESS);
	//Select the correct endianess
	payload->encapsulation = CDR_LE; 
	//serialize the object:
	p_type->serialize(ser);
	payload->length = (uint16_t)ser.getSerializedDataLength();
	
	return true;
}

bool SimplePubSubType::deserialize(SerializedPayload_t* payload, void* data)
{
	//CONVERT DATA to pointer of your type
	MyType* p_type = (MyType*) data;
	// Object that manages the raw buffer.
	eprosima::fastcdr::FastBuffer fastbuffer((char*)payload->data, payload->length);
	//select the correct endianess
	Cdr::Endianness endian = payload->encapsulation == CDR_LE ? Cdr::LITTLE_ENDIANNESS : Cdr::BIG_ENDIANNESS;
	// Object that serializes the data.
	eprosima::fastcdr::Cdr deser(fastbuffer,endian);
	//serialize the object:
	p_type->deserialize(deser);
	return true;
}

bool SimplePubSubType::getKey(void* data, InstanceHandle_t* ihandle)
{
	MyType* p_type = (MyType*) data;

	// Object that manages the raw buffer.
	eprosima::fastcdr::FastBuffer fastbuffer((char*)m_keyBuffer,MyType::getMaxCdrSerializedSize());
	// Object that serializes the data.
	eprosima::fastcdr::Cdr ser(fastbuffer,Cdr::BIG_ENDIANNESS);

	//p_type->serializeKeyMembers(ser);
	md5.init();
	md5.update(m_keyBuffer,ser.getSerializedDataLength());
	md5.finalize();
	for(uint8_t i = 0;i<16;++i)
    {
        ihandle->value[i] = md5.digest[i];
    }


	return false;
}
