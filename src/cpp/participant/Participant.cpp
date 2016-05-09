/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Participant.cpp
 *
 */

#include <fastrtps/participant/Participant.h>

#include "ParticipantImpl.h"

namespace eprosima {
namespace fastrtps {

Participant::Participant():
		mp_impl(nullptr)
{

}

Participant::~Participant() {
	// TODO Auto-generated destructor stub
}

const GUID_t& Participant::getGuid() const
{
	return mp_impl->getGuid();
}

const ParticipantAttributes& Participant::getAttributes()
{
	return mp_impl->getAttributes();
}

bool Participant::newRemoteEndpointDiscovered(const GUID_t& partguid, uint16_t endpointId,
	EndpointKind_t kind)
{
	return mp_impl->newRemoteEndpointDiscovered(partguid, endpointId, kind);
}
std::pair<StatefulReader*,StatefulReader*> Participant::getEDPReaders(){
	std::pair<StatefulReader *,StatefulReader*> buffer;

	return mp_impl->getEDPReaders();
}
int Participant::get_no_publishers(char *target_topic){
	return mp_impl->get_no_publishers(target_topic);
}
int Participant::get_no_subscribers(char *target_topic){
	return mp_impl->get_no_subscribers(target_topic);
}

} /* namespace pubsub */
} /* namespace eprosima */
