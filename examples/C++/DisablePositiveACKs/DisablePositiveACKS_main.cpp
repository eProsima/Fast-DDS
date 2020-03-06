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

/**
 * @file DisablePositiveACKs_main.cpp
 *
 */

#include "DisablePositiveACKsPublisher.h"
#include "DisablePositiveACKsSubscriber.h"

#include <fastrtps/Domain.h>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

/**
 * @brief Parses command line arguments
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @param type Publisher or subscriber
 * @param enabled A positive number to enable this Qos or zero to disable it
 * @param duration_ms Duration before setting samples to acknowledged
 * @param sleep_ms Writer sleep read from command line arguments (populated if specified)
 * @param samples Number of samples read from command line arguments (populated if specified
 * @return True if command line arguments were parsed succesfully and execution can continue
 */
bool parse_arguments(
        int argc,
        char** argv,
        int& type,
        long& enabled,
        long& duration_ms,
        long& sleep_ms,
        long& samples)
{
    if (argc == 1)
    {
        // No arguments provided
        return false;
    }

    for (int i=0; i<argc; i++)
    {
        if (!strcmp(argv[i], "--help"))
        {
            // --help command found
            return false;
        }
    }

    if (strcmp(argv[1], "publisher") == 0)
    {
        type = 1;

        int count = 2;
        while (count < argc)
        {
            if (!strcmp(argv[count], "--disable"))
            {
                enabled = 1;
                count = count + 1;
            }
            else if (!strcmp(argv[count], "--keep_duration"))
            {
                duration_ms = atoi(argv[count + 1]);
                count = count + 2;
            }
            else if (!strcmp(argv[count], "--sleep"))
            {
                sleep_ms = atoi(argv[count + 1]);
                count = count + 2;
            }
            else if (!strcmp(argv[count], "--samples"))
            {
                samples = atoi(argv[count + 1]);
                count = count + 2;
            }
            else
            {
                std::cout << "Unknown command line option " << argv[count] << " for publisher" << std::endl;
                return false;
            }
        }
        return true;
    }

    if (strcmp(argv[1], "subscriber") == 0)
    {
        type = 2;

        int count = 2;
        while (count < argc)
        {
            if (!strcmp(argv[count], "--disable"))
            {
                enabled = 1;
                count = count + 1;
            }
            else
            {
                std::cout << "Unknown command line option " << argv[2] << " for publisher" << std::endl;
                return false;
            }
        }
        return true;
    }

    return false;
}

int main(int argc, char** argv)
{
    int type = 1;

    // >0 to use disable positive acks
    long use_disable_positive_acks = 0;
    // Keep duration in milliseconds
    long keep_duration_ms = 5000;
    // Sleep time between samples
    long writer_sleep_ms = 1000;
    // Number of samples to send
    long count = 20;

    if (!parse_arguments(
                argc,
                argv,
                type,
                use_disable_positive_acks,
                keep_duration_ms,
                writer_sleep_ms,
                count))
    {
        std::cout << "Usage: " << std::endl;
        std::cout << argv[0] << " publisher ";
        std::cout << "[--disable]" ;
        std::cout << "[--keep_duration <duration_ms>] ";
        std::cout << "[--sleep <writer_sleep_ms>] ";
        std::cout << "[--samples <samples>]" << std::endl;

        std::cout << "OR" << std::endl;
        std::cout << argv[0] << " subscriber ";
        std::cout << "[--disable]" << std::endl;

        eprosima::fastdds::dds::Log::Reset();
        return 0;
    }

    switch(type)
    {
        case 1:
            {
                DisablePositiveACKsPublisher mypub;
                if (mypub.init(use_disable_positive_acks > 0, keep_duration_ms))
                {
                    mypub.run(count, writer_sleep_ms);
                }
                break;
            }
        case 2:
            {
                DisablePositiveACKsSubscriber mysub;
                if (mysub.init(use_disable_positive_acks > 0))
                {
                    mysub.run(count);
                }
                break;
            }
    }
    Domain::stopAll();
    eprosima::fastdds::dds::Log::Reset();
    return 0;
}
