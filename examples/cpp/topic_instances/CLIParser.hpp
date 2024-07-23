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

#include <algorithm>
#include <array>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <iostream>

#include <fastdds/dds/log/Log.hpp>

#ifndef FASTDDS_EXAMPLES_CPP_TOPIC_INSTANCES__CLIPARSER_HPP
#define FASTDDS_EXAMPLES_CPP_TOPIC_INSTANCES__CLIPARSER_HPP

namespace eprosima {
namespace fastdds {
namespace examples {
namespace topic_instances {

// The code here has been taken from the notes in https://en.cppreference.com/w/cpp/string/byte/toupper
// Note how the argument passed is cast into an unsigned char to avoid undefined behavior
static char my_toupper(
        char c)
{
    return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

// The code here has been taken from the notes in https://en.cppreference.com/w/cpp/string/byte/tolower
// Note how the argument passed is cast into an unsigned char to avoid undefined behavior
static char my_tolower (
        char c)
{
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

using dds::Log;

class CLIParser
{
public:

    CLIParser() = delete;

    //! Shape direction enumeration
    enum class ShapeDirection : uint8_t
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        DIAGONAL
    };

    //! Shape constants during the execution
    struct shape_configuration
    {
        // Configurable variables
        int step = 3;
        int size = 30;
        int width = 230;
        int height = 265;
        std::vector<std::string> colors;
        // Dependant variables
        int lower_th = 0;           // size   - 5
        int horizontal_th = 0;      // width  - lower_th;
        int vertical_th = 0;        // height - lower_th;
    };

    //! Entity kind enumeration
    enum class EntityKind : uint8_t
    {
        PUBLISHER,
        SUBSCRIBER,
        UNDEFINED
    };

    //! Subscriber configuration structure (shared for both publisher and subscriber applications)
    struct subscriber_config
    {
        uint16_t domain = 0;
        uint16_t instances = 4;
        uint16_t samples = 0;
        std::string topic_name = "Square";
    };

    //! Publisher application configuration structure
    struct publisher_config : public subscriber_config
    {
        uint16_t interval = 100;
        uint16_t timeout = 10;
        shape_configuration shape_config;
    };

    //! Configuration structure for the application
    struct topic_instances_config
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
        std::cout << "Usage: topic_instances <entity> [options]"                                << std::endl;
        std::cout << ""                                                                         << std::endl;
        std::cout << "Entities:"                                                                << std::endl;
        std::cout << "  publisher                       Run a publisher entity"                 << std::endl;
        std::cout << "  subscriber                      Run a subscriber entity"                << std::endl;
        std::cout << ""                                                                         << std::endl;
        std::cout << "Common options:"                                                          << std::endl;
        std::cout << "  -h, --help                      Print this help message"                << std::endl;
        std::cout << "  -d <num>, --domain <num>        Domain ID number [0 <= <num> <= 232]"   << std::endl;
        std::cout << "                                  (Default: 0)"                           << std::endl;
        std::cout << "  -i <num>, --instances <num>     Number of instances to create "         << std::endl;
        std::cout << "                                  [0 < <num> <= 10 (N colors)]"           << std::endl;
        std::cout << "                                  (Default: 4 [4 colors])"                << std::endl;
        std::cout << "  -n <str>, --name <str>          Shape Topic name. Possible values are:" << std::endl;
        std::cout << "                                   · Square "                             << std::endl;
        std::cout << "                                   · Triangle "                           << std::endl;
        std::cout << "                                   · Circle "                             << std::endl;
        std::cout << "                                  (Default: Square) "                     << std::endl;
        std::cout << "  -s <num>, --samples <num>       Number of samples per instance to send" << std::endl;
        std::cout << "                                  or receive.    [0 <= <num> <= 65535]"   << std::endl;
        std::cout << "                                  (Default: 0 [unlimited])"               << std::endl;
        std::cout << "Publisher options:"                                                       << std::endl;
        std::cout << "  -c <str>, --color <str>         Shape color. Possible values are: "     << std::endl;
        std::cout << "                                   · RED         · CYAN "                 << std::endl;
        std::cout << "                                   · BLUE        · MAGENTA "              << std::endl;
        std::cout << "                                   · GREEN       · PURPLE "               << std::endl;
        std::cout << "                                   · YELLOW      · GREY "                 << std::endl;
        std::cout << "                                   · ORANGE      · BLACK "                << std::endl;
        std::cout << "                                  Add N colors (N = --instances value)"   << std::endl;
        std::cout << "                                  by separating them with commas (','),"  << std::endl;
        std::cout << "                                  without spaces between colors."         << std::endl;
        std::cout << "                                  (Default: first 4 colors: "             << std::endl;
        std::cout << "                                   'RED,BLUE,GREEN,YELLOW')"              << std::endl;
        std::cout << "            --height <num>        Space bound height where shape moves "  << std::endl;
        std::cout << "                                  (Default: 265)"                         << std::endl;
        std::cout << "            --interval <num>      Time between samples in milliseconds "  << std::endl;
        std::cout << "                                  (Default: 100)"                         << std::endl;
        std::cout << "            --size <num>          Shape size "                            << std::endl;
        std::cout << "                                  [5 <= <num> <= 100]"                    << std::endl;
        std::cout << "                                  (Default: 30)"                          << std::endl;
        std::cout << "            --step <num>          Shape step movement "                   << std::endl;
        std::cout << "                                  (Default: 3)"                           << std::endl;
        std::cout << "  -t <num>, --timeout <num>       Wait time until automatically stop the" << std::endl;
        std::cout << "                                  publisher execution once all samples "  << std::endl;
        std::cout << "                                  have been sent, in seconds "            << std::endl;
        std::cout << "                                  (Default: 10)"                          << std::endl;
        std::cout << "  -w <num>, --width <num>         Space bound width where shape moves "   << std::endl;
        std::cout << "                                  (Default: 230)"                         << std::endl;
        std::exit(return_code);
    }

    /**
     * @brief Parse the command line options and return the topic_instances_config object
     *
     * @param argc number of arguments
     * @param argv array of arguments
     * @return topic_instances_config object with the parsed options
     *
     * @warning This method finishes the execution of the program if the input arguments are invalid
     */
    static topic_instances_config parse_cli_options(
            int argc,
            char* argv[])
    {
        topic_instances_config config;

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
            else if (arg == "-i" || arg == "--instances")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[++i]);
                        if (input <= 0 ||  input > 10)
                        {
                            throw std::out_of_range("instances argument out of range");
                        }
                        else
                        {
                            if (config.entity == CLIParser::EntityKind::PUBLISHER)
                            {
                                config.pub_config.instances = static_cast<uint16_t>(input);
                            }
                            else if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                            {
                                config.sub_config.instances = static_cast<uint16_t>(input);
                            }
                            else
                            {
                                EPROSIMA_LOG_ERROR(CLI_PARSER, "entity not specified for --instances argument");
                                print_help(EXIT_FAILURE);
                            }
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid instances argument for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "instances argument out of range for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
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
            else if (arg == "-d" || arg == "--domain")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        int input = std::stoi(argv[++i]);
                        if (input < 0 || input > 232)
                        {
                            throw std::out_of_range("domain argument out of range");
                        }
                        else
                        {
                            if (config.entity == CLIParser::EntityKind::PUBLISHER)
                            {
                                config.pub_config.domain = static_cast<uint16_t>(input);
                            }
                            else if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                            {
                                config.sub_config.domain = static_cast<uint16_t>(input);
                            }
                            else
                            {
                                EPROSIMA_LOG_ERROR(CLI_PARSER, "entity not specified for --domain argument");
                                print_help(EXIT_FAILURE);
                            }
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid domain argument for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "domain argument out of range for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-n" || arg == "--name")
            {
                if (i + 1 < argc)
                {
                    std::string input = argv[++i];
                    if (input == "CIRCLE" || input == "SQUARE" || input == "TRIANGLE" ||
                            input == "Circle" || input == "Square" || input == "Triangle" ||
                            input == "circle" || input == "square" || input == "triangle")
                    {
                        transform(input.begin(), input.begin() + 1, input.begin(), my_toupper);
                        transform(input.begin() + 1, input.end(), input.begin() + 1, my_tolower);
                        if (config.entity == CLIParser::EntityKind::PUBLISHER)
                        {
                            config.pub_config.topic_name = input;
                        }
                        else if (config.entity == CLIParser::EntityKind::SUBSCRIBER)
                        {
                            config.sub_config.topic_name = input;
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "entity not specified for --name argument");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid topic name argument for " + arg);
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--interval")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        if (config.entity == CLIParser::EntityKind::PUBLISHER)
                        {
                            config.pub_config.interval = static_cast<uint16_t>(std::stoi(argv[++i]));
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "interval can only be used with the publisher entity");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid interval argument for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "interval argument out of range for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--size")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        if (config.entity == CLIParser::EntityKind::PUBLISHER)
                        {
                            int input = std::stoi(argv[++i]);
                            if (input >= 5)
                            {
                                config.pub_config.shape_config.size = input;
                            }
                            else
                            {
                                throw std::invalid_argument("size must be equal or greater than 5");
                            }
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "size can only be used with the publisher entity");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid size argument for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "size argument out of range for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--step")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        if (config.entity == CLIParser::EntityKind::PUBLISHER)
                        {
                            int input = std::stoi(argv[++i]);
                            if (input > 0)
                            {
                                config.pub_config.shape_config.step = input;
                            }
                            else
                            {
                                throw std::invalid_argument("step must be greater than 0");
                            }
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "step can only be used with the publisher entity");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid step argument for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "step argument out of range for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-w" || arg == "--width")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        if (config.entity == CLIParser::EntityKind::PUBLISHER)
                        {
                            int input = std::stoi(argv[++i]);
                            if (input > 0)
                            {
                                config.pub_config.shape_config.width = input;
                            }
                            else
                            {
                                throw std::invalid_argument("width must be greater than 0");
                            }
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "width can only be used with the publisher entity");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid width argument for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "width argument out of range for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "--height")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        if (config.entity == CLIParser::EntityKind::PUBLISHER)
                        {
                            int input = std::stoi(argv[++i]);
                            if (input > 0)
                            {
                                config.pub_config.shape_config.height = input;
                            }
                            else
                            {
                                throw std::invalid_argument("height must be greater than 0");
                            }
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "height can only be used with the publisher entity");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid height argument for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "height argument out of range for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-t" || arg == "--timeout")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        if (config.entity == CLIParser::EntityKind::PUBLISHER)
                        {
                            config.pub_config.timeout = static_cast<uint16_t>(std::stoi(argv[++i]));
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(CLI_PARSER, "timeout can only be used with the publisher entity");
                            print_help(EXIT_FAILURE);
                        }
                    }
                    catch (const std::invalid_argument& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid timeout argument for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                    catch (const std::out_of_range& e)
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "timeout argument out of range for " + arg + ": " + e.what());
                        print_help(EXIT_FAILURE);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(CLI_PARSER, "missing argument for " + arg);
                    print_help(EXIT_FAILURE);
                }
            }
            else if (arg == "-c" || arg == "--color")
            {
                if (i + 1 < argc)
                {
                    if (config.entity == CLIParser::EntityKind::PUBLISHER)
                    {
                        std::string input = argv[++i];
                        std::istringstream iss(input);
                        std::string c;
                        while (std::getline(iss, c, ','))
                        {
                            transform(c.begin(), c.end(), c.begin(), my_toupper);
                            if (c == "RED" || c == "BLUE" || c == "GREEN" || c == "YELLOW" || c == "ORANGE" ||
                                    c == "CYAN" || c == "MAGENTA" || c == "PURPLE" || c == "GREY" || c == "BLACK")
                            {
                                config.pub_config.shape_config.colors.push_back(c);
                            }
                            else
                            {
                                EPROSIMA_LOG_ERROR(CLI_PARSER, "invalid color argument for " + arg);
                                print_help(EXIT_FAILURE);
                            }
                        }
                    }
                    else
                    {
                        EPROSIMA_LOG_ERROR(CLI_PARSER, "color can only be used with the publisher entity");
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

        if (config.entity == CLIParser::EntityKind::PUBLISHER)
        {
            // Calculate shape bounds if applies
            config.pub_config.shape_config.lower_th = config.pub_config.shape_config.size - 5;
            config.pub_config.shape_config.horizontal_th =
                    config.pub_config.shape_config.width - config.pub_config.shape_config.lower_th;
            config.pub_config.shape_config.vertical_th =
                    config.pub_config.shape_config.height - config.pub_config.shape_config.lower_th;

            // Check the number of instances and colors
            if (config.pub_config.shape_config.colors.size() == 0)
            {
                // undefined colors, using default colors
                for (int i = 0; i < config.pub_config.instances; ++i)
                {
                    config.pub_config.shape_config.colors.push_back(shape_color(i));
                }
            }
            else if (config.pub_config.shape_config.colors.size() != config.pub_config.instances)
            {
                EPROSIMA_LOG_ERROR(CLI_PARSER, "number of input colors does not match the number of instances");
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

private:

    //! Private method to declare constant array of colors
    static constexpr std::array<const char*, 10> get_shape_colors()
    {
        return { "RED", "BLUE", "GREEN", "YELLOW", "ORANGE", "CYAN", "MAGENTA", "PURPLE", "GREY", "BLACK" };
    }

public:

    //! Get the shape color as a string
    static std::string shape_color(
            int index)
    {
        constexpr int max_colors = 10;
        const auto& shape_colors = get_shape_colors();
        return shape_colors[index % max_colors];
    }

};

} // namespace topic_instances
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_TOPIC_INSTANCES__CLIPARSER_HPP
