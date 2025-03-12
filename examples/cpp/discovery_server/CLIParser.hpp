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

/**
 * @file CLIParser.hpp
 *
 */

#include <csignal>
#include <cstdlib>
#include <iostream>

#include <fastdds/dds/log/Log.hpp>

#include "Helpers.hpp"

#ifndef FASTDDS_EXAMPLES_CPP_DISCOVERY_SERVER__CLIPARSER_HPP
#define FASTDDS_EXAMPLES_CPP_DISCOVERY_SERVER__CLIPARSER_HPP

namespace eprosima {
namespace fastdds {
namespace examples {
namespace discovery_server {

using dds::Log;

class CLIParser
{
public:

    CLIParser() = delete;

    //! Entity kind enumeration
    enum class EntityKind : uint8_t
    {
        CLIENT_PUBLISHER,
        CLIENT_SUBSCRIBER,
        SERVER,
        UNDEFINED
    };

    //! Clients common configuration
    struct client_config
    {
        uint16_t connection_port{16166};
        std::string connection_address{"127.0.0.1"};
    };

    //! Configuration options for both publisher and subscriber clients
    struct pubsub_config : public client_config
    {
        bool reliable{false};
        bool transient_local{false};
        uint16_t samples{0};
        std::string topic_name{"discovery_server_topic"};
    };

    //! Publisher client configuration structure
    struct client_publisher_config : public pubsub_config
    {
        TransportKind transport_kind{TransportKind::UDPv4};
        uint16_t interval{100};
    };

    //! Subscriber client configuration structure
    struct client_subscriber_config : public pubsub_config
    {
        TransportKind transport_kind{TransportKind::UDPv4};
    };

    //! Server configuration structure
    //! A server can, in turn, act as a client
    struct server_config : public client_config
    {
        bool is_also_client{false};
        TransportKind transport_kind{TransportKind::UDPv4};
        uint16_t listening_port{16166};
        uint16_t timeout{0};
        std::string listening_address{"127.0.0.1"};
    };

    //! Configuration structure for the example
    struct ds_example_config
    {
        CLIParser::EntityKind entity = CLIParser::EntityKind::UNDEFINED;
        client_publisher_config pub_config;
        client_subscriber_config sub_config;
        server_config srv_config;

        friend std::ostream& operator << (
                std::ostream& os,
                const ds_example_config& config)
        {
            os << "Entity: " << parse_entity_kind(config.entity) << std::endl;
            os << "Common options:" << std::endl;
            os << "  Transport: " << static_cast<int>(config.pub_config.transport_kind) << std::endl;

            if (config.entity != CLIParser::EntityKind::SERVER ||
                    (config.entity == CLIParser::EntityKind::SERVER && config.srv_config.is_also_client))
            {
                os << "Client options:" << std::endl;
                os << "  Connection address: " << config.pub_config.connection_address << std::endl;
                os << "  Connection port: " << config.pub_config.connection_port << std::endl;
            }

            if (config.entity == CLIParser::EntityKind::CLIENT_PUBLISHER)
            {
                os << "Publisher options:" << std::endl;
                os << "  Topic name: " << config.pub_config.topic_name << std::endl;
                os << "  Samples: " << config.pub_config.samples << std::endl;
                os << "  Interval: " << config.pub_config.interval << std::endl;
            }
            else if (config.entity == CLIParser::EntityKind::CLIENT_SUBSCRIBER)
            {
                os << "Subscriber options:" << std::endl;
                os << "  Topic name: " << config.sub_config.topic_name << std::endl;
                os << "  Samples: " << config.sub_config.samples << std::endl;
            }
            else if (config.entity == CLIParser::EntityKind::SERVER)
            {
                os << "Server options:" << std::endl;
                os << "  Listening address: " << config.srv_config.listening_address << std::endl;
                os << "  Listening port: " << config.srv_config.listening_port << std::endl;
                os << "  Timeout: " << config.srv_config.timeout << std::endl;
            }

            return os;
        }

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
        std::cout << "Usage: discovery_server <entity> [options]"                                      << std::endl;
        std::cout << ""                                                                                << std::endl;
        std::cout << "Entities:"                                                                       << std::endl;
        std::cout << "  publisher                            Run a client publisher entity."           << std::endl;
        std::cout << "  subscriber                           Run a client subscriber entity."          << std::endl;
        std::cout << "  server                               Run a server entity."                     << std::endl;
        std::cout << ""                                                                                << std::endl;
        std::cout << "  -h,       --help                     Print this help message."                 << std::endl;
        std::cout << "Client options (common to Publisher, Subscriber and Server acting as Client):"   << std::endl;
        std::cout << "  -c <str>, --connection-address <str> Address of the Server to connect to"      << std::endl;
        std::cout << "                                       (Default address: 127.0.0.1)."            << std::endl;
        std::cout << "  -p <num>, --connection-port <num>    Port of the Server to connect to"         << std::endl;
        std::cout << "                                       (Default port: 16166)."                   << std::endl;
        std::cout << "                                       (0 by default)."                          << std::endl;
        std::cout << "            --transport <str>          [udpv4|udpv6|tcpv4|tcpv6] "               << std::endl;
        std::cout << "                                       (udpv4 by default)."                      << std::endl;
        std::cout << ""                                                                                << std::endl;
        std::cout << "Publisher options:"                                                              << std::endl;
        std::cout << "  -t <str>, --topic <str>              Topic name"                               << std::endl;
        std::cout << "                                       (Default: discovery_server_topic)."       << std::endl;
        std::cout << "  -r, --reliable                       Set Reliability QoS as reliable"          << std::endl;
        std::cout << "                                       (Default: best effort)"                   << std::endl;
        std::cout << "      --transient-local                Set Durability QoS as transient local"    << std::endl;
        std::cout << "                                       (Default: volatile)"                      << std::endl;
        std::cout << "  -s <num>, --samples <num>            Number of samples to send "               << std::endl;
        std::cout << "                                       (Default: 0 => infinite samples)."        << std::endl;
        std::cout << "  -i <num>, --interval <num>           Time between samples in milliseconds"     << std::endl;
        std::cout << "                                       (Default: 100)."                          << std::endl;
        std::cout << ""                                                                                << std::endl;
        std::cout << "Subscriber options:"                                                             << std::endl;
        std::cout << "  -t <str>, --topic <str>              Topic name"                               << std::endl;
        std::cout << "                                       (Default: discovery_server_topic)."       << std::endl;
        std::cout << "  -s <num>, --samples <num>            Number of samples to receive"             << std::endl;
        std::cout << "                                       (Default: 0 => infinite samples)."        << std::endl;
        std::cout << "  -r, --reliable                       Set Reliability QoS as reliable"          << std::endl;
        std::cout << "                                       (Default: best effort)"                   << std::endl;
        std::cout << "      --transient-local                Set Durability QoS as transient local"    << std::endl;
        std::cout << "                                       (Default: volatile)"                      << std::endl;
        std::cout << ""                                                                                << std::endl;
        std::cout << "Server options:"                                                                 << std::endl;
        std::cout << "            --listening-address <str>  Server listening address"                 << std::endl;
        std::cout << "                                       (Default address: 127.0.0.1)"             << std::endl;
        std::cout << "            --listening-port <num>     Server listening port"                    << std::endl;
        std::cout << "                                       (Default port: 16166)"                    << std::endl;
        std::cout << "            --timeout <num>            Number of seconds before finish"          << std::endl;
        std::cout << "                                       the process (Default: 0 = till ^C)."      << std::endl;
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
    static ds_example_config parse_cli_options(
            int argc,
            char* argv[])
    {
        ds_example_config config;

        if (argc < 2)
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER, "missing entity argument");
            print_help(EXIT_FAILURE);
        }

        std::string first_argument = argv[1];

        if (first_argument == "publisher" )
        {
            config.entity = CLIParser::EntityKind::CLIENT_PUBLISHER;
        }
        else if (first_argument == "subscriber")
        {
            config.entity = CLIParser::EntityKind::CLIENT_SUBSCRIBER;
        }
        else if (first_argument == "server")
        {
            config.entity = CLIParser::EntityKind::SERVER;
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

        bool uses_ipv6 = false;
        bool listening_address_was_set = false;
        bool connection_address_was_set = false;

        for (int i = 2; i < argc; ++i)
        {
            std::string arg = argv[i];

            if (arg == "-h" || arg == "--help")
            {
                print_help(EXIT_SUCCESS);
            }
            // Common options
            else if (arg == "--transport")
            {
                if (++i < argc)
                {
                    std::string input = argv[i];

                    if (input == "udpv4")
                    {
                        config.pub_config.transport_kind = TransportKind::UDPv4;
                        config.sub_config.transport_kind = TransportKind::UDPv4;
                        config.srv_config.transport_kind = TransportKind::UDPv4;
                    }
                    else if (input == "udpv6")
                    {
                        config.pub_config.transport_kind = TransportKind::UDPv6;
                        config.sub_config.transport_kind = TransportKind::UDPv6;
                        config.srv_config.transport_kind = TransportKind::UDPv6;
                        uses_ipv6 = true;
                    }
                    else if (input == "tcpv4")
                    {
                        config.pub_config.transport_kind = TransportKind::TCPv4;
                        config.sub_config.transport_kind = TransportKind::TCPv4;
                        config.srv_config.transport_kind = TransportKind::TCPv4;
                    }
                    else if (input == "tcpv6")
                    {
                        config.pub_config.transport_kind = TransportKind::TCPv6;
                        config.sub_config.transport_kind = TransportKind::TCPv6;
                        config.srv_config.transport_kind = TransportKind::TCPv6;
                        uses_ipv6 = true;
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "Unkown transport argument: " + input);
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing transport argument");
                    print_help(EXIT_FAILURE);
                }
            }
            // Client options
            else if (arg == "-c" || arg == "--connection-address")
            {
                if (++i < argc)
                {
                    config.pub_config.connection_address = argv[i];
                    config.sub_config.connection_address = argv[i];
                    config.srv_config.connection_address = argv[i];
                    if (config.entity == CLIParser::EntityKind::SERVER)
                    {
                        config.srv_config.is_also_client = true;
                    }
                    connection_address_was_set = true;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing connection-address argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-p" || arg == "--connection-port")
            {
                if (++i < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[i]);
                        if (input < std::numeric_limits<uint16_t>::min() ||
                                input > std::numeric_limits<uint16_t>::max())
                        {
                            throw std::out_of_range("port argument " + std::string(
                                              argv[i]) + " out of range [0, 65535].");
                        }
                        else
                        {
                            config.pub_config.connection_port = static_cast<uint16_t>(input);
                            config.sub_config.connection_port = static_cast<uint16_t>(input);
                            config.srv_config.connection_port = static_cast<uint16_t>(input);
                            if (config.entity == CLIParser::EntityKind::SERVER)
                            {
                                config.srv_config.is_also_client = true;
                            }
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid port argument " + std::string(
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing port argument");
                    print_help(EXIT_FAILURE);
                }
            }
            // PubSub options
            else if (arg == "-t" || arg == "--topic")
            {
                if (config.entity == CLIParser::EntityKind::CLIENT_PUBLISHER ||
                        config.entity == CLIParser::EntityKind::CLIENT_SUBSCRIBER)
                {
                    config.pub_config.topic_name = argv[i];
                    config.sub_config.topic_name = argv[i];
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER,
                            "wrong or missing entity for --topic argument: only available for publisher and subscriber");
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
                            throw std::out_of_range("samples argument out of range");
                        }
                        else
                        {
                            if (config.entity == CLIParser::EntityKind::CLIENT_PUBLISHER ||
                                    config.entity == CLIParser::EntityKind::CLIENT_SUBSCRIBER)
                            {
                                config.pub_config.samples = static_cast<uint16_t>(input);
                                config.sub_config.samples = static_cast<uint16_t>(input);
                            }
                            else
                            {
                                EPROSIMA_LOG_ERROR(CLI_PARSER, "entity error or not specified for --samples argument");
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
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "samples argument out of range for " + arg + ": " + e.what());
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
            // Publisher options
            else if (arg == "-i" || arg == "--interval")
            {
                if (config.entity == CLIParser::EntityKind::CLIENT_PUBLISHER)
                {
                    if (++i < argc)
                    {
                        try
                        {
                            int input = std::stoi(argv[i]);
                            if (input < std::numeric_limits<std::uint16_t>::min() ||
                                    input > std::numeric_limits<std::uint16_t>::max())
                            {
                                throw std::out_of_range("interval argument out of range");
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
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "interval argument is only valid for client publisher entity");
                    print_help(EXIT_FAILURE);
                }
            }
            // Server options
            else if (arg == "--listening-address")
            {
                if (++i < argc)
                {
                    if (config.entity == CLIParser::EntityKind::SERVER )
                    {
                        config.srv_config.listening_address = argv[i];
                        listening_address_was_set = true;
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "listening address  argument is only valid for server entity");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing connection-address argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--listening-port")
            {
                if (++i < argc)
                {
                    if (config.entity == CLIParser::EntityKind::SERVER)
                    {
                        try
                        {
                            int input = std::stoi(argv[i]);
                            if (input < std::numeric_limits<uint16_t>::min() ||
                                    input > std::numeric_limits<uint16_t>::max())
                            {
                                throw std::out_of_range("listening-port argument " + std::string(
                                                  argv[i]) + " out of range [0, 65535].");
                            }
                            else
                            {
                                config.srv_config.listening_port = static_cast<uint16_t>(input);
                            }
                        }
                        catch (const std::invalid_argument& e)
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid listening-port argument " + std::string(
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
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "--listening-port argument is only valid for server entity");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing port argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--timeout")
            {
                if (++i < argc)
                {
                    if (config.entity == CLIParser::EntityKind::SERVER)
                    {
                        try
                        {
                            int input = std::stoi(argv[i]);
                            if (input < std::numeric_limits<uint16_t>::min() ||
                                    input > std::numeric_limits<uint16_t>::max())
                            {
                                throw std::out_of_range("timeout argument " + std::string(
                                                  argv[i]) + " out of range [0, 65535].");
                            }
                            else
                            {
                                config.srv_config.timeout = static_cast<uint16_t>(input);
                            }
                        }
                        catch (const std::invalid_argument& e)
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid timeout argument " + std::string(
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
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "--listening-port argument is only valid for server entity");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing port argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(CLI_PARSER, "unknown option " + arg);
                print_help(EXIT_FAILURE);
            }
        }

        // change default values if IPv6 is used
        // and user did not specified ones
        if (uses_ipv6)
        {
            if (config.entity == CLIParser::EntityKind::SERVER &&
                    !listening_address_was_set)
            {
                config.srv_config.listening_address = "::1";
            }

            if (!connection_address_was_set)
            {
                config.pub_config.connection_address = "::1";
                config.sub_config.connection_address = "::1";
                if (config.srv_config.is_also_client)
                {
                    config.srv_config.connection_address = "::1";
                }
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
            case EntityKind::CLIENT_PUBLISHER:
                return "Client Publisher";
            case EntityKind::CLIENT_SUBSCRIBER:
                return "Client Subscriber";
            case EntityKind::SERVER:
                return "Discovery Server";
            case EntityKind::UNDEFINED:
            default:
                return "Undefined entity";
        }
    }

};

} // namespace discovery_server
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_DISCOVERY_SERVER__CLIPARSER_HPP
