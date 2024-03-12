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

#include <cstdlib>
#include <iostream>

struct subscriber_config
{
    bool use_waitset;
};

struct hello_world_config
{
    std::string entity;
    subscriber_config sub_config;
};


void print_help(
        uint8_t return_code)
{
    std::cout << "Usage: hello_world <entity> [options]"            << std::endl;
    std::cout << ""                                                 << std::endl;
    std::cout << "Entities:"                                        << std::endl;
    std::cout << "  publisher           Run a publisher entity"     << std::endl;
    std::cout << "  subscriber          Run a subscriber entity"    << std::endl;
    std::cout << "Common options:"                                  << std::endl;
    std::cout << "  -h, --help          Print this help message"    << std::endl;
    std::cout << "Subscriber options:"                              << std::endl;
    std::cout << "  -w, --waitset       Use waitset read condition" << std::endl;
    std::exit(return_code);
}

hello_world_config parse_cli_options (
        int argc,
        char* argv[])
{
    hello_world_config config;
    config.entity = "";
    config.sub_config.use_waitset = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            print_help(EXIT_SUCCESS);
        }
        else if (arg == "publisher" || arg == "subscriber")
        {
            config.entity = arg;
        }
        else if (arg == "-w" || arg == "--waitset")
        {
            if (config.entity == "subscriber")
            {
                config.sub_config.use_waitset = true;
            }
            else
            {
                EPROSIMA_LOG_ERROR(CLI_PARSE, "waitset can only be used with the subscriber entity");
                print_help(EXIT_FAILURE);
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(CLI_PARSE, "unknown option " + arg);
            print_help(EXIT_FAILURE);
        }
    }

    if (config.entity == "")
    {
        EPROSIMA_LOG_ERROR(CLI_PARSE, "entity not specified");
        print_help(EXIT_FAILURE);
    }

    return config;
}
