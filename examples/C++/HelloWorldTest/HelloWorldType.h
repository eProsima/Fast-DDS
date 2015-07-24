/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file HelloWorldTopic.h
 *
 */

#ifndef HELLOWORLDTYPE_H_
#define HELLOWORLDTYPE_H_

#include "fastrtps/TopicDataType.h"

using namespace eprosima::fastrtps;

#include "HelloWorld.h"

class HelloWorldType:public TopicDataType {
public:
	HelloWorldType();
	virtual ~HelloWorldType();
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
	bool getKey(void*data,InstanceHandle_t* ihandle);
	void* createData();
	void deleteData(void* data);
};



#endif /* HELLOWORLDTOPIC_H_ */
