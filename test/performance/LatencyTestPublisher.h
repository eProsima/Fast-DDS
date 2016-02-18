/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LatencyPublisher.h
 *
 */

#ifndef LATENCYPUBLISHER_H_
#define LATENCYPUBLISHER_H_

#include "LatencyTestTypes.h"

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

class TimeStats{
public:
	TimeStats() :nbytes(0), m_min(0), m_max(0), mean(0), p50(0), p90(0), p99(0), p9999(0), stdev(0){}
	~TimeStats(){}
	uint64_t nbytes;
	double m_min, m_max, mean, p50, p90, p99, p9999;
	double stdev;
};

class LatencyTestPublisher {
public:
	LatencyTestPublisher();
	virtual ~LatencyTestPublisher();

	Participant* mp_participant;
	Publisher* mp_datapub;
	Publisher* mp_commandpub;
	Subscriber* mp_datasub;
	Subscriber* mp_commandsub;
	LatencyType* mp_latency_in;
	LatencyType* mp_latency_out;
	eClock m_clock;
	Time_t m_t1,m_t2;
	double m_overhead;
	int n_subscribers;
	int n_samples;
	SampleInfo_t m_sampleinfo;
	std::vector<double> m_times;
	std::vector<TimeStats> m_stats;
	boost::interprocess::interprocess_semaphore m_disc_sema;
	boost::interprocess::interprocess_semaphore m_comm_sema;
	boost::interprocess::interprocess_semaphore m_data_sema;
	int m_status;
	int n_received;
	bool init(int n_sub,int n_sam);
	void run();
	void analizeTimes(uint32_t datasize);
	bool test(uint32_t datasize);
	void printStat(TimeStats& TS);
	class DataPubListener : public PublisherListener
	{
	public:
		DataPubListener(LatencyTestPublisher* up):mp_up(up),n_matched(0){}
		~DataPubListener(){}
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);
		LatencyTestPublisher* mp_up;
		int n_matched;
	}m_datapublistener;
	class DataSubListener : public SubscriberListener
	{
	public:
		DataSubListener(LatencyTestPublisher* up):mp_up(up),n_matched(0){}
		~DataSubListener(){}
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& into);
		void onNewDataMessage(Subscriber* sub);
		LatencyTestPublisher* mp_up;
		int n_matched;
	}m_datasublistener;
	class CommandPubListener : public PublisherListener
	{
	public:
		CommandPubListener(LatencyTestPublisher* up):mp_up(up),n_matched(0){}
		~CommandPubListener(){}
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);
		LatencyTestPublisher* mp_up;
		int n_matched;
	}m_commandpublistener;
	class CommandSubListener : public SubscriberListener
	{
	public:
		CommandSubListener(LatencyTestPublisher* up):mp_up(up),n_matched(0){}
		~CommandSubListener(){}
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& into);
		void onNewDataMessage(Subscriber* sub);
		LatencyTestPublisher* mp_up;
		int n_matched;
	}m_commandsublistener;
	LatencyDataType latency_t;
		TestCommandDataType command_t;
		
	std::vector<uint32_t> data_size_pub;
	std::stringstream output_jtl_name;
	std::stringstream output_jtl;
};


#endif /* LATENCYPUBLISHER_H_ */
