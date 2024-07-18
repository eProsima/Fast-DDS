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
#include <fastdds/rtps/flowcontrol/FlowControllerSchedulerPolicy.hpp>

#ifndef FASTDDS_FLOW_CONTROL_CLI_PARSER_HPP
#define FASTDDS_FLOW_CONTROL_CLI_PARSER_HPP

namespace eprosima {
namespace fastdds {
namespace examples {
namespace flow_control {

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

    //! Configuration structure for the application
    struct flow_control_config
    {
        CLIParser::EntityKind entity = CLIParser::EntityKind::UNDEFINED;
        uint16_t samples = 0;
        uint64_t period = 500;
        int32_t max_bytes_per_period = 300000;
        std::string bandwidth = "0";
        std::string priority = "10";
        rtps::FlowControllerSchedulerPolicy scheduler = rtps::FlowControllerSchedulerPolicy::FIFO;
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
        std::cout << "Usage: flow_control <entity> [options]"                                   << std::endl;
        std::cout << ""                                                                         << std::endl;
        std::cout << "Entities:"                                                                << std::endl;
        std::cout << "  publisher                       Run a publisher entity"                 << std::endl;
        std::cout << "  subscriber                      Run a subscriber entity"                << std::endl;
        std::cout << ""                                                                         << std::endl;
        std::cout << "Common options:"                                                          << std::endl;
        std::cout << ""                                                                         << std::endl;
        std::cout << "  -h, --help                      Print this help message"                << std::endl;
        std::cout << "  -s <num>, --samples <num>       Number of samples to send or receive"   << std::endl;
        std::cout << "                                  [0 <= <num> <= 65535]"                  << std::endl;
        std::cout << "                                  (Default: 0 [unlimited])"               << std::endl;
        std::cout << ""                                                                         << std::endl;
        std::cout << "Slow Publisher options:"                                                  << std::endl;
        std::cout << "  --max-bytes <num>               Maximum number of bytes to be sent"     << std::endl;
        std::cout << "                                  per period [0 <= <num> <= 2147483647]"  << std::endl;
        std::cout << "                                  0 = no limits."                         << std::endl;
        std::cout << "                                  (Default: 300kB)"                       << std::endl;
        std::cout << "  --period <num>                  Period of time (ms) in which the"       << std::endl;
        std::cout << "                                  flow controller is allowed"             << std::endl;
        std::cout << "                                  to send the max bytes per period."      << std::endl;
        std::cout << "                                  (Default: 1000ms)"                      << std::endl;
        std::cout << "  --scheduler <string>            Scheduler policy [FIFO, ROUND-ROBIN,"   << std::endl;
        std::cout << "                                  HIGH-PRIORITY, PRIORITY-RESERVATION]"   << std::endl;
        std::cout << "                                  (Default: FIFO)"                        << std::endl;
        std::cout << "  --bandwidth <string>            Bandwidth that the DataWriter can"      << std::endl;
        std::cout << "                                  request for PRIORITY_WITH_RESERVATION"  << std::endl;
        std::cout << "                                  express as a percentage of the total "  << std::endl;
        std::cout << "                                  flow controller limit: [0; 100]"        << std::endl;
        std::cout << "                                  (Default: 0)"                           << std::endl;
        std::cout << "  --priority <string>             Priority for HIGH_PRIORITY and"         << std::endl;
        std::cout << "                                  PRIORITY_WITH_RESERVATION schedulers"   << std::endl;
        std::cout << "                                  [-10 (highest) ; 10 (lowest)]"          << std::endl;
        std::cout << "                                  (Default: 10)"                          << std::endl;
        std::exit(return_code);
    }

    /**
     * @brief Parse the command line options and return the flow_control_config object
     *
     * @param argc number of arguments
     * @param argv array of arguments
     * @return flow_control_config object with the parsed options
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static flow_control_config parse_cli_options(
            int argc,
            char* argv[])
    {
        flow_control_config config;

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
                if (++i < argc)
                {
                    try
                    {
                        unsigned long input = std::stoul(argv[i]);
                        if (input > std::numeric_limits<uint16_t>::max())
                        {
                            throw std::out_of_range("sample argument " + std::string(
                                              argv[i]) + " out of range [0, 65535].");
                        }
                        config.samples = static_cast<uint16_t>(input);
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid sample argument " + std::string(
                                    argv[i]) + ": " + std::string(e.what()));
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, std::string(e.what()));
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing samples argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--max-bytes")
            {
                if (config.entity == CLIParser::EntityKind::PUBLISHER)
                {
                    if (++i < argc)
                    {
                        try
                        {
                            unsigned long input = std::stoul(argv[i]);

                            if (input > static_cast<unsigned long>(std::numeric_limits<int32_t>::max()))
                            {
                                throw std::out_of_range("max-bytes argument " + std::string(
                                                  argv[i]) + " out of range [0, 2147483647].");
                            }
                            config.max_bytes_per_period = static_cast<int32_t>(input);
                        }
                        catch (const std::invalid_argument& e)
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid max-bytes argument " + std::string(
                                        argv[i]) + ": " + std::string(e.what()));
                            print_help(EXIT_FAILURE);
                        }
                        catch (const std::out_of_range& e)
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, std::string(e.what()));
                            print_help(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing max-bytes argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "max-bytes argument is only valid for publisher entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--period")
            {
                if (config.entity != CLIParser::EntityKind::PUBLISHER)
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "period argument is only valid for publisher entity");
                    print_help(EXIT_FAILURE);
                }

                if (++i < argc)
                {
                    try
                    {
                        uint64_t input = std::stoull(argv[i]);
                        if (input > std::numeric_limits<uint64_t>::max())
                        {
                            throw std::out_of_range("period argument " + std::string(
                                              argv[i]) + " out of range.");
                        }
                        config.period = input;
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid period argument " + std::string(
                                    argv[i]) + ": " + std::string(e.what()));
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, std::string(e.what()));
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing period argument");
                    print_help(EXIT_FAILURE);
                }

            }
            else if (arg == "--scheduler")
            {
                if (++i < argc)
                {
                    std::string scheduler = argv[i];
                    if (config.entity == CLIParser::EntityKind::PUBLISHER)
                    {
                        if (scheduler == "FIFO")
                        {
                            config.scheduler = rtps::FlowControllerSchedulerPolicy::FIFO;
                        }
                        else if (scheduler == "ROUND-ROBIN")
                        {
                            config.scheduler = rtps::FlowControllerSchedulerPolicy::ROUND_ROBIN;
                        }
                        else if (scheduler == "HIGH-PRIORITY")
                        {
                            config.scheduler = rtps::FlowControllerSchedulerPolicy::HIGH_PRIORITY;
                        }
                        else if (scheduler == "PRIORITY-RESERVATION")
                        {
                            config.scheduler = rtps::FlowControllerSchedulerPolicy::PRIORITY_WITH_RESERVATION;
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "unknown argument ");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "scheduler argument is only valid for publisher entity");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--bandwidth")
            {
                if (++i < argc)
                {
                    try
                    {
                        unsigned long input = std::stoul(argv[i]);
                        if (input > 100)
                        {
                            throw std::out_of_range("bandwidth argument " + std::string(
                                              argv[i]) + " out of range.");
                        }

                        if (config.entity == CLIParser::EntityKind::PUBLISHER)
                        {
                            config.bandwidth = argv[i];
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "bandwidth argument is only valid for publisher entity");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid bandwidth argument " + std::string(
                                    argv[i]) + ": " + std::string(e.what()));
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, std::string(e.what()));
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--priority")
            {
                if (++i < argc)
                {
                    try
                    {
                        unsigned long input = std::stoul(argv[i]);
                        if (input > 10)
                        {
                            throw std::out_of_range("priority argument " + std::string(
                                              argv[i]) + " out of range.");
                        }

                        if (config.entity == CLIParser::EntityKind::PUBLISHER)
                        {
                            config.priority = argv[i];
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "priority argument is only valid for publisher entity");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid priority argument " + std::string(
                                    argv[i]) + ": " + std::string(e.what()));
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, std::string(e.what()));
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
                EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing argument: " + arg);
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

} // namespace flow_control
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_FLOW_CONTROL_CLI_PARSER_HPP_
