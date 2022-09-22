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
 * @file arg_configuration.h
 *
 */

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_ARG_CONFIGURATION_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_ARG_CONFIGURATION_H_

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
        if ( option.arg != nullptr)
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
    TOPIC,
    WAIT,
    SAMPLES,
    INTERVAL,
    ASYNC,
    DOMAIN_ID,
    TRANSPORT,
    RELIABLE,
    TRANSIENT_LOCAL,
    TTL
};

const option::Descriptor usage[] = {
    { UNKNOWN_OPT, 0, "", "",                Arg::None,
      "Usage: BasicConfigurationExample <publisher|subscriber>\n\nGeneral options:" },
    { HELP,    0, "h", "help",               Arg::None,      "  -h \t--help  \tProduce help message." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nPublisher options:"},
    { TOPIC, 0, "t", "topic",                  Arg::String,
      "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: HelloWorldTopic)." },
    { DOMAIN_ID, 0, "d", "domain",                Arg::NumericRange<0, 230>,
      "  -d <id> \t--domain=<id>  \tDDS domain ID (Default: 0)." },
    { WAIT, 0, "w", "wait",                 Arg::NumericRange<0, 100>,
      "  -w <num> \t--wait=<num> \tNumber of matched subscribers required to publish"
      " (Default: 0 => does not wait)." },
    { SAMPLES, 0, "s", "samples",              Arg::NumericRange<>,
      "  -s <num> \t--samples=<num>  \tNumber of samples to send (Default: 0 => infinite samples)." },
    { INTERVAL, 0, "i", "interval",            Arg::NumericRange<>,
      "  -i <num> \t--interval=<num>  \tTime between samples in milliseconds (Default: 100)." },
    { ASYNC, 0, "a", "async",               Arg::None,
      "  -a \t--async \tAsynchronous publish mode (synchronous by default)." },
    { TRANSPORT, 0, "", "transport",        Arg::Transport,
      "  \t--transport=<shm|udp|udpv6> \tUse only shared-memory, UDPv4, or UDPv6 transport."
      "If not set, use Fast DDS default transports (depending on the scenario it will use the most efficient one:"
      " data-sharing delivery mechanism > shared-memory > UDP)." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nSubscriber options:"},
    { TOPIC, 0, "t", "topic",                  Arg::String,
      "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: HelloWorldTopic)." },
    { DOMAIN_ID, 0, "d", "domain",                Arg::NumericRange<0, 230>,
      "  -d <id> \t--domain=<id>  \tDDS domain ID (Default: 0)." },
    { SAMPLES, 0, "s", "samples",              Arg::NumericRange<>,
      "  -s <num> \t--samples=<num>  \tNumber of samples to send (Default: 0 => infinite samples)." },
    { TRANSPORT, 0, "", "transport",        Arg::Transport,
      "  \t--transport=<shm|udp|udpv6> \tUse only shared-memory, UDPv4, or UDPv6 transport."
      "If not set, use Fast DDS default transports (depending on the scenario it will use the most efficient one:"
      " data-sharing delivery mechanism > shared-memory > UDP)." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nQoS options:"},
    { RELIABLE, 0, "r", "reliable",         Arg::None,
      "  -r \t--reliable \tSet reliability to reliable (best-effort by default)." },
    { TRANSIENT_LOCAL, 0, "", "transient",        Arg::None,
      "  \t--transient \tSet durability to transient local (volatile by default, ineffective when not reliable)." },

    { UNKNOWN_OPT, 0, "", "",                Arg::None,      "\nDiscovery options:"},
    { TTL, 0, "", "ttl",         Arg::NumericRange<1, 255>,
      "\t--ttl \tSet multicast discovery Time To Live on IPv4 or Hop Limit for IPv6."
      " If not set, uses Fast-DDS default (1 hop). Increase it to avoid discovery issues"
      " on scenarios with several routers. Maximum: 255."},

    { 0, 0, 0, 0, 0, 0 }
};

void print_warning(
        std::string type,
        const char* opt)
{
    std::cerr << "WARNING: " << opt << " is a " << type << " option, ignoring argument." << std::endl;
}

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_BASICCONFIGURATIONEXAMPLE_ARG_CONFIGURATION_H_ */
