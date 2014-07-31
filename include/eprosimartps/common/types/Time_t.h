/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Time_t.h 	
 */

#ifndef TIME_T_H_
#define TIME_T_H_

#include <cmath>
#include <cstdint>
#include <iostream>
namespace eprosima{
namespace rtps{
//!Structure Time_t, used to describe times.
struct Time_t{
	int32_t seconds;
	uint32_t fraction;
	//int64_t to64time(){
	//	return (int64_t)seconds+((int64_t)(fraction/pow(2.0,32)));
	//}
	Time_t()
	{
		seconds = 0;
		fraction = 0;
	}
	Time_t(int32_t sec,uint32_t frac)
	{
		seconds = sec;
		fraction = frac;
	}
	
};



static inline bool operator==(const Time_t& t1,const Time_t& t2)
{
	if(t1.seconds!=t2.seconds)
		return false;
	if(t1.fraction!=t2.fraction)
		return false;
	return true;
}

static inline bool operator!=(const Time_t& t1,const Time_t& t2)
{
	if(t1.seconds!=t2.seconds)
		return true;
	if(t1.fraction!=t2.fraction)
		return true;
	return false;
}

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

#define TIME_ZERO(t){t.seconds=0;t.fraction=0;}
#define TIME_INVALID(t){t.seconds=-1;t.fraction=0xffffffff;}
#define TIME_INFINITE(t){t.seconds=0x7fffffff;t.fraction=0xffffffff;}

const Time_t c_TimeInfinite(0x7fffffff,0xffffffff);

typedef Time_t Duration_t;



}
}

#endif /* TIME_T_H_ */
