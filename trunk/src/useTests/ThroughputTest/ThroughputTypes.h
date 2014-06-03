/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputTypes.h
 *
 *  Created on: Jun 3, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef THROUGHPUTTYPES_H_
#define THROUGHPUTTYPES_H_

#include "eprosimartps/rtps_all.h"

#define TESTTIME 30


typedef struct LatencyType{
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
}LatencyType;

bool operator==(LatencyType& lt1,LatencyType&lt2)
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


class LatencyDataType: public DDSTopicDataType
{
public:
	LatencyDataType()
{
		m_topicDataTypeName = "LatencyType";
		m_typeSize = 4+4+4096;
		m_isGetKeyDefined = false;
};
	~LatencyDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
};

//Funciones de serializacion y deserializacion para el ejemplo
bool LatencyDataType::serialize(void*data,SerializedPayload_t* payload)
{
	LatencyType* lt = (LatencyType*)data;
	*(uint32_t*)payload->data = lt->seqnum;
	*(uint32_t*)(payload->data+4) = (uint32_t)lt->data.size();
	std::copy(lt->data.begin(),lt->data.end(),payload->data+8);
	payload->length = 8+lt->data.size();
	return true;
}

bool LatencyDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	LatencyType* lt = (LatencyType*)data;
	lt->seqnum = *(uint32_t*)payload->data;
	uint32_t siz = *(uint32_t*)(payload->data+4);
	std::copy(payload->data+8,payload->data+8+siz,lt->data.begin());
	return true;
}

typedef struct ThroughputCommandType
{
	enum Command:uint32_t{
		BEGIN,
		STOP,
		ALL_OK,
	}m_command;

};

class ThroughputDataType:public DDSTopicDataType
{
public:
	ThroughputDataType()
{
		m_topicDataTypeName = "ThroughputCommand";
		m_typeSize = 4;
		m_isGetKeyDefined = false;
};
	~ThroughputDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
};

bool ThroughputDataType::serialize(void*data,SerializedPayload_t* payload)
{
	ThroughputCommandType* t = (ThroughputCommandType*)data;
	*(uint32_t*)payload->data = t->m_command;
	payload->length = 4;
	return true;
}
bool ThroughputDataType::deserialize(SerializedPayload_t* payload,void * data)
{
	ThroughputCommandType* t = (ThroughputCommandType*)data;
	 t->m_command = *(uint32_t*)payload->data;
	return true;
}


#endif /* THROUGHPUTTYPES_H_ */
