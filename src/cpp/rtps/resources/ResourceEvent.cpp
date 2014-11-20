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

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "eprosimartps/rtps/resources/ResourceEvent.h"
#include "eprosimartps/rtps/ParticipantImpl.h"
#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "ResourceEvent";

ResourceEvent::ResourceEvent():
		mp_b_thread(nullptr),
		mp_io_service(nullptr),
		mp_work(nullptr),
		mp_participantImpl(nullptr)
{
	mp_io_service = new boost::asio::io_service();
	mp_work = new boost::asio::io_service::work(*mp_io_service);
}

ResourceEvent::~ResourceEvent() {
	const char* const METHOD_NAME = "~ResourceEvent";
	logWarning(RTPS_PARTICIPANT,"Removing event thread " << mp_b_thread->get_id());
	mp_io_service->reset();
	mp_io_service->stop();
	mp_b_thread->join();
	delete(mp_b_thread);
	delete(mp_work);
	delete(mp_io_service);

}

void ResourceEvent::run_io_service()
{
	mp_io_service->run();
}

void ResourceEvent::init_thread(ParticipantImpl* pimpl)
{
	mp_participantImpl = pimpl;
	mp_b_thread = new boost::thread(&ResourceEvent::run_io_service,this);
	mp_io_service->post(boost::bind(&ResourceEvent::announce_thread,this));
	mp_participantImpl->ResourceSemaphoreWait();
}

void ResourceEvent::announce_thread()
{
	const char* const METHOD_NAME = "announce_thread";
	logInfo(RTPS_PARTICIPANT,"Thread: " << mp_b_thread->get_id() << " created and waiting for tasks.");
	mp_participantImpl->ResourceSemaphorePost();

}

} /* namespace pubsub */
} /* namespace eprosima */
