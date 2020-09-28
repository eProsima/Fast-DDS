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
 * @file HelloWorld_main.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include "HelloWorldSubscriber.h"
#include "HelloWorldServer.h"

#include <fastrtps/Domain.h>

#include <fastrtps/log/Log.h>

#include <regex>

#include "optionparser.h"

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

    static option::ArgStatus Locator(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            // we must check if its a correct ip address plus port number
            if (std::regex_match(option.arg, ipv4)
                    || std::regex_match(option.arg, ipv6))
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

    static option::ArgStatus NonEmpty(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0 && option.arg[0] != 0)
        {
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a non-empty argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static const std::regex ipv4, ipv6;
};

enum  optionIndex
{
    UNKNOWN_OPT,
    HELP,
    SAMPLES,
    INTERVAL,
    TCP,
    LOCATOR,
    TOPIC
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0, "", "",                Arg::None,
      "Usage: HelloWorldExampleDS <publisher|subscriber|server>\n\nGeneral options:" },
    { HELP,    0, "h", "help",               Arg::None,      "  -h \t--help  \tProduce help message." },
    { TCP, 0, "t", "tcp",                   Arg::None,
      "  -t \t--tcp \tUse tcp transport instead of the default UDP one." },
    { SAMPLES, 0, "c", "count",              Arg::Numeric,
      "  -c <num>, \t--count=<num>  \tNumber of datagrams to send (0 = infinite) defaults to 10." },
    { INTERVAL, 0, "i", "interval",            Arg::Numeric,
      "  -i <num>, \t--interval=<num>  \tTime between samples in milliseconds (Default: 100)." },
    { LOCATOR, 0, "l", "ip",                Arg::Locator,
      "  -l <IPaddress[:port number]>, \t--ip=<IPaddress[:port number]>  \tServer address." },
    { TOPIC, 0, "T", "topic",                Arg::NonEmpty,
      "  -T <topic-name>, \t--topic=<topic-name>  \tTopic name for the publisher/subscriber." },

    { 0, 0, 0, 0, 0, 0 }
};

/*static*/ const std::regex Arg::ipv4(R"(^((?:[0-9]{1,3}\.){3}[0-9]{1,3})?:?(?:(\d+))?$)");
/*static*/ const std::regex Arg::ipv6(R"(^\[?((?:[0-9a-fA-F]{0,4}\:){7}[0-9a-fA-F]{0,4})?(?:\]:)?(?:(\d+))?$)");

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
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
    int count = 20;
    long sleep = 100;
    std::string topic_name = "HelloWorldTopic";
    Locator_t server_address;
    server_address.port = 60006; // default physical port

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
            option::printUsage(fwrite, stdout, usage, columns);
            return 0;
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
            return 1;
        }

        for (int i = 0; i < parse.optionsCount(); ++i)
        {
            option::Option& opt = buffer[i];
            switch (opt.index())
            {
                case HELP:
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 0;

                case SAMPLES:
                    count = strtol(opt.arg, nullptr, 10);
                    break;

                case INTERVAL:
                    sleep = strtol(opt.arg, nullptr, 10);
                    break;

                case TOPIC:
                    topic_name = opt.arg;
                    break;

                // remember that options can be parsed in any order
                case TCP:
                {
                    // locators default to LOCATOR_KIND_UDPv4
                    // promote from UDP to TCP
                    if (IsAddressDefined(server_address))
                    {
                        server_address.kind =
                                ( server_address.kind == LOCATOR_KIND_UDPv4 ) ? LOCATOR_KIND_TCPv4 : LOCATOR_KIND_TCPv6;
                    }
                    else
                    {
                        server_address.kind = LOCATOR_KIND_TCPv4;
                    }

                    break;
                }

                case LOCATOR:
                {
                    std::cmatch mr;
                    std::string ip_address;
                    uint16_t port = server_address.port;
                    bool v4 = true;

                    if ((v4 = regex_match(opt.arg, mr, Arg::ipv4))
                            || regex_match(opt.arg, mr, Arg::ipv6))
                    {
                        std::cmatch::iterator it = mr.cbegin();
                        ip_address = (++it)->str();

                        if ((++it)->matched)
                        {
                            port = std::stoi(it->str());
                        }
                    }

                    // promote to v6 if needed
                    if (!v4)
                    {
                        server_address.kind =
                                (server_address.kind == LOCATOR_KIND_UDPv4) ? LOCATOR_KIND_UDPv6 : LOCATOR_KIND_TCPv6;
                    }

                    if (!ip_address.empty() && port > 1000)
                    {
                        IPLocator::setPhysicalPort(server_address, port);
                        if (v4)
                        {
                            IPLocator::setIPv4(server_address, ip_address);
                        }
                        else
                        {
                            IPLocator::setIPv6(server_address, ip_address);
                        }
                    }

                    break;
                }

                case UNKNOWN_OPT:
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 0;
                    break;

            }
        }

    }
    else
    {
        std::cout << "publisher, subscriber OR server argument needed" << std::endl;
        Log::Reset();
        return 0;
    }

    // Log::ReportFilenames(true);
    Log::SetCategoryFilter(
        std::regex("(RTPS_HISTORY)|(RTPS_WRITER_HISTORY)|(RTPS_READER_HISTORY)|(RTPS_PDP_SERVER)|(READER_PROXY)"
                   "|(RTPS_PDP)|(SERVER_PDP_THREAD)|(CLIENT_PDP_THREAD)|(DISCOVERY_DATABASE)|(RTPS_PDP_LISTENER)"));
    Log::SetVerbosity(Log::Kind::Info);

    switch (type)
    {
        case 1:
        {
            HelloWorldPublisher mypub;
            if (mypub.init(server_address, topic_name))
            {
                mypub.run(count, sleep);
            }
            break;
        }
        case 2:
        {
            HelloWorldSubscriber mysub;
            if (mysub.init(server_address, topic_name))
            {
                mysub.run();
            }
            break;
        }
        case 3:
        {
            HelloWorldServer myserver;
            if (myserver.init(server_address))
            {
                myserver.run();
            }
            break;
        }
    }
    Log::Flush();
    Domain::stopAll();
    return 0;
}
