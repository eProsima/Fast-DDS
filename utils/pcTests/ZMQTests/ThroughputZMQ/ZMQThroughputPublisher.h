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
 * @file ThroughputPublisher.h
 *
 */

#ifndef THROUGHPUTPUBLISHER_H_
#define THROUGHPUTPUBLISHER_H_

#include <zmq.hpp>

#include "ZMQThroughputTypes.h"

#include "fastrtps/utils/eClock.h"





#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <vector>
#include <string>


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
