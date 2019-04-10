// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file eClock.cpp
 *
 */
#include <cmath>
#include <iostream>
#include <fastrtps/utils/eClock.h>
using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps {

//FIXME: UTC SECONDS AUTOMATICALLY
eClock::eClock()
    : m_seconds_from_1900_to_1970(0)
    , m_utc_seconds_diff(0)
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

    //	std::cout << "ftlong: " << ftlong << std::endl;
    //std::cout << "sec from 1900 " << m_seconds_from_1900_to_1970<<std::endl;
    tnow->seconds = (int32_t)((long)(ftlong/1000000UL)+(long)m_seconds_from_1900_to_1970+(long)m_utc_seconds_diff);
    //std::cout << "seconds: " << tnow->seconds << " seconds " << std::endl;
    tnow->fraction = (uint32_t)((long)(ftlong%1000000UL)*pow(10.0,-6)*pow(2.0,32));
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
    return (m_interval2.tv_sec-m_interval1.tv_sec)*1000000+m_interval2.tv_usec-m_interval1.tv_usec;
}

#endif

} /* namespace rtps */
} /* namespace eprosima */
