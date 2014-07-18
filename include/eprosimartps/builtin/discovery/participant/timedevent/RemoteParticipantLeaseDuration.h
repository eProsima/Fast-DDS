/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantLeaseDuration.h
 *
*/

#ifndef PARTICIPANTLEASEDURATION_H_
#define PARTICIPANTLEASEDURATION_H_
#include "eprosimartps/common/types/Guid.h"
#include "eprosimartps/utils/TimedEvent.h"



namespace eprosima {
namespace rtps {

class PDPSimple;
class ResourceEvent;
class ParticipantProxyData;

/**
 * Class RemoteParticipantLeaseDuration, TimedEvent designed to remove a
 * remote Participant and all its Readers and Writers from the local Participant if it fails to
 * announce its liveliness each leaseDuration period.
 * @ingroup DISCOVERYMODULE
 */
class RemoteParticipantLeaseDuration:public TimedEvent {
public:
	RemoteParticipantLeaseDuration(PDPSimple* p_SPDP,
			ParticipantProxyData* pdata,
			ResourceEvent* pEvent,
			boost::posix_time::milliseconds interval);
	virtual ~RemoteParticipantLeaseDuration();
	/**
	 * temporal event that check if the participant is alive, and removes it if not.
	 * @param ec
	 */
	void event(const boost::system::error_code& ec);
	//!Pointer to the PDPSimple object.
	PDPSimple* mp_PDP;
	//!Pointer to the ParticipantProxyData object that contains this temporal event.
	ParticipantProxyData* mp_participantProxyData;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANTLEASEDURATION_H_ */
