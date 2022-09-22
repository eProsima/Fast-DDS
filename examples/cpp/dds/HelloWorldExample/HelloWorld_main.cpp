// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file HelloWorld_main.cpp
 *
 */

#include <limits>

#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/log/Log.h>

#include <optionparser.hpp>

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

    static option::ArgStatus Unknown(
            const option::Option& option,
            bool msg)
    {
        if (msg)
        {
            print_error("Unknown option '", option, "'\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Required(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0 && option.arg[0] != 0)
        {
            return option::ARG_OK;
        }

        if (msg)
        {
            print_error("Option '", option, "' requires an argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Numeric(
            const option::Option& option,
            bool msg)
    {
        char* endptr = 0;
        if ( option.arg != nullptr)
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

    template<long min = 0, long max = std::numeric_limits<long>::max()>
    static option::ArgStatus NumericRange(
            const option::Option& option,
            bool msg)
    {
        static_assert(min <= max, "NumericRange: invalid range provided.");

        char* endptr = 0;
        if ( option.arg != nullptr )
        {
            long value = strtol(option.arg, &endptr, 10);
            if ( endptr != option.arg && *endptr == 0 &&
                    value >= min && value <= max)
            {
                return option::ARG_OK;
            }
        }

        if (msg)
        {
            std::ostringstream os;
            os << "' requires a numeric argument in range ["
               << min << ", " << max << "]" << std::endl;
            print_error("Option '", option, os.str().c_str());
        }

        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus String(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires an argument\n");
        }
        return option::ARG_ILLEGAL;
    }

};

enum  optionIndex
{
    UNKNOWN_OPT,
    HELP,
    SAMPLES,
    INTERVAL,
    ENVIRONMENT
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0, "", "",                Arg::None,
      "Usage: HelloWorldExample <publisher|subscriber>\n\nGeneral options:" },
    { HELP,    0, "h", "help",               Arg::None,      "  -h \t--help  \tProduce help message." },
    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nPublisher options:"},
    { SAMPLES, 0, "s", "samples",            Arg::NumericRange<>,
      "  -s <num>, \t--samples=<num>  \tNumber of samples (0, default, infinite)." },
    { INTERVAL, 0, "i", "interval",          Arg::NumericRange<>,
      "  -i <num>, \t--interval=<num>  \tTime between samples in milliseconds (Default: 100)." },
    { ENVIRONMENT, 0, "e", "env",            Arg::None,       "  -e \t--env   \tLoad QoS from environment." },
    { 0, 0, 0, 0, 0, 0 }
};

int main(
        int argc,
        char** argv)
{
    int columns;

#if defined(_WIN32)
    char* buf = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, "COLUMNS") == 0 && buf != nullptr)
    {
        columns = strtol(buf, nullptr, 10);
        free(buf);
    }
    else
    {
        columns = 80;
    }
#else
    columns = getenv("COLUMNS") ? atoi(getenv("COLUMNS")) : 80;
#endif // if defined(_WIN32)

    std::cout << "Starting " << std::endl;

    int type = 1;
    uint32_t count = 10;
    uint32_t sleep = 100;
    bool use_environment_qos = false;

    argc -= (argc > 0);
    argv += (argc > 0); // skip program name argv[0] if present
    option::Stats stats(true, usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(true, usage, argc, argv, &options[0], &buffer[0]);

    try
    {
        if (parse.error())
        {
            throw 1;
        }

        if (options[HELP] || options[UNKNOWN_OPT])
        {
            throw 1;
        }

        // For backward compatibility count and sleep may be given positionally
        if (parse.nonOptionsCount() > 3 || parse.nonOptionsCount() == 0)
        {
            throw 2;
        }

        // Decide between publisher or subscriber
        const char* type_name = parse.nonOption(0);

        // make sure is the first option.
        // type_name and buffer[0].name reference the original command line char array
        // type_name must precede any other arguments in the array.
        // Note buffer[0].arg may be null for non-valued options and is not reliable for
        // testing purposes.
        if (parse.optionsCount() && type_name >= buffer[0].name)
        {
            throw 2;
        }

        if (strcmp(type_name, "publisher") == 0)
        {
            type = 1;
        }
        else if (strcmp(type_name, "subscriber") == 0)
        {
            type = 2;
        }
        else
        {
            throw 2;
        }
    }
    catch (int error)
    {
        if ( error == 2 )
        {
            std::cerr << "ERROR: first argument must be <publisher|subscriber> followed by - or -- options"
                      << std::endl;
        }
        option::printUsage(fwrite, stdout, usage, columns);
        return error;
    }

    // Decide between the old and new syntax
    if (parse.nonOptionsCount() > 1)
    {
        // old syntax, only affects publishers
        // old and new syntax cannot be mixed
        if (type != 1 || parse.optionsCount() > 0)
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 1;
        }

        count = atoi(parse.nonOption(1));

        if (parse.nonOptionsCount() == 3)
        {
            sleep = atoi(parse.nonOption(2));
        }
    }
    else
    {
        // new syntax
        option::Option* opt = options[SAMPLES];
        if (opt)
        {
            count = strtol(opt->arg, nullptr, 10);
        }

        opt = options[INTERVAL];
        if (opt)
        {
            sleep = strtol(opt->arg, nullptr, 10);
        }

        opt = options[ENVIRONMENT];
        if (opt)
        {
            use_environment_qos = true;
        }
    }

    switch (type)
    {
        case 1:
        {
            HelloWorldPublisher mypub;
            if (mypub.init(use_environment_qos))
            {
                mypub.run(count, sleep);
            }
            break;
        }
        case 2:
        {
            HelloWorldSubscriber mysub;
            if (mysub.init(use_environment_qos))
            {
                mysub.run();
            }
            break;
        }
    }
    Log::Reset();
    return 0;
}
