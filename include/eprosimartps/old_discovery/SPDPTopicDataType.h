/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SPDPTopicDataType.h
 *
 */

#ifndef SPDPTOPICDATATYPE_H_
#define SPDPTOPICDATATYPE_H_

#include "eprosimartps/dds/DDSTopicDataType.h"
#include "eprosimartps/common/types/CDRMessage_t.h"


using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

class SPDPTopicDataType : public DDSTopicDataType {
public:
	SPDPTopicDataType();
	virtual ~SPDPTopicDataType();

	bool serialize(void* data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
	bool getKey(void*data,InstanceHandle_t* ihandle);
	CDRMessage_t aux_msg;
	void* initial_data;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SPDPTOPICDATATYPE_H_ */
