// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file Mutex.cpp
 *
 */

#include "Mutex.hpp"
#include "TMutex.hpp"

#include <dlfcn.h>
#include <time.h>

int pthread_mutex_lock(pthread_mutex_t* mutex)
{
    pid_t pid = eprosima::fastrtps::g_tmutex_thread_pid;

    if (0 != pid)
    {
        if (pid == GET_TID())
        {
            eprosima::fastrtps::tmutex_record_mutex_(eprosima::fastrtps::LockType::LOCK, mutex);
        }
    }

    if(eprosima::fastrtps::g_origin_lock_func == nullptr)
    {
        eprosima::fastrtps::g_origin_lock_func = (int(*)(pthread_mutex_t*))dlsym(RTLD_NEXT, "pthread_mutex_lock");
    }

    return (*eprosima::fastrtps::g_origin_lock_func)(mutex);
}

int pthread_mutex_timedlock(pthread_mutex_t* mutex, const struct timespec* abs_timeout)
{
    pid_t pid = eprosima::fastrtps::g_tmutex_thread_pid;

    if (0 != pid)
    {
        if (pid == GET_TID())
        {
            eprosima::fastrtps::tmutex_record_mutex_(eprosima::fastrtps::LockType::TIMED_LOCK, mutex);
        }
    }

    if(eprosima::fastrtps::g_origin_timedlock_func == nullptr)
    {
        eprosima::fastrtps::g_origin_timedlock_func =
            (int(*)(pthread_mutex_t*, const struct timespec*))dlsym(RTLD_NEXT, "pthread_mutex_timedlock");
    }

    return (*eprosima::fastrtps::g_origin_timedlock_func)(mutex, abs_timeout);
}
