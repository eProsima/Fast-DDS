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

#ifndef FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__CLIPARSER_HPP
#define FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__CLIPARSER_HPP

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <thread>

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace examples {
namespace content_filter {

using dds::Log;

class CLIParser
{
public:

    CLIParser() = delete;

    //! Entity kind enumeration
    enum class EntityKind : uint8_t
    {
        PUBLISHER,
        SUBSCRIBER,
        UNDEFINED
    };

    //! Filter kind enumeration
    enum class FilterKind : uint8_t
    {
        DEFAULT,
        CUSTOM,
        NONE
    };

    //! Publisher configuration structure
    struct publisher_config
    {
        uint16_t samples = 0;
        uint16_t interval = 100;
        uint16_t max_reader_filters = 32;
        bool reliable{false};
        bool transient_local{false};
    };

    //! Subscriber application configuration structure
    struct subscriber_config
    {
        uint16_t samples = 0;
        CLIParser::FilterKind filter_kind = CLIParser::FilterKind::DEFAULT;
        std::string filter_expression = "index between %0 and %1";
        std::string upper_bound = "9";
        std::string lower_bound = "5";
        bool reliable{false};
        bool transient_local{false};
    };

    //! Configuration structure for the application
    struct content_filter_config
    {
        CLIParser::EntityKind entity = CLIParser::EntityKind::UNDEFINED;
        publisher_config pub_config;
        subscriber_config sub_config;
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
        std::cout << "Usage: content_filter <entity> [options]"                                         << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Entities:"                                                                        << std::endl;
        std::cout << " publisher                            Run a publisher entity"                     << std::endl;
        std::cout << " subscriber                           Run a subscriber entity"                    << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Common options:"                                                                  << std::endl;
        std::cout << " -h, --help                           Print this help message"                    << std::endl;
        std::cout << "     --reliable                       Set Reliability QoS as reliable"            << std::endl;
        std::cout << "                                      (Default: Best effort)"                     << std::endl;
        std::cout << "     --transient-local                Set Durability QoS as transient local"      << std::endl;
        std::cout << "                                      (Default: Volatile)"                        << std::endl;
        std::cout << " -s <num>, --samples <num>            Number of samples to send/receive"          << std::endl;
        std::cout << "Publisher options:"                                                               << std::endl;
        std::cout << "                                      (Default: 0 [unlimited])"                   << std::endl;
        std::cout << " -i <num>, --interval <num>           Time between samples in milliseconds"       << std::endl;
        std::cout << "           --reader-filters <num>     Set the maximum number of readers that the" << std::endl;
        std::cout << "                                      writer evaluates for applying the filter"   << std::endl;
        std::cout << "                                      (Default: 32)"                              << std::endl;
        std::cout << "Subscriber options:"                                                              << std::endl;
        std::cout << " --filter-kind <default/custom/none>  Kind of Content Filter to use"              << std::endl;
        std::cout << "                                      (Default: default SQL filter)"              << std::endl;
        std::cout << " --filter-expression <string>         Filter Expression of default SQL filter"    << std::endl;
        std::cout << "                                      (Default: \"index between %0 and %1\","     << std::endl;
        std::cout << "                                      where %0 and %1 are the indeces of the"     << std::endl;
        std::cout << "                                      parameters, i.e. lb and ub)"                << std::endl;
        std::cout << " -lb <num>, --lower-bound <num>       Lower bound of the data range to filter."   << std::endl;
        std::cout << "                                      (Default: 5)"                               << std::endl;
        std::cout << " -up <num>, --upper-bound <num>       Upper bound of the data range to filter."   << std::endl;
        std::cout << "                                      (Default: 9)"                               << std::endl;
        std::exit(return_code);
    }

    /**
     * @brief Parse the command line options and return the configuration_config object
     *
     * @param argc number of arguments
     * @param argv array of arguments
     * @return configuration_config object with the parsed options
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static content_filter_config parse_cli_options(
            int argc,
            char* argv[])
    {
        content_filter_config config;

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
                        int input = std::stoi(argv[++i]);
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
            else if (arg == "-i" || arg == "--interval")
            {

                if (i + 1 < argc)
                {
                    int input = std::stoi(argv[++i]);
                    if (config.entity == CLIParser::EntityKind::PUBLISHER)
                    {
                        config.pub_config.interval = static_cast<uint16_t>(input);
                    }
                    else if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "interval option can only be used with the Publisher");
                        print_help(EXIT_FAILURE);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "entity not specified for --sample argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--reader-filters")
            {
                if (i + 1 < argc)
                {
                    if (config.entity == CLIParser::EntityKind::PUBLISHER)
                    {
                        config.pub_config.max_reader_filters = static_cast<uint16_t>(std::stoi(argv[++i]));
                    }
                    else if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "--reader-filters is only valid for publisher entity");
                        print_help(EXIT_FAILURE);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "entity not specified for --reader-filters argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--reliable")
            {
                config.pub_config.reliable = true;
                config.sub_config.reliable = true;
            }
            else if (arg == "--transient-local")
            {
                config.pub_config.transient_local = true;
                config.sub_config.transient_local = true;
            }
            else if (arg == "--filter-kind")
            {
                if (i + 1 < argc)
                {
                    if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                    {
                        std::string filter_type = argv[++i];
                        if (filter_type == "custom")
                        {
                            config.sub_config.filter_kind = CLIParser::FilterKind::CUSTOM;
                        }
                        else if (filter_type == "default")
                        {
                            config.sub_config.filter_kind = CLIParser::FilterKind::DEFAULT;
                        }
                        else if (filter_type == "none")
                        {
                            config.sub_config.filter_kind = CLIParser::FilterKind::NONE;
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "unknown --filter argument");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    else if (config.entity == CLIParser::EntityKind::PUBLISHER)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "--filter is only valid for subscriber entity");
                        print_help(EXIT_FAILURE);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "entity not specified for --filter argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--filter-expression")
            {
                if (i + 1 < argc)
                {
                    if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                    {
                        std::string filter_expression = argv[++i];
                        config.sub_config.filter_expression = filter_expression;

                    }
                    else if (config.entity == CLIParser::EntityKind::PUBLISHER)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "--filter is only valid for subscriber entity");
                        print_help(EXIT_FAILURE);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "entity not specified for --filter argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }

            else if (arg == "-lb" || arg == "--lower-bound")
            {
                if (i + 1 < argc)
                {
                    if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                    {
                        config.sub_config.lower_bound = argv[++i];
                    }
                    else if (config.entity == CLIParser::EntityKind::PUBLISHER)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "lower-bound option can only be used with the Subscriber");
                        print_help(EXIT_FAILURE);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "entity not specified for --filter argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-ub" || arg == "--upper-bound")
            {
                if (i + 1 < argc)
                {
                    if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                    {
                        config.sub_config.upper_bound = argv[++i];
                    }
                    else if (config.entity == CLIParser::EntityKind::PUBLISHER)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "upper-bound option can only be used with the Subscriber");
                        print_help(EXIT_FAILURE);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "entity not specified for --filter argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
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
            case EntityKind::PUBLISHER:
                return "Publisher";
            case EntityKind::SUBSCRIBER:
                return "Subscriber";
            case EntityKind::UNDEFINED:
            default:
                return "Undefined entity";
        }
    }

};

} // namespace content_filter
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_CONTENT_FILTER__CLIPARSER_HPP
