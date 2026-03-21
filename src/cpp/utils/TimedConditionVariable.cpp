// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TimedConditionVariable.cpp
 */

#include <fastdds/utils/TimedConditionVariable.hpp>

#if defined(_WIN32)
#include <windows.h>
/*
 #define MS_PER_SEC      1000ULL     // MS = milliseconds
 #define US_PER_MS       1000ULL     // US = microseconds
 #define HNS_PER_US      10ULL       // HNS = hundred-nanoseconds (e.g., 1 hns = 100 ns)
 #define NS_PER_US       1000ULL

 #define HNS_PER_SEC     (MS_PER_SEC * US_PER_MS * HNS_PER_US)
 #define NS_PER_HNS      (100ULL)    // NS = nanoseconds
 #define NS_PER_SEC      (MS_PER_SEC * US_PER_MS * NS_PER_US)

   int clock_gettime(int, struct timespec* tv)
   {
    FILETIME ft;
    ULARGE_INTEGER hnsTime;

    GetSystemTimeAsFileTime(&ft);

    hnsTime.LowPart = ft.dwLowDateTime;
    hnsTime.HighPart = ft.dwHighDateTime;

    // To get POSIX Epoch as baseline, subtract the number of hns intervals from Jan 1, 1601 to Jan 1, 1970.
    hnsTime.QuadPart -= (11644473600ULL * HNS_PER_SEC);

    // modulus by hns intervals per second first, then convert to ns, as not to lose resolution
    tv->tv_nsec = (long)((hnsTime.QuadPart % HNS_PER_SEC) * NS_PER_HNS);
    tv->tv_sec = (long)(hnsTime.QuadPart / HNS_PER_SEC);

    return 0;
   }
 */
#ifdef MINGW_COMPILER
    #define exp7           10000000LL     //1E+7     //C-file part
    #define exp9         1000000000LL     //1E+9
    #define w2ux 116444736000000000LL     //1.jan1601 to 1.jan1970
#else
    #define exp7           10000000i64     //1E+7     //C-file part
    #define exp9         1000000000i64     //1E+9
    #define w2ux 116444736000000000i64     //1.jan1601 to 1.jan1970
#endif // ifdef MINGW_COMPILER
void unix_time(
        struct timespec* spec)
{
    __int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
    wintime -= w2ux;  spec->tv_sec = wintime / exp7;
    spec->tv_nsec = wintime % exp7 * 100;
}

int clock_gettime(
        int,
        timespec* spec)
{
    static struct timespec startspec; static double ticks2nano;
    static __int64 startticks, tps = 0;    __int64 tmp, curticks;
    QueryPerformanceFrequency((LARGE_INTEGER*)&tmp);  //some strange system can
    if (tps != tmp)
    {
        tps = tmp; //init ~~ONCE         //possibly change freq ?
        QueryPerformanceCounter((LARGE_INTEGER*)&startticks);
        unix_time(&startspec); ticks2nano = (double)exp9 / tps;
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&curticks); curticks -= startticks;
    spec->tv_sec = startspec.tv_sec + (curticks / tps);
    spec->tv_nsec = (long)(startspec.tv_nsec + (double)(curticks % tps) * ticks2nano);
    if (!(spec->tv_nsec < exp9))
    {
        spec->tv_sec++; spec->tv_nsec -= exp9;
    }
    return 0;
}

#endif // if defined(_WIN32)