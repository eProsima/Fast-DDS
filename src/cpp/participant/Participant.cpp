/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Participant.cpp
 *
 */

#include "fastrtps/participant/Participant.h"

#include "fastrtps/participant/ParticipantImpl.h"

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


} /* namespace pubsub */
} /* namespace eprosima */
