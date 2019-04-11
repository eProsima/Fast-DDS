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
 * @file TCPKeepAliveEvent.h
 *
 */

#ifndef __TRANSPORT_TCPKEEPALIVE_HPP__
#define __TRANSPORT_TCPKEEPALIVE_HPP__
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <fastrtps/rtps/resources/TimedEvent.h>

namespace eprosima {
namespace fastrtps{
namespace rtps{

class TCPTransportInterface;

class TCPKeepAliveEvent: public TimedEvent
{
    public:

        /*!
         */
        TCPKeepAliveEvent(
                TCPTransportInterface& transport,
                asio::io_service &service,
                const std::thread& event_thread,
                double interval);

        virtual ~TCPKeepAliveEvent();

        /**
         * Method invoked when the event occurs
         *
         * @param code Code representing the status of the event
         * @param msg Message associated to the event
         */
        void event(
                EventCode code,
                const char* msg= nullptr);

        //!
        TCPTransportInterface& transport_;
};

}
}
} /* namespace eprosima */
#endif
#endif // __TRANSPORT_TCPKEEPALIVE_HPP__
