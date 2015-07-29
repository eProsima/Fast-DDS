/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThroughputPublisher.h
 *
 */

#ifndef THROUGHPUTPUBLISHER_H_
#define THROUGHPUTPUBLISHER_H_

#include <zmq.hpp>

#include "ZMQThroughputTypes.h"

#include "fastrtps/utils/eClock.h"

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <vector>
#include <string>
using namespace std;

class ZMQThroughputPublisher
{
public:
	ZMQThroughputPublisher();
	virtual ~ZMQThroughputPublisher();
	boost::interprocess::interprocess_semaphore sema;
	zmq::context_t* mp_context;
	zmq::socket_t* mp_datapub;
	zmq::socket_t* mp_commandpub;
	zmq::socket_t* mp_commandsub;
	eClock m_Clock;
	Time_t m_t1,m_t2;
	double m_overhead;

	bool ready;
	bool init(std::string subIP,uint32_t basePORT);
	void run(uint32_t test_time,int demand = 0,int msg_size = 0);
	bool test(uint32_t test_time,uint32_t demand,uint32_t size);
	std::vector<TroughputResults> m_timeStats;

	bool loadDemandsPayload();
	std::map<uint32_t,std::vector<uint32_t>> m_demand_payload;

	ZMQThroughputCommandDataType m_commandDataType;
	ZMQLatencyDataType m_latencyDataType;

};



#endif /* THROUGHPUTPUBLISHER_H_ */
