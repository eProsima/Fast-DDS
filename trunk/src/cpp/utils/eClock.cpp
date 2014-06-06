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
#include <cmath>
#if defined(_WIN32)
#include <cstdint>
#else
#include <unistd.h>
#endif
#include <iostream>

#include "eprosimartps/utils/eClock.h"

namespace eprosima {
namespace rtps {

//FIXME: UTC SECONDS AUTOMATICALLY
eClock::eClock():
		m_seconds_from_1900_to_1970(2208988800),
		m_utc_seconds_diff(2*60*60)
{
	//my_gettimeofday(&m_now,NULL);
}

eClock::~eClock() {

}

bool eClock::setTimeNow(Time_t* tnow)
{
	//my_gettimeofday(&m_now,NULL);
#if defined(_WIN32)
    GetSystemTimeAsFileTime(&ft);
    unsigned long long tt = ft.dwHighDateTime;
    tt <<=32;
    tt |= ft.dwLowDateTime;
    tt /=10;
    tt -= 11644473600000000ULL;
	tnow->seconds = (int32_t)((tt/1000000)+(long)m_seconds_from_1900_to_1970+(long)m_utc_seconds_diff);
	//std::cout<<std::fixed<< "fraction bf and aft  " << (tt%1000000) << " ** " << ((tt%1000000)*pow(10.0,-6)*pow(2.0,32)) << " ++ "<<((uint32_t)((tt%1000000)*pow(10.0,-6)*pow(2.0,32)))/pow(2.0,32)*pow(10.0,6)<< std::endl;
	tnow->fraction = (uint32_t)((tt%1000000)*pow(10.0,-6)*pow(2.0,32));
#else
	gettimeofday(m_now,NULL);
	tnow->seconds = m_now.tv_sec+m_seconds_from_1900_to_1970+m_utc_seconds_diff;
	tnow->fraction = (uint32_t)(m_now.tv_usec*pow(2.0,32)*pow(10.0,-6));
#endif
	
	return true;
}


bool eClock::setTimeRealNow(TimeReal_t* tnow)
{
	//my_gettimeofday(&m_now,NULL);
#if defined(_WIN32)
    GetSystemTimeAsFileTime(&ft);
    unsigned long long tt = ft.dwHighDateTime;
    tt <<=32;
    tt |= ft.dwLowDateTime;
    tt /=10;
    tt -= 11644473600000000ULL;
	tnow->seconds = (int32_t)((tt/1000000)+(long)m_seconds_from_1900_to_1970+(long)m_utc_seconds_diff);
	std::cout<<std::fixed<< "fraction bf and aft  " << (tt%1000000) << " ** " << ((tt%1000000)*pow(10.0,-6)*pow(2.0,32)) << " ++ "<<((uint32_t)((tt%1000000)*pow(10.0,-6)*pow(2.0,32)))/pow(2.0,32)*pow(10.0,6)<< std::endl;
	tnow->nanoseconds = (uint32_t)((tt%1000000)*pow(10.0,3));
#else
	gettimeofday(m_now,NULL);
	tnow->seconds = m_now.tv_sec+m_seconds_from_1900_to_1970+m_utc_seconds_diff;
	tnow->nanoseconds = (uint32_t)(m_now.tv_usec*pow(10.0,3));
#endif
	
	return true;
}


//int eClock::my_gettimeofday(struct timeval *tv, struct timezone *tz)
//{
//	#if defined(_WIN32)
//	FILETIME ft;
//
//    GetSystemTimeAsFileTime(&ft);
//    unsigned long long tt = ft.dwHighDateTime;
//    tt <<=32;
//    tt |= ft.dwLowDateTime;
//    tt /=10;
//    tt -= 11644473600000000ULL;
//
//
// // unsigned __int64 tmpres = 0;
// // static int tzflag;
// 
//  //if (NULL != tv)
//  //{
// //   GetSystemTimeAsFileTime(&ft);
// 
// //   tmpres |= ft.dwHighDateTime;
//  //  tmpres <<= 32;
//  //  tmpres |= ft.dwLowDateTime;
// 
//    /*converting file time to unix epoch*/
//  //  tmpres -= DELTA_EPOCH_IN_MICROSECS; 
// //   tmpres /= 10;  /*convert into microseconds*/
// //   tv->tv_sec = (long)(tmpres / 1000000UL);
//  //  tv->tv_usec = (long)(tmpres % 1000000UL);
// // }
// 
// // if (NULL != tz)
////  {
//   
////  }
// 
//  return 0;
//
//#else
//
//	return gettimeofday(tv,tz);
//#endif
//}

void eClock::my_sleep(uint32_t milliseconds)
{
#if defined(_WIN32)
#pragma warning(disable: 4430)
	Sleep(milliseconds);
#else
	usleep(milliseconds*1000);
#endif
	return;
}



} /* namespace rtps */
} /* namespace eprosima */
