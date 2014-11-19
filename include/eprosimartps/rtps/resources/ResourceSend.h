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

namespace boost
{
class recursive_mutex;
}

namespace eprosima {
namespace rtps {

class ResourceSendImpl;

class ResourceSend {
public:
	ResourceSend();
	virtual ~ResourceSend();
	bool initSend(ParticipantImpl* pimpl,const Locator_t& loc, bool useIP4, bool useIP6);
	bool sendSync(CDRMessage_t* msg, const Locator_t& loc);
	boost::recursive_mutex* getMutex();
private:
	ResourceSendImpl* mp_impl;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RESOURCESEND_H_ */
