/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLPTopicDataType.h
 *
 */

#ifndef WLPTOPICDATATYPE_H_
#define WLPTOPICDATATYPE_H_

#include "eprosimartps/pubsub/TopicDataType.h"
#include "eprosimartps/common/types/CDRMessage_t.h"
#include "eprosimartps/qos/QosPolicies.h"

using namespace eprosima::pubsub;

namespace eprosima {
namespace rtps {

class WLPTopicDataType : public TopicDataType {
public:
	WLPTopicDataType();
	virtual ~WLPTopicDataType();
	bool serialize(void* data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
	bool getKey(void*data,InstanceHandle_t* ihandle);
	CDRMessage_t aux_msg;
	void* initial_data;

	GuidPrefix_t m_guidP;
	LivelinessQosPolicyKind m_liveliness;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WLPTOPICDATATYPE_H_ */
