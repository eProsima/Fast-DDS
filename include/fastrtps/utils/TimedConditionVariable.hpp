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
 * @file TimedConditionVariable.hpp
 */

#ifndef _UTILS_TIMEDCONDITIONVARIABLE_HPP_
#define _UTILS_TIMEDCONDITIONVARIABLE_HPP_
#include <fastrtps/config.h>

/*
   NOTE: Windows implementation temporary disabled due to aleatory high CPU consumption when
   calling _Cnd_timedwait function, making some tests to fail and very poor performance.
   Related task: #6274

 #if HAVE_STRICT_REALTIME && defined(_WIN32)
 #include <thr/xthreads.h>

 #define CLOCK_REALTIME 0
 #define CV_INIT_(x) _Cnd_init(x)
 #define CV_WAIT_(cv, x) _Cnd_wait(cv, (_Mtx_t)x)
 #define CV_TIMEDWAIT_(cv, x, y) _Cnd_timedwait(cv, (_Mtx_t)x, (xtime*)y)
 #define CV_SIGNAL_(cv) _Cnd_signal(cv)
 #define CV_BROADCAST_(cv) _Cnd_broadcast(cv)
 #define CV_T_ _Cnd_t

   extern int clock_gettime(int, struct timespec* tv);
 #elif HAVE_STRICT_REALTIME && defined(__linux__)
 */
#if HAVE_STRICT_REALTIME && defined(__linux__)
#include <pthread.h>

#define CV_INIT_(x) pthread_cond_init(x, NULL);
#define CV_WAIT_(cv, x) pthread_cond_wait(&cv, x)
#define CV_TIMEDWAIT_(cv, x, y) pthread_cond_timedwait(&cv, x, y)
#define CV_SIGNAL_(cv) pthread_cond_signal(&cv)
#define CV_BROADCAST_(cv) pthread_cond_broadcast(&cv)
#define CV_T_ pthread_cond_t
#else
#include <condition_variable>
#endif // if HAVE_STRICT_REALTIME && defined(__linux__)

#include <mutex>
#include <chrono>
#include <functional>

namespace eprosima {
namespace fastrtps {

#if HAVE_STRICT_REALTIME && (/*defined(_WIN32) ||*/ defined(__linux__))

class TimedConditionVariable
{
public:

    TimedConditionVariable()
    {
        CV_INIT_(&cv_);
    }

    template<typename Mutex>
    void wait(
            std::unique_lock<Mutex>& lock,
            std::function<bool()> predicate)
    {
        while (!predicate())
        {
            CV_WAIT_(cv_, lock.mutex()->native_handle());
        }
    }

    template<typename Mutex>
    void wait(
            std::unique_lock<Mutex>& lock)
    {
        CV_WAIT_(cv_, lock.mutex()->native_handle());
    }

    template<typename Mutex>
    bool wait_for(
            std::unique_lock<Mutex>& lock,
            const std::chrono::nanoseconds& max_blocking_time,
            std::function<bool()> predicate)
    {
        bool ret_value = true;
        auto nsecs = max_blocking_time;
        struct timespec max_wait = {
            0, 0
        };
        clock_gettime(CLOCK_REALTIME, &max_wait);
        nsecs = nsecs + std::chrono::nanoseconds(max_wait.tv_nsec);
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(nsecs);
        nsecs -= secs;
        max_wait.tv_sec += secs.count();
        max_wait.tv_nsec = (long)nsecs.count();
        while (ret_value && false == (ret_value = predicate()))
        {
            ret_value = (0 == CV_TIMEDWAIT_(cv_, lock.mutex()->native_handle(), &max_wait));
        }

        return ret_value;
    }

    template<typename Mutex>
    bool wait_until(
            std::unique_lock<Mutex>& lock,
            const std::chrono::steady_clock::time_point& max_blocking_time,
            std::function<bool()> predicate)
    {
        auto secs = std::chrono::time_point_cast<std::chrono::seconds>(max_blocking_time);
        auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(max_blocking_time) -
                std::chrono::time_point_cast<std::chrono::nanoseconds>(secs);
        struct timespec max_wait = {
            secs.time_since_epoch().count(), ns.count()
        };
        bool ret_value = true;
        while (ret_value && false == (ret_value = predicate()))
        {
            ret_value = (CV_TIMEDWAIT_(cv_, lock.mutex()->native_handle(), &max_wait) == 0);
        }

        return ret_value;
    }

    template<typename Mutex>
    bool wait_until(
            std::unique_lock<Mutex>& lock,
            const std::chrono::steady_clock::time_point& max_blocking_time)
    {
        auto secs = std::chrono::time_point_cast<std::chrono::seconds>(max_blocking_time);
        auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(max_blocking_time) -
                std::chrono::time_point_cast<std::chrono::nanoseconds>(secs);
        struct timespec max_wait = {
            secs.time_since_epoch().count(), ns.count()
        };
        return (CV_TIMEDWAIT_(cv_, lock.mutex()->native_handle(), &max_wait) == 0);
    }

    void notify_one()
    {
        CV_SIGNAL_(cv_);
    }

    void notify_all()
    {
        CV_BROADCAST_(cv_);
    }

private:

    CV_T_ cv_;
};
#else
using TimedConditionVariable = std::condition_variable_any;
#endif // HAVE_STRICT_REALTIME && (/*defined(_WIN32)*/ || defined(__linux__))

}  // namespace fastrtps
}  // namespace eprosima

#endif // _UTILS_TIMEDCONDITIONVARIABLE_HPP_
