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
 * @file PeriodicHeartbeat.cpp
 *
 */

#include "TCPKeepAliveEvent.hpp"
#include <fastrtps/transport/TCPTransportInterface.h>

namespace eprosima {
namespace fastrtps{
namespace rtps{


TCPKeepAliveEvent::~TCPKeepAliveEvent()
{
    destroy();
}

TCPKeepAliveEvent::TCPKeepAliveEvent(
        TCPTransportInterface& transport,
        asio::io_service &service,
        const std::thread& event_thread,
        double interval)
    : TimedEvent(service, event_thread, interval)
    , transport_(transport)
{

}

void TCPKeepAliveEvent::event(
        EventCode code,
        const char* msg)
{

    // Unused in release mode.
    (void)msg;

    if(code == EVENT_SUCCESS)
    {
        transport_.keep_alive();
        this->restart_timer();
    }
}

}
}
} /* namespace eprosima */
