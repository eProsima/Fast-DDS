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

#include <limits>
#include <sstream>
#include <string>
#include <processthreadsapi.h>

#include <fastdds/rtps/attributes/ThreadSettings.hpp>

namespace eprosima {

template<typename ... Args>
static void set_name_to_current_thread_impl(
        const char* fmt,
        Args... args)
{
    char thread_name[16]{};
    snprintf(thread_name, 16, fmt, args ...);

    std::wstringstream stream;
    stream << thread_name;
    std::wstring w_thread_name = stream.str();

    SetThreadDescription(GetCurrentThread(), w_thread_name.c_str());
}

void set_name_to_current_thread(
        const char* name)
{
    set_name_to_current_thread_impl("%s", name);
}

void set_name_to_current_thread(
        const char* fmt,
        uint32_t arg)
{
    set_name_to_current_thread_impl(fmt, arg);
}

void set_name_to_current_thread(
        const char* fmt,
        uint32_t arg1,
        uint32_t arg2)
{
    set_name_to_current_thread_impl(fmt, arg1, arg2);
}

static void configure_current_thread_priority(
        int32_t priority)
{
    if (priority != std::numeric_limits<int32_t>::min())
    {
        if (0 == SetThreadPriority(GetCurrentThread(), priority))
        {
            EPROSIMA_LOG_ERROR(SYSTEM,
                    "Error '" << GetLastError() << "' configuring priority for thread " << GetCurrentThread());
        }
    }
}

static void configure_current_thread_affinity(
        uint64_t affinity_mask)
{
    if (affinity_mask != 0)
    {
        if (0 == SetThreadAffinityMask(GetCurrentThread(), static_cast<DWORD_PTR>(affinity_mask)))
        {
            EPROSIMA_LOG_ERROR(SYSTEM,
                    "Error '" << GetLastError() << "' configuring affinity for thread " << GetCurrentThread());
        }
    }
}

void apply_thread_settings_to_current_thread(
        const fastdds::rtps::ThreadSettings& settings)
{
    configure_current_thread_priority(settings.priority);
    configure_current_thread_affinity(settings.affinity);
}

}  // namespace eprosima
