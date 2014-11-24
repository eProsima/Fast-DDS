/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLPTopicDataType.cpp
 *
 */


#include "fastrtps/builtin/liveliness/WLPTopicDataType.h"
#include "fastrtps/utils/RTPSLog.h"
#include "fastrtps/qos/ParameterList.h"

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "WLPTopicDataType";

WLPTopicDataType::WLPTopicDataType() {
	// TODO Auto-generated constructor stub
	initial_data = (void*)aux_msg.buffer;
	m_liveliness = AUTOMATIC_LIVELINESS_QOS;
}

WLPTopicDataType::~WLPTopicDataType() {
	// TODO Auto-generated destructor stub
	aux_msg.buffer = (octet*)initial_data;
}


bool WLPTopicDataType::serialize(void* data,SerializedPayload_t* payload)
{
	const char* const METHOD_NAME = "serialize";
	logError(RTPS_LIVELINESS,"This method should not have been called");
	return false;
}

bool WLPTopicDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	const char* const METHOD_NAME = "deserialize";
		logError(RTPS_LIVELINESS,"This method should not have been called");
	return false;
}

bool WLPTopicDataType::getKey(void*data,InstanceHandle_t* ihandle)
{
	SerializedPayload_t* pl = (SerializedPayload_t*) data;
	CDRMessage::initCDRMsg(&aux_msg);
	aux_msg.buffer = pl->data;
	aux_msg.length = pl->length;
	aux_msg.max_size = pl->max_size;
	aux_msg.msg_endian = pl->encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
	for(uint8_t i =0;i<16;++i)
	{
		ihandle->value[i] = aux_msg.buffer[i];
	}

	return true;
}



} /* namespace rtps */
} /* namespace eprosima */
