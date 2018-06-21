// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RTPSParticipantLeaseDuration.cpp
 *
 */

#include <fastrtps/transport/timedevent/CleanTCPSocketsEvent.h>
#include <fastrtps/transport/TCPTransportInterface.h>
#include <fastrtps/log/Log.h>
#include <mutex>



namespace eprosima {
namespace fastrtps{
namespace rtps {


CleanTCPSocketsEvent::CleanTCPSocketsEvent(TCPTransportInterface* p_transport, asio::io_service& service,
    const std::thread& thread, double interval)
: TimedEvent(service, thread, interval, TimedEvent::NONE)
, mp_transport(p_transport)
{

}

CleanTCPSocketsEvent::~CleanTCPSocketsEvent()
{
    destroy();
}

void CleanTCPSocketsEvent::event(EventCode code, const char* /*msg*/)
{
    if(code == EVENT_SUCCESS)
    {
        mp_transport->CleanDeletedSockets();
    }
}

}
} /* namespace rtps */
} /* namespace eprosima */
