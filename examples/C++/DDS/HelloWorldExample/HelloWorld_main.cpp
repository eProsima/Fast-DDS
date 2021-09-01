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

#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"

#include <string>

#include <optionparser.h>

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
        if (option.arg != 0 && strtol(option.arg, &endptr, 10))
        {
        }
        if (endptr != option.arg && *endptr == 0)
        {
            return option::ARG_OK;
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
        if (option.arg != 0)
        {
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a string argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Transport(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            std::string transport = std::string(option.arg);
            if (transport != "shm" && transport != "udp")
            {
                if (msg)
                {
                    print_error("Option '", option, "' only accepts <shm|udp> values\n");
                }
                return option::ARG_ILLEGAL;
            }
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a string argument\n");
        }
        return option::ARG_ILLEGAL;
    }

};

enum  optionIndex
{
    UNKNOWN_OPT,
    HELP,
    TOPIC,
    WAIT,
    SAMPLES,
    INTERVAL,
    ASYNC,
    DOMAIN,
    TRANSPORT,
    RELIABLE,
    TRANSIENT
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0, "", "",                Arg::None,
      "Usage: HelloWorldExample <publisher|subscriber>\n\nGeneral options:" },
    { HELP,    0, "h", "help",               Arg::None,      "  -h \t--help  \tProduce help message." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nPublisher options:"},
    { TOPIC, 0, "t", "topic",                  Arg::String,
      "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: HelloWorldTopic)." },
    { DOMAIN, 0, "d", "domain",                Arg::Numeric,
      "  -d <id> \t--domain=<id>  \tDDS domain ID (Default: 0)." },
    { WAIT, 0, "w", "wait",                 Arg::Numeric,
      "  -w <num> \t--wait=<num> \tNumber of matched subscribers required to publish"
      "(Default: 0 => does not wait)." },
    { SAMPLES, 0, "s", "samples",              Arg::Numeric,
      "  -s <num> \t--samples=<num>  \tNumber of samples to send (Default: 0 => infinite samples)." },
    { INTERVAL, 0, "i", "interval",            Arg::Numeric,
      "  -i <num> \t--interval=<num>  \tTime between samples in milliseconds (Default: 100)." },
    { ASYNC, 0, "a", "async",               Arg::None,
      "  -a \t--async \tAsynchronous publish mode (synchronous by default)." },
    { TRANSPORT, 0, "", "transport",        Arg::Transport,
      "  \t--transport=<shm|udp> \tUse shared-memory|UDP transport (Default: data-sharing > shared-memory > UDP "
      "in this order of priority depending on execution context)." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nSubscriber options:"},
    { TOPIC, 0, "t", "topic",                  Arg::String,
      "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: HelloWorldTopic)." },
    { DOMAIN, 0, "d", "domain",                Arg::Numeric,
      "  -d <id> \t--domain=<id>  \tDDS domain ID (Default: 0)." },
    { SAMPLES, 0, "s", "samples",              Arg::Numeric,
      "  -s <num> \t--samples=<num>  \tNumber of samples to wait for (Default: 0 => infinite samples)." },
    { TRANSPORT, 0, "", "transport",        Arg::Transport,
      "  \t--transport=<shm|udp> \tUse shared-memory|UDP transport (Default: data-sharing > shared-memory > UDP "
      "in this order of priority depending on execution context)." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nQoS options:"},
    { RELIABLE, 0, "r", "reliable",         Arg::None,
      "  -r \t--reliable \tSet reliability to reliable (best-effort by default)." },
    { TRANSIENT, 0, "", "transient",        Arg::None,
      "  \t--transient \tSet durability to transient local (volatile by default)." },


    { 0, 0, 0, 0, 0, 0 }
};

void print_warning(
        std::string type,
        const char* opt)
{
    std::cerr << "WARNING: " << opt << " is a " << type << " option, ignoring argument." << std::endl;
}

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
    std::string topic_name = "HelloWorldTopic";
    int count = 0;
    long sleep = 100;
    int numWaitMatched = 0;
    int domain = 0;
    bool async = false;
    std::string transport;
    bool reliable = false;
    bool transient = false;
    if (argc > 1)
    {
        if (strcmp(argv[1], "publisher") == 0)
        {
            type = 1;
        }
        else if (strcmp(argv[1], "subscriber") == 0)
        {
            type = 2;
        }
        else
        {
            std::cerr << "ERROR: first argument can only be <publisher|subscriber>" << std::endl;
            option::printUsage(fwrite, stdout, usage, columns);
            return 1;
        }

        argc -= (argc > 0);
        argv += (argc > 0); // skip program name argv[0] if present
        --argc; ++argv; // skip pub/sub argument
        option::Stats stats(usage, argc, argv);
        std::vector<option::Option> options(stats.options_max);
        std::vector<option::Option> buffer(stats.buffer_max);
        option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

        if (parse.error())
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 1;
        }

        if (options[HELP])
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
        }

        for (int i = 0; i < parse.optionsCount(); ++i)
        {
            option::Option& opt = buffer[i];
            switch (opt.index())
            {
                case HELP:
                    // not possible, because handled further above and exits the program
                    break;

                case TOPIC:
                    topic_name = std::string(opt.arg);
                    break;

                case DOMAIN:
                    domain = strtol(opt.arg, nullptr, 10);
                    break;

                case SAMPLES:
                    count = strtol(opt.arg, nullptr, 10);
                    break;

                case INTERVAL:
                    if (type == 1)
                    {
                        sleep = strtol(opt.arg, nullptr, 10);
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case WAIT:
                    if (type == 1)
                    {
                        numWaitMatched = strtol(opt.arg, nullptr, 10);
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case ASYNC:
                    if (type == 1)
                    {
                        async = true;
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case TRANSPORT:
                    transport = std::string(opt.arg);
                    break;

                case RELIABLE:
                    reliable = true;
                    break;

                case TRANSIENT:
                    transient = true;
                    break;

                case UNKNOWN_OPT:
                    std::cerr << "ERROR: " << opt.name << " is not a valid argument." << std::endl;
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 1;
                    break;
            }
        }
    }
    else
    {
        std::cerr << "ERROR: <publisher|subscriber> argument is required." << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    switch (type)
    {
        case 1:
        {
            HelloWorldPublisher mypub;
            if (mypub.init(topic_name, static_cast<uint32_t>(domain), async, transport, reliable, transient))
            {
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep),
                        static_cast<uint32_t>(numWaitMatched));
            }
            break;
        }
        case 2:
        {
            HelloWorldSubscriber mysub;
            if (mysub.init(topic_name, static_cast<uint32_t>(count), static_cast<uint32_t>(domain), transport,
                    reliable, transient))
            {
                mysub.run(static_cast<uint32_t>(count));
            }
            break;
        }
    }
    return 0;
}
