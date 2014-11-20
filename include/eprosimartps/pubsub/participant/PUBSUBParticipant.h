/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PUBSUBParticipant.h
 *
 */

#ifndef PUBSUBPARTICIPANT_H_
#define PUBSUBPARTICIPANT_H_

#include "eprosimartps/rtps/attributes/RTPSParticipantAttributes.h"

using namespace eprosima::rtps;

namespace eprosima {
namespace pubsub {

class RTPSParticipant;
class RTPSParticipantListener;

class PUBSUBParticipant {
private:
	PUBSUBParticipant(RTPSParticipantAttributes att);
	virtual ~PUBSUBParticipant();

	Publisher* createPublisher(PublisherAttributes& att, PublisherListener* listen= nullptr);

	RTPSParticipant* mp_participant;
	RTPSParticipantListener mp_participantListener;
};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* PUBSUBPARTICIPANT_H_ */
