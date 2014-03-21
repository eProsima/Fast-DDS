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

namespace eprosima {
namespace rtps{

PeriodicHeartbeat::PeriodicHeartbeat() {
	// TODO Auto-generated constructor stub

}

PeriodicHeartbeat::~PeriodicHeartbeat() {
	// TODO Auto-generated destructor stub
}

PeriodicHeartbeat::PeriodicHeartbeat(StatefulWriter* SW_ptr,boost::posix_time::milliseconds interval):
		TimedEvent(SW_ptr->eventTh.io_service,interval),
		SW(SW_ptr)
{

}


}
} /* namespace eprosima */
