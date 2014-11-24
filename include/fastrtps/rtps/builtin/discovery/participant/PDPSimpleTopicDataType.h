/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimpleTopicDataType.h
 *
 */

#ifndef PDPSIMPLETOPICDATATYPE_H_
#define PDPSIMPLETOPICDATATYPE_H_

#include "fastrtps/pubsub/TopicDataType.h"
#include "fastrtps/common/types/CDRMessage_t.h"


using namespace eprosima::pubsub;

namespace eprosima {
namespace rtps {
/**
 * Class PDPSimpleTopicDataType used to deserialize the key from unregistering or disposing messages received in this topic.
 * @ingroup DISCOVERYMODULE
 */
class PDPSimpleTopicDataType : public TopicDataType {
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
