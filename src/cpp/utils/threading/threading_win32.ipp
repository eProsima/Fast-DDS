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

#include <array>
#include <limits>
#include <sstream>
#include <string>
#include <processthreadsapi.h>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <utils/threading/thread_logging.hpp>

namespace eprosima {

template<typename ... Args>
static void set_name_to_current_thread_impl(
        std::array<char, 16>& thread_name_buffer,
        const char* fmt,
        Args... args)
{
    snprintf(thread_name_buffer.data(), 16, fmt, args ...);

    std::wstringstream stream;
    stream << thread_name_buffer.data();
    std::wstring w_thread_name = stream.str();

    SetThreadDescription(GetCurrentThread(), w_thread_name.c_str());
}

void set_name_to_current_thread(
        std::array<char, 16>& thread_name_buffer,
        const char* name)
{
    set_name_to_current_thread_impl(thread_name_buffer, "%s", name);
}

void set_name_to_current_thread(
        std::array<char, 16>& thread_name_buffer,
        const char* fmt,
        uint32_t arg)
{
    set_name_to_current_thread_impl(thread_name_buffer, fmt, arg);
}

void set_name_to_current_thread(
        std::array<char, 16>& thread_name_buffer,
        const char* fmt,
        uint32_t arg1,
        uint32_t arg2)
{
    set_name_to_current_thread_impl(thread_name_buffer, fmt, arg1, arg2);
}

static void configure_current_thread_priority(
        const char* thread_name,
        int32_t priority)
{
    if (priority != std::numeric_limits<int32_t>::min())
    {
        if (0 == SetThreadPriority(GetCurrentThread(), priority))
        {
            THREAD_EPROSIMA_LOG_ERROR(thread_name,
                    "Problem to set priority of thread with id [" << GetCurrentThreadId() << "," << thread_name << "] to value " << priority <<
                    ". Error '" << GetLastError() << "'");
        }
    }
}

static void configure_current_thread_affinity(
        const char* thread_name,
        uint64_t affinity_mask)
{
    if (affinity_mask != 0)
    {
        if (0 == SetThreadAffinityMask(GetCurrentThread(), static_cast<DWORD_PTR>(affinity_mask)))
        {
            THREAD_EPROSIMA_LOG_ERROR(thread_name,
                    "Problem to set affinity of thread with id [" << GetCurrentThreadId() << "," << thread_name << "] to value " << affinity_mask <<
                    ". Error '" << GetLastError() << "'");
        }
    }
}

void apply_thread_settings_to_current_thread(
        const char* thread_name,
        const fastdds::rtps::ThreadSettings& settings)
{
    configure_current_thread_priority(thread_name, settings.priority);
    configure_current_thread_affinity(thread_name, settings.affinity);
}

}  // namespace eprosima
