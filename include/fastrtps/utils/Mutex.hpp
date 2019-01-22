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

#ifndef __UTILS_MUTEX_HPP__
#define __UTILS_MUTEX_HPP__

#include <mutex>

#if defined(EPROSIMA_MUTEX_TEST)
#include <atomic>
#include <type_traits>

#if defined(_WIN32)
#define GET_PID _getpid
using pid_t = int;
#include <process.h>
#else
#define GET_PID getpid
#include <sys/types.h>
#include <unistd.h>
#endif //_WIN32
#endif //EPROSIMA_MUTEX_TEST

namespace eprosima {
namespace fastrtps {

#if !defined(EPROSIMA_MUTEX_TEST)

using Mutex = std::mutex;
using RecursiveMutex = std::recursive_mutex;
using TimedMutex = std::timed_mutex;
using RecursiveTimedMutex = std::recursive_timed_mutex;

#else

//! Stores the thread pid whose mutexes will be recorded.
extern std::atomic<pid_t> g_tmutex_thread_pid;

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
 * @param[in] mutex_type_id Identifier of the mutex type.
 * @param[in] mutex Pointer of the mutex to be recorded.
 */
void tmutex_record_typed_mutex_(int mutex_type_id, void* mutex);

template<class T>
void tmutex_record_mutex_(T*) { static_assert(sizeof(T) == 0, "mutex type not supported yet"); }

/*!
 * @brief Gets the pointer of the selected recorded mutex.
 * @param[in] index Position of the recorded mutex.
 * @return Pointer to the selected mutex.
 */
void* tmutex_get_mutex(size_t index);

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

template<class MutexType>
class TMutex : private MutexType
{
    using type = MutexType;

    public:

        using type::unlock;

        void lock()
        {
            pid_t pid = g_tmutex_thread_pid;

            if (0 != pid)
            {
                if (pid == GET_PID())
                {
                    tmutex_record_mutex_(this);
                }
            }
            else
            {
            }

            type::lock();
        }

        template<class T = MutexType, class Clock, class Duration>
        typename std::enable_if<std::is_same<T, std::timed_mutex>::value ||
            std::is_same<T, std::recursive_timed_mutex>::value, void>::type try_lock_until(
                const std::chrono::time_point<Clock, Duration>& timeout_time)
        {
            pid_t pid = g_tmutex_thread_pid;

            if (0 != pid)
            {
                if (pid == GET_PID())
                {
                    tmutex_record_mutex_(this);
                }
            }
            else
            {
            }

            type::try_lock_until(timeout_time);
        }
};

using Mutex = TMutex<std::mutex>;
using RecursiveMutex = TMutex<std::recursive_mutex>;
using TimedMutex = TMutex<std::timed_mutex>;
using RecursiveTimedMutex = TMutex<std::recursive_timed_mutex>;

template<>
inline
void tmutex_record_mutex_<Mutex>(Mutex* mutex)
{
    tmutex_record_typed_mutex_(1, reinterpret_cast<void*>(mutex));
}

template<>
inline
void tmutex_record_mutex_<RecursiveMutex>(RecursiveMutex* mutex)
{
    tmutex_record_typed_mutex_(2, reinterpret_cast<void*>(mutex));
}

template<>
inline
void tmutex_record_mutex_<TimedMutex>(TimedMutex* mutex)
{
    tmutex_record_typed_mutex_(3, reinterpret_cast<void*>(mutex));
}

template<>
inline
void tmutex_record_mutex_<RecursiveTimedMutex>(RecursiveTimedMutex* mutex)
{
    tmutex_record_typed_mutex_(4, reinterpret_cast<void*>(mutex));
}

#endif

} //namespace fastrtps
} //namespace eprosima

#endif //__UTILS_MUTEX_HPP__
