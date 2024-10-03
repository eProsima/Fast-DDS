// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file PublisherMain.cpp
 */
#include "PublisherModule.hpp"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

using namespace eprosima::fastdds::dds;

/* ARGUMENTS
 * --exit_on_lost_liveliness
 * --fixed_type
 * --zero_copy
 * --seed <int>
 * --wait <int>
 * --samples <int>
 * --interval <int>
 * --magic <str>
 * --xmlfile <path>
 * --rescan <int>
 */

int main(
        int argc,
        char** argv)
{
    int arg_count = 1;
    bool exit_on_lost_liveliness = false;
    bool fixed_type = false;
    bool zero_copy = false;
    uint32_t seed = 7800;
    uint32_t wait = 0;
    char* xml_file = nullptr;
    uint32_t samples = 4;
    uint32_t interval = 250;
    uint32_t rescan_interval_seconds = 0;
    std::string magic;

    while (arg_count < argc)
    {
        if (strcmp(argv[arg_count], "--exit_on_lost_liveliness") == 0)
        {
            exit_on_lost_liveliness = true;
        }
        else if (strcmp(argv[arg_count], "--fixed_type") == 0)
        {
            fixed_type = true;
        }
        else if (strcmp(argv[arg_count], "--zero_copy") == 0)
        {
            zero_copy = true;
        }
        else if (strcmp(argv[arg_count], "--seed") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--seed expects a parameter" << std::endl;
                return -1;
            }

            seed = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--wait") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--wait expects a parameter" << std::endl;
                return -1;
            }

            wait = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--samples") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--samples expects a parameter" << std::endl;
                return -1;
            }

            samples = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--interval") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--interval expects a parameter" << std::endl;
                return -1;
            }

            interval = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--magic") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--magic expects a parameter" << std::endl;
                return -1;
            }

            magic = argv[arg_count];
        }
        else if (strcmp(argv[arg_count], "--xmlfile") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--xmlfile expects a parameter" << std::endl;
                return -1;
            }

            xml_file = argv[arg_count];
        }
        else if (strcmp(argv[arg_count], "--rescan") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--rescan expects a parameter" << std::endl;
                return -1;
            }

            rescan_interval_seconds = strtol(argv[arg_count], nullptr, 10);
        }
        else
        {
            std::cout << "Wrong argument " << argv[arg_count] << std::endl;
            return -1;
        }

        ++arg_count;
    }

    if (xml_file)
    {
        DomainParticipantFactory::get_instance()->load_XML_profiles_file(xml_file);
    }

    PublisherModule publisher(exit_on_lost_liveliness, fixed_type, zero_copy);

    if (publisher.init(seed, magic))
    {
        if (wait > 0)
        {
            publisher.wait_discovery(wait);
        }

        publisher.run(samples, rescan_interval_seconds, 0, interval);
        return 0;
    }

    return -1;
}
