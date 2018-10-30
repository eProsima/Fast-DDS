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
 * @file ClientServerTypes.cpp
 *
 */

#include "ClientServerTypes.h"

using namespace clientserver;
using namespace eprosima::fastrtps::rtps;

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

std::function<uint32_t()> OperationDataType::getSerializedSizeProvider(void* /*data*/)
{
    return []() -> uint32_t { return 16 + 4 + 4 + 4 + 4; };
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

std::function<uint32_t()> ResultDataType::getSerializedSizeProvider(void* /*data*/)
{
    return []() -> uint32_t { return 16 + 4 + 4 + 4; };
}

void* ResultDataType::createData()
{
	return (void*)new Result();
}
void ResultDataType::deleteData(void* data)
{
	delete((Result*)data);
}