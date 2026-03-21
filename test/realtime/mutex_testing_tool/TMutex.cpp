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

#include "TMutex.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <pthread.h>

// TODO contar que solo bloquea una vez y nunca mas despues de timeout.
// TODO si se bloquea el dos, que no se bloqueen los posteriores

using namespace eprosima::fastdds;

namespace eprosima {
namespace fastdds {

std::atomic<pid_t> g_tmutex_thread_pid(0);
// *INDENT-OFF* Uncrustify parse this as a function declaration instead of a function pointer.
int (*g_origin_lock_func)(pthread_mutex_t*){nullptr};
int (*g_origin_timedlock_func)(pthread_mutex_t*, const struct timespec*){nullptr};
int (*g_origin_clocklock_func)(pthread_mutex_t*, clockid_t, const struct timespec*){nullptr};
// *INDENT-ON*

typedef struct
{
    LockType type;
    pthread_mutex_t* mutex;
    uint32_t count;
} tmutex_record;

constexpr size_t g_tmutex_records_max_length = 30;
std::array<tmutex_record, g_tmutex_records_max_length>  g_tmutex_records{{{LockType::LOCK, nullptr, 0}}};
int32_t g_tmutex_records_end = -1;

int32_t tmutex_find_record(
        pthread_mutex_t* mutex)
{
    int32_t returned_position = -1;

    for (int32_t position = 0; position <= g_tmutex_records_end; ++position)
    {
        if (mutex == g_tmutex_records[position].mutex)
        {
            returned_position = position;
            break;
        }
    }

    return returned_position;
}

} //namespace fastdds
} //namespace eprosima

void eprosima::fastdds::tmutex_start_recording()
{
    assert(0 == g_tmutex_thread_pid);
    g_tmutex_thread_pid = GET_TID();
    g_tmutex_records = {{{LockType::LOCK, nullptr, 0}}};
    g_tmutex_records_end = -1;
}

void eprosima::fastdds::tmutex_stop_recording()
{
    assert(0 < g_tmutex_thread_pid);
    g_tmutex_thread_pid = 0;
}

void eprosima::fastdds::tmutex_record_mutex_(
        LockType type,
        pthread_mutex_t* mutex)
{
    assert(0 < g_tmutex_thread_pid);

    // Search if mutex already has an entry.
    int32_t position =  tmutex_find_record(mutex);

    if (-1 >= position)
    {
        assert(g_tmutex_records_max_length > size_t(g_tmutex_records_end + 1));
        position = ++g_tmutex_records_end;
        g_tmutex_records[position].type = type;
        g_tmutex_records[position].mutex = mutex;
    }

    ++g_tmutex_records[position].count;
}

size_t eprosima::fastdds::tmutex_get_num_mutexes()
{
    assert(0 == g_tmutex_thread_pid);
    return g_tmutex_records_end + 1;
}

size_t eprosima::fastdds::tmutex_get_num_lock_type()
{
    size_t counter = 0;

    if (-1 < g_tmutex_records_end)
    {
        std::for_each(g_tmutex_records.begin(), g_tmutex_records.begin() + g_tmutex_records_end + 1,
                [&](const tmutex_record& record)
                {
                    if (record.type == LockType::LOCK)
                    {
                        ++counter;
                    }
                });
    }

    return counter;
}

size_t eprosima::fastdds::tmutex_get_num_timedlock_type()
{
    size_t counter = 0;

    if (-1 < g_tmutex_records_end)
    {
        std::for_each(g_tmutex_records.begin(), g_tmutex_records.begin() + g_tmutex_records_end + 1,
                [&](const tmutex_record& record)
                {
                    if (record.type == LockType::TIMED_LOCK)
                    {
                        ++counter;
                    }
                });
    }

    return counter;
}

pthread_mutex_t* eprosima::fastdds::tmutex_get_mutex(
        const size_t index)
{
    assert(index <= size_t(g_tmutex_records_end));
    return g_tmutex_records[index].mutex;
}

bool eprosima::fastdds::tmutex_lock_mutex(
        const size_t index)
{
    assert(index <= size_t(g_tmutex_records_end));
    if (LockType::TIMED_LOCK == g_tmutex_records[index].type)
    {

        if (g_origin_lock_func != nullptr)
        {
            (*g_origin_lock_func)(g_tmutex_records[index].mutex);
        }

        return true;
    }

    return false;
}

void eprosima::fastdds::tmutex_unlock_mutex(
        const size_t index)
{
    assert(index <= size_t(g_tmutex_records_end));
    pthread_mutex_unlock(g_tmutex_records[index].mutex);
}
