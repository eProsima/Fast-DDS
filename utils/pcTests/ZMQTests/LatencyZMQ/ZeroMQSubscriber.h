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
 * @file ZeroMQSubscriber.h
 *
 */

#ifndef ZEROMQSUBSCRIBER_H_
#define ZEROMQSUBSCRIBER_H_

#include <zmq.hpp>
#include <iostream>
#include "fastrtps/utils/eClock.h"
#include "fastrtps/rtps/common/Time_t.h"






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
