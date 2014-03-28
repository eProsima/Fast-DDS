/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThreadEvent.cpp
 *
 *  Created on: Mar 21, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */


#include "eprosimartps/threadtype/ThreadEvent.h"

namespace eprosima {
namespace rtps {

ThreadEvent::ThreadEvent():
		b_thread(NULL),
		work(io_service)
{

}

ThreadEvent::~ThreadEvent() {
	pWarning( "Removing thread " << b_thread->get_id() << std::endl);
	b_thread->interrupt();
}

void ThreadEvent::init_thread()
{
	b_thread = new boost::thread(boost::bind(&boost::asio::io_service::run,&io_service));
	io_service.post(boost::bind(&ThreadEvent::announce_thread,this));
}

void ThreadEvent::announce_thread()
{
	pInfo(BLUE<<"Thread: " << b_thread->get_id() << " created and waiting for tasks."<<DEF<<endl);
}

} /* namespace dds */
} /* namespace eprosima */
