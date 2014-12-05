/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RemoteParticipantLeaseDuration.h
 *
*/

#ifndef RTPSPARTICIPANTLEASEDURATION_H_
#define RTPSPARTICIPANTLEASEDURATION_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "fastrtps/rtps/resources/TimedEvent.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

class PDPSimple;
class ParticipantProxyData;

/**
 * Class RemoteRTPSParticipantLeaseDuration, TimedEvent designed to remove a
 * remote RTPSParticipant and all its Readers and Writers from the local RTPSParticipant if it fails to
 * announce its liveliness each leaseDuration period.
 *@ingroup DISCOVERY_MODULE
 */
class RemoteParticipantLeaseDuration:public TimedEvent
{
public:
	/**
	 * 
	 * @param p_SPDP
	 * @param pdata
	 * @param interval
	 */
	RemoteParticipantLeaseDuration(PDPSimple* p_SPDP,
			ParticipantProxyData* pdata,
			double interval);
	virtual ~RemoteParticipantLeaseDuration();

 	/**
	*  Temporal event that check if the RTPSParticipant is alive, and removes it if not.
	*
	* @param code Code representing the status of the event
	* @param msg Message associated to the event
	*/
	void event(EventCode code, const char* msg= nullptr);
	//!Pointer to the PDPSimple object.
	PDPSimple* mp_PDP;
	//!Pointer to the RTPSParticipantProxyData object that contains this temporal event.
	ParticipantProxyData* mp_participantProxyData;

};

}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* RTPSPARTICIPANTLEASEDURATION_H_ */
