/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimpleTopicDataType.h
 *
 */

#ifndef PDPSIMPLETOPICDATATYPE_H_
#define PDPSIMPLETOPICDATATYPE_H_

#include "eprosimartps/dds/DDSTopicDataType.h"
#include "eprosimartps/common/types/CDRMessage_t.h"


using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

class PDPSimpleTopicDataType : public DDSTopicDataType {
public:
	PDPSimpleTopicDataType();
	virtual ~PDPSimpleTopicDataType();

	bool serialize(void* data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
	bool getKey(void*data,InstanceHandle_t* ihandle);
	CDRMessage_t aux_msg;
	void* initial_data;

};









} /* namespace rtps */
} /* namespace eprosima */

#endif /* PDPSIMPLETOPICDATATYPE_H_ */
