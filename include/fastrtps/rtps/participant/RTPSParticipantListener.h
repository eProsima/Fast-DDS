/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipantListener.h
 *
 */

#ifndef RTPSPARTICIPANTLISTENER_H_
#define RTPSPARTICIPANTLISTENER_H_

#include "fastrtps/rtps/participant/RTPSParticipantDiscoveryInfo.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{

class RTPSParticipant;

class RTPSParticipantListener
{
public:
	RTPSParticipantListener(){};
	virtual ~RTPSParticipantListener(){};
	virtual void onRTPSParticipantDiscovery(RTPSParticipant* part, RTPSParticipantDiscoveryInfo info){};
};
}
}
}



#endif /* RTPSPARTICIPANTLISTENER_H_ */
