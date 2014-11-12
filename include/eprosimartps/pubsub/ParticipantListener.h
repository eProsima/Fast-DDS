/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantListener.h
 *
 */

#ifndef PARTICIPANTLISTENER_H_
#define PARTICIPANTLISTENER_H_

#include "eprosimartps/pubsub/ParticipantDiscoveryInfo.h"


namespace eprosima {

namespace rtps
{
class Participant;
}

namespace pubsub {

class ParticipantListener {
public:
	ParticipantListener() {};
	virtual ~ParticipantListener() {};
	virtual void onParticipantDiscovery(Participant* part,ParticipantDiscoveryInfo info) {};
};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* PARTICIPANTLISTENER_H_ */
