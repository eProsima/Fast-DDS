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
 * @file HelloWorld_main.cpp
 *
 */

#include <string>

#include "arg_configuration.h"
#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"

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

    // TCP transport
    bool tcp = false;

    // Discovery Server
    bool discovery_server = false;
    eprosima::fastdds::rtps::Locator discovery_server_locator;
    std::cmatch discovery_server_mr;
    std::string discovery_server_address;
    uint16_t discovery_server_port = 60006;  // default physical port
    discovery_server_locator.port = discovery_server_port;

    // Remote Discovery Server
    bool discovery_client = false;
    eprosima::fastdds::rtps::Locator discovery_remote_locator;
    std::cmatch discovery_remote_mr;
    std::string discovery_remote_address;
    uint16_t discovery_remote_port = 60006;  // default physical port
    discovery_remote_locator.port = discovery_remote_port;

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
                case optionIndex::HELP:
                    // not possible, because handled further above and exits the program
                    break;

                case optionIndex::TOPIC:
                    topic_name = std::string(opt.arg);
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

                case optionIndex::DISCOVERY_SERVER_LOCATOR:
                    discovery_server = true;
                    if (regex_match(opt.arg, discovery_server_mr, Arg::ipv4))
                    {
                        std::cmatch::iterator it = discovery_server_mr.cbegin();
                        discovery_server_address = (++it)->str();

                        if ((++it)->matched)
                        {
                            int port_int = std::stoi(it->str());
                            if (port_int <= 65535)
                            {
                                discovery_server_port = static_cast<uint16_t>(port_int);
                            }
                        }
                    }

                    if (!discovery_server_address.empty() && discovery_server_port > 1000)
                    {
                        eprosima::fastrtps::rtps::IPLocator::setPhysicalPort(discovery_server_locator,
                                discovery_server_port);
                        eprosima::fastrtps::rtps::IPLocator::setLogicalPort(discovery_server_locator,
                                discovery_server_port);
                        eprosima::fastrtps::rtps::IPLocator::setIPv4(discovery_server_locator,
                                discovery_server_address);
                        eprosima::fastrtps::rtps::IPLocator::setWan(discovery_server_locator,
                                discovery_server_address);
                    }
                    break;

                case optionIndex::DISCOVERY_REMOTE_LOCATOR:
                    discovery_client = true;
                    if (regex_match(opt.arg, discovery_remote_mr, Arg::ipv4))
                    {
                        std::cmatch::iterator it = discovery_remote_mr.cbegin();
                        discovery_remote_address = (++it)->str();

                        if ((++it)->matched)
                        {
                            int port_int = std::stoi(it->str());
                            if (port_int <= 65535)
                            {
                                discovery_remote_port = static_cast<uint16_t>(port_int);
                            }
                        }
                    }

                    if (!discovery_remote_address.empty() && discovery_remote_port > 1000)
                    {
                        eprosima::fastrtps::rtps::IPLocator::setPhysicalPort(discovery_remote_locator,
                                discovery_remote_port);
                        eprosima::fastrtps::rtps::IPLocator::setLogicalPort(discovery_remote_locator,
                                discovery_remote_port);
                        eprosima::fastrtps::rtps::IPLocator::setIPv4(discovery_remote_locator,
                                discovery_remote_address);
                    }
                    break;

                case optionIndex::TCP:
                    tcp = true;
                    break;

                case optionIndex::UNKNOWN_OPT:
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

    if (!discovery_server && !discovery_client)
    {
        std::cerr << "ERROR: at least one (remote) discovery server address is required." << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    switch (type)
    {
        case PUBLISHER:
        {
            HelloWorldPublisher mypub;
            if (mypub.init(topic_name, static_cast<uint32_t>(num_wait_matched), tcp,
                discovery_server, discovery_server_locator, discovery_server_address, static_cast<uint16_t>(discovery_server_port),
                discovery_client, discovery_remote_locator))
            {
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep));
            }
            break;
        }
        case SUBSCRIBER:
        {
            HelloWorldSubscriber mysub;
            if (mysub.init(topic_name, static_cast<uint32_t>(count), tcp,
                discovery_server, discovery_server_locator, discovery_server_address, static_cast<uint16_t>(discovery_server_port),
                discovery_client, discovery_remote_locator))
            {
                mysub.run(static_cast<uint32_t>(count));
            }
            break;
        }
    }
    return 0;
}
