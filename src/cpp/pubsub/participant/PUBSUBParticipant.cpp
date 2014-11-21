/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PUBSUBParticipant.cpp
 *
 */

#include "eprosimartps/pubsub/participant/PUBSUBParticipant.h"

#include "eprosimartps/pubsub/participant/PUBSUBParticipantImpl.h"

namespace eprosima {
namespace pubsub {

PUBSUBParticipant::PUBSUBParticipant():
		mp_impl(nullptr)
{

}

PUBSUBParticipant::~PUBSUBParticipant() {
	// TODO Auto-generated destructor stub
}

const GUID_t& PUBSUBParticipant::getGuid() const
{
	return mp_impl->getGuid();
}



} /* namespace pubsub */
} /* namespace eprosima */
