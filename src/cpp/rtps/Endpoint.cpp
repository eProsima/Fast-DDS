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

Endpoint::Endpoint(ParticipantImpl* pimpl,
					GuidPrefix_t guid,
					EntityId_t entityId,
					StateKind_t state,
					EndpointKind_t end,
					int16_t userDefinedId):
mp_participant(pimpl),m_guid(guid,entityId),m_stateType(state),m_endpointKind(end),m_userDefinedId(userDefinedId),mp_mutex(new boost::recursive_mutex())
{

}

Endpoint::~Endpoint() {

}

} /* namespace rtps */
} /* namespace eprosima */
