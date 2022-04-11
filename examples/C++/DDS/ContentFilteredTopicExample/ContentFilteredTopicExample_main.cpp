// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file CustomFilter_main.cpp
 *
 */

#include <vector>

#include <fastdds/dds/log/Log.hpp>
#include <optionparser.hpp>

#include "ContentFilteredTopicExamplePublisher.hpp"
#include "ContentFilteredTopicExampleSubscriber.hpp"

using eprosima::fastdds::dds::Log;
namespace option = eprosima::option;

// Argument checkers
struct Arg : public option::Arg
{
    // Auxiliary method for printing errors while checking arguments
    static void print_error(
            const char* msg1,
            const option::Option& opt,
            const char* msg2)
    {
        fprintf(stderr, "%s", msg1);
        fwrite(opt.name, opt.namelen, 1, stderr);
        fprintf(stderr, "%s", msg2);
    }

    // Argument checker for numeric options
    static option::ArgStatus Numeric(
            const option::Option& option,
            bool msg)
    {
        char* endptr = 0;
        if (nullptr != option.arg)
        {
            strtol(option.arg, &endptr, 10);
            if (endptr != option.arg && *endptr == 0)
            {
                return option::ARG_OK;
            }
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a numeric argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    // Argument checker for string options
    static option::ArgStatus String(
            const option::Option& option,
            bool msg)
    {
        if (nullptr != option.arg)
        {
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a string argument\n");
        }
        return option::ARG_ILLEGAL;
    }

};

// Possible options
enum optionIndex
{
    UNKNOWN_OPTION,
    HELP,
    PUBLISHER,
    SUBSCRIBER,
    SAMPLES,
    INTERVAL,
    FILTER
};

// Usage description
const option::Descriptor usage[] = {
    { UNKNOWN_OPTION, 0, "", "", Arg::None,
      "Usage: ContentFilteredTopicExample [--publisher|--subscriber] [OPTIONS]\n\nGeneral options:" },
    { HELP, 0, "h", "help", Arg::None, "  -h\t--help\tProduce help message." },
    { PUBLISHER, 0, "", "publisher", Arg::None, "\t--publisher\tLaunch publisher application." },
    { SUBSCRIBER, 0, "", "subscriber", Arg::None, "\t--subscriber\tLaunch subscriber application." },

    { UNKNOWN_OPTION, 0, "", "", Arg::None, "\nPublisher options:" },
    { SAMPLES, 0, "s", "samples", Arg::Numeric,
      "  -s <num>\t--samples=<num>\tNumber of samples (Default: 0 => infinite samples)." },
    { INTERVAL, 0, "i", "interval", Arg::Numeric,
      "  -i <num>\t--interval=<num>\tTime between samples in milliseconds (Default: 100 ms)." },

    { UNKNOWN_OPTION, 0, "", "", Arg::None, "\nSubscriber options:" },
    { FILTER, 0, "f", "filter", Arg::String,
      "  -f <default/custom>\t--filter=<default/custom>\tKind of Content Filter to use (Default: DDS SQL default filter"
    },

    { 0, 0, 0, 0, 0, 0 }
};

int main(
        int argc,
        char** argv)
{
    std::cout << "Starting " << std::endl;

    // Parse arguments using optionparser
    // skip program name argv[0] if present (optionparser limitation)
    argc -= (argc > 0);
    argv += (argc > 0);
    option::Stats stats(usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);
    if (parse.error())
    {
        return 1;
    }

    // If help option selected, print usage description and exit
    if (options[HELP])
    {
        option::printUsage(fwrite, stdout, usage);
        return 0;
    }
    // If option is not recognized, print usage description and exit with error code
    else if (options[UNKNOWN_OPTION])
    {
        std::cerr << "ERROR: " << options[UNKNOWN_OPTION].name << " is not a valid argument." << std::endl;
        option::printUsage(fwrite, stdout, usage);
        return 1;
    }

    // Initialize variables with default values
    int type = 1;
    int count = 0;
    int sleep = 100;
    bool custom_filter = false;
    // If both publisher and subscriber options are selected, print usage description and exit with error code
    if (options[PUBLISHER] && options[SUBSCRIBER])
    {
        std::cerr << "ERROR: select either publisher or subscriber option" << std::endl;
        option::printUsage(fwrite, stdout, usage);
        return 1;
    }
    // Publisher option selected
    else if (options[PUBLISHER])
    {
        // If any subscriber option is selected, print usage description and exit with error code
        if (options[FILTER])
        {
            std::cerr << "ERROR: option filter is a subscriber option" << std::endl;
            option::printUsage(fwrite, stdout, usage);
            return 1;
        }
        // If any optional publisher option is selected, set the corresponding value
        if (options[SAMPLES])
        {
            count = strtol(options[SAMPLES].arg, nullptr, 10);
        }
        if (options[INTERVAL])
        {
            sleep = strtol(options[INTERVAL].arg, nullptr, 10);
        }
    }
    // Subscriber option selected
    else if (options[SUBSCRIBER])
    {
        type = 2;
        // If any publisher option is selected, print usage description and exit with error code
        if (options[SAMPLES] || options[INTERVAL])
        {
            std::cerr << "ERROR: options samples and interval are publisher options" << std::endl;
            option::printUsage(fwrite, stdout, usage);
            return 1;
        }
        // If any optional subscriber option is selected, set the corresponding value
        if (options[FILTER])
        {
            if (0 == strcmp(options[FILTER].arg, "custom"))
            {
                custom_filter = true;
            }
            // If filter option does not have one of the expected values, print usage description and exit with error
            // code
            else if (0 != strcmp(options[FILTER].arg, "default"))
            {
                std::cerr << "ERROR: filter option should be either custom or default" << std::endl;
                option::printUsage(fwrite, stdout, usage);
                return 1;
            }
        }
    }
    // If no publisher or subscriber option has been selected, print usage description and exit with error code
    else
    {
        std::cerr << "ERROR: select either publisher or subscriber option" << std::endl;
        option::printUsage(fwrite, stdout, usage);
        return 1;
    }

    switch (type)
    {
        case 1:
        {
            // Initialize and run publisher application
            ContentFilteredTopicExamplePublisher mypub;
            if (mypub.init())
            {
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep));
            }
            break;
        }
        case 2:
        {
            // Initialize and run subscriber application
            ContentFilteredTopicExampleSubscriber mysub;
            if (mysub.init(custom_filter))
            {
                mysub.run();
            }
            break;
        }
    }
    // Flush Fast DDS Log before closing application
    Log::Reset();
    return 0;
}
