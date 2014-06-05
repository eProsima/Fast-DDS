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

#define TESTTIME 5
#define SAMPLESIZE 1024

typedef struct TroughputTimeStats{
	uint32_t nsamples;
	uint64_t totaltime_us;
	uint32_t samplesize;
	uint32_t demand;
	float Mbitsec;
	uint32_t lostsamples;
	void compute()
	{
		Mbitsec = (float)(samplesize*8*nsamples)/((float)totaltime_us);
	}
}TroughputTimeStats;

inline std::ostream& operator<<(std::ostream& output,const TroughputTimeStats& ts)
{
	return output << ts.nsamples << "||"<<ts.totaltime_us<< "||"<<ts.Mbitsec;
}

inline void printTimeStatsPublisher(const TroughputTimeStats& ts )
{
	printf("%6u,%12u, %7.2f,%12lu,%6u\n",ts.samplesize,ts.nsamples,ts.Mbitsec,ts.totaltime_us,ts.demand);
}

inline void printTimeStatsSubscriber(const TroughputTimeStats& ts )
{
	printf("%6u,%6u,%12u,%6u,%6u,%6u, %7.2f,%12lu\n",ts.samplesize,ts.demand,ts.nsamples,ts.lostsamples,0,0,ts.Mbitsec,ts.totaltime_us);
}

inline void printLabelsSubscriber()
{
	printf(" bytes,demand,     samples,  lost,   UNK,   UNK, Mbits/s,    time(us)\n");
	printf("------ ------ ------------ ------ ------ ------  ------- ------------\n");
}

inline void printLabelsPublisher()
{
	printf(" bytes,     samples, Mbits/s,    time(us),demand\n");
	printf("------ ------------  ------- ------------ ------\n");
}



typedef struct LatencyType{
	uint32_t seqnum;
	std::vector<uint8_t> data;
//	LatencyType():
//		seqnum(0)
//	{
//		seqnum = 0;
//	}
	LatencyType(uint16_t number):
		seqnum(0),
		data(number,0)
	{

	}
}LatencyType;

inline bool operator==(LatencyType& lt1,LatencyType& lt2)
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

enum e_Command:uint32_t{
	DEFAULT,
	READY_TO_START,
	BEGIN,
	TEST_STARTS,
	TEST_ENDS,
	ALL_STOPS
};

typedef struct ThroughputCommandType
{
	e_Command m_command;
	ThroughputCommandType(){
		m_command = DEFAULT;
	}
	ThroughputCommandType(e_Command com):m_command(com){}
}ThroughputCommandType;


inline std::ostream& operator<<(std::ostream& output,const ThroughputCommandType& com)
{
	switch(com.m_command)
	{
	case (DEFAULT): return output << "DEFAULT";
	case (READY_TO_START): return output << "READY_TO_START";
	case (BEGIN): return output << "BEGIN";
	case (TEST_STARTS): return output << "TEST_STARTS";
	case (TEST_ENDS): return output << "TEST_ENDS";
	case (ALL_STOPS): return output << "ALL_STOPS";
	default: return output << B_RED<<"UNKNOWN COMMAND"<<DEF;
	}
	return output;
}


class ThroughputCommandDataType:public DDSTopicDataType
{
public:
	ThroughputCommandDataType()
{
		m_topicDataTypeName = "ThroughputCommand";
		m_typeSize = 4;
		m_isGetKeyDefined = false;
};
	~ThroughputCommandDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
};


#endif /* THROUGHPUTTYPES_H_ */
