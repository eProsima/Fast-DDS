/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantAttributes.h
 *
 */

#ifndef PARTICIPANTATTRIBUTES_H_
#define PARTICIPANTATTRIBUTES_H_

#include "../rtps/attributes/RTPSParticipantAttributes.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima{
namespace fastrtps{

/**
 * Class ParticipantAttributes, used by the user to define the attributes of a Participant.
 * The Participants in the Publisher-Subscriber layer are only a container to a RTPSParticipant, so their attributes are the same.
 * Still to maintain the equivalence this class is used to define them.
 * @ingroup FASTRTPS_ATTRIBUTES_MODULE
 */
class ParticipantAttributes
{
public:
	ParticipantAttributes(){};
	virtual ~ParticipantAttributes(){};
	//!Attributes of the associated RTPSParticipant.
	RTPSParticipantAttributes rtps;
};

}
}



#endif /* PARTICIPANTATTRIBUTES_H_ */
