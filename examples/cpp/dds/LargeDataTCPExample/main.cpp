// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file main.cpp
 *
 */

#include <cstdlib>
#include <cstring>

#include <iostream>

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

#include "arg_configuration.h"
#include "LargeDataPublisher.h"
#include "LargeDataSubscriber.h"

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;

enum EntityType
{
    PUBLISHER,
    SUBSCRIBER
};

int main(
        int argc,
        char** argv)
{
    std::cout << "Starting " << std::endl;

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
    ReliabilityQosPolicyKind rel_kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    DurabilityQosPolicyKind dur_kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
    int domain = 0;
    uint16_t pub_rate = 50;
    uint32_t data_size = 100;
    TCPMode tcp_mode = TCPMode::NONE;
    std::string wan_ip = "127.0.0.1";
    int wan_port = 20000;

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
    try
    {
        if (parse.nonOptionsCount() != 1)
        {
            throw 1;
        }

        const char* type_name = parse.nonOption(0);

        // make sure is the first option.
        // type_name and buffer[0].name reference the original command line char array
        // type_name must precede any other arguments in the array.
        // Note buffer[0].arg may be null for non-valued options and is not reliable for
        // testing purposes.
        if (parse.optionsCount() && type_name >= buffer[0].name)
        {
            throw 1;
        }

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
            throw 1;
        }
    }
    catch (int error)
    {
        std::cerr << "ERROR: first argument must be <publisher|subscriber> followed by - or -- options" << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return error;
    }

    for (int i = 0; i < parse.optionsCount(); ++i)
    {
        option::Option& opt = buffer[i];
        switch (opt.index())
        {
            case optionIndex::HELP:
                // not possible, because handled further above and exits the program
                break;

            case optionIndex::DOMAIN_ID:
                domain = strtol(opt.arg, nullptr, 10);
                break;

            case optionIndex::RELIABLE:
                rel_kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
                break;

            case optionIndex::TRANSIENT_LOCAL:
                dur_kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
                break;

            case optionIndex::PUB_RATE:
                pub_rate = strtol(opt.arg, nullptr, 10);
                break;

            case optionIndex::DATA_SIZE:
                data_size = strtol(opt.arg, nullptr, 10);
                break;

            case optionIndex::TCP_MODE:
                tcp_mode = static_cast<TCPMode>(strtol(opt.arg, nullptr, 10));
                break;

            case optionIndex::WAN_ADDRESS:
                wan_ip = std::string(opt.arg);
                break;

            case optionIndex::WAN_PORT:
                wan_port = (uint64_t)strtol(opt.arg, nullptr, 10);
                break;

            case optionIndex::UNKNOWN_OPT:
                std::cerr << "ERROR: " << opt.name << " is not a valid argument." << std::endl;
                option::printUsage(fwrite, stdout, usage, columns);
                return 1;
                break;
        }
    }

    switch (type)
    {
        case EntityType::PUBLISHER:
        {
            LargeDataPublisher mypub(data_size);
            if (mypub.init(domain, rel_kind, dur_kind, pub_rate, tcp_mode, wan_ip, wan_port))
            {
                mypub.run();
            }
            break;
        }
        case EntityType::SUBSCRIBER:
        {
            LargeDataSubscriber mysub;
            if (mysub.init(domain, rel_kind, dur_kind, tcp_mode, wan_ip, wan_port))
            {
                mysub.run();
            }
            break;
        }
    }
    Log::Reset();
    return 0;
}
