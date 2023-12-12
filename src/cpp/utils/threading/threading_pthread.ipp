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

#include <cstdio>
#include <cstring>
#include <limits>

#include <pthread.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>

namespace eprosima {

template<typename... Args>
static void set_name_to_current_thread_impl(
    const char* fmt, Args... args)
{
    char thread_name[16]{};
    snprintf(thread_name, 16, fmt, args...);
    auto id = pthread_self();
    pthread_setname_np(id, thread_name);
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

static void configure_current_thread_scheduler(
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

    if((sched_class == SCHED_OTHER) ||
       (sched_class == SCHED_BATCH) ||
       (sched_class == SCHED_IDLE))
    {
        //
        // BATCH and IDLE do not have explicit priority values.
        // - Requires priorty value to be zero (0).

        result = pthread_setschedparam(self_tid, sched_class, &param);

        //
        // Sched OTHER has a nice value, that we pull from the priority parameter.
        //

        if(0 == result && sched_class == SCHED_OTHER && change_priority)
        {
            result = setpriority(PRIO_PROCESS, gettid(), sched_priority);
        }
    }
    else if((sched_class == SCHED_FIFO) ||
            (sched_class == SCHED_RR))
    {
        //
        // RT Policies use a different priority numberspace.
        //

        param.sched_priority = change_priority ? sched_priority : current_param.sched_priority;
        result = pthread_setschedparam(self_tid, sched_class, &param);
    }

    if (0 != result)
    {
        EPROSIMA_LOG_ERROR(SYSTEM, "Error '" << strerror(result) << "' configuring scheduler for thread " << self_tid);
    }
}

static void configure_current_thread_affinity(
        uint64_t affinity_mask)
{
    int a;
    int result;
    int cpu_count;
    cpu_set_t cpu_set;
    pthread_t self_tid = pthread_self();

    result = 0;

    //
    // Rebuilt the cpu set from scratch...
    //

    CPU_ZERO(&cpu_set);

    //
    // If the bit is set in our mask, set it into the cpu_set
    // We only consider up to the total number of CPU's the
    // system has.
    //
    cpu_count = get_nprocs_conf();

    for(a = 0; a < cpu_count; a++)
    {
        if(0 != (affinity_mask & 1))
        {
            CPU_SET(a, &cpu_set);
            result++;
        }
        affinity_mask >>= 1;
    }

    if (affinity_mask > 0)
    {
        EPROSIMA_LOG_ERROR(SYSTEM, "Affinity mask has more processors than the ones present in the system");
    }

    if(result > 0)
    {
#ifdef ANDROID
        result = sched_setaffinity(self_tid, sizeof(cpu_set_t), &cpu_set);
#else
        result = pthread_setaffinity_np(self_tid, sizeof(cpu_set_t), &cpu_set);
#endif
    }

    if (0 != result)
    {
        EPROSIMA_LOG_ERROR(SYSTEM, "Error '" << strerror(result) << "' configuring affinity for thread " << self_tid);
    }
}

void apply_thread_settings_to_current_thread(
        const fastdds::rtps::ThreadSettings& settings)
{
    configure_current_thread_scheduler(settings.scheduling_policy, settings.priority);
    configure_current_thread_affinity(settings.affinity);
}

}  // namespace eprosima
