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
 */


#include "eprosimartps/rtps/resources/ResourceEvent.h"
#include "eprosimartps/rtps/ParticipantImpl.h"
#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "ResourceEvent";

ResourceEvent::ResourceEvent():
		b_thread(nullptr),
		work(io_service),
		mp_participantImpl(nullptr)
{

}

ResourceEvent::~ResourceEvent() {
	const char* const METHOD_NAME = "~ResourceEvent";
	logWarning(RTPS_PARTICIPANT,"Removing event thread " << b_thread->get_id());
	io_service.reset();
	io_service.stop();
	b_thread->join();
	delete(b_thread);

}

void ResourceEvent::run_io_service()
{
	io_service.run();
}

void ResourceEvent::init_thread(ParticipantImpl* pimpl)
{
	mp_participantImpl = pimpl;
	b_thread = new boost::thread(&ResourceEvent::run_io_service,this);
	io_service.post(boost::bind(&ResourceEvent::announce_thread,this));
	mp_participantImpl->ResourceSemaphoreWait();
}

void ResourceEvent::announce_thread()
{
	const char* const METHOD_NAME = "announce_thread";
	logInfo(RTPS_PARTICIPANT,"Thread: " << b_thread->get_id() << " created and waiting for tasks.");
	mp_participantImpl->ResourceSemaphorePost();

}

} /* namespace pubsub */
} /* namespace eprosima */
