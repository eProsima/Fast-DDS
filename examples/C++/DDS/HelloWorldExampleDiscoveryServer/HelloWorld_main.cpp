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
#include "HelloWorldServer.h"
#include "HelloWorldSubscriber.h"

enum EntityType
{
    PUBLISHER,
    SUBSCRIBER,
    SERVER
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

    // TCP transport
    bool tcp = false;

    // Discovery Server
    std::cmatch mr;
    std::string server_address = "127.0.0.1";   // default ip address
    uint16_t server_port = 60006;   // default physical port

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
        else if (!strcmp(argv[1], "server"))
        {
            type = SERVER;
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
                    if (type == SERVER)
                    {
                        print_warning("publisher|subscriber", opt.name);
                    }
                    else
                    {
                        topic_name = std::string(opt.arg);
                    }
                    break;

                case optionIndex::SAMPLES:
                    if (type == SERVER)
                    {
                        print_warning("publisher|subscriber", opt.name);
                    }
                    else
                    {
                        count = strtol(opt.arg, nullptr, 10);
                    }
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

                case optionIndex::SERVER_LOCATOR:
                    if ((type == PUBLISHER || type == SUBSCRIBER) &&
                            !std::string("--listening-address").compare(0, std::string::npos, opt.name, opt.namelen))
                    {
                        print_warning("server", opt.name);
                    }
                    else if (type == SERVER &&
                            !std::string("--connection-address").compare(0, std::string::npos, opt.name, opt.namelen))
                    {
                        print_warning("publisher|subscriber", opt.name);
                    }
                    else
                    {
                        if (regex_match(opt.arg, mr, Arg::ipv4))
                        {
                            std::cmatch::iterator it = mr.cbegin();
                            server_address = (++it)->str();
                            if (server_address.empty())
                            {
                                std::cerr << "ERROR: " << opt.arg << " is an invalid server address." << std::endl;
                                option::printUsage(fwrite, stdout, usage, columns);
                                return 1;
                            }

                            if ((++it)->matched)
                            {
                                int port_int = std::stoi(it->str());
                                if (port_int > 1000 && port_int <= 65535)
                                {
                                    server_port = static_cast<uint16_t>(port_int);
                                }
                                else
                                {
                                    std::cerr << "ERROR: " << port_int << " is an invalid port number." << std::endl;
                                    option::printUsage(fwrite, stdout, usage, columns);
                                    return 1;
                                }
                            }
                        }
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
        std::cerr << "ERROR: <publisher|subscriber|server> argument is required." << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    switch (type)
    {
        case PUBLISHER:
        {
            HelloWorldPublisher mypub;
            if (mypub.init(topic_name, server_address, server_port, tcp))
            {
                mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep));
            }
            break;
        }
        case SUBSCRIBER:
        {
            HelloWorldSubscriber mysub;
            if (mysub.init(topic_name, static_cast<uint32_t>(count), server_address, server_port, tcp))
            {
                mysub.run(static_cast<uint32_t>(count));
            }
            break;
        }
        case SERVER:
        {
            HelloWorldServer myserver;
            if (myserver.init(server_address, server_port, tcp))
            {
                myserver.run();
            }
            break;
        }
    }
    return 0;
}
