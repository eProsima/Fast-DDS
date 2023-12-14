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
 * @file DiscoveryServer_main.cpp
 *
 */

#include <string>

#include "arg_configuration.h"
#include "DiscoveryServerPublisher.h"
#include "DiscoveryServerServer.h"
#include "DiscoveryServerSubscriber.h"

enum class EntityKind
{
    PUBLISHER,
    SUBSCRIBER,
    SERVER,
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

    EntityKind type = EntityKind::PUBLISHER;
    std::string topic_name = "HelloWorldTopic";
    int count = 0;
    long sleep = 100;

    // Transport
    TransportKind transport = TransportKind::UDPv4;

    // Discovery Server connection
    std::string connection_address = "127.0.0.1";   // default ip address
    uint16_t connection_port = 16166;   // default physical port
    uint16_t connection_ds_id = 0;   // default DS id
    bool id_ds_set = false;

    // Discovery Server listening
    std::string listening_address = "127.0.0.1";   // default ip address
    uint16_t listening_port = 16166;   // default physical port
    uint16_t listening_ds_id = 0;   // default DS id
    uint32_t timeout = 0;   // default DS id

    if (argc > 1)
    {
        if (!strcmp(argv[1], "publisher"))
        {
            type = EntityKind::PUBLISHER;
        }
        else if (!strcmp(argv[1], "subscriber"))
        {
            type = EntityKind::SUBSCRIBER;
        }
        else if (!strcmp(argv[1], "server"))
        {
            type = EntityKind::SERVER;
        }
        // check if first argument is help, needed because we skip it when parsing
        else if (!(strcmp(argv[1], "-h") && strcmp(argv[1], "--help")))
        {
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
        }
        else
        {
            std::cerr << "ERROR: first argument can only be <publisher|subscriber|server>" << std::endl;
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
                    if (type == EntityKind::SERVER)
                    {
                        print_warning("publisher|subscriber", opt.name);
                    }
                    else
                    {
                        topic_name = std::string(opt.arg);
                    }
                    break;

                case optionIndex::SAMPLES:
                    if (type == EntityKind::SERVER)
                    {
                        print_warning("publisher|subscriber", opt.name);
                    }
                    else
                    {
                        count = strtol(opt.arg, nullptr, 10);
                    }
                    break;

                case optionIndex::INTERVAL:
                    if (type == EntityKind::PUBLISHER)
                    {
                        sleep = strtol(opt.arg, nullptr, 10);
                    }
                    else
                    {
                        print_warning("publisher", opt.name);
                    }
                    break;

                case optionIndex::TRANSPORT:
                {
                    std::string transport_str(opt.arg);
                    if (transport_str == "udpv4")
                    {
                        transport = TransportKind::UDPv4;
                    }
                    else if (transport_str == "udpv6")
                    {
                        transport = TransportKind::UDPv6;
                    }
                    else if (transport_str == "tcpv4")
                    {
                        transport = TransportKind::TCPv4;
                    }
                    else if (transport_str == "tcpv6")
                    {
                        transport = TransportKind::TCPv6;
                    }
                    else
                    {
                        print_warning("udpv4|udpv6|tcpv4|tcpv6", opt.name);
                    }

                    break;
                }

                case optionIndex::CONNECTION_PORT:
                    connection_port = static_cast<uint16_t>(strtol(opt.arg, nullptr, 10));
                    break;

                case optionIndex::CONNECTION_ADDRESS:
                    connection_address = opt.arg;
                    break;

                case optionIndex::CONNECTION_DISCOVERY_SERVER_ID:
                    id_ds_set = true;
                    connection_ds_id = static_cast<uint16_t>(strtol(opt.arg, nullptr, 10));
                    break;

                case optionIndex::LISTENING_PORT:
                    if (type != EntityKind::SERVER)
                    {
                        print_warning("server", opt.name);
                        break;
                    }
                    listening_port = static_cast<uint16_t>(strtol(opt.arg, nullptr, 10));
                    break;

                case optionIndex::LISTENING_ADDRESS:
                    if (type != EntityKind::SERVER)
                    {
                        print_warning("server", opt.name);
                        break;
                    }
                    listening_address = opt.arg;

                    break;

                case optionIndex::LISTENING_DISCOVERY_SERVER_ID:
                    if (type != EntityKind::SERVER)
                    {
                        print_warning("server", opt.name);
                        break;
                    }
                    listening_ds_id = static_cast<uint16_t>(strtol(opt.arg, nullptr, 10));
                    break;

                case optionIndex::TIMEOUT:
                    if (type != EntityKind::SERVER)
                    {
                        print_warning("server", opt.name);
                        break;
                    }
                    timeout = strtol(opt.arg, nullptr, 10);
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
        std::cerr << "ERROR: <publisher|subscriber|server> argument is required." << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    // Check that ip matches transport
    // if (transport == TransportKind::UDPv4 && !eprosima::fastrtps::rtps::IPLocator::isIPv4(listening_address))
    // {
    //     std::cerr << "ERROR: IPv4 is needed to use UDPv4. Wrong IP address: " << listening_address << std::endl;
    //     option::printUsage(fwrite, stdout, usage, columns);
    //     return 1;
    // }
    // else if (transport == TransportKind::UDPv6 && !eprosima::fastrtps::rtps::IPLocator::isIPv6(listening_address))
    // {
    //     std::cerr << "ERROR: IPv6 is needed to use UDPv6. Wrong IP address: " << listening_address << std::endl;
    //     option::printUsage(fwrite, stdout, usage, columns);
    //     return 1;
    // }

    // Check that a DS has not same id itself and connection
    if (id_ds_set && type == EntityKind::SERVER && listening_ds_id == connection_ds_id)
    {
        std::cerr << "ERROR: Discovery Servers ids must be different, "
                  << " cannot connect to a server with same id " << listening_ds_id << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    // Check that a DS has not same ip and port in listening and connection
    if (id_ds_set &&
            type == EntityKind::SERVER &&
            listening_address == connection_address &&
            listening_port == connection_port)
    {
        std::cerr << "ERROR: Discovery Servers ports must be different, "
                  << " cannot connect to a server with same listening address "
                  << listening_address << "(" << listening_port << ")" << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    switch (type)
    {
        case EntityKind::PUBLISHER:
        {
            HelloWorldPublisher mypub;
            if (mypub.init(
                        topic_name,
                        connection_address,
                        connection_port,
                        connection_ds_id,
                        transport))
            {
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep));
            }
            else
            {
                std::cerr << "ERROR: when initializing Publisher." << std::endl;
                return 1;
            }
            break;
        }
        case EntityKind::SUBSCRIBER:
        {
            HelloWorldSubscriber mysub;
            if (mysub.init(
                        topic_name,
                        static_cast<uint32_t>(count),
                        connection_address,
                        connection_port,
                        connection_ds_id,
                        transport))
            {
                mysub.run(static_cast<uint32_t>(count));
            }
            else
            {
                std::cerr << "ERROR: when initializing Subscriber." << std::endl;
                return 1;
            }
            break;
        }
        case EntityKind::SERVER:
        {
            DiscoveryServer myserver;
            if (myserver.init(
                        listening_address,
                        listening_port,
                        listening_ds_id,
                        transport,
                        id_ds_set,
                        connection_address,
                        connection_port,
                        connection_ds_id))
            {
                myserver.run(timeout);
            }
            else
            {
                std::cerr << "ERROR: when initializing Server." << std::endl;
                return 1;
            }
            break;
        }
    }
    return 0;
}
