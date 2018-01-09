// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file HelloWorldTopic.h
 *
 */

#ifndef HELLOWORLDTYPE_H_
#define HELLOWORLDTYPE_H_

#include "fastrtps/TopicDataType.h"



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
