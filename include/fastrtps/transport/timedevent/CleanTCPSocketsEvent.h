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
 * @file CleanTCPSocketsEvent.h
 *
*/

#ifndef CLEAN_TCP_SOCKETS_EVENT_H_
#define CLEAN_TCP_SOCKETS_EVENT_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <asio.hpp>
#include "fastrtps/rtps/resources/TimedEvent.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

class TCPTransportInterface;

/**
 * Class CleanTCPSocketsEvent, TimedEvent designed to clean the deleted sockets of the TCPTransportInterface.
 */
class CleanTCPSocketsEvent :public TimedEvent
{
public:
	/**
	 * Constructor
	 * @param p_transport Pointer to the TCPTransportInterface object.
	 * @param interval Interval in ms.
	 */
    CleanTCPSocketsEvent(TCPTransportInterface* p_transport, asio::io_service& service, const std::thread& thread,
        double interval);
	virtual ~CleanTCPSocketsEvent();

 	/**
	*  Temporal event that calls to the transport to clean the deleted sockets pool.
	* @param code Code representing the status of the event
	* @param msg Message associated to the event
	*/
	void event(EventCode code, const char* msg = nullptr);

    //!Pointer to the TCPTransportInterface object.
    TCPTransportInterface* mp_transport;
};

}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* CLEAN_TCP_SOCKETS_EVENT_H_ */
