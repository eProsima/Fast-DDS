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
 * @file TimedMutex.hpp
 */

#ifndef _UTILS_TIMEDMUTEX_HPP_
#define _UTILS_TIMEDMUTEX_HPP_

#include <mutex>
#include <chrono>
#include <thread>

#if defined(_WIN32)
extern int clock_gettime(int, struct timespec* tv);
#endif

namespace eprosima {
namespace fastrtps {

#if !defined(_WIN32)
using TimedMutex = std::timed_mutex;
using RecursiveTimedMutex = std::recursive_timed_mutex;
#else
class TimedMutex
{
public:

    TimedMutex()
    {
        _Mtx_init(&mutex_, _Mtx_timed);
    }

    TimedMutex(const TimedMutex&) = delete;
    TimedMutex& operator=(const TimedMutex&) = delete;

    ~TimedMutex()
    {
        _Mtx_destroy(mutex_);
    }

    void lock()
    {
        _Mtx_lock(mutex_);
    }

    void unlock()
    {
        _Mtx_unlock(mutex_);
    }

    template <class Rep, class Period>
    bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
    {
        return try_lock_until(chrono::steady_clock::now() + rel_time);
    }

    template <class Clock, class Duration>
    bool try_lock_until(
        const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        std::chrono::nanoseconds nsecs = abs_time - std::chrono::steady_clock::now();
        struct timespec max_wait = { 0, 0 };
        clock_gettime(1, &max_wait);
        nsecs = nsecs + std::chrono::nanoseconds(max_wait.tv_nsec);
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(nsecs);
        nsecs -= secs;
        max_wait.tv_sec += secs.count();
        max_wait.tv_nsec = (long)nsecs.count();
        return (_Thrd_success == _Mtx_timedlock(mutex_, (xtime*)&max_wait));
    }

    void* native_handle() noexcept
    {
        return mutex_;
    }

private:

    _Mtx_t mutex_;
};

class RecursiveTimedMutex
{
public:

    RecursiveTimedMutex()
    {
        _Mtx_init(&mutex_, _Mtx_timed | _Mtx_recursive);
    }

    RecursiveTimedMutex(const TimedMutex&) = delete;
    RecursiveTimedMutex& operator=(const TimedMutex&) = delete;

    ~RecursiveTimedMutex()
    {
        _Mtx_destroy(mutex_);
    }

    void lock()
    {
        _Mtx_lock(mutex_);
    }

    void unlock()
    {
        _Mtx_unlock(mutex_);
    }

    template <class Rep, class Period>
    bool try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)
    {
        return try_lock_until(chrono::steady_clock::now() + rel_time);
    }

    template <class Clock, class Duration>
    bool try_lock_until(
        const std::chrono::time_point<Clock, Duration>& abs_time)
    {
        std::chrono::nanoseconds nsecs = abs_time - std::chrono::steady_clock::now();
        if (0 > nsecs.count())
        {
            nsecs = std::chrono::nanoseconds(1000000);
        }
        struct timespec max_wait = { 0, 0 };
        clock_gettime(1, &max_wait);
        nsecs = nsecs + std::chrono::nanoseconds(max_wait.tv_nsec);
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(nsecs);
        nsecs -= secs;
        max_wait.tv_sec += secs.count();
        max_wait.tv_nsec = (long)nsecs.count();
        return (_Thrd_success == _Mtx_timedlock(mutex_, (xtime*)& max_wait));
    }

    void* native_handle() noexcept
    {
        return mutex_;
    }

private:

    _Mtx_t mutex_;
};
#endif //__linux__

} //namespace fastrtps
} //namespace eprosima

#endif // _UTILS_TIMEDMUTEX_HPP_