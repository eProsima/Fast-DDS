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

#ifndef ECLOCK_H_
#define ECLOCK_H_

//#include <ctime>
#include <sys/time.h>
#include <chrono>
#include "eprosimartps/rtps_all.h"
namespace eprosima {
namespace rtps {

/**
 * Clock used to obtain the time in us since 1900.
 * @ingroup UTILITIESMODULE
 */
class eClock {
public:
	eClock();
	virtual ~eClock();
	int32_t m_seconds_from_1900_to_1970;
	int32_t m_utc_seconds_diff;
	timeval m_now;
	bool setTimeNow(Time_t* now);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* CLOCK_H_ */
