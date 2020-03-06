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
};



enum  optionIndex {
    UNKNOWN_OPT,
    HELP,
    SAMPLES,
    INTERVAL,
    IP,
    PORT,
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
    { IP,0,"a","address",                   Arg::String,
        "  -a <address>, \t--address=<address> \tPublic IP Address of the publisher (Default: None)." },
    { PORT, 0, "p", "port",                 Arg::Numeric,
        "  -p <num>, \t--port=<num>  \tPhysical Port to listening incoming connections (Default: 5100)." },

    { UNKNOWN_OPT, 0,"", "",                Arg::None,      "\nSubscriber options:"},
    { IP,0,"a","address",                   Arg::String,
        "  -a <address>, \t--address=<address> \tIP Address of the publisher (Default: 127.0.0.1)." },
    { PORT, 0, "p", "port",                 Arg::Numeric,
        "  -p <num>, \t--port=<num>  \tPhysical Port where the publisher is listening for connections (Default: 5100)." },

    { 0, 0, 0, 0, 0, 0 }
};

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

    std::cout << "Starting " << std::endl;
    int type = 1;
    int count = 0;
    long sleep = 100;
    std::string wan_ip;
    int port = 5100;
    bool use_tls = false;
    std::vector<std::string> whitelist;

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

                case IP:
                {
                    if (opt.arg != nullptr)
                    {
                        wan_ip = std::string(opt.arg);
                    }
                    else
                    {
                        option::printUsage(fwrite, stdout, usage, columns);
                        return 0;
                    }
                    break;
                }

                case PORT:
                    port = strtol(opt.arg, nullptr, 10);
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


    switch (type)
    {
        case 1:
            {
                HelloWorldPublisher mypub;
                if (mypub.init(wan_ip, static_cast<uint16_t>(port), use_tls, whitelist))
                {
                    mypub.run(count, sleep);
                }
                break;
            }
        case 2:
            {
                HelloWorldSubscriber mysub;
                if (mysub.init(wan_ip, static_cast<uint16_t>(port), use_tls, whitelist))
                {
                    mysub.run();
                }
                break;
            }
    }
    Domain::stopAll();
    return 0;
}
