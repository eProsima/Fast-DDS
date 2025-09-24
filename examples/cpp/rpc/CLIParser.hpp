// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_EXAMPLES_CPP_RPC__CLIPARSER_HPP
#define FASTDDS_EXAMPLES_CPP_RPC__CLIPARSER_HPP

#include <csignal>
#include <cstdlib>
#include <iostream>

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace fastdds {
namespace examples {
namespace rpc {

using dds::Log;

class CLIParser
{
public:

    CLIParser() = delete;

    //! Entity kind enumeration
    enum class EntityKind : uint8_t
    {
        CLIENT,
        SERVER,
        UNDEFINED
    };

    //! Operation kind enumeration
    enum class OperationKind : uint8_t
    {
        ADDITION,
        SUBSTRACTION,
        REPRESENTATION_LIMITS,
        UNDEFINED
    };

    //! Configuration structure for the application

    struct config
    {
        CLIParser::EntityKind entity = CLIParser::EntityKind::UNDEFINED; // Entity kind (Client or Server)
        CLIParser::OperationKind operation = CLIParser::OperationKind::UNDEFINED; // Operation kind
        std::uint8_t filter_kind = 0; // Filter kind for the input feed
        std::uint16_t timeout = 0; // Seconds to live for the server (0 means infinite)
        std::int32_t x = 0; // First operand for addition and substraction
        std::int32_t y = 0; // Second operand for addition and substraction
        std::uint32_t n_results = 0; // Number of results to return in the Fibonacci sequence
        std::size_t connection_attempts = 10; // Number of attempts to connect to the server
        std::size_t thread_pool_size = 0; // Size of the thread pool for the server
    };

    /**
     * @brief Print usage help message and exit with the given return code
     *
     * @param [in] return_code return code to exit with
     *
     * @warning This method finishes the execution of the program with the input return code
     */
    static void print_help(
            const std::uint8_t return_code)
    {
        std::cout << "Service to perform basic arithmetic operations  "                                    << std::endl;
        std::cout << "(addition and subtraction) on two 32-bit integers"                                   << std::endl;
        std::cout << "or compute the representation limits of a 32-bit integer."                           << std::endl;
        std::cout << ""                                                                                    << std::endl;
        std::cout << "Usage: rpc <entity> [options]"                                                       << std::endl;
        std::cout << ""                                                                                    << std::endl;
        std::cout << "Entities:"                                                                           << std::endl;
        std::cout << "  server                                               Run a server entity"          << std::endl;
        std::cout << "  client                                               Run a client entity"          << std::endl;
        std::cout << ""                                                                                    << std::endl;
        std::cout << "Common options:"                                                                     << std::endl;
        std::cout << "  -h, --help                                           Print this help message"      << std::endl;
        std::cout << ""                                                                                    << std::endl;
        std::cout << "Client arguments:"                                                                   << std::endl;
        std::cout << "  -a <num_1> <num_2>, --addition <num_1> <num_2>       Adds two numbers"             << std::endl;
        std::cout << "                                                       [-2^31 <= <num_i> <= 2^31-1]" << std::endl;
        std::cout << "                                                                                   " << std::endl;
        std::cout << "  -s <num_1> <num_2>, --substraction <num_1> <num_2>   Substracts two numbers"       << std::endl;
        std::cout << "                                                       [-2^31 <= <num_i> <= 2^31-1]" << std::endl;
        std::cout << "                                                                                   " << std::endl;
        std::cout << "  -r, --representation-limits                          Computes the representation"  << std::endl;
        std::cout << "                                                       limits of a 32-bit integer"   << std::endl;
        std::cout << "                                                                                   " << std::endl;
        std::cout << "      --connection-attempts <num>                      Number of attempts to connect" <<
            std::endl;
        std::cout << "                                                       to a server before failing"   << std::endl;
        std::cout << "                                                       [default: 10]"                << std::endl;
        std::cout << "Server arguments:"                                                                   << std::endl;
        std::cout << "      --thread-pool-size <num>                         The size of the thread pool"  << std::endl;
        std::cout << "                                                       to use for processing"        << std::endl;
        std::cout << "                                                       requests."                    << std::endl;
        std::cout << "                                                       When set to 0, a new thread"  << std::endl;
        std::cout << "                                                       will be created when"         << std::endl;
        std::cout << "                                                       no threads are available"     << std::endl;
        std::cout << "                                                       [default: 0] "                << std::endl;
        std::cout << "      --timeout <num>                                  Number of seconds to live"    << std::endl;
        std::cout << "                                                       (Default: 0 = till ^C)."      << std::endl;
        std::exit(return_code);
    }

    /**
     * @brief Parse the command line options and return the config object
     *
     * @param [in] argc number of arguments
     * @param [in] argv array of arguments
     * @return config object with the parsed options
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static config parse_cli_options(
            int argc,
            char* argv[])
    {
        config config;

        if (argc < 2)
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER, "missing entity argument");
            print_help(EXIT_FAILURE);
        }

        std::string first_argument = argv[1];

        if (first_argument == "server")
        {
            config.entity = CLIParser::EntityKind::SERVER;
        }
        else if (first_argument == "client")
        {
            config.entity = CLIParser::EntityKind::CLIENT;
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
            else if (arg == "-a" || arg == "--addition")
            {
                if (config.entity == CLIParser::EntityKind::CLIENT)
                {
                    if (CLIParser::OperationKind::UNDEFINED != config.operation)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "Only one operation can be selected");
                        print_help(EXIT_FAILURE);
                    }

                    if (++i < argc)
                    {
                        config.x = consume_integer_argument<std::int32_t>(argv[i], arg);

                        if (++i < argc)
                        {
                            config.y = consume_integer_argument<std::int32_t>(argv[i], arg);
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "missing addition argument");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "missing addition argument");
                        print_help(EXIT_FAILURE);
                    }

                    config.operation = CLIParser::OperationKind::ADDITION;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "addition argument is only valid for client entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-s" || arg == "--substraction")
            {
                if (config.entity == CLIParser::EntityKind::CLIENT)
                {
                    if (CLIParser::OperationKind::UNDEFINED != config.operation)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "Only one operation can be selected");
                        print_help(EXIT_FAILURE);
                    }

                    if (++i < argc)
                    {
                        config.x = consume_integer_argument<std::int32_t>(argv[i], arg);

                        if (++i < argc)
                        {
                            config.y = consume_integer_argument<std::int32_t>(argv[i], arg);
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "missing substraction argument");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "missing substraction argument");
                        print_help(EXIT_FAILURE);
                    }

                    config.operation = CLIParser::OperationKind::SUBSTRACTION;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "addition argument is only valid for client entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-r" || arg == "--representation-limits")
            {
                if (config.entity == CLIParser::EntityKind::CLIENT)
                {
                    if (CLIParser::OperationKind::UNDEFINED != config.operation)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "Only one operation can be selected");
                        print_help(EXIT_FAILURE);
                    }

                    config.operation = CLIParser::OperationKind::REPRESENTATION_LIMITS;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "representation-limits argument is only valid for client entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--connection-attempts")
            {
                if (config.entity == CLIParser::EntityKind::CLIENT)
                {
                    if (++i < argc)
                    {
                        config.connection_attempts = consume_integer_argument<std::size_t>(argv[i], arg);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "missing connection-attempts argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "connection-attempts argument is only valid for client entity");
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--thread-pool-size")
            {
                if (config.entity == CLIParser::EntityKind::SERVER)
                {
                    if (++i < argc)
                    {
                        config.thread_pool_size = consume_integer_argument<std::size_t>(argv[i], arg);
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "missing thread-pool-size argument");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "thread-pool-size argument is only valid for server entity");
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
                                config.timeout = static_cast<uint16_t>(input);
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
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "--timeout argument is only valid for server entity");
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing timeout argument");
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
     * @param [in] signum signal number
     *
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
     * @param [in] entity entity kind
     *
     * @return std::string entity kind
     */
    static std::string parse_entity_kind(
            const EntityKind& entity)
    {
        switch (entity)
        {
            case EntityKind::SERVER:
                return "Server";
            case EntityKind::CLIENT:
                return "Client";
            case EntityKind::UNDEFINED:
            default:
                return "Undefined entity";
        }
    }

private:

    /**
     * @brief Consume an integer argument and return it
     *
     * @param [in] arg_value string argument value to consume
     * @param [in] arg_name name of the argument to print in case of error
     *
     * @return integer argument of a given type
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    template <typename T>
    static T consume_integer_argument(
            const std::string& arg_value,
            const std::string& arg_name)
    {
        T value = 0;

        try
        {
            long long input = std::stoll(arg_value);

            if (std::is_unsigned<T>::value)
            {
                if (input < 0)
                {
                    throw std::invalid_argument("negative value for unsigned integer");
                }

                // Cast to unsigned long long safe because input is >= 0
                unsigned long long unsigned_input = static_cast<unsigned long long>(input);
                if (unsigned_input < std::numeric_limits<T>::min() ||
                        unsigned_input > std::numeric_limits<T>::max())
                {
                    throw std::out_of_range("unsigned integer argument out of range");
                }
            }
            else if (input < static_cast<long long>(std::numeric_limits<T>::min()) ||
                    input > static_cast<long long>(std::numeric_limits<T>::max()))
            {
                throw std::out_of_range("integer argument out of range");
            }

            value = static_cast<T>(input);
        }
        catch (const std::invalid_argument& e)
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER,
                    "invalid integer value for argument " << arg_name << ": [ " + arg_value + " ]. " + e.what());
            print_help(EXIT_FAILURE);
        }
        catch (const std::out_of_range& e)
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER,
                    "integer value out of range for argument " << arg_name << ": [ " + arg_value + " ]. " + e.what());
            print_help(EXIT_FAILURE);
        }

        return value;
    }

};

} // namespace rpc
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_RPC__CLIPARSER_HPP