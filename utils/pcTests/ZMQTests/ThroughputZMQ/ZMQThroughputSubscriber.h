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


#include <zmq.hpp>

#include "ZMQThroughputTypes.h"

#include "fastrtps/utils/eClock.h"

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <fstream>
#include <iostream>
using namespace std;

class ZMQThroughputSubscriber
{
public:
	ZMQThroughputSubscriber();
	virtual ~ZMQThroughputSubscriber();
	boost::interprocess::interprocess_semaphore sema;
	zmq::context_t* mp_context;
	zmq::socket_t* mp_commandpub;
	zmq::socket_t* mp_datasub;
	zmq::socket_t* mp_commandsub;
	eClock m_Clock;
	Time_t m_t1,m_t2;
	double m_overhead;
	bool ready;
	uint32_t m_datasize;
	uint32_t m_demand;
	bool init(std::string pubIP,uint32_t basePORT);
	void run();
	ZMQThroughputCommandDataType m_commandDataType;
	ZMQLatencyDataType m_latencyDataType;
	ThroughputCommandType m_commandin;
	ThroughputCommandType m_commandout;

	zmq::message_t command_msg;

	uint32_t lastseqnum,saved_lastseqnum;
	uint32_t lostsamples,saved_lostsamples;
	int commandReceived();

	LatencyType* latencyin;

	void resetResults();
	void saveNumbers();

	class DataSubListener:public SubscriberListener
	{
	public:
		DataSubListener(ThroughputSubscriber& up);
		virtual ~DataSubListener();
		ThroughputSubscriber& m_up;

		uint32_t lastseqnum,saved_lastseqnum;
		uint32_t lostsamples,saved_lostsamples;
		bool first;
		LatencyType* latencyin;
		SampleInfo_t info;
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo info);
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
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo info);
		void onNewDataMessage(Subscriber* sub);
		void saveNumbers();
	}m_CommandSubListener;
	class CommandPubListener:public PublisherListener
	{
	public:
		CommandPubListener(ThroughputSubscriber& up);
		virtual ~CommandPubListener();
		ThroughputSubscriber& m_up;
		void onPublicationMatched(Publisher* pub,MatchingInfo info);
	}m_CommandPubListener;

};



#endif /* THROUGHPUTSUBSCRIBER_H_ */
