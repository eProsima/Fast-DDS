/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Participant.h
 *
 */

#ifndef PARTICIPANT_H_
#define PARTICIPANT_H_

#include "fastrtps/rtps/common/Guid.h"

#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps{


class ParticipantImpl;
class ParticipantAttributes;


class RTPS_DllAPI Participant {
	friend class Domain;
	friend class ParticipantImpl;
private:
	Participant();
	virtual ~Participant();


	ParticipantImpl* mp_impl;

public:

	const GUID_t& getGuid()const ;

	const ParticipantAttributes& getAttributes();

};

}
} /* namespace eprosima */

#endif /* PARTICIPANT_H_ */
