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
 * @file Mutex.hpp
 *
 */

#ifndef __TEST_REALTIME_TMUTEX_HPP__
#define __TEST_REALTIME_TMUTEX_HPP__

#include <atomic>

#if defined(_WIN32)
#define GET_PID _getpid
using pid_t = int;
#include <process.h>
#else
#define GET_PID getpid
#include <sys/types.h>
#include <unistd.h>
#endif //_WIN32

#include <bits/pthreadtypes.h>

namespace eprosima {
namespace fastrtps {

//! Stores the thread pid whose mutexes will be recorded.
extern std::atomic<pid_t> g_tmutex_thread_pid;

//! Store the original pthread_mutex_lock function
extern int (*g_origin_lock_func)(pthread_mutex_t*); 

/*!
 * @brief Records all mutexes used by the thread that calls this function.
 */
void tmutex_start_recording();

/*!
 * @brief Stops recording the mutexes.
 */
void tmutex_stop_recording();

/*!
 * @brief If recording process is active then the mutex will be recorded.
 * @param[in] mutex Pointer of the mutex to be recorded.
 */
void tmutex_record_mutex_(pthread_mutex_t* mutex);

/*!
 * @brief Gets the pointer of the selected recorded mutex.
 * @param[in] index Position of the recorded mutex.
 * @return Pointer to the selected mutex.
 */
pthread_mutex_t* tmutex_get_mutex(size_t index);

/*!
 * @brief Locks the selected recorded mutex.
 * @param[in] index Position of the recorded mutex.
 */
void tmutex_lock_mutex(size_t index);

/*!
 * @brief Unlocks the selected recorded mutex.
 * @param[in] index Position of the recorded mutex.
 */
void tmutex_unlock_mutex(size_t index);

/*!
 * @brief Returns the number of mutex recorded.
 * @return Number of mutex recorded.
 */
size_t tmutex_get_num_mutexes();

} //namespace fastrtps
} //namespace eprosima

#endif // __TEST_REALTIME_TMUTEX_HPP__
