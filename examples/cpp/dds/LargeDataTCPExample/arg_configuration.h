// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file arg_configuration.h
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_LARGEDATATCPEXAMPLE_ARG_CONFIGURATION_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_LARGEDATATCPEXAMPLE_ARG_CONFIGURATION_H_

#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include <optionparser.hpp>

namespace option = eprosima::option;

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
        if ( option.arg != nullptr )
        {
            strtol(option.arg, &endptr, 10);
            if (endptr != option.arg && *endptr == 0)
            {
                return option::ARG_OK;
            }
        }

        if (msg)
        {
            print_error("Option '", option, "' requires a numeric argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    template<long min = 0, long max = std::numeric_limits<long>::max()>
    static option::ArgStatus NumericRange(
            const option::Option& option,
            bool msg)
    {
        static_assert(min <= max, "NumericRange: invalid range provided.");

        char* endptr = 0;
        if ( option.arg != nullptr )
        {
            long value = strtol(option.arg, &endptr, 10);
            if ( endptr != option.arg && *endptr == 0 &&
                    value >= min && value <= max)
            {
                return option::ARG_OK;
            }
        }

        if (msg)
        {
            std::ostringstream os;
            os << "' requires a numeric argument in range ["
               << min << ", " << max << "]" << std::endl;
            print_error("Option '", option, os.str().c_str());
        }

        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus String(
            const option::Option& option,
            bool msg)
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

    static option::ArgStatus Transport(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            std::string transport = std::string(option.arg);
            if (transport != "shm" && transport != "udp" && transport != "udpv4" && transport != "udpv6")
            {
                if (msg)
                {
                    print_error("Option '", option, "' only accepts <shm|udp[v4]|udpv6> values\n");
                }
                return option::ARG_ILLEGAL;
            }
            return option::ARG_OK;
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a string argument\n");
        }
        return option::ARG_ILLEGAL;
    }

};

enum optionIndex
{
    UNKNOWN_OPT,
    HELP,
    DOMAIN_ID,
    RELIABLE,
    TRANSIENT_LOCAL,
    PUB_RATE,
    DATA_SIZE,
    TCP_MODE,
    WAN_ADDRESS,
    WAN_PORT
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0, "", "",                Arg::None,
      "Usage: LargeDataTCPExample <publisher|subscriber>\n\nGeneral options:" },
    { HELP,    0, "h", "help",               Arg::None,      "  -h \t--help  \tProduce help message." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nPublisher options:"},
    { DOMAIN_ID, 0, "d", "domain",                Arg::NumericRange<0, 230>,
      "  -d <id> \t--domain=<id>  \tDDS domain ID (Default: 0)." },
    { RELIABLE, 0, "r", "reliable",         Arg::None,
      "  -r \t--reliable \tSet reliability to reliable (best-effort by default)." },
    { TRANSIENT_LOCAL, 0, "", "transient",        Arg::None,
      "  \t--transient \tSet durability to transient local (volatile by default, ineffective when not reliable)." },
    { PUB_RATE, 0, "R", "rate",                 Arg::NumericRange<0, 500>,
      "  -R <num> \t--rate=<num> \tPublishing rate in Hz (Default 50)"
      " (Default: 0 => does not wait)." },
    { DATA_SIZE, 0, "D", "data-size",              Arg::NumericRange<>,
      "  -D <num> \t--data-size=<num>  \tSize of the samples to send (bytes) (Default: 100)" },
    { TCP_MODE, 0, "m", "tcp-mode",              Arg::NumericRange<0,2>,
      "  -m <num> \t--tcp-mode=<num>  \tTCP mode (0 -> NONE, 1 -> CLIENT, 2 -> SERVER). Default 0" },
    { WAN_ADDRESS, 0, "wa", "wan-addr",              Arg::String,
      "  -wa <string> \t--wan-addr=<string>  \t WAN Address (Default 127.0.0.1). Requires tcp_mode != NONE" },
    { WAN_PORT, 0, "wp", "wan-port",              Arg::NumericRange<0,65535>,
      "  -wp <num> \t--wan-port=<num>  \tWAN Port (Default 20000). Requires tcp_mode != NONE" },
   { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nSubscriber options:"},
   { DOMAIN_ID, 0, "d", "domain",                Arg::NumericRange<0, 230>,
      "  -d <id> \t--domain=<id>  \tDDS domain ID (Default: 0)." },
   { RELIABLE, 0, "r", "reliable",         Arg::None,
      "  -r \t--reliable \tSet reliability to reliable (best effort by default)." },
   { TRANSIENT_LOCAL, 0, "", "transient",        Arg::None,
      "  \t--transient \tSet durability to transient local (volatile by default, ineffective when not reliable)." },
   { TCP_MODE, 0, "m", "tcp-mode",              Arg::NumericRange<0,2>,
      "  -m <num> \t--tcp-mode=<num>  \tTCP mode (0 -> NONE, 1 -> CLIENT, 2 -> SERVER). Default 0" },
   { WAN_ADDRESS, 0, "wa", "wan-addr",              Arg::String,
      "  -wa <string> \t--wan-addr=<string>  \t WAN Address (Default 127.0.0.1). Requires tcp_mode != NONE" },
   { WAN_PORT, 0, "wp", "wan-port",              Arg::NumericRange<0,65535>,
      "  -wp <num> \t--wan-port=<num>  \tWAN Port (Default 20000). Requires tcp_mode != NONE" },
   { 0, 0, 0, 0, 0, 0 }
};

void print_warning(
        std::string type,
        const char* opt)
{
    std::cerr << "WARNING: " << opt << " is a " << type << " option, ignoring argument." << std::endl;
}

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_ARG_CONFIGURATION_H_ */
