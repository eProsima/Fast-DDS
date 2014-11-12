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

#include "eprosimartps/Endpoint.h"

namespace eprosima {
namespace rtps {

Endpoint::Endpoint(GuidPrefix_t guid,
					EntityId_t entityId,
					TopicAttributes topic,
					TopicDataType* ptype,
					StateKind_t state,
					EndpointKind_t end,
					int16_t userDefinedId):
		m_guid(guid,entityId),m_topic(topic),m_stateType(state),m_endpointKind(end),m_userDefinedId(userDefinedId)
{
	mp_type = ptype;
	mp_send_thr = NULL;
	mp_event_thr = NULL;
}

Endpoint::~Endpoint() {

}

} /* namespace rtps */
} /* namespace eprosima */
