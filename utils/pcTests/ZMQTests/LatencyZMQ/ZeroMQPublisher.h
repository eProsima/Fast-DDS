/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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
using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
using namespace std;

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
	void analizeTimes(uint32_t datasize);
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
