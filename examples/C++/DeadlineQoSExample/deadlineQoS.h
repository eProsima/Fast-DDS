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

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
#endif  // _MSC_VER
#include "boost/thread.hpp"
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER
#include "boost/bind.hpp"
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4005)
#endif  // _MSC_VER
#include "boost/asio.hpp"
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER
#include "boost/date_time/posix_time/posix_time.hpp"
#include "mapableKey.h"
#include <iostream>
#include <map>

using namespace boost:: asio;
using namespace eprosima;
using namespace eprosima::fastrtps;

//typedef struct deadlineQoS_A{
//	boost::mutex mtx;
//	bool flag;
//} deadlineQoS_struct;

class deadlineQoS
{
	public:
		deadlineQoS(deadline_timer &timer, io_service &ioserv): t(timer),io(ioserv){
			init();
		}
		void callback();
		void setFlag(mapable_key target);
		
		void run();
		void stop();
		//deadlineQoS_struct deadlineQoSlist[32];
		std::map<mapable_key,bool> deadlineQoSmap;
		boost::mutex mapmtx;
	private:
		deadline_timer &t;
		io_service &io;
		void runner();
		void init();
		void wait();
};



#endif
