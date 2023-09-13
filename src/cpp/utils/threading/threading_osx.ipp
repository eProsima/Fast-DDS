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

#include <pthread.h>
#include <string.h>
#include <stdio.h>

namespace eprosima {

template<typename... Args>
static void set_name_to_current_thread_impl(
    const char* fmt, Args... args)
{
    char thread_name[16]{};
    snprintf(thread_name, 16, fmt, args...);
    pthread_setname_np(thread_name);
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

void apply_thread_settings_to_current_thread(
        const fastdds::rtps::ThreadSettings& /*settings*/)
{
}

}  // namespace eprosima
