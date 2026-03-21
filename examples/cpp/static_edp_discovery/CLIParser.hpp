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
#include <fstream>

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif // ifndef NOMINMAX

#include <windows.h>

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif // ifndef PATH_MAX

#else
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#endif // ifdef _WIN32

#include <fastdds/dds/log/Log.hpp>

#ifndef FASTDDS_STATIC_EDP_DISCOVERY__CLIPARSER_HPP
#define FASTDDS_STATIC_EDP_DISCOVERY__CLIPARSER_HPP

namespace eprosima {
namespace fastdds {
namespace examples {
namespace static_edp_discovery {

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

    //! DeliveryMechanisms structure for the application
    struct static_edp_discovery_config
    {
        CLIParser::EntityKind entity = CLIParser::EntityKind::UNDEFINED;
        uint16_t samples = 0;
        uint32_t domain = 0;
        std::string xml_path = "";
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
        std::cout << "Usage: static_edp_discovery <entity> [options]"                                   << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Entities:"                                                                        << std::endl;
        std::cout << "  publisher                           Run a publisher entity"                     << std::endl;
        std::cout << "  subscriber                          Run a subscriber entity"                    << std::endl;
        std::cout << ""                                                                                 << std::endl;
        std::cout << "Common options:"                                                                  << std::endl;
        std::cout << "  -d <num>, --domain <num>            Domain ID number [0 <= <num> <= 232]"       << std::endl;
        std::cout << "                                      (Default: 0)"                               << std::endl;
        std::cout << "  -h, --help                          Print this help message"                    << std::endl;
        std::cout << "  -s <num>, --samples <num>           Number of samples to send or receive"       << std::endl;
        std::cout << "                                      [0 <= <num> <= 65535]"                      << std::endl;
        std::cout << "                                      (Default: 0 [unlimited])"                   << std::endl;
        std::cout << "  -x <path>, --xml <path>             Path to the XML file with the static EDP"   << std::endl;
        std::cout << "                                      configuration.                (Default: "   << std::endl;
        std::cout << "                                        <app_path>/HelloWorld_static_disc.xml)"   << std::endl;
        std::exit(return_code);
    }

    /**
     * @brief Parse the command line options and return the static_edp_discovery_config object
     *
     * @param argc number of arguments
     * @param argv array of arguments
     * @return static_edp_discovery_config object with the parsed options
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static static_edp_discovery_config parse_cli_options(
            int argc,
            char* argv[])
    {
        static_edp_discovery_config config;

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
            else if (arg == "-x" || arg == "--xml")
            {
                if (++i < argc)
                {
                    config.xml_path = argv[i];
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing xml argument");
                    print_help(EXIT_FAILURE);
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(CLI_PARSER, "parsing argument: " + arg);
                print_help(EXIT_FAILURE);
            }
        }

        // Check if xml argument has been provided
        if (config.xml_path.empty())
        {
            config.xml_path = "file://" + get_path("HelloWorld_static_disc.xml");
        }
        else
        {
            config.xml_path = "file://" + config.xml_path;
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

    //! Obtain the path of the application
    static std::string get_application_path()
    {
        char result[PATH_MAX];

#ifdef _WIN32
        GetModuleFileName(NULL, result, PATH_MAX);
#else
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        if (count == -1)
        {
            return "";
        }
        result[count] = '\0';
#endif // ifdef _WIN32

        return std::string(result);
    }

    //! Obtain the path of the file assuming that it is in the same directory as the current application
    static std::string get_path(
            const std::string& filename)
    {
        std::string ret = "";
        std::string app_path = get_application_path();
        if (!app_path.empty())
        {
#ifdef _WIN32
            char drive[_MAX_DRIVE];
            char dir[_MAX_DIR];
            _splitpath_s(app_path.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
            ret = std::string(drive) + std::string(dir) + filename;
#else
            char* dir = dirname(&app_path[0]);
            ret = std::string(dir) + "/" + filename;
#endif // ifdef _WIN32
        }
        return ret;
    }

};

} // namespace static_edp_discovery
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_STATIC_EDP_DISCOVERY__CLIPARSER_HPP
