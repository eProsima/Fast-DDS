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
 * @file ThroughputTypes.h
 *
 */

#ifndef THROUGHPUTTYPES_H_
#define THROUGHPUTTYPES_H_

#include "fastrtps/TopicDataType.h"
#include <fastdds/dds/log/Log.hpp>
#include <zmq.hpp>




struct TroughputResults
{
	uint32_t payload_size;
	uint32_t demand;
	struct PublisherResults
	{
		uint64_t totaltime_us;
		uint64_t send_samples;
		double MBitssec;
	}publisher;
	struct SubscriberResults
	{
		uint64_t totaltime_us;
		uint64_t recv_samples;
		uint32_t lost_samples;
		double MBitssec;
	}subscriber;
	void compute()
	{
		publisher.MBitssec = (double)publisher.send_samples*payload_size*8/(double)publisher.totaltime_us;//bits/us==Mbits/s)
		subscriber.MBitssec = (double)subscriber.recv_samples*payload_size*8/(double)subscriber.totaltime_us;
	}
};


inline void printResultTitle()
{
	printf("[     TEST     ][              PUBLISHER              ][                     SUBSCRIBER                 ]\n");
	printf("[ Bytes, Demand][Sent Samples,Send Time(us), MBits/sec][Rec Samples,Lost Samples,Rec Time(us), MBits/sec]\n");
	printf("[------,-------][------------,-------------,----------][-----------,------------,------------,----------]\n");
}
#if defined(_WIN32)
inline void printResults(TroughputResults& res)
{
	printf("%7u,%7u,%12.0f,%13.0f,%10.3f,%12.0f,%12.0f,%13.0f,%10.3f\n",res.payload_size,res.demand,(double)res.publisher.send_samples,
			(double)res.publisher.totaltime_us,res.publisher.MBitssec,
			(double)res.subscriber.recv_samples,(double)res.subscriber.lost_samples,(double)res.subscriber.totaltime_us,
			(double)res.subscriber.MBitssec);
	//cout << "res: " <<res.payload_size << " "<<res.demand<< " "<<res.publisher.send_samples<< " "<<
	//															res.publisher.totaltime_us<< " "<<res.publisher.MBitssec<< " "<<
	//															res.subscriber.recv_samples<< " "<<res.subscriber.lost_samples<< " "<<res.subscriber.totaltime_us<< " "<<
	//															res.subscriber.MBitssec<< " "<<endl;
}
#else
inline void printResults(TroughputResults& res)
{
	printf("%7u,%7u,%12lu,%13lu,%10.3f,%12lu,%12u,%13lu,%10.3f\n",res.payload_size,res.demand,res.publisher.send_samples,
			res.publisher.totaltime_us,res.publisher.MBitssec,
			res.subscriber.recv_samples,res.subscriber.lost_samples,res.subscriber.totaltime_us,
			res.subscriber.MBitssec);
}

#endif


typedef struct LatencyType{
	uint32_t seqnum;
	std::vector<uint8_t> data;
	LatencyType(uint16_t number):
		seqnum(0),
		data(number,0)
	{
		//cout << "Created vector of "<< number << "/"<<data.size() << endl;
	}
}LatencyType;

inline bool operator==(const LatencyType& lt1,const LatencyType& lt2)
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


class ZMQLatencyDataType
{
public:
	ZMQLatencyDataType()
{

};
	~ZMQLatencyDataType(){};
	bool serialize(void*data,zmq::message_t* payload);
	bool deserialize(zmq::message_t* payload,void * data);

};

enum e_Command:uint32_t{
	DEFAULT,
	READY_TO_START,
	BEGIN,
	TEST_STARTS,
	TEST_ENDS,
	ALL_STOPS,
	TEST_RESULTS
};

typedef struct ThroughputCommandType
{
	e_Command m_command;
	uint32_t m_size;
	uint32_t m_demand;
	uint32_t m_lostsamples;
	uint64_t m_lastrecsample;
	uint64_t m_totaltime;
	ThroughputCommandType(){
		m_command = DEFAULT;
		m_size = 0;
		m_demand = 0;
		m_lostsamples = 0;
		m_lastrecsample = 0;
		m_totaltime = 0;
	}
	ThroughputCommandType(e_Command com):m_command(com),m_size(0),m_demand(0),
			m_lostsamples(0),m_lastrecsample(0),m_totaltime(0){}
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
	case (TEST_RESULTS): return output << "TEST RESULTS";
	default: return output << C_B_RED<<"UNKNOWN COMMAND"<<C_DEF;
	}
	return output;
}


class ZMQThroughputCommandDataType
{
public:
	ZMQThroughputCommandDataType()
{

};
	~ZMQThroughputCommandDataType(){};
	bool serialize(void*data,zmq::message_t* payload);
	bool deserialize(zmq::message_t* payload,void * data);

};


#endif /* THROUGHPUTTYPES_H_ */
