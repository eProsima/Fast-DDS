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

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/BuiltinTransports.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

#ifndef FASTDDS_EXAMPLES_CPP_CONFIGURATION__CLIPARSER_HPP
#define FASTDDS_EXAMPLES_CPP_CONFIGURATION__CLIPARSER_HPP

namespace eprosima {
namespace fastdds {
namespace examples {
namespace configuration {

using namespace eprosima::fastdds::dds;
using dds::Log;

class CLIParser
{
    //! Entity configuration structure (shared for both publisher and subscriber applications)
    struct entity_config
    {
        bool disable_positive_ack = false;
        uint8_t ttl = 1;
        uint16_t samples = 0;
        uint32_t domain = 0;
        int32_t history_depth = 1;
        int32_t max_samples = 5000;
        int32_t max_instances = 10;
        int32_t max_samples_per_instance = 400;
        uint32_t deadline = 0;
        uint32_t lifespan = 0;
        uint32_t liveliness_lease = 0;
        uint32_t liveliness_assert = 0;
        std::string partitions = "";
        std::string profile_participant = "";
        std::string topic_name = "configuration_topic";
        eprosima::fastdds::rtps::BuiltinTransports transport = eprosima::fastdds::rtps::BuiltinTransports::DEFAULT;
        DurabilityQosPolicyKind durability = DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS;
        HistoryQosPolicyKind history_kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
        LivelinessQosPolicyKind liveliness = LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
        OwnershipQosPolicyKind ownership = OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS;
        ReliabilityQosPolicyKind reliability = ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
    };

public:

    CLIParser() = delete;

    //! Entity kind enumeration
    enum class EntityKind : uint8_t
    {
        PUBLISHER,
        SUBSCRIBER,
        UNDEFINED
    };

    //! Publisher application configuration structure
    struct publisher_config : public entity_config
    {
        uint16_t wait = 0;
        uint32_t ack_keep_duration = 0;
        uint32_t interval = 100;
        uint32_t msg_size = 10;
        uint32_t ownership_strength = 0;
        std::string profile_writer = "";
        PublishModeQosPolicyKind publish_mode = PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE;
    };

    //! Subscriber application configuration structure
    struct subscriber_config : public entity_config
    {
        std::string profile_reader = "";
    };

    //! Configuration structure for the application
    struct configuration_config
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
        std::cout << "Usage: configuration <entity> [options]"                                          << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Entities:"                                                                        << std::endl;
        std::cout << "  publisher                           Run a publisher entity"                     << std::endl;
        std::cout << "  subscriber                          Run a subscriber entity"                    << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Common options:"                                                                  << std::endl;
        std::cout << "      --deadline <num>                Set deadline period in milliseconds"        << std::endl;
        std::cout << "                                      [1 <= <num> <= 4294967]"                    << std::endl;
        std::cout << "                                      (Default: unlimited)"                       << std::endl;
        std::cout << "      --disable-positive-ack          Disables positive acks"                     << std::endl;
        std::cout << "                                      (Default: positive acks enabled)"           << std::endl;
        std::cout << "  -d <num>, --domain <num>            Domain ID number [0 <= <num> <= 232]"       << std::endl;
        std::cout << "                                      (Default: 0)"                               << std::endl;
        std::cout << "  -h, --help                          Print this help message"                    << std::endl;
        std::cout << "      --keep-all                      Set History QoS as keep all"                << std::endl;
        std::cout << "                                      (Default: keep last 1)"                     << std::endl;
        std::cout << "  -k <num>, --keep-last <num>         Set History QoS as keep last <num>"         << std::endl;
        std::cout << "                                      [1 <= <num> <= 2147483647]"                 << std::endl;
        std::cout << "                                      (Default: keep last 1)"                     << std::endl;
        std::cout << "      --lifespan <num>                Set Lifespan QoS as <num> milliseconds"     << std::endl;
        std::cout << "                                      [1 <= <num> <= 4294967]"                    << std::endl;
        std::cout << "                                      (Default: unlimited)"                       << std::endl;
        std::cout << "  -l <num>, --liveliness <num>        Set liveliness lease duration in "          << std::endl;
        std::cout << "                                      milliseconds"                               << std::endl;
        std::cout << "                                      [1 <= <num> <= 4294967]"                    << std::endl;
        std::cout << "                                      (Default: unlimited)"                       << std::endl;
        std::cout << "      --liveliness-assert <num>       Set liveliness assert period in "           << std::endl;
        std::cout << "                                      milliseconds"                               << std::endl;
        std::cout << "                                      [1 <= <num> <= 4294967]"                    << std::endl;
        std::cout << "                                      (Default: unlimited)"                       << std::endl;
        std::cout << "      --liveliness-kind <str>         Set liveliness kind:"                       << std::endl;
        std::cout << "                                       · AUTOMATIC"                               << std::endl;
        std::cout << "                                       · MANUAL_BY_PARTICIPANT"                   << std::endl;
        std::cout << "                                       · MANUAL_BY_TOPIC"                         << std::endl;
        std::cout << "                                      (Default: AUTOMATIC)"                       << std::endl;
        std::cout << "      --max-samples <num>             Set ResourceLimitsQoS max_samples value"    << std::endl;
        std::cout << "                                      [0 <= <num> <= 2147483647]"                 << std::endl;
        std::cout << "                                      (Default: 5000)"                            << std::endl;
        std::cout << "      --max-samples-per-instance <num>  Set History QoS max_samples_per_instance" << std::endl;
        std::cout << "                                      value"                                      << std::endl;
        std::cout << "                                      [0 <= <num> <= 2147483647]"                 << std::endl;
        std::cout << "                                      (Default: 400)"                             << std::endl;
        std::cout << "      --max-instances <num>           Set ResourceLimitsQoS max_instances value"  << std::endl;
        std::cout << "                                      [0 <= <num> <= 2147483647]"                 << std::endl;
        std::cout << "                                      (Default: 10)"                              << std::endl;
        std::cout << "  -n <str>, --name <str>              Custom topic name"                          << std::endl;
        std::cout << "                                      (Default: configuration_topic)"             << std::endl;
        std::cout << "  -o, --ownership                     Use Topic with exclusive ownership."        << std::endl;
        std::cout << "                                      (Default: shared ownership)"                << std::endl;
        std::cout << "  -p <str>, --partition <str>         Partitions to match, separated by ';'"      << std::endl;
        std::cout << "                                      Single or double quotes (' or \") required" << std::endl;
        std::cout << "                                      for multiple partitions. No partitions"     << std::endl;
        std::cout << "                                      used if empty string ('') provided"         << std::endl;
        std::cout << "                                      (Default: '')"                              << std::endl;
        std::cout << "      --profile-participant <str>     Profile name from already exported"         << std::endl;
        std::cout << "                                      XML file to configure DomainParticipant"    << std::endl;
        std::cout << "  -r, --reliable                      Set Reliability QoS as reliable"            << std::endl;
        std::cout << "                                      (Default: best effort)"                     << std::endl;
        std::cout << "  -s <num>, --samples <num>           Number of samples to send or receive"       << std::endl;
        std::cout << "                                      [0 <= <num> <= 65535]"                      << std::endl;
        std::cout << "                                      (Default: 0 [unlimited])"                   << std::endl;
        std::cout << "      --transient-local               Set Durability QoS as transient local"      << std::endl;
        std::cout << "                                      (Default: volatile)"                        << std::endl;
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
        std::cout << "      --ack-keep-duration <num>       Set acknowledgment keep duration in"        << std::endl;
        std::cout << "                                      milliseconds (DisablePositiveAckQoS)"       << std::endl;
        std::cout << "                                      [1 <= <num> <= 4294967]"                    << std::endl;
        std::cout << "                                      (Default: unlimited)"                       << std::endl;
        std::cout << "  -a, --async                         Asynchronous publish mode"                  << std::endl;
        std::cout << "                                      (Default: synchronous)"                     << std::endl;
        std::cout << "  -i <num>, --interval <num>          Time between samples in milliseconds"       << std::endl;
        std::cout << "                                      [1 <= <num> <= 4294967]"                    << std::endl;
        std::cout << "                                      (Default: 100)"                             << std::endl;
        std::cout << "  -m <num>, --msg-size <num>          Size in bytes of the data to be sent"       << std::endl;
        std::cout << "                                      (Default: 10)"                              << std::endl;
        std::cout << "      --ownership-strength <num>      Set <num> as publisher ownership strength." << std::endl;
        std::cout << "                                      This flag forces the exclusive ownership"   << std::endl;
        std::cout << "                                      configuration in the publisher ('-o' flag)" << std::endl;
        std::cout << "                                      [0 <= <num> <= 4294967295]"                 << std::endl;
        std::cout << "                                      (Default: 0 [unused, shared ownership])"    << std::endl;
        std::cout << "      --profile-writer <str>          Profile name from already exported"         << std::endl;
        std::cout << "                                      XML file to configure DataWriter"           << std::endl;
        std::cout << "  -w <num>, --wait <num>              Number of matched subscribers required to"  << std::endl;
        std::cout << "                                      start publishing"                           << std::endl;
        std::cout << "                                      [0 <= <num> <= 4294967]"                    << std::endl;
        std::cout << "                                      (Default: 0 [does not wait])"               << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Subscriber options:"                                                              << std::endl;
        std::cout << "      --profile-reader <str>          Profile name from already exported"         << std::endl;
        std::cout << "                                      XML file to configure DataReader"           << std::endl;
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
    static configuration_config parse_cli_options(
            int argc,
            char* argv[])
    {
        configuration_config config;

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
                            throw std::out_of_range("domain argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing ttl argument");
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
            else if (arg == "-s" || arg == "--samples")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input < std::numeric_limits<uint16_t>::min() ||
                                input > std::numeric_limits<uint16_t>::max())
                        {
                            throw std::out_of_range("sample argument " + std::string(
                                              argv[i]) + " out of range [0, 65535].");
                        }
                        else
                        {
                            config.pub_config.samples = static_cast<uint16_t>(input);
                            config.sub_config.samples = static_cast<uint16_t>(input);
                        }
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
            else if (arg == "--profile-participant")
            {
                if (++i < argc)
                {
                    config.pub_config.profile_participant = argv[i];
                    config.sub_config.profile_participant = argv[i];
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing profile-participant argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--profile-reader")
            {
                if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                {
                    if (++i < argc)
                    {
                        config.sub_config.profile_reader = argv[i];
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing profile-reader argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "profile-reader argument is only valid for subscriber entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--profile-writer")
            {
                if (config.entity == CLIParser::EntityKind::PUBLISHER)
                {
                    if (++i < argc)
                    {
                        config.pub_config.profile_writer = argv[i];
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing profile-writer argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "profile-writer argument is only valid for publisher entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-r" || arg == "--reliable")
            {
                config.pub_config.reliability = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
                config.sub_config.reliability = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
            }
            else if (arg == "--transient-local")
            {
                config.pub_config.durability = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
                config.sub_config.durability = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

            }
            else if (arg == "--keep-all")
            {
                config.pub_config.history_kind = HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
                config.sub_config.history_kind = HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
                config.pub_config.history_depth = -1;
                config.sub_config.history_depth = -1;
            }
            else if (arg == "-k" || arg == "--keep-last")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input < 1 || input > std::numeric_limits<int32_t>::max())
                        {
                            throw std::out_of_range("keep-last depth argument " + std::string(
                                              argv[i]) + " out of range [1, 2147483647].");
                        }
                        else
                        {
                            config.pub_config.history_depth = static_cast<int32_t>(input);
                            config.sub_config.history_depth = static_cast<int32_t>(input);
                            config.pub_config.history_kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
                            config.sub_config.history_kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid keep-last argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing keep-last argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--max-samples")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input > std::numeric_limits<int32_t>::max())
                        {
                            throw std::out_of_range("max-samples argument " + std::string(
                                              argv[i]) + " out of range [1, 2147483647].");
                        }
                        else
                        {
                            config.pub_config.max_samples = static_cast<int32_t>(input);
                            config.sub_config.max_samples = static_cast<int32_t>(input);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid max-samples argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing max-samples argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--max-instances")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input > std::numeric_limits<int32_t>::max())
                        {
                            throw std::out_of_range("max-instances argument " + std::string(
                                              argv[i]) + " out of range [1, 2147483647].");
                        }
                        else
                        {
                            config.pub_config.max_instances = static_cast<int32_t>(input);
                            config.sub_config.max_instances = static_cast<int32_t>(input);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid max-instances argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing max-instances argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--max-samples-per-instance")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input > std::numeric_limits<int32_t>::max())
                        {
                            throw std::out_of_range("max-samples-per-instance argument " + std::string(
                                              argv[i]) + " out of range [1, 2147483647].");
                        }
                        else
                        {
                            config.pub_config.max_samples_per_instance = static_cast<int32_t>(input);
                            config.sub_config.max_samples_per_instance = static_cast<int32_t>(input);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid max-samples-per-instance argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing max-samples-per-instance argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--deadline")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input < 1 || static_cast<long>(input) > static_cast<long>(max_duration))
                        {
                            throw std::out_of_range("deadline argument " + std::string(
                                              argv[i]) + " out of range [1, " + std::to_string(
                                              max_duration) + "].");
                        }
                        else
                        {
                            config.pub_config.deadline = static_cast<uint32_t>(input);
                            config.sub_config.deadline = static_cast<uint32_t>(input);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid deadline argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing deadline argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--disable-positive-ack")
            {
                config.pub_config.disable_positive_ack = true;
                config.sub_config.disable_positive_ack = true;
            }
            else if (arg == "--lifespan")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input < 1 || static_cast<long>(input) > static_cast<long>(max_duration))
                        {
                            throw std::out_of_range("lifespan argument " + std::string(
                                              argv[i]) + " out of range [1, " + std::to_string(
                                              max_duration) + "].");
                        }
                        else
                        {
                            config.pub_config.lifespan = static_cast<uint32_t>(input);
                            config.sub_config.lifespan = static_cast<uint32_t>(input);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid lifespan argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing lifespan argument");
                }
            }
            else if (arg == "-l" || arg == "--liveliness")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input < 1 || static_cast<long>(input) > static_cast<long>(max_duration))
                        {
                            throw std::out_of_range("liveliness argument " + std::string(
                                              argv[i]) + " out of range [1, " + std::to_string(
                                              max_duration) + "].");
                        }
                        else
                        {
                            config.pub_config.liveliness_lease = static_cast<uint32_t>(input);
                            config.sub_config.liveliness_lease = static_cast<uint32_t>(input);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid liveliness argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing liveliness argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--liveliness-kind")
            {
                if (++i < argc)
                {
                    std::string kind = argv[i];
                    if (kind == "AUTOMATIC")
                    {
                        config.pub_config.liveliness = LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
                        config.sub_config.liveliness = LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
                    }
                    else if (kind == "MANUAL_BY_PARTICIPANT")
                    {
                        config.pub_config.liveliness = LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
                        config.sub_config.liveliness = LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
                    }
                    else if (kind == "MANUAL_BY_TOPIC")
                    {
                        config.pub_config.liveliness = LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS;
                        config.sub_config.liveliness = LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS;
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing liveliness-kind argument " + kind);
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing liveliness-kind argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--liveliness-assert")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input < 1 || static_cast<long>(input) > static_cast<long>(max_duration))
                        {
                            throw std::out_of_range("liveliness-assert argument " + std::string(
                                              argv[i]) + " out of range [1, " + std::to_string(
                                              max_duration) + "].");
                        }
                        else
                        {
                            config.pub_config.liveliness_assert = static_cast<uint32_t>(input);
                            config.sub_config.liveliness_assert = static_cast<uint32_t>(input);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid liveliness-assert argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing liveliness-assert argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-o" || arg == "--ownership")
            {
                config.pub_config.ownership = OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS;
                config.sub_config.ownership = OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS;
            }
            else if (arg == "--partition")
            {
                if (++i < argc)
                {
                    config.pub_config.partitions = argv[i];
                    config.sub_config.partitions = argv[i];
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing partition argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-w" || arg == "--wait")
            {
                if (config.entity == CLIParser::EntityKind::PUBLISHER)
                {
                    if (++i < argc)
                    {
                        try
                        {
                            int input = std::stoi(argv[i]);
                            if (input < 0 || static_cast<long>(input) > static_cast<long>(max_duration))
                            {
                                throw std::out_of_range("wait argument " + std::string(
                                                  argv[i]) + " out of range [0, " + std::to_string(
                                                  max_duration) + "].");
                            }
                            else
                            {
                                config.pub_config.wait = static_cast<uint16_t>(input);
                            }
                        }
                        catch (const std::invalid_argument& e)
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid wait argument " + std::string(
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
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing wait argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "wait argument is only valid for publisher entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-m" || arg == "--msg-size")
            {
                if (config.entity == CLIParser::EntityKind::PUBLISHER)
                {
                    if (++i < argc)
                    {
                        try
                        {
                            int input = std::stoi(argv[i]);
                            if (input < 1 ||
                                    static_cast<long>(input) >
                                    static_cast<long>(std::numeric_limits<uint32_t>::max()))
                            {
                                throw std::out_of_range("msg-size argument " + std::string(
                                                  argv[i]) + " out of range [1, " +
                                              std::to_string(std::numeric_limits<uint32_t>::max()) + "].");
                            }
                            else
                            {
                                config.pub_config.msg_size = static_cast<uint32_t>(input);
                            }
                        }
                        catch (const std::invalid_argument& e)
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid msg-size argument " + std::string(
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
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing msg-size argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "msg-size argument is only valid for publisher entity");
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
                                config.pub_config.interval = static_cast<uint32_t>(input);
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
            else if (arg == "-a" || arg == "--async")
            {
                if (config.entity == CLIParser::EntityKind::PUBLISHER)
                {
                    config.pub_config.publish_mode = PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "async argument is only valid for publisher entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--ack-keep-duration")
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
                                throw std::out_of_range("ack-keep-duration argument " + std::string(
                                                  argv[i]) + " out of range [1, " + std::to_string(
                                                  max_duration) + "].");
                            }
                            else
                            {
                                config.pub_config.ack_keep_duration = static_cast<uint32_t>(input);
                            }
                        }
                        catch (const std::invalid_argument& e)
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid ack-keep-duration argument " + std::string(
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
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing ack-keep-duration argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER,
                            "ack-keep-duration argument is only valid for publisher entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--ownership-strength")
            {
                if (config.entity == CLIParser::EntityKind::PUBLISHER)
                {
                    if (++i < argc)
                    {
                        try
                        {
                            int input = std::stoi(argv[i]);
                            if (static_cast<long>(input) < static_cast<long>(std::numeric_limits<uint32_t>::min()) ||
                                    static_cast<long>(input) > static_cast<long>(std::numeric_limits<uint32_t>::max()))
                            {
                                throw std::out_of_range("ownership strength argument " + std::string(
                                                  argv[i]) + " out of range [0, " +
                                              std::to_string(std::numeric_limits<uint32_t>::max()) + "].");
                            }
                            else
                            {
                                config.pub_config.ownership = OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS;
                                config.pub_config.ownership_strength = static_cast<uint32_t>(input);
                            }
                        }
                        catch (const std::invalid_argument& e)
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid ownership strength argument " + std::string(
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
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing ownership strength argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "ownership strength argument is only valid for publisher entity");
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

} // namespace configuration
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_CONFIGURATION__CLIPARSER_HPP
