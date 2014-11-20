/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResourceSend.h
 *
 */

#ifndef RESOURCESEND_H_
#define RESOURCESEND_H_

#include <cstdint>

namespace boost
{
class recursive_mutex;
}

namespace eprosima {
namespace rtps {

class ResourceSendImpl;
class RTPSParticipantImpl;
class Locator_t;
class CDRMessage_t;

class ResourceSend {
public:
	ResourceSend();
	virtual ~ResourceSend();
	bool initSend(RTPSParticipantImpl* pimpl,const Locator_t& loc,
			uint32_t sendsockBuffer, bool useIP4, bool useIP6);
	void sendSync(CDRMessage_t* msg, const Locator_t& loc);
	boost::recursive_mutex* getMutex();
private:
	ResourceSendImpl* mp_impl;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RESOURCESEND_H_ */
