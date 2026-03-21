// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "utils/thread.hpp"

#include <pthread.h>

namespace eprosima {

thread::native_handle_type thread::start_thread_impl(
        int32_t stack_size,
        thread::start_routine_type start,
        void* arg)
{
    int errnum;

    // Construct the attributes object.
    pthread_attr_t attr;
    if ((errnum = ::pthread_attr_init(&attr)) != 0)
    {
        throw std::system_error(errnum, std::system_category(), "pthread_attr_init failed");
    }

    // Ensure the attributes object is destroyed
    auto attr_deleter = [](pthread_attr_t* a)
            {
                int err;
                if ((err = ::pthread_attr_destroy(a)) != 0)
                {
                    throw std::system_error(err, std::system_category(), "pthread_attr_destroy failed");
                }
            };
    std::unique_ptr<pthread_attr_t, decltype(attr_deleter)> attr_scope_destroy(&attr, attr_deleter);

    // Set the requested stack size, if given.
    if (stack_size >= 0)
    {
        if (sizeof(unsigned) <= sizeof(int32_t) &&
                stack_size > static_cast<int32_t>(std::numeric_limits<unsigned>::max() / 2))
        {
            throw std::invalid_argument("Cannot cast stack_size into unsigned");
        }

        if ((errnum = ::pthread_attr_setstacksize(&attr, stack_size)) != 0)
        {
            throw std::system_error(errnum, std::system_category(), "pthread_attr_setstacksize failed");
        }
    }

    // Construct and execute the thread.
    pthread_t hnd;
    if ((errnum = ::pthread_create(&hnd, &attr, start, arg)) != 0)
    {
        throw std::system_error(errnum, std::system_category(), "pthread_create failed");
    }

    return hnd;
}

thread::id thread::get_thread_id_impl(
        thread::native_handle_type hnd)
{
    return hnd;
}

void thread::join_thread_impl(
        thread::native_handle_type hnd)
{
    int errnum;
    if ((errnum = ::pthread_join(hnd, nullptr)) != 0)
    {
        throw std::system_error(std::make_error_code(std::errc::no_such_process), "pthread_join failed");
    }
}

void thread::detach_thread_impl(
        thread::native_handle_type hnd)
{
    int errnum;

    if ((errnum = ::pthread_detach(hnd)) != 0)
    {
        throw std::system_error(errnum, std::system_category(), "pthread_detach failed");
    }
}

thread::id thread::get_current_thread_id_impl()
{
    return ::pthread_self();
}

} // eprosima
