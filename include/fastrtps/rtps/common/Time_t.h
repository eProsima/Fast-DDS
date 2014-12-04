/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Time_t.h 	
 */

#ifndef TIME_T_H_
#define TIME_T_H_
#include "fastrtps/config/fastrtps_dll.h"
#include <cmath>
#include <cstdint>
#include <iostream>

namespace eprosima{
namespace fastrtps{
namespace rtps{
//!Structure Time_t, used to describe times.
struct RTPS_DllAPI Time_t{
	//!Seconds
	int32_t seconds;
	//!Fraction of second
	uint32_t fraction;
	//int64_t to64time(){
	//	return (int64_t)seconds+((int64_t)(fraction/pow(2.0,32)));
	//}
	//! Default constructor. Sets values to zero.
	Time_t()
	{
		seconds = 0;
		fraction = 0;
	}
	/**
	* @param sec Seconds
	* @param frac Fraction of second
	*/
	Time_t(int32_t sec,uint32_t frac)
	{
		seconds = sec;
		fraction = frac;
	}
	
};


/**
* Comparison assignment
* @param t1 First Time_t to compare
* @param t2 Second Time_t to compare
* @return True if equal
*/
static inline bool operator==(const Time_t& t1,const Time_t& t2)
{
	if(t1.seconds!=t2.seconds)
		return false;
	if(t1.fraction!=t2.fraction)
		return false;
	return true;
}

/**
* Comparison assignment
* @param t1 First Time_t to compare
* @param t2 Second Time_t to compare
* @return True if not equal
*/
static inline bool operator!=(const Time_t& t1,const Time_t& t2)
{
	if(t1.seconds!=t2.seconds)
		return true;
	if(t1.fraction!=t2.fraction)
		return true;
	return false;
}

/**
 * Checks if a Time_t is less than other.
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if the first Time_t is less than the second
 */
static inline bool operator<(const Time_t& t1,const Time_t& t2)
{
	if(t1.seconds < t2.seconds)
		return true;
	else if(t1.seconds > t2.seconds)
		return false;
	else
	{
		if(t1.fraction < t2.fraction)
			return true;
		else
			return false;
	}
}

/**
 * Checks if a Time_t is less or equal than other.
 * @param t1 First Time_t to compare
 * @param t2 Second Time_t to compare
 * @return True if the first Time_t is less or equal than the second
 */
static inline bool operator<=(const Time_t& t1,const Time_t& t2)
{
	if(t1.seconds < t2.seconds)
		return true;
	else if(t1.seconds > t2.seconds)
		return false;
	else
	{
		if(t1.fraction <= t2.fraction)
			return true;
		else
			return false;
	}
}

inline std::ostream& operator<<(std::ostream& output,const Time_t& t)
{
	return output << t.seconds<<"."<<t.fraction;
}

const Time_t c_TimeInfinite(0x7fffffff,0xffffffff);
const Time_t c_TimeZero(0,0);
const Time_t c_TimeInvalid(-1,0xffffffff);

typedef Time_t Duration_t;


}
}
}

#endif /* TIME_T_H_ */
