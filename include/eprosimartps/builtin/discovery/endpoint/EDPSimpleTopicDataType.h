/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimpleTopicDataType.h
 *
 */

#ifndef EDPSIMPLETOPICDATATYPE_H_
#define EDPSIMPLETOPICDATATYPE_H_

#include "eprosimartps/pubsub/TopicDataType.h"
#include "eprosimartps/common/types/CDRMessage_t.h"

using namespace eprosima::pubsub;

namespace eprosima {
namespace rtps {

/**
 * Class EDPSimpleTopicDataType, used by the history to extract the key in case the data comes serialized and not as inlineQos.
 * @ingroup DISCOVERYMODULE
 */
class EDPSimpleTopicDataType: public TopicDataType {
public:
	EDPSimpleTopicDataType();
	virtual ~EDPSimpleTopicDataType();

	bool serialize(void* data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
	bool getKey(void*data,InstanceHandle_t* ihandle);
	CDRMessage_t aux_msg;
	void* initial_data;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDPSIMPLETOPICDATATYPE_H_ */
