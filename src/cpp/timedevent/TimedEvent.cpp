/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TimedEvent.cpp
 *
 *  Created on: Mar 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */


#include "eprosimartps/timedevent/TimedEvent.h"

namespace eprosima{
namespace rtps{


TimedEvent::TimedEvent(boost::asio::io_service* serv,boost::posix_time::milliseconds interval):
		timer(new boost::asio::deadline_timer(*serv,interval))
{

}

}
}
