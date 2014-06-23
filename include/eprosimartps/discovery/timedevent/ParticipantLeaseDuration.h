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

class ParticipantDiscoveryProtocol;
class ResourceEvent;

/**
 * Class ParticipantLeaseDuration, TimedEvent designed to remove a
 * remote Participant and all its Readers and Writers from the local Participant.
 */
class ParticipantLeaseDuration:public TimedEvent {
public:
	ParticipantLeaseDuration(ParticipantDiscoveryProtocol* p_SPDP,
			const GuidPrefix_t& pguid,
			ResourceEvent* pEvent,
			boost::posix_time::milliseconds interval);
	virtual ~ParticipantLeaseDuration();

	void event(const boost::system::error_code& ec);
	ParticipantDiscoveryProtocol* mp_PDP;
	const GuidPrefix_t& m_remoteParticipantGuid;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANTLEASEDURATION_H_ */
