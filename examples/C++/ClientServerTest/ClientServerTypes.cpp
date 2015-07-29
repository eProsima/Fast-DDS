/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ClientServerTypes.cpp
 *
 */

#include "ClientServerTypes.h"

namespace clientserver{

bool OperationDataType::serialize(void* data, SerializedPayload_t* payload)
{
	Operation* op = (Operation*)data;
	payload->length = this->m_typeSize;
	uint32_t pos = 0;
	memcpy(payload->data,op->m_guid.guidPrefix.value,12);pos+=12;
	memcpy(payload->data+pos,op->m_guid.entityId.value,4);pos+=4;
	*(uint32_t*)(payload->data+pos) = (uint32_t)op->m_operationId;pos+=4;
	*(Operation::OPERATIONTYPE*)(payload->data+pos) = op->m_operationType;pos+=4;
	*(int32_t*)(payload->data+pos) = op->m_num1;pos+=sizeof(int32_t);
	*(int32_t*)(payload->data+pos) = op->m_num2;pos+=sizeof(int32_t);
	return true;
}

bool OperationDataType::deserialize(SerializedPayload_t* payload, void* data)
{
	Operation* op = (Operation*)data;
	if(payload->length == this->m_typeSize)
	{
		uint32_t pos = 0;
		memcpy(op->m_guid.guidPrefix.value,payload->data,12);pos+=12;
		memcpy(op->m_guid.entityId.value,payload->data+pos,4);pos+=4;
		op->m_operationId = *(uint32_t*)(payload->data+pos);pos+=4;
		op->m_operationType = *(Operation::OPERATIONTYPE*)(payload->data+pos);pos+=4;
		op->m_num1 = *(int32_t*)(payload->data+pos);pos+=sizeof(int32_t);
		op->m_num2 = *(int32_t*)(payload->data+pos);pos+=sizeof(int32_t);
		return true;
	}
	return false;
}

void* OperationDataType::createData()
{
	return (void*)new Operation();
}
	void OperationDataType::deleteData(void* data)
	{
		delete((Operation*)data);
	}

bool ResultDataType::serialize(void* data, SerializedPayload_t* payload)
{
	Result* res = (Result*)data;
	payload->length = this->m_typeSize;
	uint32_t pos = 0;
	memcpy(payload->data,res->m_guid.guidPrefix.value,12);pos+=12;
	memcpy(payload->data+pos,res->m_guid.entityId.value,4);pos+=4;
	*(uint32_t*)(payload->data+pos) = (uint32_t)res->m_operationId;pos+=4;
	*(Result::RESULTTYPE*)(payload->data+pos) = res->m_resultType;pos+=4;
	*(int32_t*)(payload->data+pos) = res->m_result;
	return true;
}

bool ResultDataType::deserialize(SerializedPayload_t* payload, void* data)
{
	Result* res = (Result*)data;
	if(payload->length == this->m_typeSize)
	{
		uint32_t pos = 0;
		memcpy(res->m_guid.guidPrefix.value,payload->data,12);pos+=12;
		memcpy(res->m_guid.entityId.value,payload->data+pos,4);pos+=4;
		res->m_operationId = *(uint32_t*)(payload->data+pos);pos+=4;
		res->m_resultType =  *(Result::RESULTTYPE*)(payload->data+pos);pos+=4;
		res->m_result = *(int32_t*)(payload->data+pos);
		return true;
	}
	return false;
}

void* ResultDataType::createData()
{
	return (void*)new Result();
}
void ResultDataType::deleteData(void* data)
{
	delete((Result*)data);
}

}
