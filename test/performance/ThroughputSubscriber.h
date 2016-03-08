/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputSubscriber.h
 *
 */

#ifndef THROUGHPUTSUBSCRIBER_H_
#define THROUGHPUTSUBSCRIBER_H_

#include <boost/asio.hpp>

#include "ThroughputTypes.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/subscriber/SampleInfo.h>


using namespace eprosima::fastrtps;

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/chrono.hpp>

#include <fstream>
#include <iostream>

using namespace std;

class ThroughputSubscriber
{
public:
	ThroughputSubscriber(bool reliable, uint32_t pid, bool hostname);
	virtual ~ThroughputSubscriber();
	Participant* mp_par;
	Subscriber* mp_datasub;
	Publisher* mp_commandpubli;
	Subscriber* mp_commandsub;
    boost::chrono::steady_clock::time_point t_start_, t_end_;
    boost::chrono::duration<double, boost::micro> t_overhead_;
    boost::mutex mutex_;
    int disc_count_;
    boost::condition_variable disc_cond_;
    int stop_count_;
    boost::condition_variable stop_cond_;
	class DataSubListener:public SubscriberListener
	{
	public:
		DataSubListener(ThroughputSubscriber& up);
		virtual ~DataSubListener();
		ThroughputSubscriber& m_up;
		void reset();
		uint32_t lastseqnum,saved_lastseqnum;
		uint32_t lostsamples,saved_lostsamples;
		bool first;
		ThroughputType* throughputin;
		SampleInfo_t info;
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info);
		void onNewDataMessage(Subscriber* sub);
		void saveNumbers();
		std::ofstream myfile;
	}m_DataSubListener;

	class CommandSubListener:public SubscriberListener
	{
	public:
		CommandSubListener(ThroughputSubscriber& up);
		virtual ~CommandSubListener();
		ThroughputSubscriber& m_up;
		ThroughputCommandType m_commandin;
		SampleInfo_t info;
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo& info);
		void onNewDataMessage(Subscriber* sub);
		void saveNumbers();

	private:

		CommandSubListener& operator=(const CommandSubListener&);
	}m_CommandSubListener;
	class CommandPubListener:public PublisherListener
	{
	public:
		CommandPubListener(ThroughputSubscriber& up);
		virtual ~CommandPubListener();
		ThroughputSubscriber& m_up;
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);

	private:

		CommandPubListener& operator=(const CommandPubListener&);
	}m_CommandPubListener;
	bool ready;
	uint32_t m_datasize;
	uint32_t m_demand;
	void run();
	ThroughputDataType throughput_t;
	ThroughputCommandDataType throuputcommand_t;
};



#endif /* THROUGHPUTSUBSCRIBER_H_ */
