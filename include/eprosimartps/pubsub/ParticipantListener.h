/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipantListener.h
 *
 */

#ifndef RTPSParticipantLISTENER_H_
#define RTPSParticipantLISTENER_H_

#include "eprosimartps/pubsub/RTPSParticipantDiscoveryInfo.h"


namespace eprosima {

namespace rtps
{
class RTPSParticipant;
}

namespace pubsub {

class RTPSParticipantListener {
public:
	RTPSParticipantListener() {};
	virtual ~RTPSParticipantListener() {};
	virtual void onRTPSParticipantDiscovery(RTPSParticipant* part,RTPSParticipantDiscoveryInfo info) {};
};

} /* namespace pubsub */
} /* namespace eprosima */

#endif /* RTPSParticipantLISTENER_H_ */
