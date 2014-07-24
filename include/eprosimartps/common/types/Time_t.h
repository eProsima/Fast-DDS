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

//static inline double Time_t2Seconds(const Time_t& t)
//{
//	return (double)t.seconds + (double)(t.fraction/(double)pow(2.0,32));
//}
//
//static inline double Time_t2MicroSec(const Time_t& t)
//{
//	return (Time_t2Seconds(t)*(double)pow(10.0,6));
//}
//
//static inline double Time_t2MilliSec(const Time_t& t)
//{
//	return (Time_t2Seconds(t)*(double)pow(10.0,3));
//}
//
//static inline Time_t MilliSec2Time_t(uint32_t millisec)
//{
//    Time_t time;
//    time.seconds = (int32_t)millisec/1000;
//    if(millisec>1000)
//        time.fraction = (uint32_t)((millisec%1000)*pow(10.0,-3)*pow(2.0,32));
//    else
//        time.fraction = (uint32_t)(millisec*pow(10.0,-3)*pow(2.0,32));
//    return time;
//}

//static inline uint32_t Time_tAbsDiff2Millisec(const Time_t& t1,const Time_t& t2)
//{
//	uint32_t result = 0;
//	result +=(uint32_t)abs((t2.seconds-t1.seconds)*1000);
//	result +=(uint32_t)abs((t2.fraction-t1.fraction)/pow(2.0,32)*1000);
//	return result;
//}


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
