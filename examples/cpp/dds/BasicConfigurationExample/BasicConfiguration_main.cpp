// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file BasicConfiguration_main.cpp
 *
 */

#include <string>
#include <vector>

#include "arg_configuration.h"
#include "BasicConfigurationPublisher.h"
#include "BasicConfigurationSubscriber.h"
#include "types.hpp"

enum EntityType
{
    PUBLISHER,
    SUBSCRIBER
};

std::vector<std::string> split(
        const std::string& s,
        char delim)
{
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim))
    {
        result.push_back (item);
    }

    return result;
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

    EntityType type = PUBLISHER;
    std::vector<std::string> topic_names = {"HelloWorldTopic"};
    int count = 0;
    long sleep = 100;
    long init_sleep = 0;
    int num_wait_matched = 0;
    bool single_thread = true;
    int domain = 0;
    bool async = false;
    bool random = false;
    TransportType transport = DEFAULT;
    bool reliable = false;
    bool transient = false; // transient local
    bool realloc = false;
    bool dynamic = false;
    long msg_size = 20;
    if (argc > 1)
    {
        if (!strcmp(argv[1], "publisher"))
        {
            type = PUBLISHER;
        }
        else if (!strcmp(argv[1], "subscriber"))
        {
            type = SUBSCRIBER;
        }
        // check if first argument is help, needed because we skip it when parsing
        else if (!(strcmp(argv[1], "-h") && strcmp(argv[1], "--help")))
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
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

        if (options[optionIndex::HELP])
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
        }

        for (int i = 0; i < parse.optionsCount(); ++i)
        {
            option::Option& opt = buffer[i];
            switch (opt.index())
            {
                case optionIndex::HELP:
                    // not possible, because handled further above and exits the program
                    break;

                case optionIndex::TOPIC:
                    topic_names = split(std::string(opt.arg), ',');
                    break;

                case optionIndex::DOMAIN_ID:
                    domain = strtol(opt.arg, nullptr, 10);
                    break;

                case optionIndex::SAMPLES:
                    count = strtol(opt.arg, nullptr, 10);
                    break;

                case optionIndex::INTERVAL:
                    if (type == PUBLISHER)
                    {
                        sleep = strtol(opt.arg, nullptr, 10);
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case optionIndex::MSG_SIZE:
                    if (type == PUBLISHER)
                    {
                        msg_size = strtol(opt.arg, nullptr, 10);
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case optionIndex::INIT_SLEEP:
                    if (type == PUBLISHER)
                    {
                        init_sleep = strtol(opt.arg, nullptr, 10);
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case optionIndex::MULTITHREADING:
                    single_thread = false;
                    break;

                case optionIndex::WAIT:
                    if (type == PUBLISHER)
                    {
                        num_wait_matched = strtol(opt.arg, nullptr, 10);
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case optionIndex::ASYNC:
                    if (type == PUBLISHER)
                    {
                        async = true;
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case optionIndex::RANDOM:
                    if (type == PUBLISHER)
                    {
                        random = true;
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case optionIndex::TRANSPORT:
                    if (strcmp(opt.arg, "shm") == 0)
                    {
                        transport = SHM;
                    }
                    else if (strcmp(opt.arg, "udp") == 0 || (strcmp(opt.arg, "udpv4") == 0))
                    {
                        transport = UDPv4;
                    }
                    else if (strcmp(opt.arg, "udpv6") == 0)
                    {
                        transport = UDPv6;
                    }
                    break;

                case optionIndex::RELIABLE:
                    reliable = true;
                    break;

                case optionIndex::TRANSIENT_LOCAL:
                    transient = true;
                    break;

                case optionIndex::REALLOC:
                    realloc = true;
                    dynamic = false;
                    break;

                case optionIndex::DYNAMIC:
                    dynamic = true;
                    realloc = false;
                    break;

                case optionIndex::UNKNOWN_OPT:
                    std::cerr << "ERROR: " << opt.name << " is not a valid argument." << std::endl;
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 1;
                    break;
            }
        }
        if (transient && !reliable)
        {
            std::cerr << "WARNING: --transient will take no effect since not reliable." << std::endl;
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
        case PUBLISHER:
        {
            HelloWorldPublisher mypub;
            if (mypub.init(topic_names, static_cast<uint32_t>(domain), static_cast<uint32_t>(num_wait_matched), async,
                    transport, reliable, transient, realloc, dynamic, msg_size))
            {
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep), static_cast<uint32_t>(init_sleep),
                        single_thread, random);
            }
            break;
        }
        case SUBSCRIBER:
        {
            HelloWorldSubscriber mysub;
            if (mysub.init(topic_names, static_cast<uint32_t>(count), static_cast<uint32_t>(domain), transport,
                    reliable, transient, realloc, dynamic))
            {
                mysub.run(static_cast<uint32_t>(count));
            }
            break;
        }
    }
    return 0;
}
