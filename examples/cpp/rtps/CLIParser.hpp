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

#include <csignal>
#include <cstdlib>
#include <iostream>

#include <fastdds/dds/log/Log.hpp>

#ifndef FASTDDS_EXAMPLES_CPP_RTPS__CLIPARSER_HPP
#define FASTDDS_EXAMPLES_CPP_RTPS__CLIPARSER_HPP

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rtps {

using dds::Log;

class CLIParser
{
public:

    CLIParser() = delete;

    //! Entity kind enumeration
    enum class EntityKind : uint8_t
    {
        WRITER,
        READER,
        UNDEFINED
    };

    //! Configuration structure for the application
    struct rtps_config
    {
        CLIParser::EntityKind entity = CLIParser::EntityKind::UNDEFINED;
        uint16_t samples = 0;
        uint16_t matched = 1;
    };

    /**
     * @brief Print usage help message and exit with the given return code
     *
     * @param return_code return code to exit with
     *
     * @warning This method finishes the execution of the program with the input return code
     */
    static void print_help(
            uint8_t return_code)
    {
        std::cout << "Usage: rtps <entity> [options]"                                           << std::endl;
        std::cout << ""                                                                         << std::endl;
        std::cout << "Entities:"                                                                << std::endl;
        std::cout << "  writer                          Run a RTPS Writer entity"               << std::endl;
        std::cout << "  reader                          Run a RTPS Reader entity"               << std::endl;
        std::cout << ""                                                                         << std::endl;
        std::cout << "Common options:"                                                          << std::endl;
        std::cout << "  -h, --help                      Print this help message"                << std::endl;
        std::cout << "  -s <num>, --samples <num>       Number of samples to send or receive"   << std::endl;
        std::cout << "                                  [0 <= <num> <= 65535]"                  << std::endl;
        std::cout << "                                  (Default: 0 [unlimited])"               << std::endl;
        std::cout << "Writer options:"                                                          << std::endl;
        std::cout << "  -m, --matched                   Number of readers to match"             << std::endl;
        std::cout << "                                  before start publishing (Default: 1)"   << std::endl;
        std::exit(return_code);
    }

    /**
     * @brief Parse the command line options and return the rtps_config object
     *
     * @param argc number of arguments
     * @param argv array of arguments
     * @return rtps_config object with the parsed options
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static rtps_config parse_cli_options(
            int argc,
            char* argv[])
    {
        rtps_config config;

        if (argc < 2)
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER, "missing entity argument");
            print_help(EXIT_FAILURE);
        }

        std::string first_argument = argv[1];

        if (first_argument == "writer" )
        {
            config.entity = CLIParser::EntityKind::WRITER;
        }
        else if (first_argument == "reader")
        {
            config.entity = CLIParser::EntityKind::READER;
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
                        unsigned long input = std::stoul(argv[++i]);
                        if (static_cast<uint16_t>(input) > std::numeric_limits<std::uint16_t>::max())
                        {
                            throw std::out_of_range("sample argument out of range");
                        }

                        config.samples = static_cast<uint16_t>(input);
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid sample argument for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "sample argument out of range for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-m" || arg == "--matched")
            {
                try
                {
                    int input = std::stoi(argv[++i]);
                    if (input < std::numeric_limits<std::uint16_t>::min() ||
                            input > std::numeric_limits<std::uint16_t>::max())
                    {
                        throw std::out_of_range("matched argument out of range");
                    }
                    else
                    {
                        if (config.entity == CLIParser::EntityKind::WRITER)
                        {
                            config.matched = static_cast<uint16_t>(input);
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "matched can only be used with the writer entity");
                            print_help(EXIT_FAILURE);
                        }
                    }
                }
                catch (const std::invalid_argument& e)
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid sample argument for " + arg + ": " + e.what());
                    print_help(EXIT_FAILURE);
                }
                catch (const std::out_of_range& e)
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "sample argument out of range for " + arg + ": " + e.what());
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

    /**
     * @brief Parse the signal number into the signal name
     *
     * @param signum signal number
     * @return std::string signal name
     */
    static std::string parse_signal(
            const int& signum)
    {
        switch (signum)
        {
            case SIGINT:
                return "SIGINT";
            case SIGTERM:
                return "SIGTERM";
#ifndef _WIN32
            case SIGQUIT:
                return "SIGQUIT";
            case SIGHUP:
                return "SIGHUP";
#endif // _WIN32
            default:
                return "UNKNOWN SIGNAL";
        }
    }

    /**
     * @brief Parse the entity kind into std::string
     *
     * @param entity entity kind
     * @return std::string entity kind
     */
    static std::string parse_entity_kind(
            const EntityKind& entity)
    {
        switch (entity)
        {
            case EntityKind::WRITER:
                return "RTPS Writer";
            case EntityKind::READER:
                return "RTPS Reader";
            case EntityKind::UNDEFINED:
            default:
                return "Undefined entity";
        }
    }

};

} // namespace rtps
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_RTPS__CLIPARSER_HPP
