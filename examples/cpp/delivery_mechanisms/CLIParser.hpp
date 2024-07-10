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
#include <fastdds/rtps/attributes/BuiltinTransports.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

#ifndef FASTDDS_EXAMPLES_CPP_DELIVERY_MECHANISMS__CLIPARSER_HPP
#define FASTDDS_EXAMPLES_CPP_DELIVERY_MECHANISMS__CLIPARSER_HPP

namespace eprosima {
namespace fastdds {
namespace examples {
namespace delivery_mechanisms {

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
        PUBSUB,
        UNDEFINED
    };

    //! Delivery mechanism enumeration
    enum class DeliveryMechanismKind : uint8_t
    {
        DATA_SHARING,
        INTRA_PROCESS,
        LARGE_DATA,
        SHM,
        TCPv4,
        TCPv6,
        UDPv4,
        UDPv6,
        DEFAULT
    };

    //! DeliveryMechanisms structure for the application
    struct delivery_mechanisms_config
    {
        CLIParser::EntityKind entity = CLIParser::EntityKind::UNDEFINED;
        bool ignore_local_endpoints = false;
        uint16_t samples = 0;
        uint32_t domain = 0;
        DeliveryMechanismKind delivery_mechanism = DeliveryMechanismKind::DEFAULT;
        std::string tcp_ip_address = "";
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
        std::cout << "Usage: delivery_mechanisms <entity> [options]"                                    << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Entities:"                                                                        << std::endl;
        std::cout << "  publisher                           Run a publisher entity"                     << std::endl;
        std::cout << "  subscriber                          Run a subscriber entity"                    << std::endl;
        std::cout << "  pubsub                              Run both publisher and subscriber entities" << std::endl;
        std::cout << "                                      in the same participant"                    << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Common options:"                                                                  << std::endl;
        std::cout << "  -a <ip>, --address <ip>             TCP IP address (only available if selected" << std::endl;
        std::cout << "                                      delivery mechanism is TCPv4 or TCPv6)"      << std::endl;
        std::cout << "                                      (Default: localhost [127.0.0.1 or ::1])"    << std::endl;
        std::cout << "  -d <num>, --domain <num>            Domain ID number [0 <= <num> <= 232]"       << std::endl;
        std::cout << "                                      (Default: 0)"                               << std::endl;
        std::cout << "  -h, --help                          Print this help message"                    << std::endl;
        std::cout << "  -m <string>, --mechanism <string>   Select delivery mechanism <string>:"        << std::endl;
        std::cout << "                                       · DATA-SHARING:  Data-sharing mechanism"   << std::endl;
        std::cout << "                                       · INTRA-PROCESS: Intra-process mechanism"  << std::endl;
        std::cout << "                                          (only allowed with \"pubsub\" entity)"  << std::endl;
        std::cout << "                                       · LARGE-DATA:    Large data mechanism"     << std::endl;
        std::cout << "                                       · TCPv4:         TCP transport over IPv4"  << std::endl;
        std::cout << "                                       · TCPv6:         TCP transport over IPv6"  << std::endl;
        std::cout << "                                       · UDPv4:         UDP transport over IPv4"  << std::endl;
        std::cout << "                                       · UDPv6:         UDP transport over IPv6"  << std::endl;
        std::cout << "                                       · SHM:           Shared Memory Transport"  << std::endl;
        std::cout << "                                      (Default: default builtin transports"       << std::endl;
        std::cout << "                                       [SHM and UDPv4, SHM prior UDPv4])"         << std::endl;
        std::cout << "  -s <num>, --samples <num>           Number of samples to send or receive"       << std::endl;
        std::cout << "                                      [0 <= <num> <= 65535]"                      << std::endl;
        std::cout << "                                      (Default: 0 [unlimited])"                   << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "\"pubsub\" options:"                                                              << std::endl;
        std::cout << "  -i , --ignore-local-endpoints       Avoid matching compatible datareaders and"  << std::endl;
        std::cout << "                                      datawriters that belong to the same domain" << std::endl;
        std::cout << "                                      participant"                                << std::endl;
        std::cout << "                                      Default(false [they match])"                << std::endl;
        std::exit(return_code);
    }

    /**
     * @brief Parse the command line options and return the delivery_mechanisms_config object
     *
     * @param argc number of arguments
     * @param argv array of arguments
     * @return delivery_mechanisms_config object with the parsed options
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static delivery_mechanisms_config parse_cli_options(
            int argc,
            char* argv[])
    {
        delivery_mechanisms_config config;

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
        else if ( first_argument == "pubsub")
        {
            config.entity = CLIParser::EntityKind::PUBSUB;
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
            else if (arg == "-a" || arg == "--address")
            {
                if (++i < argc)
                {
                    config.tcp_ip_address = argv[i];
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing address argument");
                    print_help(EXIT_FAILURE);
                }
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
                            config.domain = static_cast<uint16_t>(input);
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
                            config.samples = static_cast<uint16_t>(input);
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
            else if (arg == "-m" || arg == "--mechanism")
            {
                if (++i < argc)
                {
                    std::string mechanism = argv[i];
                    if (mechanism == "DATA-SHARING" || mechanism == "data-sharing"
                            || mechanism == "DATA_SHARING" || mechanism == "data_sharing")
                    {
                        config.delivery_mechanism = DeliveryMechanismKind::DATA_SHARING;
                    }
                    else if (mechanism == "INTRA-PROCESS" || mechanism == "intra-process"
                            || mechanism == "INTRA_PROCESS" || mechanism == "intra_process")
                    {
                        if (config.entity == EntityKind::PUBSUB)
                        {
                            config.delivery_mechanism = DeliveryMechanismKind::INTRA_PROCESS;
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER,
                                    "intra-process mechanism only allowed with \"pubsub\" entity");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    else if (mechanism == "LARGE-DATA" || mechanism == "large-data"
                            || mechanism == "LARGE_DATA" || mechanism == "large_data")
                    {
                        config.delivery_mechanism = DeliveryMechanismKind::LARGE_DATA;
                    }
                    else if (mechanism == "TCP" || mechanism == "tcp" || mechanism == "TCPv4" || mechanism == "tcpv4"
                            || mechanism == "TCPV4")
                    {
                        config.delivery_mechanism = DeliveryMechanismKind::TCPv4;
                    }
                    else if (mechanism == "TCPv6" || mechanism == "tcpv6" || mechanism == "TCPV6")
                    {
                        config.delivery_mechanism = DeliveryMechanismKind::TCPv6;
                    }
                    else if (mechanism == "UDP" || mechanism == "udp" || mechanism == "UDPv4" || mechanism == "udpv4"
                            || mechanism == "UDPV4")
                    {
                        config.delivery_mechanism = DeliveryMechanismKind::UDPv4;
                    }
                    else if (mechanism == "UDPv6" || mechanism == "udpv6" || mechanism == "UDPV6")
                    {
                        config.delivery_mechanism = DeliveryMechanismKind::UDPv6;
                    }
                    else if (mechanism == "SHM" || mechanism == "shm")
                    {
                        config.delivery_mechanism = DeliveryMechanismKind::SHM;
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing mechanism argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing mechanism argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-i" || arg == "--ignore-local-endpoints")
            {
                if (config.entity == EntityKind::PUBSUB)
                {
                    config.ignore_local_endpoints = true;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "ignore-local-endpoints option only allowed with \"pubsub\" entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing argument: " + arg);
                print_help(EXIT_FAILURE);
            }
        }

        // Pubsub entity does not support TCP transport without ignore-local-endpoints option
        if (config.entity == CLIParser::EntityKind::PUBSUB && !config.ignore_local_endpoints &&
                (config.delivery_mechanism == DeliveryMechanismKind::TCPv4 ||
                config.delivery_mechanism == DeliveryMechanismKind::TCPv6))
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER,
                    "Unsupported corner case: TCP delivery mechanism is not allowed for \"pubsub\" without ignore-local-endpoints option");
            print_help(EXIT_FAILURE);
        }

        // Address argument is only allowed with TCP transport
        if (!config.tcp_ip_address.empty() &&
                !(config.delivery_mechanism == DeliveryMechanismKind::TCPv4 ||
                config.delivery_mechanism == DeliveryMechanismKind::TCPv6))
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER, "address argument only allowed with TCP delivery mechanism");
            print_help(EXIT_FAILURE);
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
            case EntityKind::PUBSUB:
                return "PubSub";
            case EntityKind::UNDEFINED:
            default:
                return "Undefined entity";
        }
    }

};

} // namespace delivery_mechanisms
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_DELIVERY_MECHANISMS__CLIPARSER_HPP
