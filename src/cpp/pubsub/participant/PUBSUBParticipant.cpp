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

#include "eprosimartps/rtps/RTPSDomain.h"

namespace eprosima {
namespace pubsub {

PUBSUBParticipant::PUBSUBParticipant(RTPSParticipantAttributes att):
		mp_participant(nullptr)
{

	//mp_participant = RTPSDomain::createRTPSParticipant(att,)

}

PUBSUBParticipant::~PUBSUBParticipant() {
	// TODO Auto-generated destructor stub
}


Publisher* PUBSUBParticipant::createPublisher(PublisherAttributes& att, PublisherListener* listen)
{

}


} /* namespace pubsub */
} /* namespace eprosima */
