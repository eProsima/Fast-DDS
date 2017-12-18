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
 * @file LatencyTestTypes.h
 *
 */

#ifndef LATENCYTESTTYPES_H_
#define LATENCYTESTTYPES_H_

#include "fastrtps/fastrtps_all.h"



class LatencyType{
public:
	uint32_t seqnum;
	std::vector<uint8_t> data;
	LatencyType():
		seqnum(0)
	{
		seqnum = 0;
	}
	LatencyType(uint16_t number):
		seqnum(0),
		data(number,0)
	{

	}
	~LatencyType()
	{

	}
};


inline bool operator==(const LatencyType& lt1, const LatencyType& lt2)
{
	if(lt1.seqnum!=lt2.seqnum)
		return false;
	if(lt1.data.size()!=lt2.data.size())
		return false;
	for(size_t i = 0;i<lt1.data.size();++i)
	{
		if(lt1.data.at(i) != lt2.data.at(i))
			return false;
	}
	return true;
}

class LatencyDataType: public TopicDataType
{
public:
	LatencyDataType()
{
		m_topicDataTypeName = "LatencyType";
		m_typeSize = 15000;
		m_isGetKeyDefined = false;
};
	~LatencyDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
};

enum TESTCOMMAND:uint32_t{
	DEFAULT,
	READY,
	BEGIN,
	STOP,
	STOP_ERROR
};

typedef struct TestCommandType
{
	TESTCOMMAND m_command;
	TestCommandType(){
		m_command = DEFAULT;
	}
	TestCommandType(TESTCOMMAND com):m_command(com){}
}TestCommandType;

class TestCommandDataType:public TopicDataType
{
public:
	TestCommandDataType()
{
		m_topicDataTypeName = "TestCommandType";
		m_typeSize = 4;
		m_isGetKeyDefined = false;
};
	~TestCommandDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
};


#endif /* LATENCYTESTTYPES_H_ */
