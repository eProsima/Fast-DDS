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


#include "eprosimartps/resources/ResourceEvent.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

ResourceEvent::ResourceEvent():
		b_thread(NULL),
		work(io_service)
{

}

ResourceEvent::~ResourceEvent() {
	pWarning( "Removing event thread " << b_thread->get_id() << std::endl);
	io_service.reset();
	io_service.stop();
	b_thread->join();
	delete(b_thread);

}

void ResourceEvent::run_io_service()
{
	io_service.run();
}

void ResourceEvent::init_thread()
{
	b_thread = new boost::thread(&ResourceEvent::run_io_service,this);

	io_service.post(boost::bind(&ResourceEvent::announce_thread,this));
}

void ResourceEvent::announce_thread()
{
	pInfo(BLUE<<"Thread: " << b_thread->get_id() << " created and waiting for tasks."<<DEF<<endl);

}

} /* namespace dds */
} /* namespace eprosima */
