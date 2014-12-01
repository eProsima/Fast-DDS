/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ClientServerTypes.h
 *
 */

#ifndef CLIENTSERVERTYPES_H_
#define CLIENTSERVERTYPES_H_

#include "fastrtps/rtps_all.h"

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
	GUID_t m_guid;
	uint32_t m_operationId;
	OPERATIONTYPE m_operationType;
	int32_t m_num1;
	int32_t m_num2;
	Operation():m_operationId(0),m_operationType(ADDITION),
			m_num1(0),m_num2(0){}
	~Operation(){}
};

class OperationDataType:public DDSTopicDataType
{
public:
	OperationDataType()
{
		m_topicDataTypeName = "Operation";
		m_typeSize = 16+4+4+2*sizeof(int32_t);
		m_isGetKeyDefined = false;
};
	~OperationDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
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
	GUID_t m_guid;
	uint32_t m_operationId;
	RESULTTYPE m_resultType;
	int32_t m_result;
	Result():m_operationId(0),m_resultType(GOOD_RESULT),m_result(0){}
	~Result(){}
};


class ResultDataType:public DDSTopicDataType
{
public:
	ResultDataType()
{
		m_topicDataTypeName = "Result";
		m_typeSize = 16+4+4+sizeof(int32_t);
		m_isGetKeyDefined = false;
};
	~ResultDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
};
}
#endif /* CLIENTSERVERTYPES_H_ */
