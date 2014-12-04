/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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
*
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
private:
	ResourceSendImpl* mp_impl;
};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* RESOURCESEND_H_ */
