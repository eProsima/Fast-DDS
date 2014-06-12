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
 *  Created on: Jun 12, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef PARTICIPANTLEASEDURATION_H_
#define PARTICIPANTLEASEDURATION_H_

#include "eprosimartps/utils/TimedEvent.h"

namespace eprosima {
namespace rtps {

class ParticipantLeaseDuration:public TimedEvent {
public:
	ParticipantLeaseDuration();
	virtual ~ParticipantLeaseDuration();
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANTLEASEDURATION_H_ */
