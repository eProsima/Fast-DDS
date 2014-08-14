/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputSubscriber.h
 *
 */

#ifndef THROUGHPUTSUBSCRIBER_H_
#define THROUGHPUTSUBSCRIBER_H_

#include "eprosimartps/rtps_all.h"
#include "ThroughputTypes.h"
#include <fstream>
#include <iostream>

class ThroughputSubscriber
{
public:
	ThroughputSubscriber();
	virtual ~ThroughputSubscriber();
	Participant* mp_par;
	Subscriber* mp_datasub;
	Publisher* mp_commandpub;
	Subscriber* mp_commandsub;
	eClock m_Clock;

	Time_t m_t1,m_t2;
	uint64_t m_overhead;
	boost::interprocess::interprocess_semaphore sema;
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
		LatencyType* latencyin;
		SampleInfo_t info;
		void onSubscriptionMatched(MatchingInfo info);
		void onNewDataMessage();
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
		void onSubscriptionMatched(MatchingInfo info);
		void onNewDataMessage();
		void saveNumbers();
	}m_CommandSubListener;
	class CommandPubListener:public PublisherListener
	{
	public:
		CommandPubListener(ThroughputSubscriber& up);
		virtual ~CommandPubListener();
		ThroughputSubscriber& m_up;
		void onPublicationMatched(MatchingInfo info);
	}m_CommandPubListener;
	bool ready;
	uint32_t m_datasize;
	uint32_t m_demand;
	void run();
};



#endif /* THROUGHPUTSUBSCRIBER_H_ */
