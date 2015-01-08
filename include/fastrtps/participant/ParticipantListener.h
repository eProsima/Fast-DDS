/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantListener.h
 *
 */

#ifndef PARTICIPANTLISTENER_H_
#define PARTICIPANTLISTENER_H_

#include "fastrtps/participant/ParticipantDiscoveryInfo.h"

namespace eprosima {
namespace fastrtps {

/**
 * Class ParticipantListener, used by the user to override certain virtual methods to program behaviour to certain events.
 * @ingroup FASTRTPS_MODULE
 */
class ParticipantListener {
public:
	ParticipantListener(){};
	virtual ~ParticipantListener(){};
	
	/**
	* This method is called when a new Participant is discovered, or a previously discovered participant changes its QOS or its removed.
	* @param p Pointer to the Participant
	* @param info DiscoveryInfo.
	*/
	virtual void onParticipantDiscovery(Participant* p, ParticipantDiscoveryInfo info){};
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* PARTICIPANTLISTENER_H_ */
