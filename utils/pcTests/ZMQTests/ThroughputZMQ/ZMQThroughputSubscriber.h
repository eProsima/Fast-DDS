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
#include <boost/thread.hpp>
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
	zmq::context_t* mp_dataContext;
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

	//zmq::message_t command_msg;
	//zmq::message_t latencymsg;
	uint32_t lastseqnum,saved_lastseqnum;
	uint32_t lostsamples,saved_lostsamples;
	int commandReceived();

	LatencyType* latencyin;
	boost::thread* mp_latencyThread;
	string publisherIP;
	uint32_t basePORT;
	void resetResults();
	void saveNumbers();

	void processMessages();


};



#endif /* THROUGHPUTSUBSCRIBER_H_ */
