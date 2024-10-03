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
 * @file SubscriberMain.cpp
 */

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include "SubscriberModule.hpp"

using namespace eprosima::fastdds::dds;

/* ARGUMENTS
 * --notexit
 * --fixed_type
 * --zero_copy
 * --succeed_on_timeout
 * --seed <int>
 * --samples <int>
 * --magic <str>
 * --timeout <int>
 * --xmlfile <path>
 * --publishers <int>
 * --die_on_data_received
 * --rescan <int>
 */

int main(
        int argc,
        char** argv)
{
    int arg_count = 1;
    bool notexit = false;
    bool fixed_type = false;
    bool zero_copy = false;
    bool die_on_data_received = false;
    bool succeed_on_timeout = false;
    uint32_t seed = 7800;
    uint32_t samples = 4;
    uint32_t publishers = 1;
    uint32_t timeout = 86400000; // 24 h in ms
    uint32_t rescan_interval_seconds = 0;
    char* xml_file = nullptr;
    std::string magic;

    while (arg_count < argc)
    {
        if (strcmp(argv[arg_count], "--notexit") == 0)
        {
            notexit = true;
        }
        else if (strcmp(argv[arg_count], "--fixed_type") == 0)
        {
            fixed_type = true;
        }
        else if (strcmp(argv[arg_count], "--zero_copy") == 0)
        {
            zero_copy = true;
        }
        else if (strcmp(argv[arg_count], "--succeed_on_timeout") == 0)
        {
            succeed_on_timeout = true;
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
        else if (strcmp(argv[arg_count], "--samples") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--samples expects a parameter" << std::endl;
                return -1;
            }

            samples = strtol(argv[arg_count], nullptr, 10);
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
        else if (strcmp(argv[arg_count], "--timeout") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--run-for expects a parameter" << std::endl;
                return -1;
            }

            timeout = strtol(argv[arg_count], nullptr, 10);
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
        else if (strcmp(argv[arg_count], "--publishers") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--publishers expects a parameter" << std::endl;
                return -1;
            }

            publishers = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--die_on_data_received") == 0)
        {
            die_on_data_received = true;
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

    SubscriberModule subscriber(publishers, samples, fixed_type, zero_copy, succeed_on_timeout, die_on_data_received);

    if (subscriber.init(seed, magic))
    {
        return subscriber.run(notexit, rescan_interval_seconds, timeout) ? 0 : -1;
    }

    return -1;
}
