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
#include <iostream>
#include "eprosimartps/utils/eClock.h"
namespace eprosima {
namespace rtps {
	//FIXME: UTC SECONDS AUTOMATICALLY
eClock::eClock():
		m_seconds_from_1900_to_1970(2208988800),
		m_utc_seconds_diff(2*60*60)
{
#if defined(_WIN32)
	QueryPerformanceFrequency(&freq);
#endif
}

eClock::~eClock() {

}


#if defined(_WIN32)
#include <cstdint>

bool eClock::setTimeNow(Time_t* tnow)
{

    GetSystemTimeAsFileTime(&ft);
    ftlong = ft.dwHighDateTime;
    ftlong <<=32;
    ftlong |= ft.dwLowDateTime;
    ftlong /=10;
    ftlong -= 11644473600000000ULL;
	tnow->seconds = (int32_t)((ftlong/1000000)+(long)m_seconds_from_1900_to_1970+(long)m_utc_seconds_diff);
	//std::cout<<std::fixed<< "fraction bf and aft  " << (tt%1000000) << " ** " << ((tt%1000000)*pow(10.0,-6)*pow(2.0,32)) << " ++ "<<((uint32_t)((tt%1000000)*pow(10.0,-6)*pow(2.0,32)))/pow(2.0,32)*pow(10.0,6)<< std::endl;
	tnow->fraction = (uint32_t)((ftlong%1000000)*pow(10.0,-6)*pow(2.0,32));
	return true;
}


void eClock::my_sleep(uint32_t milliseconds)
{
	#pragma warning(disable: 4430)
	Sleep(milliseconds);
	return;
}


void eClock::intervalStart()
{
	  GetSystemTimeAsFileTime(&ft1);
	  QueryPerformanceCounter(&li1);
}
uint64_t eClock::intervalEnd()
{
	GetSystemTimeAsFileTime(&ft2);
	QueryPerformanceCounter(&li2);
	return 0;

}















#else //UNIX VERSION
#include <unistd.h>

bool eClock::setTimeNow(Time_t* tnow)
{
	gettimeofday(&m_now,NULL);
	tnow->seconds = m_now.tv_sec+m_seconds_from_1900_to_1970+m_utc_seconds_diff;
	tnow->fraction = (uint32_t)(m_now.tv_usec*pow(2.0,32)*pow(10.0,-6));
	return true;
}

void eClock::my_sleep(uint32_t milliseconds)
{
	usleep(milliseconds*1000);
	return;
}

void eClock::intervalStart()
{
	gettimeofday(&m_interval1,NULL);
}

uint64_t eClock::intervalEnd()
{
	gettimeofday(&m_interval2,NULL);
	return (m_interval2.tv_sec-m_interval1.tv_sec)*1000000+m_interval2.tv_usec-m_interval1-tv_usec;
}



#endif



} /* namespace rtps */
} /* namespace eprosima */
