/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Endpoint.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "eprosimartps/Endpoint.h"

namespace eprosima {
namespace rtps {

Endpoint::Endpoint(GuidPrefix_t guid,
					EntityId_t entityId,
					TopicAttributes topic,
					StateKind_t state,
					EndpointKind_t end,
					int16_t userDefinedId):
		m_guid(guid,entityId),m_topic(topic),m_stateType(state),m_endpointKind(end),m_userDefinedId(userDefinedId)
{
	mp_type = NULL;
	mp_send_thr = NULL;
	mp_event_thr = NULL;
}

Endpoint::~Endpoint() {

}

} /* namespace rtps */
} /* namespace eprosima */
