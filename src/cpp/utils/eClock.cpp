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
#include <cmath>
namespace eprosima {
namespace rtps {

//FIXME: UTC SECONDS AUTOMATICALLY
eClock::eClock():
		m_seconds_from_1900_to_1970(2208988800),
		m_utc_seconds_diff(2*60*60)
{
	my_gettimeofday(&m_now,NULL);
}

eClock::~eClock() {

}

bool eClock::setTimeNow(Time_t* tnow)
{

	my_gettimeofday(&m_now,NULL);
	tnow->seconds = m_now.tv_sec+m_seconds_from_1900_to_1970+m_utc_seconds_diff;
	tnow->nanoseconds = (uint32_t)(m_now.tv_usec*pow(2.0,32)*pow(10.0,-6));
	return true;
}

int eClock::my_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	#if defined(_WIN32)
	FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;
 
  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
 
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
 
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tmpres /= 10;  /*convert into microseconds*/
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }
 
  if (NULL != tz)
  {
   
  }
 
  return 0;

#else

	return gettimeofday(tv,tz);
#endif
}

} /* namespace rtps */
} /* namespace eprosima */
