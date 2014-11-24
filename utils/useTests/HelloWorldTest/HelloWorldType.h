/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldTopic.h
 *
 */

#ifndef HELLOWORLDTYPE_H_
#define HELLOWORLDTYPE_H_

#include "fastrtps/rtps_all.h"


class HelloWorldType:public TopicDataType {
public:
	HelloWorldType();
	virtual ~HelloWorldType();
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
	bool getKey(void*data,InstanceHandle_t* ihandle);
};



#endif /* HELLOWORLDTOPIC_H_ */
