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

#include <mach/mach.h>
#include <pthread.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>

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
    
    if(sched_class == SCHED_OTHER) 
    {               
        
        //
        // Sched OTHER has a nice value, that we pull from the priority parameter.
        // - Requires priorty value to be zero (0).
        // 
        result = pthread_setschedparam(self_tid, sched_class, &param);
        if(0 == result && change_priority)
        {
            uint64_t tid;
            pthread_threadid_np(NULL, &tid);
            result = setpriority(PRIO_PROCESS, tid, sched_priority);
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
        uint64_t affinity)
{
    if (affinity <= static_cast<uint64_t>(std::numeric_limits<integer_t>::max()))
    {
        thread_affinity_policy_data_t policy = { static_cast<integer_t>(affinity) };
        pthread_t self_tid = pthread_self();
        thread_policy_set(pthread_mach_thread_np(self_tid), THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
    }
}

void apply_thread_settings_to_current_thread(
        const fastdds::rtps::ThreadSettings& settings)
{
    configure_current_thread_scheduler(settings.scheduling_policy, settings.priority);
    configure_current_thread_affinity(settings.affinity);
}

}  // namespace eprosima
