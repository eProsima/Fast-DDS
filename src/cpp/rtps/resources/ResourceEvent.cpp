/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ThreadEvent.cpp
 *
 */

#include <fastrtps/rtps/resources/ResourceEvent.h>

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4005)
#endif  // _MSC_VER
#include <boost/asio.hpp>
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
#endif  // _MSC_VER
#include <boost/thread.hpp>
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER
#include <boost/bind.hpp>
#include "../participant/RTPSParticipantImpl.h"
#include <fastrtps/utils/RTPSLog.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "ResourceEvent";

ResourceEvent::ResourceEvent():
		mp_b_thread(nullptr),
		mp_io_service(nullptr),
		mp_work(nullptr),
		mp_RTPSParticipantImpl(nullptr)
{
	mp_io_service = new boost::asio::io_service();
	mp_work = (void*)new boost::asio::io_service::work(*mp_io_service);
}

ResourceEvent::~ResourceEvent() {
	const char* const METHOD_NAME = "~ResourceEvent";
	logInfo(RTPS_PARTICIPANT,"Removing event thread " << mp_b_thread->get_id());
	mp_io_service->stop();
	mp_b_thread->join();
	delete(mp_b_thread);
	delete((boost::asio::io_service::work*)mp_work);
	delete(mp_io_service);

}

void ResourceEvent::run_io_service()
{
	mp_io_service->run();
}

void ResourceEvent::init_thread(RTPSParticipantImpl* pimpl)
{
	mp_RTPSParticipantImpl = pimpl;
	mp_b_thread = new boost::thread(&ResourceEvent::run_io_service,this);
	mp_io_service->post(boost::bind(&ResourceEvent::announce_thread,this));
	mp_RTPSParticipantImpl->ResourceSemaphoreWait();
}

void ResourceEvent::announce_thread()
{
	const char* const METHOD_NAME = "announce_thread";
	logInfo(RTPS_PARTICIPANT,"Thread: " << boost::this_thread::get_id() << " created and waiting for tasks.");
	mp_RTPSParticipantImpl->ResourceSemaphorePost();

}
}
} /* namespace */
} /* namespace eprosima */
