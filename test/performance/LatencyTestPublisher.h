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
#include <boost/chrono.hpp>

class TimeStats{
public:
	TimeStats() :nbytes(0), received(0), m_min(0), m_max(0), p50(0), p90(0), p99(0), p9999(0), mean(0), stdev(0){}
	~TimeStats(){}
	uint64_t nbytes;
    unsigned int received;
    boost::chrono::duration<double, boost::micro>  m_min, m_max;
	double p50, p90, p99, p9999, mean, stdev;
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
    boost::chrono::steady_clock::time_point t_start_, t_end_;
    boost::chrono::duration<double, boost::micro> t_overhead_;
	int n_subscribers;
	unsigned int n_samples;
	SampleInfo_t m_sampleinfo;
	std::vector<boost::chrono::duration<double, boost::micro>> times_;
	std::vector<TimeStats> m_stats;
	boost::interprocess::interprocess_semaphore m_disc_sema;
	boost::interprocess::interprocess_semaphore m_comm_sema;
	boost::interprocess::interprocess_semaphore m_data_sema;
	int m_status;
	unsigned int n_received;
	bool init(int n_sub,int n_sam, bool reliable, uint32_t pid);
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
		
	std::stringstream output_file_name;
	std::stringstream output_file;
};


#endif /* LATENCYPUBLISHER_H_ */
