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

#include <process.h>
#include <windows.h>

namespace eprosima {

thread::native_handle_type thread::start_thread_impl(
        int32_t stack_size,
        thread::start_routine_type start,
        void* arg)
{
        // Set the requested stack size, if given.
        unsigned stack_attr = 0;
        if (stack_size > 0)
        {
            if (sizeof(unsigned) <= sizeof(int32_t) &&
                    stack_size > static_cast<int32_t>(std::numeric_limits<unsigned>::max() / 2))
            {
                throw std::invalid_argument("Cannot cast stack_size into unsigned");
            }

            stack_attr = static_cast<unsigned>(stack_size);
        }

        // Construct and execute the thread.
        HANDLE hnd = (HANDLE) ::_beginthreadex(NULL, stack_attr, start, arg, 0, NULL);
        if(!hnd)
        {
            throw std::system_error(std::make_error_code(std::errc::resource_unavailable_try_again));
        }

        return hnd;
    }

thread::id thread::get_thread_id_impl(
        thread::native_handle_type hnd)
{
    return ::GetThreadId(hnd);
}

void thread::join_thread_impl(
        thread::native_handle_type hnd)
{
    if (::WaitForSingleObject(hnd, INFINITE) == WAIT_FAILED)
    {
        throw std::system_error(std::make_error_code(std::errc::no_such_process));
    }
}

void thread::detach_thread_impl(
        thread::native_handle_type hnd)
{
    if (::CloseHandle(hnd) == FALSE)
    {
        throw std::system_error(std::make_error_code(std::errc::no_such_process));
    }
}

thread::id thread::get_current_thread_id_impl()
{
    return ::GetCurrentThreadId();
}

} // eprosima
