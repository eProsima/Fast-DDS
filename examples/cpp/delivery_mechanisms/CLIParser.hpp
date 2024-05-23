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

#ifndef _FASTDDS_DELIVERY_MECHANISMS_CLI_PARSER_HPP_
#define _FASTDDS_DELIVERY_MECHANISMS_CLI_PARSER_HPP_

namespace eprosima {
namespace fastdds {
namespace examples {
namespace delivery_mechanisms {

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
        PUBSUB,
        UNDEFINED
    };

    //! Delivery mechanism enumeration
    enum class DeliveryMechanismKind : uint8_t
    {
        TCP,
        UDP,
        SHM,
        DATA_SHARING,
        INTRA_PROCESS,
        DEFAULT
    };

    //! Entity configuration structure (shared for both publisher and subscriber applications)
    struct entity_config
    {
        uint16_t samples = 0;
        uint32_t domain = 0;
        bool ignore_local_endpoints = false;
        DeliveryMechanismKind delivery_mechanism = DeliveryMechanismKind::DEFAULT;
    };

    //! DeliveryMechanisms structure for the application
    struct delivery_mechanisms_config
    {
        CLIParser::EntityKind entity = CLIParser::EntityKind::UNDEFINED;
        entity_config entity_configuration;
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
        std::cout << "  -d <num>, --domain <num>            Domain ID number [0 <= <num> <= 232]"       << std::endl;
        std::cout << "                                      (Default: 0)"                               << std::endl;
        std::cout << "  -h, --help                          Print this help message"                    << std::endl;
        std::cout << "  -m <string>, --mechanism <string>   Select delivery mechanism <string>:"        << std::endl;
        std::cout << "                                       · TCP: TCP transport over IPv4"            << std::endl;
        std::cout << "                                       · UDP: UDP transport over IPv4"            << std::endl;
        std::cout << "                                       · SHM: Shared Memory Transport"            << std::endl;
        std::cout << "                                       · DATA-SHARING:  Data-sharing mechanism"   << std::endl;
        std::cout << "                                       · INTRA-PROCESS: Intra-process mechanism"  << std::endl;
        std::cout << "                                          (only allowed with \"pubsub\" entity)"  << std::endl;
        std::cout << "                                      (Default: UDP)"                             << std::endl;
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
                            config.entity_configuration.domain = static_cast<uint16_t>(input);
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
                            config.entity_configuration.samples = static_cast<uint16_t>(input);
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
                    if (mechanism == "TCP" || mechanism == "tcp")
                    {
                        config.entity_configuration.delivery_mechanism = DeliveryMechanismKind::TCP;
                    }
                    else if (mechanism == "UDP" || mechanism == "udp")
                    {
                        config.entity_configuration.delivery_mechanism = DeliveryMechanismKind::UDP;
                    }
                    else if (mechanism == "SHM" || mechanism == "shm")
                    {
                        config.entity_configuration.delivery_mechanism = DeliveryMechanismKind::SHM;
                    }
                    else if (mechanism == "DATA-SHARING" || mechanism == "data-sharing")
                    {
                        config.entity_configuration.delivery_mechanism = DeliveryMechanismKind::DATA_SHARING;
                    }
                    else if (mechanism == "INTRA-PROCESS" || mechanism == "intra-process")
                    {
                        if (config.entity == EntityKind::PUBSUB)
                        {
                            config.entity_configuration.delivery_mechanism = DeliveryMechanismKind::INTRA_PROCESS;
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER,
                                    "intra-process mechanism only allowed with \"pubsub\" entity");
                            print_help(EXIT_FAILURE);
                        }
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
                    config.entity_configuration.ignore_local_endpoints = true;
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

#endif // _FASTDDS_DELIVERY_MECHANISMS_CLI_PARSER_HPP_
