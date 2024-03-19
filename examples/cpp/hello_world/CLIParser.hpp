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

#include <fastdds/dds/log/Log.hpp>

#ifndef _FASTDDS_HELLO_WORLD_CLI_PARSER_HPP_
#define _FASTDDS_HELLO_WORLD_CLI_PARSER_HPP_

namespace eprosima {
namespace fastdds {
namespace examples {
namespace hello_world {

using dds::Log;

class CLIParser
{
public:

    CLIParser() = delete;

    enum EntityKind
    {
        PUBLISHER,
        SUBSCRIBER,
        UNDEFINED
    };

    struct publisher_config
    {
        uint16_t samples = 0;
    };

    struct subscriber_config : public publisher_config
    {
        bool use_waitset = false;
    };

    struct hello_world_config
    {
        CLIParser::EntityKind entity = CLIParser::EntityKind::UNDEFINED;
        publisher_config pub_config;
        subscriber_config sub_config;
    };

    static void print_help(
            uint8_t return_code)
    {
        std::cout << "Usage: hello_world <entity> [options]"                                    << std::endl;
        std::cout << ""                                                                         << std::endl;
        std::cout << "Entities:"                                                                << std::endl;
        std::cout << "  publisher                       Run a publisher entity"                 << std::endl;
        std::cout << "  subscriber                      Run a subscriber entity"                << std::endl;
        std::cout << ""                                                                         << std::endl;
        std::cout << "Common options:"                                                          << std::endl;
        std::cout << "  -h, --help                      Print this help message"                << std::endl;
        std::cout << "  -s <num>, --samples <num>       Number of samples to send or receive"   << std::endl;
        std::cout << "                                   [0 <= <num> <= 65535]"                 << std::endl;
        std::cout << "                                  (Default: 0 [unlimited])"               << std::endl;
        std::cout << "Subscriber options:"                                                      << std::endl;
        std::cout << "  -w, --waitset                   Use waitset read condition"             << std::endl;
        std::exit(return_code);
    }

    static hello_world_config parse_cli_options(
            int argc,
            char* argv[])
    {
        hello_world_config config;

        if (argc < 2)
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER, "missing entity argument");
            print_help(EXIT_FAILURE);
        }

        std::string first_argument = argv[1];

        if (first_argument == "publisher" )
        {
            config.entity = CLIParser::EntityKind::PUBLISHER;
        }
        else if (first_argument == "subscriber")
        {
            config.entity = CLIParser::EntityKind::SUBSCRIBER;
        }
        else if (first_argument == "-h" || first_argument == "--help")
        {
            print_help(EXIT_SUCCESS);
        }
        else
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing entity argument " + first_argument);
            print_help(EXIT_FAILURE);
        }

        for (int i = 2; i < argc; ++i)
        {
            std::string arg = argv[i];
            if (arg == "-h" || arg == "--help")
            {
                print_help(EXIT_SUCCESS);
            }
            else if (arg == "-s" || arg == "--samples")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        int16_t input = static_cast<int16_t>(std::stoi(argv[++i]));
                        if (input < std::numeric_limits<std::uint16_t>::min() ||
                                input > std::numeric_limits<std::uint16_t>::max())
                        {
                            throw std::out_of_range("sample argument out of range");
                        }
                        else
                        {
                            if (config.entity == CLIParser::EntityKind::PUBLISHER)
                            {
                                config.pub_config.samples = static_cast<uint16_t>(input);
                            }
                            else if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                            {
                                config.sub_config.samples = static_cast<uint16_t>(input);
                            }
                            else
                            {
                                EPROSIMA_LOG_ERROR(CLI_PARSER, "entity not specified for --sample argument");
                                print_help(EXIT_FAILURE);
                            }
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid sample argument for " + arg);
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "sample argument out of range for " + arg);
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-w" || arg == "--waitset")
            {
                if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                {
                    config.sub_config.use_waitset = true;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "waitset can only be used with the subscriber entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(CLI_PARSER, "unknown option " + arg);
                print_help(EXIT_FAILURE);
            }
        }

        return config;
    }

};

} // namespace hello_world
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_HELLO_WORLD_CLI_PARSER_HPP_
