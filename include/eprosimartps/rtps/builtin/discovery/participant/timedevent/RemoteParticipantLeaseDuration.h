/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RemoteRTPSParticipantLeaseDuration.h
 *
*/

#ifndef RTPSParticipantLEASEDURATION_H_
#define RTPSParticipantLEASEDURATION_H_
#include "eprosimartps/common/types/Guid.h"
#include "eprosimartps/utils/TimedEvent.h"



namespace eprosima {
namespace rtps {

class PDPSimple;
class ResourceEvent;
class RTPSParticipantProxyData;

/**
 * Class RemoteRTPSParticipantLeaseDuration, TimedEvent designed to remove a
 * remote RTPSParticipant and all its Readers and Writers from the local RTPSParticipant if it fails to
 * announce its liveliness each leaseDuration period.
 * @ingroup DISCOVERYMODULE
 */
class RemoteRTPSParticipantLeaseDuration:public TimedEvent {
public:
	RemoteRTPSParticipantLeaseDuration(PDPSimple* p_SPDP,
			RTPSParticipantProxyData* pdata,
			ResourceEvent* pEvent,
			boost::posix_time::milliseconds interval);
	virtual ~RemoteRTPSParticipantLeaseDuration();
	/**
	 * temporal event that check if the RTPSParticipant is alive, and removes it if not.
	 * @param ec
	 */
	void event(const boost::system::error_code& ec);
	//!Pointer to the PDPSimple object.
	PDPSimple* mp_PDP;
	//!Pointer to the RTPSParticipantProxyData object that contains this temporal event.
	RTPSParticipantProxyData* mp_RTPSParticipantProxyData;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSParticipantLEASEDURATION_H_ */
