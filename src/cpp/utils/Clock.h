/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Clock.h
 *
 *  Created on: Apr 4, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef CLOCK_H_
#define CLOCK_H_

//#include <ctime>
#include <sys/time.h>

namespace eprosima {
namespace rtps {

class Clock {
public:
	Clock();
	virtual ~Clock();
	time_t time_epoch_seconds;
	tm time_epoch;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* CLOCK_H_ */
