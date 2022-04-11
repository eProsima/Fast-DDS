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

struct Arg : public option::Arg
{
    static void print_error(
            const char* msg1,
            const option::Option& opt,
            const char* msg2)
    {
        fprintf(stderr, "%s", msg1);
        fwrite(opt.name, opt.namelen, 1, stderr);
        fprintf(stderr, "%s", msg2);
    }

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
    argc -= (argc > 0);
    argv += (argc > 0); // skip program name argv[0] if present
    option::Stats stats(usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);
    if (parse.error())
    {
        return 1;
    }

    if (options[HELP])
    {
        option::printUsage(fwrite, stdout, usage);
        return 0;
    }
    else if (options[UNKNOWN_OPTION])
    {
        std::cerr << "ERROR: " << options[UNKNOWN_OPTION].name << " is not a valid argument." << std::endl;
        option::printUsage(fwrite, stdout, usage);
        return 1;
    }

    int type = 1;
    int count = 0;
    int sleep = 100;
    bool custom_filter = false;
    if (options[PUBLISHER] && options[SUBSCRIBER])
    {
        std::cerr << "ERROR: select either publisher or subscriber option" << std::endl;
        option::printUsage(fwrite, stdout, usage);
        return 1;
    }
    else if (options[PUBLISHER])
    {
        if (options[FILTER])
        {
            std::cerr << "ERROR: option filter is a subscriber option" << std::endl;
            option::printUsage(fwrite, stdout, usage);
            return 1;
        }
        if (options[SAMPLES])
        {
            count = strtol(options[SAMPLES].arg, nullptr, 10);
        }
        if (options[INTERVAL])
        {
            sleep = strtol(options[INTERVAL].arg, nullptr, 10);
        }
    }
    else if (options[SUBSCRIBER])
    {
        type = 2;
        if (options[SAMPLES] || options[INTERVAL])
        {
            std::cerr << "ERROR: options samples and interval are publisher options" << std::endl;
            option::printUsage(fwrite, stdout, usage);
            return 1;
        }
        if (options[FILTER])
        {
            if (0 == strcmp(options[FILTER].arg, "custom"))
            {
                custom_filter = true;
            }
            else if (0 != strcmp(options[FILTER].arg, "default"))
            {
                std::cerr << "ERROR: filter option should be either custom or default" << std::endl;
                option::printUsage(fwrite, stdout, usage);
                return 1;
            }
        }
    }
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
            ContentFilteredTopicExamplePublisher mypub;
            if (mypub.init())
            {
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep));
            }
            break;
        }
        case 2:
        {
            ContentFilteredTopicExampleSubscriber mysub;
            if (mysub.init(custom_filter))
            {
                mysub.run();
            }
            break;
        }
    }
    Log::Reset();
    return 0;
}
