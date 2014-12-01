/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputTypes.h
 *
 */

#ifndef THROUGHPUTTYPES_H_
#define THROUGHPUTTYPES_H_

#include "fastrtps/TopicDataType.h"
#include "fastrtps/utils/RTPSLog.h"
using namespace eprosima;
using namespace eprosima::fastrtps;

//typedef struct TroughputTimeStats{
//	uint32_t nsamples;
//	uint64_t totaltime_us;
//	uint32_t samplesize;
//	uint32_t demand;
//	double Mbitsec;
//	uint32_t lostsamples;
//	void compute()
//	{
//		Mbitsec = (((double)samplesize*8*nsamples))/(totaltime_us);
//	}
//}TroughputTimeStats;

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

//
//inline std::ostream& operator<<(std::ostream& output,const TroughputTimeStats& ts)
//{
//	return output << ts.nsamples << "||"<<ts.totaltime_us<< "||"<<ts.Mbitsec;
//}
//
//inline void printTimeStatsPublisher(const TroughputTimeStats& ts )
//{
//	//cout << "demand here; " << ts.demand << endl;
//	//printf("%6u",ts.demand);
//	printf("%6u,%12u, %7.2f,%6u,%12lu \n",ts.samplesize,ts.nsamples,ts.Mbitsec,ts.demand,ts.totaltime_us);
//}
//
//inline void printTimeStatsSubscriber(const TroughputTimeStats& ts )
//{
//	printf("%6u,%6u,%12u,%6u,%6u,%6u, %7.2f,%12lu \n",ts.samplesize,ts.demand,ts.nsamples,ts.lostsamples,0,0,ts.Mbitsec,ts.totaltime_us);
//}
//
//inline void printLabelsSubscriber()
//{
//	printf(" bytes,demand,     samples,  lost,   UNK,   UNK, Mbits/s,    time(us)\n");
//	printf("------ ------ ------------ ------ ------ ------  ------- ------------\n");
//}
//
//inline void printLabelsPublisher()
//{
//	printf(" bytes,     samples, Mbits/s,demand,    time(us)\n");
//	printf("------ ------------  ------- ------ ------------\n");
//}



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
		//cout << "Created vector of "<< number << "/"<<data.size() << endl;
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


class LatencyDataType: public TopicDataType
{
public:
	LatencyDataType()
{
		setName("LatencyType");
		m_typeSize = 25000;
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


class ThroughputCommandDataType:public TopicDataType
{
public:
	ThroughputCommandDataType()
{
		setName("ThroughputCommand");
		m_typeSize = 4*sizeof(uint32_t)+2*sizeof(uint64_t)+sizeof(double);
		m_isGetKeyDefined = false;
};
	~ThroughputCommandDataType(){};
	bool serialize(void*data,SerializedPayload_t* payload);
	bool deserialize(SerializedPayload_t* payload,void * data);
};


#endif /* THROUGHPUTTYPES_H_ */
