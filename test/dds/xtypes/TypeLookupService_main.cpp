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
 * @file TypeLookupService_main.cpp
 *
 */

#include "TypeLookupServicePublisher.h"
#include "TypeLookupServiceSubscriber.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

struct CommandLineArgs
{
    int kind;
    int samples;
    int timeout;
    int expected_matches;
    std::vector<std::string> known_types;
    uint32_t seed {10800};
};

CommandLineArgs parse_args(
        int argc,
        char** argv)
{
    CommandLineArgs args = {0, 0, 0, 0, {}};

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        std::stringstream ss(arg);

        std::string key, value;
        std::getline(ss, key, '=');
        std::getline(ss, value, '=');

        if (key == "kind")
        {
            if (value == "publisher")
            {
                args.kind = 1;
            }
            else if (value == "subscriber")
            {
                args.kind = 2;
            }
        }
        else if (key == "samples")
        {
            args.samples = std::stoi(value);
        }
        else if (key == "timeout")
        {
            args.timeout = std::stoi(value);
        }
        else if (key == "expected_matches")
        {
            args.expected_matches = std::stoi(value);
        }
        else if (key == "known_types")
        {
            std::replace(value.begin(), value.end(), ',', ' ');
            std::stringstream types_ss(value);
            std::string type;
            while (types_ss >> type)
            {
                args.known_types.push_back(type);
            }
        }
        else if (key == "seed")
        {
            args.seed = strtol(value.c_str(), nullptr, 10);
        }
    }

    return args;
}

int main(
        int argc,
        char** argv)
{
    // Print all command-line arguments
    // std::cout << "Command-line arguments:" << std::endl;
    // for (int i = 0; i < argc; ++i)
    // {
    //     std::cout << "argv[" << i << "]: " << argv[i] << std::endl;
    // }

    CommandLineArgs args = parse_args(argc, argv);

    try
    {
        switch (args.kind){
            case 1: {
                eprosima::fastdds::dds::TypeLookupServicePublisher pub;
                return (pub.init(args.seed % 230, args.known_types) &&
                       pub.wait_discovery(args.expected_matches, args.timeout) &&
                       pub.run(args.samples, args.timeout) &&
                       pub.wait_discovery(0, args.timeout)) ? 0 : -1;
            }
            case 2: {
                eprosima::fastdds::dds::TypeLookupServiceSubscriber sub;
                return (sub.init(args.seed % 230, args.known_types) &&
                       sub.wait_discovery(args.expected_matches, args.timeout) &&
                       sub.run(args.samples, args.timeout)) ? 0 : -1;
            }
            default:
                std::cout << "Invalid participant type. Use 'publisher' or 'subscriber'." << std::endl;
                return -1;
        }
    }
    catch (std::exception const& e)
    {
        std::cout << "Tests failed: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
