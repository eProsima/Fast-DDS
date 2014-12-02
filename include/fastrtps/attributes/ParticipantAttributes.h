/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantAttributes.h
 *
 */

#ifndef PARTICIPANTATTRIBUTES_H_
#define PARTICIPANTATTRIBUTES_H_

#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima{
namespace fastrtps{

/**
 * Class ParticipantAttributes, used by the user to define the attributes of a Participant.
 * @ingroup ATTRIBUTESMODULE
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
