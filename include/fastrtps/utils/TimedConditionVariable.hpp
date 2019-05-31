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
 * @file TimedConditionVariable.h
 */

#ifndef _UTILS_TIMEDCONDITIONVARIABLE_HPP_
#define _UTILS_TIMEDCONDITIONVARIABLE_HPP_

#include <mutex>
#include <chrono>
#include <functional>

#ifdef _WIN32
#include <condition_variable>
#else
#include <pthread.h>
#endif

namespace eprosima {
namespace fastrtps {

#ifdef _WIN32
using TimedConditionVariable = std::condition_variable_any;
#else

class TimedConditionVariable
{
    public:

    TimedConditionVariable()
    {
        pthread_cond_init(&cv_, NULL);
    }

    template<typename Mutex>
    void wait(
            std::unique_lock<Mutex>& lock,
            std::function<bool()> predicate)
    {
        while (!predicate())
        {
            pthread_cond_wait(&cv_, lock.mutex()->native_handle());
        }
    }

    template<typename Mutex>
    bool wait_for(
            std::unique_lock<Mutex>& lock,
            const std::chrono::nanoseconds& max_blocking_time,
            std::function<bool()> predicate)
    {
        bool ret_value = true;
        auto nsecs = max_blocking_time;
        struct timespec max_wait = {0, 0};
        clock_gettime(CLOCK_REALTIME, &max_wait);
        nsecs = nsecs + std::chrono::nanoseconds(max_wait.tv_nsec);
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(nsecs);
        nsecs -= secs;
        max_wait.tv_sec += secs.count();
        max_wait.tv_nsec = nsecs.count();
        while (ret_value && !(ret_value = predicate()))
        {
            ret_value = (pthread_cond_timedwait(&cv_, lock.mutex()->native_handle(), &max_wait) == 0);
        }

        return ret_value;
    }

    template<typename Mutex>
    bool wait_until(
            std::unique_lock<Mutex>& lock,
            const std::chrono::steady_clock::time_point& max_blocking_time,
            std::function<bool()> predicate)
    {
        bool ret_value = true;
        std::chrono::nanoseconds nsecs = max_blocking_time - std::chrono::steady_clock::now();
        struct timespec max_wait = {0, 0};
        clock_gettime(CLOCK_REALTIME, &max_wait);
        nsecs = nsecs + std::chrono::nanoseconds(max_wait.tv_nsec);
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(nsecs);
        nsecs -= secs;
        max_wait.tv_sec += secs.count();
        max_wait.tv_nsec = nsecs.count();
        while (ret_value && !(ret_value = predicate()))
        {
            ret_value = (pthread_cond_timedwait(&cv_, lock.mutex()->native_handle(), &max_wait) == 0);
        }

        return ret_value;
    }

    void notify_one()
    {
        pthread_cond_signal(&cv_);
    }

    void notify_all()
    {
        pthread_cond_broadcast(&cv_);
    }

    private:

    pthread_cond_t cv_;
};
#endif

}
}

#endif // _UTILS_TIMEDCONDITIONVARIABLE_HPP_
