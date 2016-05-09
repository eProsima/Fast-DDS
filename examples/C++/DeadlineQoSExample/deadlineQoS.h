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

#include "boost/thread.hpp"
#include "boost/bind.hpp"
#include "boost/asio.hpp"
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
