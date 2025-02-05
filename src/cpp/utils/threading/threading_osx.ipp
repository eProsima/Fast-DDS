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
#include <cstdio>
#include <cstring>
#include <limits>

#include <mach/mach.h>
#include <pthread.h>

#include <fastdds/dds/log/Log.hpp>
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
    pthread_setname_np(thread_name_buffer.data());
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

static void configure_current_thread_scheduler(
        const char* thread_name,
        int sched_class,
        int sched_priority)
{
    pthread_t self_tid = pthread_self();
    sched_param param;
    sched_param current_param;
    int current_class;
    int result = 0;
    bool change_priority = (std::numeric_limits<int32_t>::min() != sched_priority);

    // Get current scheduling parameters
    memset(&current_param, 0, sizeof(current_param));
    pthread_getschedparam(self_tid, &current_class, &current_param);

    memset(&param, 0, sizeof(param));
    param.sched_priority = 0;
    sched_class = (sched_class == -1) ? current_class : sched_class;

    //
    // Set Scheduler Class and Priority
    //

    if (sched_class == SCHED_OTHER)
    {

        //
        // Sched OTHER has a nice value, that we pull from the priority parameter.
        // - Requires priorty value to be zero (0).
        //
        result = pthread_setschedparam(self_tid, sched_class, &param);
        if (0 == result && change_priority)
        {
            uint64_t tid;
            pthread_threadid_np(NULL, &tid);
            result = setpriority(PRIO_PROCESS, tid, sched_priority);
            if (0 != result)
            {
                THREAD_EPROSIMA_LOG_ERROR(thread_name, "Problem to set priority of thread with id [" << tid << "," << thread_name << "] to value " << sched_priority << ". Error '" << strerror(
                            result) << "'");
            }
        }
        else if (0 != result)
        {
            THREAD_EPROSIMA_LOG_ERROR(thread_name, "Problem to set scheduler of thread with id [" << self_tid << "," << thread_name << "] to value " << sched_class << ". Error '" << strerror(
                        result) << "'");
        }
    }
    else if ((sched_class == SCHED_FIFO) ||
            (sched_class == SCHED_RR))
    {
        //
        // RT Policies use a different priority numberspace.
        //
        param.sched_priority = change_priority ? sched_priority : current_param.sched_priority;
        result = pthread_setschedparam(self_tid, sched_class, &param);
        if (0 != result)
        {
            THREAD_EPROSIMA_LOG_ERROR(thread_name, "Problem to set scheduler of thread with id [" << self_tid << "," << thread_name << "] to value " << sched_class << " with priority " << param.sched_priority << ". Error '" << strerror(
                        result) << "'");
        }
    }
}

static void configure_current_thread_affinity(
        const char* thread_name,
        uint64_t affinity)
{
    if (affinity <= static_cast<uint64_t>(std::numeric_limits<integer_t>::max()))
    {
        int result = 0;
        thread_affinity_policy_data_t policy = { static_cast<integer_t>(affinity) };
        pthread_t self_tid = pthread_self();
        result =
                thread_policy_set(pthread_mach_thread_np(self_tid), THREAD_AFFINITY_POLICY, (thread_policy_t)&policy,
                        1);
        if (0 != result)
        {
            THREAD_EPROSIMA_LOG_ERROR(thread_name, "Problem to set affinity of thread with id [" << self_tid << "," << thread_name << "] to value " << affinity << ". Error '" << strerror(
                        result) << "'");
        }
    }
}

void apply_thread_settings_to_current_thread(
        const char* thread_name,
        const fastdds::rtps::ThreadSettings& settings)
{
    configure_current_thread_scheduler(thread_name, settings.scheduling_policy, settings.priority);
    configure_current_thread_affinity(thread_name, settings.affinity);
}

}  // namespace eprosima
