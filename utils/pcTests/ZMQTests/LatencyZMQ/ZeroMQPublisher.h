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
 * @file ZeroMQPublisher.h
 *
 */

#ifndef ZEROMQPUBLISHER_H_
#define ZEROMQPUBLISHER_H_

#include <zmq.hpp>
#include <iostream>
#include "fastrtps/utils/eClock.h"
#include "fastrtps/rtps/common/Time_t.h"
#include <vector>





class TimeStats{
public:
	TimeStats():nbytes(0),tmin(0),tmax(0),mean(0),p50(0),p90(0),p99(0),p9999(0),stdev(0){}
	~TimeStats(){}
	uint64_t nbytes;
	double tmin,tmax,mean,p50,p90,p99,p9999;
	double stdev;
};

class ZeroMQPublisher {
public:
	ZeroMQPublisher();
	virtual ~ZeroMQPublisher();
	bool init(std::vector<std::string>subIP,int n_samples=1000);
	void run();
	void analyzeTimes(uint32_t datasize);
	bool test(uint32_t datasize);
	void printStat(TimeStats& TS);
	std::vector<double> m_times;
	std::vector<TimeStats> m_stats;
	eClock m_clock;
	Time_t m_t1,m_t2;
	double m_overhead;
private:
	zmq::context_t* mp_context;
	zmq::socket_t* mp_datapub;
	zmq::socket_t* mp_commandpub;
	zmq::socket_t* mp_datasub;
	zmq::socket_t* mp_commandsub;
	int n_samples;
	int n_sub;
};

#endif /* ZEROMQPUBLISHER_H_ */
