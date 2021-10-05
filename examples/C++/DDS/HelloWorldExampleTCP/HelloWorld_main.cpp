// Copyright 2016-2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/Domain.h>
#include <fastrtps/log/Log.h>

#include <iostream>
#include <regex>
#include <string>

#include "optionparser.h"

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
            // we must check if it is a correct ip address plus port number
            if (std::regex_match(option.arg, ipv4))
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
    SAMPLES,
    INTERVAL,
    SERVER_LOCATOR,
    REMOTE_LOCATOR,
    TLS,
    WHITELIST
};

/*

        std::cout << "There was an error with the input arguments." << std::endl << std::endl;
        std::cout << "This example needs at least the argument to set if it is going to work" << std::endl;
        std::cout << "as a 'publisher' or as a 'subscriber'." << std::endl << std::endl;

        std::cout << "The publisher is going to work as a TCP server and if the test" << std::endl;
        std::cout << "is through a NAT it must have its public IP in the wan_ip argument." << std::endl << std::endl;
        std::cout << "The optional arguments are: publisher [times] [interval] [wan_ip] [port] " << std::endl;
        std::cout << "\t- times: Number of messages to send (default: unlimited = 0). " << std::endl;
        std::cout << "\t\t If times is set greater than 0, no messages will be sent until a subscriber matches. " << std::endl;
        std::cout << "\t- interval: Milliseconds between messages (default: 100). " << std::endl;
        std::cout << "\t- wap_ip: Public IP Address of the publisher. " << std::endl;
        std::cout << "\t- port: Physical Port to listening incoming connections, this port must be allowed in" << std::endl;
        std::cout << "\t\tthe router of the publisher if the test is going to use WAN IP. " << std::endl << std::endl;

        std::cout << "The subscriber is going to work as a TCP client. If the test is through a NAT" << std::endl;
        std::cout << "server_ip must have the WAN IP of the publisher and if the test is on LAN" << std::endl;
        std::cout << "it must have the LAN IP of the publisher" << std::endl << std::endl;
        std::cout << "The optional arguments are: subscriber [server_ip] [port] " << std::endl;
        std::cout << "\t- server_ip: IP Address of the publisher. " << std::endl;
        std::cout << "\t- port: Physical Port where the publisher is listening for connections." << std::endl << std::endl;
*/

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0,"", "",                Arg::None,
        "Usage: HelloWorldExampleTCP <publisher|subscriber>\n\nGeneral options:" },
    { HELP,    0,"h", "help",               Arg::None,      "  -h \t--help  \tProduce help message." },
    { TLS, 0, "t", "tls",          Arg::None,      "  -t \t--tls \tUse TLS." },
    { WHITELIST, 0, "w", "whitelist",       Arg::String,    "  -w \t--whitelist \tUse Whitelist." },

    { UNKNOWN_OPT, 0,"", "",                Arg::None,      "\nPublisher options:"},
    { SAMPLES,0,"s","samples",              Arg::Numeric,
        "  -s <num>, \t--samples=<num>  \tNumber of samples (0, default, infinite)." },
    { INTERVAL,0,"i","interval",            Arg::Numeric,
        "  -i <num>, \t--interval=<num>  \tTime between samples in milliseconds (Default: 100)." },
    { SERVER_LOCATOR, 0, "", "server",      Arg::Locator,
      "  \t--server=<IPaddress[:port number]>  \tTCP server address." },
    { REMOTE_LOCATOR, 0, "", "remote",      Arg::Locator,
      "  \t--remote=<IPaddress[:port number]>  \tAddress of remote TCP server." },

    { UNKNOWN_OPT, 0,"", "",                Arg::None,      "\nSubscriber options:"},
    { SERVER_LOCATOR, 0, "", "server",      Arg::Locator,
      "  \t--server=<IPaddress[:port number]>  \tTCP server address." },
    { REMOTE_LOCATOR, 0, "", "remote",      Arg::Locator,
      "  \t--remote=<IPaddress[:port number]>  \tAddress of remote TCP server." },
    { 0, 0, 0, 0, 0, 0 }
};

/*static*/ const std::regex Arg::ipv4(R"(^((?:[0-9]{1,3}\.){3}[0-9]{1,3})?:?(?:(\d+))?$)");

using namespace eprosima;
using namespace fastrtps;
using namespace rtps;
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

    int type = 1;
    int count = 0;
    long sleep = 100;
    bool use_tls = false;
    std::vector<std::string> whitelist;

    // TCP server
    bool server = false;
    std::cmatch server_mr;
    std::string server_ip_address;
    uint16_t server_port = 5100;

    // Remote TCP server
    bool client = false;
    std::cmatch remote_mr;
    std::string remote_ip_address;
    uint16_t remote_port = 5100;

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

                case SAMPLES:
                    count = strtol(opt.arg, nullptr, 10);
                    break;

                case INTERVAL:
                    sleep = strtol(opt.arg, nullptr, 10);
                    break;

                case SERVER_LOCATOR:
                    server = true;
                    if (regex_match(opt.arg, server_mr, Arg::ipv4))
                    {
                        std::cmatch::iterator it = server_mr.cbegin();
                        server_ip_address = (++it)->str();

                        if ((++it)->matched)
                        {
                            int port_int = std::stoi(it->str());
                            if (port_int <= 65535)
                            {
                                server_port = static_cast<uint16_t>(port_int);
                            }
                        }
                    }
                    break;

                case REMOTE_LOCATOR:
                    client = true;
                    if (regex_match(opt.arg, remote_mr, Arg::ipv4))
                    {
                        std::cmatch::iterator it = remote_mr.cbegin();
                        remote_ip_address = (++it)->str();

                        if ((++it)->matched)
                        {
                            int port_int = std::stoi(it->str());
                            if (port_int <= 65535)
                            {
                                remote_port = static_cast<uint16_t>(port_int);
                            }
                        }
                    }
                    break;

                case TLS:
                    use_tls = true;
                    break;

                case WHITELIST:
                    whitelist.emplace_back(opt.arg);
                    break;

                case UNKNOWN_OPT:
                    option::printUsage(fwrite, stdout, usage, columns);
                    return 0;
                    break;
            }
        }
    }
    else
    {
        option::printUsage(fwrite, stdout, usage, columns);
        return 0;
    }

    if (!server && !client)
    {
        std::cerr << "ERROR: at least one (remote) server address is required." << std::endl;
        option::printUsage(fwrite, stdout, usage, columns);
        return 1;
    }

    std::cout << "Starting " << std::endl;
    switch (type)
    {
        case 1:
            {
                HelloWorldPublisher mypub;
                if (mypub.init(server, server_ip_address, static_cast<uint16_t>(server_port), client, remote_ip_address,
                    static_cast<uint16_t>(remote_port), use_tls, whitelist))
                {
                    mypub.run(count, sleep);
                }
                break;
            }
        case 2:
            {
                HelloWorldSubscriber mysub;
                if (mysub.init(server, server_ip_address, static_cast<uint16_t>(server_port), client, remote_ip_address,
                    static_cast<uint16_t>(remote_port), use_tls, whitelist))
                {
                    mysub.run();
                }
                break;
            }
    }
    Domain::stopAll();
    return 0;
}
