/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * @file Endpoint.cpp
 *
 */

#include "eprosimartps/rtps/Endpoint.h"

#include <boost/thread/recursive_mutex.hpp>

namespace eprosima {
namespace rtps {

Endpoint::Endpoint(RTPSParticipantImpl* pimpl,GUID_t guid,EndpointAttributes att):
		mp_RTPSParticipant(pimpl),
		m_guid(guid),
		m_att(att),
		mp_mutex(new boost::recursive_mutex())
{

}

Endpoint::~Endpoint() {

}

} /* namespace rtps */
} /* namespace eprosima */
