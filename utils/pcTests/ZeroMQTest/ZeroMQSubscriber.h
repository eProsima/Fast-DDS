/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
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
#include "eprosimartps/rtps_all.h"
using namespace std;

class ZeroMQSubscriber {
public:
	ZeroMQSubscriber();
	virtual ~ZeroMQSubscriber();
	bool init(string pubip, int n_samples=1000);
	void run();
	bool test(uint32_t datasize);
private:
	zmq::context_t* mp_context;
	zmq::socket_t* mp_datapub;
	zmq::socket_t* mp_commandpub;
	zmq::socket_t* mp_datasub;
	zmq::socket_t* mp_commandsub;
	int n_samples;
};

#endif /* ZEROMQSUBSCRIBER_H_ */
