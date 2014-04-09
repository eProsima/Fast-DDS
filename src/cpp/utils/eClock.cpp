/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Clock.cpp
 *
 *  Created on: Apr 4, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/utils/eClock.h"

namespace eprosima {
namespace rtps {

eClock::eClock():
		m_seconds_from_1900_to_1970(2208988800),
		m_utc_seconds_diff(2*60*60)
{
	gettimeofday(&m_now,NULL);
}

eClock::~eClock() {
	// TODO Auto-generated destructor stub
}

bool eClock::setTimeNow(Time_t* tnow)
{
	//FIXME: UTC SECONDS AUTOMATICALLY
	gettimeofday(&m_now,NULL);
	tnow->seconds = m_now.tv_sec+m_seconds_from_1900_to_1970+m_utc_seconds_diff;
	tnow->fraction = m_now.tv_usec*pow(2.0,32)*pow(10.0,-6);
	return true;
}

} /* namespace rtps */
} /* namespace eprosima */
