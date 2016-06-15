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
 * @file ResourceSend.h
 *
 */

#ifndef RESOURCESEND_H_
#define RESOURCESEND_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <cstdint>

namespace boost
{
class recursive_mutex;
}

namespace eprosima {
namespace fastrtps{
namespace rtps {

class ResourceSendImpl;
class RTPSParticipantImpl;
class Locator_t;
struct CDRMessage_t;

/**
*@ingroup MANAGEMENT_MODULE
*/
class ResourceSend {
public:
	ResourceSend();
	virtual ~ResourceSend();
	
	/**
	* Initialize the sending socket. 
	*
	* @param pimpl
	* @param loc Locator of the address from where to start the sending socket.
	* @param sendsockBuffer
	* @param useIP4 Booleand telling whether to use IPv4
	* @param useIP6 Booleand telling whether to use IPv6
	* @return true on success
	*/
	bool initSend(RTPSParticipantImpl* pimpl,const Locator_t& loc,
			uint32_t sendsockBuffer, bool useIP4, bool useIP6);
			
	/**
	* Send a message to a locator syncrhonously
	*
	* @param msg Message to send
	* @param loc Destination locator
	*/
	void sendSync(CDRMessage_t* msg, const Locator_t& loc);
	
	/**
	* Get associated mutex
	* @return Associated mutex
	*/
	boost::recursive_mutex* getMutex();

	void loose_next_change();

private:
	ResourceSendImpl* mp_impl;
};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* RESOURCESEND_H_ */
