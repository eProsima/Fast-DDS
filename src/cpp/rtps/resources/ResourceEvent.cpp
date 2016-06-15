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
