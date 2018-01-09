// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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





#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <fstream>
#include <iostream>


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
