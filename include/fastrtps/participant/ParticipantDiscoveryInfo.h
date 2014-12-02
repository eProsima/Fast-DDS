/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantDiscoveryInfo.h
 *
 */

#ifndef PARTICIPANTDISCOVERYINFO_H_
#define PARTICIPANTDISCOVERYINFO_H_

#include "fastrtps/rtps/participant/RTPSParticipantDiscoveryInfo.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

/**
 * 
 */
class ParticipantDiscoveryInfo {
public:
	ParticipantDiscoveryInfo(){};
	virtual ~ParticipantDiscoveryInfo(){};
	//!
	RTPSParticipantDiscoveryInfo rtps;
};

} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* PARTICIPANTDISCOVERYINFO_H_ */
