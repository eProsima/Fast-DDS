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

#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"
#include "HelloWorldServer.h"

#include <string>
#include <regex>

#include <optionparser.h>

struct Arg: public option::Arg
{
    static void print_error(const char* msg1, const option::Option& opt, const char* msg2)
    {
        fprintf(stderr, "%s", msg1);
        fwrite(opt.name, opt.namelen, 1, stderr);
        fprintf(stderr, "%s", msg2);
    }

    static option::ArgStatus Unknown(const option::Option& option, bool msg)
    {
        if (msg) print_error("Unknown option '", option, "'\n");
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Required(const option::Option& option, bool msg)
    {
        if (option.arg != 0 && option.arg[0] != 0)
        return option::ARG_OK;

        if (msg) print_error("Option '", option, "' requires an argument\n");
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Numeric(const option::Option& option, bool msg)
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

    static option::ArgStatus String(const option::Option& option, bool msg)
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

    static option::ArgStatus Locator(const option::Option& option, bool msg)
    {
        if (option.arg != 0)
        {
            // we must check if it is a correct ip address plus port number
            if(std::regex_match(option.arg, ipv4))
            {
                return option::ARG_OK;
            }
        }
        if (msg)
        {
            print_error("Option '", option, "' requires an IPaddress[:portnumber] argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static const std::regex ipv4;
};

enum  optionIndex {
    UNKNOWN_OPT,
    HELP,
    TOPIC,
    WAIT,
    SAMPLES,
    INTERVAL,
    LOCATOR
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0,"", "",                Arg::None,
        "Usage: HelloWorldExampleDS <publisher|subscriber|server>\n\nGeneral options:" },
    { HELP,    0,"h", "help",               Arg::None,      "  -h \t--help  \tProduce help message." },

    { UNKNOWN_OPT, 0,"", "",                Arg::None,      "\nPublisher options:"},
    { TOPIC,0,"t","topic",                  Arg::String,
        "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: HelloWorldTopic)." },
    { WAIT, 0, "w", "wait",                 Arg::Numeric,
        "  -w <num> \t--wait=<num> \tNumber of matched subscribers required to publish"
        "(Default: 0 => does not wait)." },
    { SAMPLES,0,"s","samples",              Arg::Numeric,
        "  -s <num> \t--samples=<num>  \tNumber of samples to send (Default: 0 => infinite samples)." },
    { INTERVAL,0,"i","interval",            Arg::Numeric,
        "  -i <num> \t--interval=<num>  \tTime between samples in milliseconds (Default: 100)." },
    { LOCATOR, 0, "", "ip",                 Arg::Locator,
        "  \t--ip=<IPaddress[:port number]>  \tServer address (Default address: 127.0.0.1, default port: 60006)." },

    { UNKNOWN_OPT, 0,"", "",                Arg::None,      "\nSubscriber options:"},
    { TOPIC,0,"t","topic",                  Arg::String,
        "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: HelloWorldTopic)." },
    { SAMPLES,0,"s","samples",              Arg::Numeric,
        "  -s <num> \t--samples=<num>  \tNumber of samples to wait for (Default: 0 => infinite samples)." },
    { LOCATOR, 0, "", "ip",                 Arg::Locator,
        "  \t--ip=<IPaddress[:port number]>  \tServer address (Default address: 127.0.0.1, default port: 60006)." },

    { UNKNOWN_OPT, 0,"", "",                Arg::None,      "\nDiscoveryServer options:"},
    { LOCATOR, 0, "", "ip",                 Arg::Locator,
        "  \t--ip=<IPaddress[:port number]>  \tServer address (Default address: 127.0.0.1, default port: 60006)." },

    { 0, 0, 0, 0, 0, 0 }
};

/*static*/ const std::regex Arg::ipv4(R"(^((?:[0-9]{1,3}\.){3}[0-9]{1,3})?:?(?:(\d+))?$)");

void print_warning(std::string type, const char* opt)
{
    std::cerr << "WARNING: " << opt << " is a " << type << " option, ignoring argument." << std::endl;
}

int main(int argc, char** argv)
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
#endif

    std::cout << "Starting " << std::endl;
    int type = 1;
    std::string topic_name = "HelloWorldTopic";
    int count = 0;
    long sleep = 100;
    int numWaitMatched = 0;
    // Discovery Server
    eprosima::fastdds::rtps::Locator server_address;
    std::cmatch mr;
    std::string ip_address;
    uint16_t port = 60006;  // default physical port
    server_address.port = port;
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
        else if (strcmp(argv[1], "server") == 0)
        {
            type = 3;
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
                case HELP:
                    // not possible, because handled further above and exits the program
                    break;

                case TOPIC:
                    if (type == 3)
                        print_warning("publisher|subscriber", opt.name);
                    else
                        topic_name = std::string(opt.arg);
                    break;

                case SAMPLES:
                    if (type == 3)
                        print_warning("publisher|subscriber", opt.name);
                    else
                        count = strtol(opt.arg, nullptr, 10);
                    break;

                case INTERVAL:
                    if (type == 1)
                        sleep = strtol(opt.arg, nullptr, 10);
                    else
                        print_warning("publisher", opt.name);
                    break;

                case WAIT:
                    if (type == 1)
                        numWaitMatched = strtol(opt.arg, nullptr, 10);
                    else
                        print_warning("publisher", opt.name);
                    break;

                case LOCATOR:
                    port = server_address.port;

                    if(regex_match(opt.arg, mr, Arg::ipv4))
                    {
                        std::cmatch::iterator it = mr.cbegin();
                        ip_address = (++it)->str();

                        if((++it)->matched)
                        {
                            port = std::stoi(it->str());
                        }
                    }

                    if(!ip_address.empty() && port > 1000)
                    {
                        eprosima::fastrtps::rtps::IPLocator::setPhysicalPort(server_address, port);
                        eprosima::fastrtps::rtps::IPLocator::setIPv4(server_address, ip_address);
                    }
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
        std::cerr << "ERROR: <publisher|subscriber|server> argument is required." << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    // Set default IP address if not specified
    if(!IsAddressDefined(server_address))
    {
        eprosima::fastrtps::rtps::IPLocator::setIPv4(server_address, 127, 0, 0, 1);
    }

    switch(type)
    {
        case 1:
            {
                HelloWorldPublisher mypub;
                if(mypub.init(topic_name, server_address))
                {
                    mypub.run(static_cast<uint32_t>(count), static_cast<uint32_t>(sleep),
                            static_cast<uint32_t>(numWaitMatched));
                }
                break;
            }
        case 2:
            {
                HelloWorldSubscriber mysub;
                if(mysub.init(topic_name, static_cast<uint32_t>(count), server_address))
                {
                    mysub.run(static_cast<uint32_t>(count));
                }
                break;
            }
        case 3:
            {
                HelloWorldServer myserver;
                if(myserver.init(server_address))
                {
                    myserver.run();
                }
                break;
            }
    }
    return 0;
}
