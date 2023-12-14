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

#ifndef _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_ARG_CONFIGURATION_H_
#define _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_ARG_CONFIGURATION_H_

#include <iostream>
#include <regex>
#include <string>

#include <optionparser.hpp>
#include <fastrtps/utils/IPLocator.h>

#include "common.h"

namespace option = eprosima::option;

const std::regex IPv4_REGEX(R"(^((?:[0-9]{1,3}\.){3}[0-9]{1,3})?:?(?:(\d+))?$)");

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

    static option::ArgStatus Locator(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            // we must check if it is a correct ip address plus port number
            std::string ip_str(option.arg);
            if (
                eprosima::fastrtps::rtps::IPLocator::isIPv4(ip_str) ||
                eprosima::fastrtps::rtps::IPLocator::isIPv6(ip_str))
            {
                return option::ARG_OK;
            }
        }
        if (msg)
        {
            print_error("Option '", option, "' requires an <IPaddress> v4 or v6 argument\n");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus Transport(
            const option::Option& option,
            bool msg)
    {
        if (option.arg != 0)
        {
            // we must check if it is a correct ip address plus port number
            std::string transport = std::string(option.arg);
            if (
                // transport == "shm" ||
                transport == "udpv4" ||
                transport == "udpv6" ||
                transport == "tcpv4" ||
                transport == "tcpv6"
                )
            {
                return option::ARG_OK;
            }
        }
        if (msg)
        {
            print_error("Option '", option, "' requires a string argument\n");
        }
        return option::ARG_ILLEGAL;
    }

};

enum  optionIndex
{
    UNKNOWN_OPT,
    HELP,

    TOPIC,
    SAMPLES,
    INTERVAL,
    TRANSPORT,

    CONNECTION_ADDRESS,
    CONNECTION_PORT,
    CONNECTION_DISCOVERY_SERVER_ID,

    LISTENING_ADDRESS,
    LISTENING_PORT,
    LISTENING_DISCOVERY_SERVER_ID,
    TIMEOUT,
};

const option::Descriptor usage[] = {
    {UNKNOWN_OPT, 0, "", "", Arg::None,
     "Usage: DiscoveryServerExample <publisher|subscriber|server>\n\nGeneral options:" },
    {
        HELP,
        0,
        "h",
        "help",
        Arg::None,
        "  -h \t--help  \tProduce help message."
    },

    /// PUBLISHER OPTIONS
    {UNKNOWN_OPT, 0, "", "", Arg::None, "\nPublisher options:"},
    {
        TOPIC,
        0,
        "t",
        "topic",
        Arg::String,
        "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: HelloWorldTopic)."
    },
    {
        SAMPLES,
        0,
        "s",
        "samples",
        Arg::Numeric,
        "  -s <num> \t--samples=<num>  \tNumber of samples to send (Default: 0 => infinite samples)."
    },
    {
        INTERVAL,
        0,
        "i",
        "interval",
        Arg::Numeric,
        "  -i <num> \t--interval=<num>  \tTime between samples in milliseconds (Default: 100)."
    },
    {
        CONNECTION_ADDRESS,
        0,
        "c",
        "connection-address",
        Arg::String,
        "  -c <IPaddress> \t--connection-address=<IPaddress>  \tServer address (Default address: 127.0.0.1)."
    },
    {
        CONNECTION_PORT,
        0,
        "p",
        "connection-port",
        Arg::Numeric,
        "  -p <num> \t--connection-port=<num>  \tServer listening port (Default port: 16166)."
    },
    {
        TRANSPORT,
        0,
        "",
        "transport",
        Arg::Transport,
        "  \t--transport <trans> \tUse Transport Protocol [udpv4|udpv6|tcpv4|tcpv6] (UDPv4 by default)."
    },
    {
        CONNECTION_DISCOVERY_SERVER_ID,
        0,
        "d",
        "discovery-server-id",
        Arg::Numeric,
        "  -d <num>\t--connection-discovery-server-id <num> \tId of the Discovery Server to connect with. "
        "GUID will be calculated from id (0 by default)."
    },

    /// SUBSCRIBER OPTIONS
    {UNKNOWN_OPT, 0, "", "", Arg::None, "\nSubscriber options:"},
    {
        TOPIC,
        0,
        "t",
        "topic",
        Arg::String,
        "  -t <topic_name> \t--topic=<topic_name>  \tTopic name (Default: HelloWorldTopic)."
    },
    {
        SAMPLES,
        0,
        "s",
        "samples",
        Arg::Numeric,
        "  -s <num> \t--samples=<num>  \tNumber of samples to send (Default: 0 => infinite samples)."
    },
    {
        CONNECTION_ADDRESS,
        0,
        "c",
        "connection-address",
        Arg::String,
        "  -c <IPaddress> \t--connection-address=<IPaddress>  \tServer address (Default address: 127.0.0.1)."
    },
    {
        CONNECTION_PORT,
        0,
        "p",
        "connection-port",
        Arg::Numeric,
        "  -p <num> \t--connection-port=<num>  \tServer listening port (Default port: 16166)."
    },
    {
        TRANSPORT,
        0,
        "",
        "transport",
        Arg::Transport,
        "  \t--transport <trans> \tUse Transport Protocol [udpv4|udpv6|tcpv4|tcpv6] (UDPv4 by default)."
    },
    {
        CONNECTION_DISCOVERY_SERVER_ID,
        0,
        "d",
        "discovery-server-id",
        Arg::Numeric,
        "  -d <num>\t--connection-discovery-server-id <num> \tId of the Discovery Server to connect with. "
        "GUID will be calculated from id (0 by default)."
    },

    /// SERVER OPTIONS
    {UNKNOWN_OPT, 0, "", "", Arg::None, "\nDiscovery Server options:"},
    {
        LISTENING_ADDRESS,
        0,
        "",
        "listening-address",
        Arg::String,
        "  \t--listening-address=<IPaddress>  \tServer address (Default address: 127.0.0.1)."
    },
    {
        LISTENING_DISCOVERY_SERVER_ID,
        0,
        "",
        "id",
        Arg::Numeric,
        "  \t--id <num> \tId of this Discovery Server. GUID will be calculated from id (0 by default)."
    },
    {
        LISTENING_PORT,
        0,
        "",
        "listening-port",
        Arg::Numeric,
        "  \t--listening-port=<num>  \tServer listening port (Default port: 16166)."
    },
    {
        TRANSPORT,
        0,
        "",
        "transport",
        Arg::Transport,
        "  \t--transport <trans> \tUse Transport Protocol [udpv4|udpv6|tcpv4|tcpv6] (UDPv4 by default)."
    },
    {
        CONNECTION_PORT,
        0,
        "p",
        "connection-port",
        Arg::Numeric,
        "  -p <num> \t--connection-port=<num>  \tServer listening port (Default port: 16166)."
    },
    {
        CONNECTION_ADDRESS,
        0,
        "c",
        "connection-address",
        Arg::String,
        "  -c <num> \t--connection-address=<IPaddress>  \tServer address (Default address: 127.0.0.1)."
    },
    {
        CONNECTION_DISCOVERY_SERVER_ID,
        0,
        "d",
        "connection-discovery-server-id",
        Arg::Numeric,
        "  -d <num>\t--connection-discovery-server-id <num> \tId of the Discovery Server to connect with. "
        "GUID will be calculated from id (if not set, this DS will not connect to other server)."
    },
    {
        TIMEOUT,
        0,
        "z",
        "timeout",
        Arg::Numeric,
        "  -z <num>\t--timeout <num> \tNumber of seconds before finish the process (Default: 0 = till ^C). "
    },

    { 0, 0, 0, 0, 0, 0 }
};

void print_warning(
        std::string type,
        const char* opt)
{
    std::cerr << "WARNING: " << opt << " is a " << type << " option, ignoring argument." << std::endl;
}

#endif /* _EPROSIMA_FASTDDS_EXAMPLES_CPP_DDS_DISCOVERYSERVEREXAMPLE_ARG_CONFIGURATION_H_ */
