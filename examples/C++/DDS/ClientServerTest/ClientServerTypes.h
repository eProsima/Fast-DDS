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
 * @file ClientServerTypes.h
 *
 */

#ifndef CLIENTSERVERTYPES_H_
#define CLIENTSERVERTYPES_H_

#include "fastrtps/TopicDataType.h"
#include "fastrtps/rtps/common/all_common.h"

namespace clientserver{

class Operation
{
public:
	enum OPERATIONTYPE:uint32_t{
		ADDITION,
		SUBTRACTION,
		MULTIPLICATION,
		DIVISION,
	};
	eprosima::fastrtps::rtps::GUID_t m_guid;
	uint32_t m_operationId;
	OPERATIONTYPE m_operationType;
	int32_t m_num1;
	int32_t m_num2;
	Operation():m_operationId(0),m_operationType(ADDITION),
			m_num1(0),m_num2(0){}
	~Operation(){}
};

class OperationDataType:public eprosima::fastrtps::TopicDataType
{
public:
	OperationDataType()
{
		setName("Operation");
		m_typeSize = 16+4+4+2*sizeof(int32_t);
		m_isGetKeyDefined = false;
};
	~OperationDataType(){};
	bool serialize(void*data, eprosima::fastrtps::rtps::SerializedPayload_t* payload);
	bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t* payload,void * data);
    std::function<uint32_t()> getSerializedSizeProvider(void* data);
	bool getKey(void*, eprosima::fastrtps::rtps::InstanceHandle_t*, bool) { return false; }

	void* createData();
	void deleteData(void* data);
};


class Result
{
public:
	enum RESULTTYPE:uint32_t
	{
		GOOD_RESULT,
		ERROR_RESULT,
		SERVER_NOT_READY
	};
	eprosima::fastrtps::rtps::GUID_t m_guid;
	uint32_t m_operationId;
	RESULTTYPE m_resultType;
	int32_t m_result;
	Result():m_operationId(0),m_resultType(GOOD_RESULT),m_result(0){}
	~Result(){}
};


class ResultDataType:public eprosima::fastrtps::TopicDataType
{
public:
	ResultDataType()
{
		setName("Result");
		m_typeSize = 16+4+4+sizeof(int32_t);
		m_isGetKeyDefined = false;
};
	~ResultDataType(){};
	bool serialize(void*data, eprosima::fastrtps::rtps::SerializedPayload_t* payload);
	bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t* payload,void * data);
    std::function<uint32_t()> getSerializedSizeProvider(void* data);
	bool getKey(void*, eprosima::fastrtps::rtps::InstanceHandle_t*, bool) { return false; }
	void* createData();
	void deleteData(void* data);
};
}
#endif /* CLIENTSERVERTYPES_H_ */
