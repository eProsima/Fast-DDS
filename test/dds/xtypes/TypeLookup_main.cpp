// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeLookup_main.cpp
 *
 */

#include "TypeLookupPublisher.h"
#include "TypeLookupSubscriber.h"

#include <fastrtps/log/Log.h>

#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <random>


using namespace eprosima::fastrtps;

int main(
        int argc,
        char** argv)
{
    // Seed for random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, 10);
    int randomSeconds = distr(gen);
    // Sleep for the random duration
    std::this_thread::sleep_for(std::chrono::seconds(randomSeconds));

    std::cout << "Starting " << randomSeconds << std::endl;
    // Print all command-line arguments
    std::cout << "Command-line arguments:" << std::endl;
    for (int i = 0; i < argc; ++i)
    {
        std::cout << "argv[" << i << "]: " << argv[i] << std::endl;
    }

    int type = 0;
    std::vector<std::string> known_types;

    if (argc > 1)
    {
        if (strcmp(argv[1], "publisher") == 0)
        {
            type = 1;
        }
        else if (strcmp(argv[1], "subscriber") == 0)
        {
            type = 2;
        }

        for (int i = 2; i < argc; ++i)
        {
            known_types.push_back(argv[i]);
        }
    }
    else
    {
        std::cout << "publisher OR subscriber argument needed" << std::endl;
        Log::Reset();
        return 0;
    }

    switch (type)
    {
        case 1:
        {
            eprosima::fastdds::dds::TypeLookupPublisher pub;
            if (pub.init(known_types))
            {
                pub.wait_discovery(1);
                pub.run(10);
            }
            break;
        }
        case 2:
        {
            eprosima::fastdds::dds::TypeLookupSubscriber sub;
            if (sub.init(known_types))
            {
                sub.run(30);
            }
            break;
        }
        default:
            std::cout << "publisher OR subscriber argument needed" << std::endl;
    }
    Log::Reset();
    return 0;
}