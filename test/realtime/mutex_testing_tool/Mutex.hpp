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
 * @file Mutex.h
 *
 */

#ifndef __TEST_REALTIME_MUTEX_HPP__
#define __TEST_REALTIME_MUTEX_HPP__

#include <bits/pthreadtypes.h>
#include <bits/types/clockid_t.h>

extern "C" int pthread_mutex_lock(
        pthread_mutex_t* mutex);

extern "C" int pthread_mutex_timedlock(
        pthread_mutex_t* mutex,
        const struct timespec* abs_timeout);

extern "C" int pthread_mutex_clocklock(
        pthread_mutex_t* mutex,
        clockid_t clock,
        const struct timespec* abs_timeout);

#endif // __TEST_REALTIME_MUTEX_HPP__
