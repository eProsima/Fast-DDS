/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PeriodicHeartbeat.cpp
 *
 *  Created on: Mar 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/timedevent/PeriodicHeartbeat.h"
#include "eprosimartps/writer/StatefulWriter.h"


namespace eprosima {
namespace rtps{

PeriodicHeartbeat::PeriodicHeartbeat() {
	// TODO Auto-generated constructor stub

}

PeriodicHeartbeat::~PeriodicHeartbeat() {
	// TODO Auto-generated destructor stub
}

PeriodicHeartbeat::PeriodicHeartbeat(StatefulWriter* SW_ptr,boost::posix_time::milliseconds interval):
		TimedEvent(&SW_ptr->eventTh.io_service,interval),
		SW(SW_ptr)
{

}

void PeriodicHeartbeat::event(const boost::system::error_code& ec)
{
	if(!ec)
	{
		std::vector<ReaderProxy*>::iterator rit;
		for(rit=SW->matched_readers.begin();rit!=SW->matched_readers.end();rit++)
		{
			std::vector<ChangeForReader_t*> unack;
			(*rit)->unacked_changes(&unack);
			if(!unack.empty())
			{
				//FIXME: send HB
			}
		}
		//Reset TIMER, the cancellation is managed in the receiving thread,
		//when an acknack msg acknowledges all cachechanges.
		timer->async_wait(boost::bind(&PeriodicHeartbeat::event,&SW->periodicHB,
				boost::asio::placeholders::error));
	}
	if(ec==boost::asio::error::operation_aborted)
	{
		pInfo("Periodic Heartbeat aborted");
	}
	else
	{
		pError(ec.message()<<endl);
	}
}


}
} /* namespace eprosima */
