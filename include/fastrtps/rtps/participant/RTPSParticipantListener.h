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

/**
* Class with virtual method that the user can overload to respond to certain events.
*/
class RTPS_DllAPI RTPSParticipantListener
{
public:
	RTPSParticipantListener(){};
	virtual ~RTPSParticipantListener(){};
	
	/**
	* This method is invoked when a new participant is discovered
	* @param part Discovered participant
	* @param info Discovery information of the participant
	*/
	virtual void onRTPSParticipantDiscovery(RTPSParticipant* part, RTPSParticipantDiscoveryInfo info){};
};
}
}
}



#endif /* RTPSPARTICIPANTLISTENER_H_ */
