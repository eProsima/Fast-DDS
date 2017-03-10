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

#include <asio.hpp>
#include <thread>
#include <functional>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <fastrtps/log/Log.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {


ResourceEvent::ResourceEvent():
    mp_b_thread(nullptr),
    mp_io_service(nullptr),
    mp_work(nullptr),
    mp_RTPSParticipantImpl(nullptr)
    {
        mp_io_service = new asio::io_service();
        mp_work = (void*)new asio::io_service::work(*mp_io_service);
    }

ResourceEvent::~ResourceEvent() {
    logInfo(RTPS_PARTICIPANT,"Removing event thread");
    mp_io_service->stop();
    mp_b_thread->join();
    delete(mp_b_thread);
    delete((asio::io_service::work*)mp_work);
    delete(mp_io_service);

}

void ResourceEvent::run_io_service()
{
    mp_io_service->run();
}

void ResourceEvent::init_thread(RTPSParticipantImpl* pimpl)
{
    mp_RTPSParticipantImpl = pimpl;
    mp_b_thread = new std::thread(&ResourceEvent::run_io_service,this);
    mp_io_service->post(std::bind(&ResourceEvent::announce_thread,this));
    mp_RTPSParticipantImpl->ResourceSemaphoreWait();
}

void ResourceEvent::announce_thread()
{
    logInfo(RTPS_PARTICIPANT,"Thread: " << std::this_thread::get_id() << " created and waiting for tasks.");
    mp_RTPSParticipantImpl->ResourceSemaphorePost();

}
}
} /* namespace */
} /* namespace eprosima */
