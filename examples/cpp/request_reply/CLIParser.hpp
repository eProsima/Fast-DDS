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

#include <cassert>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include <fastdds/dds/log/Log.hpp>

#ifndef FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__CLIPARSER_HPP
#define FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__CLIPARSER_HPP

namespace eprosima {
namespace fastdds {
namespace examples {
namespace request_reply {

using dds::Log;

class CLIParser
{
public:

    CLIParser() = delete;

    //! Entity kind enumeration
    enum class EntityKind : std::uint8_t
    {
        SERVER,
        CLIENT,
        UNDEFINED
    };

    //! Configuration structure for the application
    struct config
    {
        CLIParser::EntityKind entity = CLIParser::EntityKind::UNDEFINED;
        std::int16_t x = 0;
        std::int16_t y = 0;
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
        std::cout << "Service to perform several operations (addition, "         << std::endl;
        std::cout << "subtraction, multiplication, and division) on two 16-bit"  << std::endl;
        std::cout << "integers"                                                  << std::endl;
        std::cout << ""                                                          << std::endl;
        std::cout << "Usage: request_reply <entity> [options] [arguments]"       << std::endl;
        std::cout << ""                                                          << std::endl;
        std::cout << "Example:"                                                  << std::endl;
        std::cout << "  - request_reply server"                                  << std::endl;
        std::cout << "  - request_reply client 4 5"                              << std::endl;
        std::cout << ""                                                          << std::endl;
        std::cout << "Entities:"                                                 << std::endl;
        std::cout << "  server                        Run a server entity"       << std::endl;
        std::cout << "  client                        Run a client entity"       << std::endl;
        std::cout << ""                                                          << std::endl;
        std::cout << "Common options:"                                           << std::endl;
        std::cout << "  -h, --help                    Print this help message"   << std::endl;
        std::cout << ""                                                          << std::endl;
        std::cout << "Client arguments:"                                         << std::endl;
        std::cout << "  [NUMBER] [NUMBER]             The numbers of which the"  << std::endl;
        std::cout << "                                operations are performed"  << std::endl;
        std::exit(return_code);
    }

    /**
     * @brief Parse the command line options and return the config object
     *
     * @param [in] argc number of arguments
     * @param [in] argv array of arguments
     *
     * @return config object with the parsed options
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static config parse_cli_options(
            const int argc,
            const char* const argv[])
    {
        config config;

        if (argc < 2)
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER, "missing entity argument");
            print_help(EXIT_FAILURE);
        }

        std::string first_argument = argv[1];

        if (first_argument == "server" )
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
            if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "--help")
            {
                print_help(EXIT_SUCCESS);
            }
        }

        if (CLIParser::EntityKind::CLIENT == config.entity)
        {
            if (argc != 4)
            {
                EPROSIMA_LOG_ERROR(CLI_PARSER, "Incorrect number of arguments for client entity");
                print_help(EXIT_FAILURE);
            }

            consume_client_arguments(argv, config);
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
     * @brief Consume the client arguments and store them in the config object
     *
     * @pre argc == 4
     *
     * @param [in] argv array of arguments
     * @param [in,out] config config object to store the arguments
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static void consume_client_arguments(
            const char* const argv[],
            config& config)
    {
        config.x = consume_int16_argument(argv[2]);
        config.y = consume_int16_argument(argv[3]);
    }

    /**
     * @brief Consume an int16 argument and return it
     *
     * @param [in] arg string argument to consume
     *
     * @return std::int16_t int16 argument
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static std::int16_t consume_int16_argument(
            const std::string& arg)
    {
        std::int16_t value = 0;

        try
        {
            int input = std::stoi(arg);

            if (input < std::numeric_limits<std::int16_t>::min() ||
                    input > std::numeric_limits<std::int16_t>::max())
            {
                throw std::out_of_range("int16 argument out of range");
            }

            value = static_cast<std::int16_t>(input);
        }
        catch (const std::invalid_argument& e)
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid int16 argument for " + arg + ": " + e.what());
            print_help(EXIT_FAILURE);
        }
        catch (const std::out_of_range& e)
        {
            EPROSIMA_LOG_ERROR(CLI_PARSER, "int16 argument out of range for " + arg + ": " + e.what());
            print_help(EXIT_FAILURE);
        }

        return value;
    }

};

} // namespace request_reply
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__CLIPARSER_HPP
