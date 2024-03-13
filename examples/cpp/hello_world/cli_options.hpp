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

#ifndef _FASTDDS_HELLO_WORLD_CLI_PARSER_HPP_
#define _FASTDDS_HELLO_WORLD_CLI_PARSER_HPP_

class CLIParser
{
public:

    CLIParser() = delete;

    struct publisher_config
    {
        uint16_t samples;
    };

    struct subscriber_config
    {
        bool use_waitset;
        uint16_t samples;
    };

    struct hello_world_config
    {
        std::string entity;
        publisher_config pub_config;
        subscriber_config sub_config;
    };

    static void print_help(
            uint8_t return_code)
    {
        std::cout << "Usage: hello_world <entity> [options]"                    << std::endl;
        std::cout << ""                                                         << std::endl;
        std::cout << "Entities:"                                                << std::endl;
        std::cout << "  publisher           Run a publisher entity"             << std::endl;
        std::cout << "  subscriber          Run a subscriber entity"            << std::endl;
        std::cout << "Common options:"                                          << std::endl;
        std::cout << "  -h, --help          Print this help message"            << std::endl;
        std::cout << "  -s, --samples       Amount of samples to be sent or"    << std::endl;
        std::cout << "                      received (default: 0 [unlimited])"  << std::endl;
        std::cout << "Subscriber options:"                                      << std::endl;
        std::cout << "  -w, --waitset       Use waitset read condition"         << std::endl;
        std::exit(return_code);
    }

    static hello_world_config parse_cli_options(
            int argc,
            char* argv[])
    {
        hello_world_config config;
        config.entity = "";
        config.pub_config.samples = 0;
        config.sub_config.samples = 0;
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
            else if (arg == "-s" || arg == "--samples")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        uint16_t samples = std::stoi(argv[++i]);
                        if (config.entity == "publisher")
                        {
                            config.pub_config.samples = samples;
                        }
                        else if (config.entity == "subscriber")
                        {
                            config.sub_config.samples = samples;
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSE, "entity not specified for --sample argument");
                            print_help(EXIT_FAILURE);

                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSE, "invalid sample argument for " + arg);
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSE, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
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

};

#endif // _FASTDDS_HELLO_WORLD_CLI_PARSER_HPP_
