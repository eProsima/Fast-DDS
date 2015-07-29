/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ZeroMQSubscriber.h
 *
 */

#ifndef ZEROMQSUBSCRIBER_H_
#define ZEROMQSUBSCRIBER_H_

#include <zmq.hpp>
#include <iostream>
#include "fastrtps/utils/eClock.h"
#include "fastrtps/rtps/common/Time_t.h"

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
using namespace std;

class ZeroMQSubscriber {
public:
	ZeroMQSubscriber();
	virtual ~ZeroMQSubscriber();
	bool init(int n_sub,string pubip, int n_samples=1000);
	void run();
	bool test(uint32_t datasize);
private:
	zmq::context_t* mp_context;
	zmq::socket_t* mp_datapub;
	zmq::socket_t* mp_commandpub;
	zmq::socket_t* mp_datasub;
	zmq::socket_t* mp_commandsub;
	int n_samples;
	int n_sub;
};

#endif /* ZEROMQSUBSCRIBER_H_ */
