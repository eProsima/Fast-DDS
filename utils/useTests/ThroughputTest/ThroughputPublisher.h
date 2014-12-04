/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputPublisher.h
 *
 */

#ifndef THROUGHPUTPUBLISHER_H_
#define THROUGHPUTPUBLISHER_H_

#include "ThroughputTypes.h"
#include "fastrtps/fastrtps_fwd.h"
#include "fastrtps/utils/eClock.h"

#include "fastrtps/publisher/PublisherListener.h"
#include "fastrtps/subscriber/SubscriberListener.h"

using namespace eprosima::fastrtps;

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <vector>
#include <string>
using namespace std;

class ThroughputPublisher
{
public:
	ThroughputPublisher();
	virtual ~ThroughputPublisher();
	Participant* mp_par;
	Publisher* mp_datapub;
	Publisher* mp_commandpub;
	Subscriber* mp_commandsub;
	eClock m_Clock;
	Time_t m_t1,m_t2;
	double m_overhead;
	boost::interprocess::interprocess_semaphore sema;
	class DataPubListener:public PublisherListener
	{
	public:
		DataPubListener(ThroughputPublisher& up);
		virtual ~DataPubListener();
		ThroughputPublisher& m_up;
		void onPublicationMatched(Publisher* pub,MatchingInfo info);
	}m_DataPubListener;

	class CommandSubListener:public SubscriberListener
	{
	public:
		CommandSubListener(ThroughputPublisher& up);
		virtual ~CommandSubListener();
		ThroughputPublisher& m_up;
		void onSubscriptionMatched(Subscriber* sub,MatchingInfo info);
	}m_CommandSubListener;
	class CommandPubListener:public PublisherListener
	{
	public:
		CommandPubListener(ThroughputPublisher& up);
		virtual ~CommandPubListener();
		ThroughputPublisher& m_up;
		void onPublicationMatched(Publisher* pub,MatchingInfo info);
	}m_CommandPubListener;


	bool ready;

	void run(uint32_t test_time,int demand = 0,int msg_size = 0);
	bool test(uint32_t test_time,uint32_t demand,uint32_t size);
	std::vector<TroughputResults> m_timeStats;
	LatencyDataType latency_t;
		ThroughputCommandDataType throuputcommand_t;

	bool loadDemandsPayload();
	std::map<uint32_t,std::vector<uint32_t>> m_demand_payload;
};



#endif /* THROUGHPUTPUBLISHER_H_ */
