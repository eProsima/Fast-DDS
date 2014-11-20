/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimpleTopicDataType.cpp
 *
 */

#include "eprosimartps/builtin/discovery/endpoint/EDPSimpleTopicDataType.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/qos/ParameterList.h"

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "EDPSimpleTopicDataType";


EDPSimpleTopicDataType::EDPSimpleTopicDataType()
{

	initial_data = (void*)aux_msg.buffer;

}

EDPSimpleTopicDataType::~EDPSimpleTopicDataType()
{
	aux_msg.buffer = (octet*)initial_data;
}

bool EDPSimpleTopicDataType::serialize(void* data,SerializedPayload_t* payload)
{
	const char* const METHOD_NAME = "serialize";
	logError(RTPS_EDP,"This method should not have been called");
	return false;
}

bool EDPSimpleTopicDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	const char* const METHOD_NAME = "deserialize";
		logError(RTPS_EDP,"This method should not have been called");
	return false;
}

bool EDPSimpleTopicDataType::getKey(void*data,InstanceHandle_t* ihandle)
{
	SerializedPayload_t* pl = (SerializedPayload_t*) data;
	CDRMessage::initCDRMsg(&aux_msg);
	aux_msg.buffer = pl->data;
	aux_msg.length = pl->length;
	aux_msg.max_size = pl->max_size;
	aux_msg.msg_endian = pl->encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
	bool valid = false;
	uint16_t pid;
	uint16_t plength;
	while(aux_msg.pos < aux_msg.length)
	{
		valid = true;
		valid&=CDRMessage::readUInt16(&aux_msg,(uint16_t*)&pid);
		valid&=CDRMessage::readUInt16(&aux_msg,&plength);
		if(pid == PID_SENTINEL)
		{
			break;
		}
		if(pid == PID_KEY_HASH)
		{
			valid &= CDRMessage::readData(&aux_msg,ihandle->value,16);
			return true;
		}
		if(pid == PID_ENDPOINT_GUID)
		{
			valid &= CDRMessage::readData(&aux_msg,ihandle->value,16);
			return true;
		}
		aux_msg.pos+=plength;
	}
	return false;
}



} /* namespace rtps */
} /* namespace eprosima */
