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

#include "arg_configuration.h"
#include "BasicConfigurationPublisher.h"
#include "BasicConfigurationSubscriber.h"
#include "types.hpp"

enum EntityType
{
    PUBLISHER,
    SUBSCRIBER
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

    EntityType type = PUBLISHER;
    std::string topic_name = "HelloWorldTopic";
    int count = 0;
    long sleep = 100;
    int num_wait_matched = 0;
    int domain = 0;
    bool async = false;
    TransportType transport = DEFAULT;
    int hops = -1;
    bool reliable = false;
    bool transient = false; // transient local
                            //
    argc -= (argc > 0);
    argv += (argc > 0); // skip program name argv[0] if present
    option::Stats stats(true, usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(true, usage, argc, argv, &options[0], &buffer[0]);

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

    // Decide between publisher or subscriber
    if (parse.nonOptionsCount())
    {
        const char* type_name = parse.nonOption(0);

        if (strcmp(type_name, "publisher") == 0)
        {
            type = PUBLISHER;
        }
        else if (strcmp(type_name, "subscriber") == 0)
        {
            type = SUBSCRIBER;
        }
        else
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 1;
        }
    }
    else
    {
        std::cerr << "ERROR: first argument can only be <publisher|subscriber>" << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
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
                topic_name = std::string(opt.arg);
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

            case optionIndex::TTL:
                hops = strtol(opt.arg, nullptr, 10);
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

    switch (type)
    {
        case PUBLISHER:
        {
            HelloWorldPublisher mypub;
            if (mypub.init(topic_name, static_cast<uint32_t>(domain), static_cast<uint32_t>(num_wait_matched), async,
                    transport, reliable, transient, hops))
            {
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep));
            }
            break;
        }
        case SUBSCRIBER:
        {
            HelloWorldSubscriber mysub;
            if (mysub.init(topic_name, static_cast<uint32_t>(count), static_cast<uint32_t>(domain), transport,
                    reliable, transient, hops))
            {
                mysub.run(static_cast<uint32_t>(count));
            }
            break;
        }
    }
    return 0;
}
