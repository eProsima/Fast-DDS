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

typedef struct TroughputTimeStats{
	uint32_t nsamples;
	uint64_t totaltime_us;
	uint32_t samplesize;
	float Mbitsec;
	void compute()
	{
		Mbitsec = samplesize*8*nsamples*1000000/(totaltime_us);
	}
}TroughputTimeStats;


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

inline bool operator==(LatencyType& lt1,LatencyType&lt2)
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

enum Command:uint32_t{
	DEFAULT,
	READY_TO_START,
	BEGIN,
	STOP_TEST
};

typedef struct ThroughputCommandType
{
	Command m_command;
	ThroughputCommandType(){
		m_command = DEFAULT;
	}
}ThroughputCommandType;

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


#endif /* THROUGHPUTTYPES_H_ */
