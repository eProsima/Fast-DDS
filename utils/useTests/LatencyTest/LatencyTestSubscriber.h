/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LatencyTestSubscriber.h
 *
 */

#ifndef LATENCYTESTSUBSCRIBER_H_
#define LATENCYTESTSUBSCRIBER_H_

#include "LatencyTestTypes.h"
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
class LatencyTestSubscriber {
public:
	LatencyTestSubscriber();
	virtual ~LatencyTestSubscriber();

	Participant* mp_participant;
	Publisher* mp_datapub;
	Publisher* mp_commandpub;
	Subscriber* mp_datasub;
	Subscriber* mp_commandsub;
	LatencyType* mp_latency;
	SampleInfo_t m_sampleinfo;
	boost::interprocess::interprocess_semaphore m_disc_sema;
	boost::interprocess::interprocess_semaphore m_comm_sema;
	boost::interprocess::interprocess_semaphore m_data_sema;
	int m_status;
	int n_received;
	int n_samples;
	bool init(bool echo,int nsam);
	void run();
	bool test(uint32_t datasize);
	class DataPubListener : public PublisherListener
	{
	public:
		DataPubListener(LatencyTestSubscriber* up):mp_up(up){}
		~DataPubListener(){}
		void onPublicationMatched(Publisher* pub,MatchingInfo info);
		LatencyTestSubscriber* mp_up;
	}m_datapublistener;
	class DataSubListener : public SubscriberListener
	{
	public:
		DataSubListener(LatencyTestSubscriber* up):mp_up(up){}
		~DataSubListener(){}
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo into);
		void onNewDataMessage(Subscriber* sub);
		LatencyTestSubscriber* mp_up;
	}m_datasublistener;
	class CommandPubListener : public PublisherListener
	{
	public:
		CommandPubListener(LatencyTestSubscriber* up):mp_up(up){}
		~CommandPubListener(){}
		void onPublicationMatched(Publisher* pub,MatchingInfo info);
		LatencyTestSubscriber* mp_up;
	}m_commandpublistener;
	class CommandSubListener : public SubscriberListener
	{
	public:
		CommandSubListener(LatencyTestSubscriber* up):mp_up(up){}
		~CommandSubListener(){}
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo into);
		void onNewDataMessage(Subscriber* sub);
		LatencyTestSubscriber* mp_up;
	}m_commandsublistener;
	bool m_echo;
	LatencyDataType latency_t;
	TestCommandDataType command_t;

};

#endif /* LATENCYTESTSUBSCRIBER_H_ */
