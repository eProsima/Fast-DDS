/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Participant.h
*/


#ifndef PARTICIPANT_H_
#define PARTICIPANT_H_

//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/types.h>
//
//#if defined(_WIN32)
//#include <process.h>
//#else
//#include <unistd.h>
//#endif

#include <cstdlib>
#include "eprosimartps/config/eprosimartps_dll.h"
#include "eprosimartps/rtps/common/Guid.h"

namespace eprosima {
namespace rtps {

class ParticipantImpl;

/**
 * @brief Class Participant, contains the public API for a Participant.
 * @ingroup MANAGEMENTMODULE
 */
class RTPS_DllAPI Participant
{
	friend class ParticipantImpl;
public:
	Participant(ParticipantImpl* pimpl);
	virtual ~ Participant();
	//!Get the GUID_t of the participant.
	const GUID_t& getGuid() const ;
	//!Force the announcement of the participant state.
	void announceParticipantState();
	//!Method to loose the next change (ONLY FOR TEST).
	void loose_next_change();
	//!Stop the participant announcement period.
	void stopParticipantAnnouncement();
	//!Reset the participant announcement period.
	void resetParticipantAnnouncement();

	bool newRemoteWriterDiscovered(const GUID_t& pguid, int16_t userDefinedId);
	bool newRemoteReaderDiscovered(const GUID_t& pguid, int16_t userDefinedId);

	uint32_t getParticipantID() const;
	private:
	ParticipantImpl* mp_impl;
};


} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANT_H_ */





