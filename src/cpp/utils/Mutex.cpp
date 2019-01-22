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

#include <fastrtps/utils/Mutex.hpp>
#include <iostream>

#if defined(EPROSIMA_MUTEX_TEST)
#include <typeinfo>
#include <cassert>
#endif

using namespace eprosima::fastrtps;

#if defined(EPROSIMA_MUTEX_TEST)
namespace eprosima {
namespace fastrtps {

std::atomic<pid_t> g_tmutex_thread_pid(0);

typedef struct
{
    int id;
    void* mutex;
    uint32_t count;

} tmutex_record;

constexpr size_t g_tmutex_records_max_length = 10;
std::array<tmutex_record, g_tmutex_records_max_length>  g_tmutex_records{{{-1, nullptr, 0}}};
int32_t g_tmutex_records_end = -1;

int32_t tmutex_find_record(void* mutex)
{
    int32_t returned_position = -1;

    for(int32_t position = 0; position < g_tmutex_records_end; ++position)
    {
        if (mutex == g_tmutex_records[position].mutex)
        {
            returned_position = position;
            break;
        }
    }

    return returned_position;
}

} //namespace fastrtps
} //namespace eprosima

void eprosima::fastrtps::tmutex_start_recording()
{
    assert(0 == g_tmutex_thread_pid);
    g_tmutex_thread_pid = GET_PID();
    g_tmutex_records = {{{-1, nullptr, 0}}};
    g_tmutex_records_end = -1;
}

void eprosima::fastrtps::tmutex_stop_recording()
{
    assert(0 < g_tmutex_thread_pid);
    g_tmutex_thread_pid = 0;
}

void eprosima::fastrtps::tmutex_record_typed_mutex_(int mutex_type_id, void* mutex)
{
    assert(0 < g_tmutex_thread_pid);

    // Search if mutex already has an entry.
    int32_t position =  tmutex_find_record(mutex);

    if (-1 >= position)
    {
        assert(g_tmutex_records_max_length > g_tmutex_records_end + 1);
        position = ++g_tmutex_records_end;
        g_tmutex_records[position].id = mutex_type_id;
        g_tmutex_records[position].mutex = mutex;
    }

    ++g_tmutex_records[position].count;
}

size_t eprosima::fastrtps::tmutex_get_num_mutexes()
{
    assert(0 == g_tmutex_thread_pid);
    return g_tmutex_records_end + 1;
}

void* eprosima::fastrtps::tmutex_get_mutex(const size_t index)
{
    assert(index <= g_tmutex_records_end);
    return g_tmutex_records[index].mutex;
}

void eprosima::fastrtps::tmutex_lock_mutex(const size_t index)
{
    assert(index <= g_tmutex_records_end);
    switch (g_tmutex_records[index].id)
    {
        case 1:
        {
            std::mutex* mutex = reinterpret_cast<std::mutex*>(g_tmutex_records[index].mutex);
            mutex->lock();
            break;
        }
        case 2:
        {
            std::recursive_mutex* mutex = reinterpret_cast<std::recursive_mutex*>(g_tmutex_records[index].mutex);
            mutex->lock();
            break;
        }
        case 3:
        {
            std::timed_mutex* mutex = reinterpret_cast<std::timed_mutex*>(g_tmutex_records[index].mutex);
            mutex->lock();
            break;
        }
        case 4:
        {
            std::recursive_timed_mutex* mutex =
                reinterpret_cast<std::recursive_timed_mutex*>(g_tmutex_records[index].mutex);
            mutex->lock();
            break;
        }
    }
}

void eprosima::fastrtps::tmutex_unlock_mutex(const size_t index)
{
    assert(index <= g_tmutex_records_end);
    switch (g_tmutex_records[index].id)
    {
        case 1:
        {
            std::mutex* mutex = reinterpret_cast<std::mutex*>(g_tmutex_records[index].mutex);
            mutex->unlock();
            break;
        }
        case 2:
        {
            std::recursive_mutex* mutex = reinterpret_cast<std::recursive_mutex*>(g_tmutex_records[index].mutex);
            mutex->unlock();
            break;
        }
        case 3:
        {
            std::timed_mutex* mutex = reinterpret_cast<std::timed_mutex*>(g_tmutex_records[index].mutex);
            mutex->unlock();
            break;
        }
        case 4:
        {
            std::recursive_timed_mutex* mutex =
                reinterpret_cast<std::recursive_timed_mutex*>(g_tmutex_records[index].mutex);
            mutex->unlock();
            break;
        }
    }
}

#endif
