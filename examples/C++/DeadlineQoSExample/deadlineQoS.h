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

/* DeadlineQoS

	Helper class to implement a way to check if the deadline constrains is being met on each key
	of the topic at hand.

*/


#ifndef _DEADLINEQOS_H_
#define _DEADLINEQOS_H_

#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include <fastrtps/Domain.h>

#include <asio.hpp>
#include <asio/steady_timer.hpp>
#include "mapableKey.h"
#include <iostream>
#include <map>
#include <thread>

using namespace asio;
using namespace eprosima;
using namespace eprosima::fastrtps;

class deadlineQoS
{
	public:
		deadlineQoS(steady_timer &timer, io_service &ioserv): t(timer),io(ioserv){
			init();
		}
		~deadlineQoS(){
			delete dlqos;
		}
		void callback();
		void setFlag(mapable_key target);

		void run();
		void stop();
		//deadlineQoS_struct deadlineQoSlist[32];
		std::map<mapable_key,bool> deadlineQoSmap;
		std::mutex mapmtx;
	private:
		steady_timer &t;
		io_service &io;
		std::thread* dlqos;
		void runner();
		void init();
		void wait();
};



#endif
