/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file eClock.h
 *
 */

#ifndef ECLOCK_H_
#define ECLOCK_H_

#if defined(_WIN32)
#include <time.h>
#include <windows.h> //I've ommited this line.
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
 
struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};
 

#else
#include <sys/time.h>
#include <chrono>

#endif

#include "fastrtps/rtps/common/Time_t.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps{


/**
 * Clock used to obtain the time in us since 1900.
 * @ingroup UTILITIES_MODULE
 */
class RTPS_DllAPI eClock {
public:
	eClock();
	virtual ~eClock();
	//!Seconds from 1900 to 1970
	int32_t m_seconds_from_1900_to_1970;
	//!Difference from UTC in seconds
	int32_t m_utc_seconds_diff;
	
	/**
	* Fill a Time_t with the current time
	* @param now Pointer to a Time_t instance to fill with the current time
	* @return true on success
	*/
	bool setTimeNow(Time_t* now);
	
	/**
	* Method to start measuring an interval in us.
	*/
	void intervalStart();
	
	/**
	* Method to finish measuring an interval in us.
	* @return Time of the interval in us
	*/
	uint64_t intervalEnd();
	
	/**
	* Sleep a thread
	* @param milliseconds Time to sleep
	*/
	static void my_sleep(uint32_t milliseconds);
	
#if defined(_WIN32)
	FILETIME ft;
	unsigned long long ftlong;
	FILETIME ft1,ft2;
	LARGE_INTEGER freq;
	LARGE_INTEGER li1,li2;
#else
	timeval m_now;
	timeval m_interval1,m_interval2;
#endif
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* CLOCK_H_ */
