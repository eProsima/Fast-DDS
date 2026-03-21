// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima)
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

/**
 * @file ParticipantMain.cpp
 */

#include <condition_variable>
#include <csignal>
#include <cstring>
#include <iostream>
#include <mutex>
#include <regex>
#include <string>

#include "ParticipantModule.hpp"

#include <fastdds/dds/log/Log.hpp>

using Log = eprosima::fastdds::dds::Log;

/**
 * ARGUMENTS
 * --guid_prefix <str>
 * --discovery_protocol <str>
 * --unicast_metatraffic_locator <str>
 */

volatile sig_atomic_t signal_status = 0;
std::mutex signal_mutex;
std::condition_variable signal_cv;

void sigint_handler(
        int signum)
{
    signal_status = signum;
    signal_cv.notify_one();
}

int main(
        int argc,
        char** argv)
{
    Log::SetVerbosity(Log::Warning);
    Log::SetCategoryFilter(std::regex("(RTPS_QOS_CHECK)"));
    Log::SetErrorStringFilter(std::regex("(Discovery Servers)"));

    int arg_count = 1;
    std::string guid_prefix;
    std::string discovery_protocol;
    std::string unicast_metatraffic_locator;

    while (arg_count < argc)
    {
        if (strcmp(argv[arg_count], "--guid_prefix") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--guid_prefix expects a parameter\n";
                return -1;
            }
            guid_prefix = argv[arg_count];
        }
        else if (strcmp(argv[arg_count], "--discovery_protocol") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--discovery_protocol expects a parameter\n";
                return -1;
            }
            discovery_protocol = argv[arg_count];
        }
        else if (strcmp(argv[arg_count], "--unicast_metatraffic_locator") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--unicast_metatraffic_locator expects a parameter\n";
                return -1;
            }
            unicast_metatraffic_locator = argv[arg_count];
        }
        else
        {
            std::cout << "Wrong argument " << argv[arg_count] << std::endl;
            return -1;
        }
        ++arg_count;
    }

    eprosima::fastdds::dds::ParticipantModule participant(discovery_protocol, guid_prefix, unicast_metatraffic_locator);

    if (participant.init())
    {
        std::unique_lock<std::mutex> lock(signal_mutex);
        signal(SIGINT, sigint_handler);
        signal_cv.wait(lock, []
                {
                    return 0 != signal_status;
                });
        return 0;
    }
    return -1;
}
