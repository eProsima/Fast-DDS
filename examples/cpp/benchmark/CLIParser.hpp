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
#include <string>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/BuiltinTransports.hpp>

#ifndef FASTDDS_EXAMPLES_CPP_BENCHMARK__CLIPARSER_HPP
#define FASTDDS_EXAMPLES_CPP_BENCHMARK__CLIPARSER_HPP

namespace eprosima {
namespace fastdds {
namespace examples {
namespace benchmark {

using namespace eprosima::fastdds::dds;
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

    //! Message size kind enumeration
    enum class MsgSizeKind : uint8_t
    {
        NONE,
        SMALL,
        MEDIUM,
        BIG
    };

    //! Entity benchmark structure (shared for both publisher and subscriber applications)
    struct entity_config
    {
        uint8_t ttl = 1;
        uint32_t domain = 0;
        std::string topic_name = "benchmark_topic";
        uint16_t samples = 0;
        eprosima::fastdds::rtps::BuiltinTransports transport = eprosima::fastdds::rtps::BuiltinTransports::DEFAULT;
        ReliabilityQosPolicyKind reliability = ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
        DurabilityQosPolicyKind durability = DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS;
        CLIParser::MsgSizeKind msg_size = CLIParser::MsgSizeKind::NONE;
    };

    //! Publisher application benchmark structure
    struct publisher_config : public entity_config
    {
        uint16_t interval = 100;
        uint16_t timeout = 10000;
    };

    //! Subscriber application benchmark structure
    struct subscriber_config : public entity_config
    {
    };

    //! Benchmark structure for the application
    struct benchmark_config
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
        std::cout << "Usage: benchmark <entity> [options]"                                              << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Entities:"                                                                        << std::endl;
        std::cout << "  publisher                           Run a publisher entity"                     << std::endl;
        std::cout << "  subscriber                          Run a subscriber entity"                    << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Common options:"                                                                  << std::endl;
        std::cout << "  -h, --help                          Print this help message"                    << std::endl;
        std::cout << "  -d <num>, --domain <num>            Domain ID number [0 <= <num> <= 232]"       << std::endl;
        std::cout << "                                      (Default: 0)"                               << std::endl;
        std::cout << "  -n <str>, --name <str>              Custom topic name"                          << std::endl;
        std::cout << "                                      (Default: benchmark_topic)"                 << std::endl;
        std::cout << "  -r, --reliable                      Set Reliability QoS as reliable"            << std::endl;
        std::cout << "                                      (Default: best effort)"                     << std::endl;
        std::cout << "  --transient-local                   Set Durability QoS as transient local"      << std::endl;
        std::cout << "                                      (Default: volatile)"                        << std::endl;
        std::cout << "  -m <num>, --msg-size <num>          Size of the message"                        << std::endl;
        std::cout << "                                       · NONE:    Only an int value"              << std::endl;
        std::cout << "                                       · SMALL:   int value + array of 16Kb"      << std::endl;
        std::cout << "                                       · MEDIUM:  int value + array of 512Kb"     << std::endl;
        std::cout << "                                       · BIG:     int value + array of 8Mb"       << std::endl;
        std::cout << "                                      (Default: NONE)"                            << std::endl;
        std::cout << "  -s <num>, --samples <num>           Number of samples to send/receive"          << std::endl;
        std::cout << "                                      If a value is given timeout is ignore"      << std::endl;
        std::cout << "                                      [0 <= <num> <= 65535]"                      << std::endl;
        std::cout << "                                      (Default: 0 [unlimited])"                   << std::endl;
        std::cout << "  -t <transp>, --transport <transp>   Select builtin transport <transp>:"         << std::endl;
        std::cout << "                                       · DEFAULT: SHM & UDPv4 (SHM prior UDP)"    << std::endl;
        std::cout << "                                       · SHM:     Shared Memory Transport only"   << std::endl;
        std::cout << "                                       · UDPv4:   UDP over IPv4 only"             << std::endl;
        std::cout << "                                       · LARGE_DATA: Large data mode"             << std::endl;
        std::cout << "                                         (refer to Fast DDS documentation)"       << std::endl;
        std::cout << "                                      (Default: DEFAULT)"                         << std::endl;
        std::cout << "      --ttl <num>                     Number of multicast discovery Time To Live" << std::endl;
        std::cout << "                                      hops  [0 <= <num> <= 255]"                  << std::endl;
        std::cout << "                                      (Default: 1)"                               << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Publisher options:"                                                               << std::endl;
        std::cout << "  -i <num>, --interval <num>          Time between samples in milliseconds"       << std::endl;
        std::cout << "                                      [1 <= <num> <= 4294967]"                    << std::endl;
        std::cout << "                                      (Default: 100 [0.1s])"                      << std::endl;
        std::cout << "  -to <num>, --timeout <num>          Time running the example in milliseconds"   << std::endl;
        std::cout << "                                      [1 <= <num> <= 4294967]"                    << std::endl;
        std::cout << "                                      (Default: 10000 [10s])"                     << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::exit(return_code);
    }

    /**
     * @brief Parse the command line options and return the benchmark_config object
     *
     * @param argc number of arguments
     * @param argv array of arguments
     * @return benchmark_config object with the parsed options
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static benchmark_config parse_cli_options(
            int argc,
            char* argv[])
    {
        benchmark_config config;

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
        else if ( first_argument == "subscriber")
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

        // max value allowed taking into account that the input is receiving millisecond values
        uint32_t max_duration = static_cast<uint32_t>(floor(std::numeric_limits<uint32_t>::max() * 1e-3)); // = 4294967

        for (int i = 2; i < argc; ++i)
        {
            std::string arg = argv[i];

            if (arg == "-h" || arg == "--help")
            {
                print_help(EXIT_SUCCESS);
            }
            else if (arg == "-d" || arg == "--domain")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input < 0 || input > 232)
                        {
                            throw std::out_of_range("domain argument " + std::string(
                                              argv[i]) + " out of range [0, 232].");
                        }
                        else
                        {
                            config.pub_config.domain = static_cast<uint16_t>(input);
                            config.sub_config.domain = static_cast<uint16_t>(input);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid domain argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing domain argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-n" || arg == "--name")
            {
                if (++i < argc)
                {
                    config.pub_config.topic_name = argv[i];
                    config.sub_config.topic_name = argv[i];
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing name argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-r" || arg == "--reliable")
            {
                config.pub_config.reliability = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
                config.sub_config.reliability = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
            }
            else if (arg == "-m" || arg == "--msg-size")
            {
                if (++i < argc)
                {
                    std::string size = argv[i];
                    if (size == "NONE")
                    {
                        config.pub_config.msg_size = CLIParser::MsgSizeKind::NONE;
                        config.sub_config.msg_size = CLIParser::MsgSizeKind::NONE;
                    }
                    else if (size == "SMALL")
                    {
                        config.pub_config.msg_size = CLIParser::MsgSizeKind::SMALL;
                        config.sub_config.msg_size = CLIParser::MsgSizeKind::SMALL;
                    }
                    else if (size == "MEDIUM")
                    {
                        config.pub_config.msg_size = CLIParser::MsgSizeKind::MEDIUM;
                        config.sub_config.msg_size = CLIParser::MsgSizeKind::MEDIUM;
                    }
                    else if (size == "BIG")
                    {
                        config.pub_config.msg_size = CLIParser::MsgSizeKind::BIG;
                        config.sub_config.msg_size = CLIParser::MsgSizeKind::BIG;
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid msg_size argument " + std::string(
                                    argv[i]) + ": " + std::string("valid values are NONE, SMALL, MEDIUM, BIG"));
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing msg_size argument");
                    print_help(EXIT_FAILURE);
                }
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
            else if (arg == "-t" || arg == "--transport")
            {
                if (++i < argc)
                {
                    std::string transport = argv[i];
                    if (transport == "DEFAULT")
                    {
                        config.pub_config.transport = eprosima::fastdds::rtps::BuiltinTransports::DEFAULT;
                        config.sub_config.transport = eprosima::fastdds::rtps::BuiltinTransports::DEFAULT;
                    }
                    else if (transport == "SHM")
                    {
                        config.pub_config.transport = eprosima::fastdds::rtps::BuiltinTransports::SHM;
                        config.sub_config.transport = eprosima::fastdds::rtps::BuiltinTransports::SHM;
                    }
                    else if (transport == "UDPv4")
                    {
                        config.pub_config.transport = eprosima::fastdds::rtps::BuiltinTransports::UDPv4;
                        config.sub_config.transport = eprosima::fastdds::rtps::BuiltinTransports::UDPv4;
                    }
                    else if (transport == "LARGE_DATA")
                    {
                        config.pub_config.transport = eprosima::fastdds::rtps::BuiltinTransports::LARGE_DATA;
                        config.sub_config.transport = eprosima::fastdds::rtps::BuiltinTransports::LARGE_DATA;
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing transport argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing transport argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--ttl")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input < 0 || input > 255)
                        {
                            throw std::out_of_range("ttl argument " + std::string(
                                              argv[i]) + " out of range [0, 255].");
                        }
                        else
                        {
                            config.pub_config.ttl = static_cast<uint8_t>(input);
                            config.sub_config.ttl = static_cast<uint8_t>(input);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid ttl argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing ttl argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-i" || arg == "--interval")
            {
                if (config.entity == CLIParser::EntityKind::PUBLISHER)
                {
                    if (++i < argc)
                    {
                        try
                        {
                            int input = std::stoi(argv[i]);
                            if (input < 1 || static_cast<long>(input) > static_cast<long>(max_duration))
                            {
                                throw std::out_of_range("interval argument " + std::string(
                                                  argv[i]) + " out of range [1, " + std::to_string(
                                                  max_duration) + "].");
                            }
                            else
                            {
                                config.pub_config.interval = static_cast<uint16_t>(input);
                            }
                        }
                        catch (const std::invalid_argument& e)
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid interval argument " + std::string(
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
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing interval argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "interval argument is only valid for publisher entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-to" || arg == "--timeout")
            {
                if (config.entity == CLIParser::EntityKind::PUBLISHER)
                {
                    if (++i < argc)
                    {
                        try
                        {
                            int input = std::stoi(argv[i]);
                            if (input < 1 || static_cast<long>(input) > static_cast<long>(max_duration))
                            {
                                throw std::out_of_range("end argument " + std::string(
                                                  argv[i]) + " out of range [1, " + std::to_string(
                                                  max_duration) + "].");
                            }
                            else
                            {
                                config.pub_config.timeout = static_cast<uint16_t>(input);
                            }
                        }
                        catch (const std::invalid_argument& e)
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid end argument " + std::string(
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
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing end argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "end argument is only valid for publisher entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--transient-local")
            {
                config.pub_config.durability = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
                config.sub_config.durability = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

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

} // namespace benchmark
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_BENCHMARK__CLIPARSER_HPP
