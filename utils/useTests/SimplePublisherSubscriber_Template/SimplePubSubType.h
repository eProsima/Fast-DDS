/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldTopic.h
 *
 */

#ifndef HELLOWORLDTYPE_H_
#define HELLOWORLDTYPE_H_

#include "eprosimartps/rtps_all.h"
#include "YOURTYPE.h"

class SimplePubSubType:public DDSTopicDataType {
public:
	SimplePubSubType();
	virtual ~SimplePubSubType();
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
	bool getKey(void*data,InstanceHandle_t* ihandle);
};



#endif /* HELLOWORLDTOPIC_H_ */
