/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResourceSend.cpp
 *
 */

#include "eprosimartps/rtps/resources/ResourceSend.h"
#include "eprosimartps/rtps/resources/ResourceSendImpl.h"
#include "eprosimartps/rtps/ParticipantImpl.h"

namespace eprosima {
namespace rtps {

ResourceSend::ResourceSend()
{
	// TODO Auto-generated constructor stub
	mp_impl = new ResourceSendImpl();

}

ResourceSend::~ResourceSend() {
	// TODO Auto-generated destructor stub
}

bool ResourceSend::initSend(ParticipantImpl* pimpl, const Locator_t& loc,
		uint32_t sendsockBuffer,bool useIP4, bool useIP6)
{
	return mp_impl->initSend(pimpl,loc,sendsockBuffer,useIP4,useIP6);
}

void ResourceSend::sendSync(CDRMessage_t* msg, const Locator_t& loc)
{
	return mp_impl->sendSync(msg,loc);
}

boost::recursive_mutex* ResourceSend::getMutex()
{
	return mp_impl->getMutex();
}

} /* namespace rtps */
} /* namespace eprosima */
