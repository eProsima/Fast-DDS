// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_CLI_DISCOVERY_PARSER_HPP
#define FASTDDS_CLI_DISCOVERY_PARSER_HPP

// Parsing setup
#include <iostream>
#include <optionparser.hpp>
#include <sstream>
#include <string>

#include <fastdds/dds/log/Log.hpp>

namespace option = eprosima::option;

enum  optionIndex
{
    UNKNOWN,
    HELP,
    VERSION,
    SERVERID,
    UDPADDRESS,
    UDP_PORT,
    TCPADDRESS,
    TCP_PORT,
    BACKUP,
    XML_FILE,
    EXAMPLES_OPT,
    DOMAIN_OPT,
};

struct Arg : public option::Arg
{
    static option::ArgStatus check_server_id(
            const option::Option& option,
            bool msg)
    {
        // The argument is required
        if (nullptr != option.arg)
        {
            // It must be a number within 0 and 255
            std::stringstream is;
            is << option.arg;
            int id;

            if (is >> id
                    && is.eof()
                    && id >= 0
                    && id <  256 )
            {
                return option::ARG_OK;
            }
        }

        if (msg)
        {
            if (strcmp(option.name, "i") == 0)
            {
                EPROSIMA_LOG_ERROR(CLI, "Error in option '" << option.name << "' value. Remember it "
                                                            << "is optional. It should be a key identifier between 0 and 255.");
            }
            else if (strcmp(option.name, "d") == 0)
            {
                EPROSIMA_LOG_ERROR(CLI, "Error in option '" << option.name << "' value. "
                                                            << "It should be a key identifier between 0 and 255.");
            }
        }

        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus required(
            const option::Option& option,
            bool msg)
    {
        if (nullptr != option.arg)
        {
            return option::ARG_OK;
        }

        if (msg)
        {
            EPROSIMA_LOG_ERROR(CLI, "Option: '" << option.desc->longopt << "' requires an argument.");
        }
        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus check_udp_port(
            const option::Option& option,
            bool msg)
    {
        // The argument is required
        if (nullptr != option.arg)
        {
            // It must be in an ephemeral port range
            std::stringstream is;
            is << option.arg;
            int id;

            if (is >> id
                    && is.eof()
                    && id > 1024
                    && id < 65536)
            {
                return option::ARG_OK;
            }
        }

        if (msg)
        {
            EPROSIMA_LOG_ERROR(CLI,
                    "Option: '" << option.name << "' value should be an UDP port between 1025 and 65535.");
        }

        return option::ARG_ILLEGAL;
    }

    static option::ArgStatus check_tcp_port(
            const option::Option& option,
            bool msg)
    {
        // The argument is required
        if (nullptr != option.arg)
        {
            // It must be in an ephemeral port range
            std::stringstream is;
            is << option.arg;
            int id;

            if (is >> id
                    && is.eof()
                    && id > 1024
                    && id < 65536)
            {
                return option::ARG_OK;
            }
        }

        if (msg)
        {
            EPROSIMA_LOG_ERROR(CLI,
                    "Option: '" << option.name << "' value should be an TCP port between 1025 and 65535.");
        }

        return option::ARG_ILLEGAL;
    }

};

const option::Descriptor usage[] = {

    { UNKNOWN,   0, "",   "",             Arg::None,
      "\neProsima Discovery Server auxiliary generator tool. Version " FAST_SERVER_VERSION "\n"
      "It can be used for both deploying Servers and inspecting active ones. \n"
      "\nUsage: " FAST_SERVER_BINARY " [optional parameters] \n\nGeneral options:" },

    { HELP,      0, "h",  "help",         Arg::None,
      "  -h\t --help        Produce help message.\n" },

    { VERSION,   0, "v",  "version",      Arg::None,
      "  -v  \t--version     Show Fast DDS version information.\n" },

    { UDPADDRESS, 0, "l", "udp-address",   Arg::OptionalAny,
      "  -l\t --udp-address IPv4/IPv6 address chosen to listen the clients. \n"
      "\t               Defaults to any (0.0.0.0/::0). Instead of an \n"
      "\t               address, a name can be specified.\n"},

    { UDP_PORT,  0, "p",  "udp-port",       Arg::check_udp_port,
      "  -p\t --udp-port    UDP port chosen to listen the clients. Defaults to \n"
      "\t               11811.\n"},

    { TCPADDRESS, 0, "t", "tcp-address",   Arg::OptionalAny,
      "  -t\t --tcp-address IPv4/IPv6 address chosen to listen the clients \n"
      "\t               using TCP transport. Defaults to any\n"
      "\t               (0.0.0.0/::0). Instead of an address, a name\n"
      "\t               can be specified.\n"},

    { TCP_PORT,  0, "q",  "tcp-port",         Arg::check_tcp_port,
      "  -q\t --tcp-port    TCP port chosen to listen the clients. Defaults to\n"
      "\t               42100.\n"},

    { BACKUP,    0, "b",  "backup",       Arg::None,
      "  -b\t --backup      Creates a server with a backup file associated.\n" },

    { XML_FILE,  0, "x",  "xml-file",     Arg::required,
      "  -x\t --xml-file    Gets config from XML file. If there is any \n"
      "\t               argument in common with the config of the XML, the \n"
      "\t               XML argument will be overriden. A XML file with \n"
      "\t               several profiles will take the profile with \n"
      "\t               \"is_default_profile=\"true\"\" unless another \n"
      "\t               profile using uri with \"@\" character is defined.\n"},

    { SERVERID,  0, "i", "server-id",    Arg::check_server_id,
      "  -i\t --server-id   Unique server identifier. Its functionality its\n"
      "\t               deprecated. It can be used to select a fixed GUID.\n" },

    { EXAMPLES_OPT,  0, "e", "examples",    Arg::None,
      "  -e \t--examples     List usage examples of eProsima Discovery Server \n"
      "\t               tool.\n"},

    { UNKNOWN,   0, "",   "",             Arg::None,
      "\nDaemon options:\n  start\t Start the Discovery Server daemon with the remote connections\n"
      "\t specified. (Example: start -d 1 \"10.0.0.1:1\")."},

    { UNKNOWN,   0, "",   "",             Arg::None,
      "\n  stop\t Stop the Discovery Server daemon if it is active. If a domain\n"
      "\t is specified with the '-d' arg it will only stop the\n"
      "\t corresponding server and the daemon will remain alive."},

    { UNKNOWN,   0, "",   "",             Arg::None,
      "\n  add\t Add new remote Discovery Servers to the local server. This\n"
      "\t will connect both servers and their sub-networks without\n"
      "\t modifying existing remote servers."},

    { UNKNOWN,   0, "",   "",             Arg::None,
      "\n  set\t Rewrite the remotes Discovery Servers connected to the local \n"
      "\t server. This will replace existing remote servers with the new \n"
      "\t connections."},

    { UNKNOWN,   0, "",   "",             Arg::None,
      "\n  list\t List local active discovery servers created with the CLI Tool." },

    { UNKNOWN,   0, "",   "",             Arg::None,
      "\n  info         \t Inspect the Discovery Server in the specified domain. Feature\n"
      "\t not implemented yet.\n\n "
      "  Daemon parameters: \n "},

    { DOMAIN_OPT,  0, "d", "domain",    Arg::check_server_id,
      "  -d \t--domain       Selects the domain of the server to target for \n"
      "\t               this action. It defaults to 0 if arg is missing\n"
      "\t               and no value is found in ROS_DOMAIN_ID env. var.\n"},

    { 0, 0, 0, 0, 0, 0 }
};

const std::string EXAMPLES =

        "Examples:\n"

        "\t1.  Launch a default server listening on all available interfaces on\n"
        "\t    UDP port 11811. Only one server can use default values per machine.\n\n"
        "\t    $ " FAST_SERVER_BINARY "\n\n"

        "\t2.  Launch a default server listening on localhost on UDP port 14520.\n"
        "\t    Only localhost clients can reach the server, for example using\n"
        "\t    ROS_DISCOVERY_SERVER=127.0.0.1:14520\n\n"
        "\t    $ " FAST_SERVER_BINARY " -l 127.0.0.1 -p 14520\n\n"

        "\t3.  Launch a default server listening on all available interfaces on\n"
        "\t    TCP port 42100. Only one server can use default values per machine.\n\n"
        "\t    $ " FAST_SERVER_BINARY " -t\n\n"

        "\t4.  Launch a default server with GUID corresponding to id 1 (deprecated\n"
        "\t    parameter) listening on IPv6 address on UDP port 14520.\n\n"
        "\t    $ " FAST_SERVER_BINARY " -i 1 -l 2a02:ec80:600:ed1a::3 -p 14520\n\n"

        "\t5.  Launch a default server listening on WiFi (192.168.36.34) and Ethernet\n"
        "\t    (172.20.96.1) local interfaces on UDP ports 8783 and 51083 \n"
        "\t    respectively (addresses and ports are made up for the example).\n\n"
        "\t    $ " FAST_SERVER_BINARY " -l 192.163.6.34 -p 8783 -l 172.20.96.1\n"
        "\t    -p 51083\n\n"

        "\t6.  Launch a default server listening on 172.31.44.1 on UDP port 12345\n"
        "\t    and provided with a backup file. If the server crashes it will\n"
        "\t    automatically restore its previous state when reenacted.\n\n"
        "\t    $ " FAST_SERVER_BINARY " -l 172.31.44.1 -p 12345 -b\n\n"

        "\t7.  Launch a server reading default configuration from XML file.\n\n"
        "\t    $ " FAST_SERVER_BINARY " -x config.xml\n\n"

        "\t8.  Launch a server reading a profile_name configuration from XML file.\n\n"
        "\t    $ " FAST_SERVER_BINARY " -x profile_name@config.xml\n\n"

        "\t9.  Launch a server listening on localhost with default TCP port 42100\n\n"
        "\t    $ " FAST_SERVER_BINARY " -t 127.0.0.1\n\n"

        "\t10. Launch a server listening on localhost and Wi-Fi (192.163.6.34).\n"
        "\t    Two TCP ports need to be specified because TCP Transports cannot\n"
        "\t    share ports.\n\n"
        "\t    $ " FAST_SERVER_BINARY " -t 127.0.0.1 -q 42100 -t 192.163.6.34 \n"
        "\t    -q 42101\n\n"

        "Daemon Examples:\n"

        "\t1.  Start a DS in domain 0:\n\n"
        "\t    $ " FAST_SERVER_BINARY " start -d 0 127.0.0.1:0\n\n"

        "\t2.  Stop all running DS and shut down Fast DDS daemon:\n\n"
        "\t    $ " FAST_SERVER_BINARY " stop\n\n"

        "\t3.  Stop DS running in domain 0:\n\n"
        "\t    $ " FAST_SERVER_BINARY " stop -d 0\n\n"

        "\t4.  Start a DS in domain 4 pointing to remote DS in domain 4:\n\n"
        "\t    $ " FAST_SERVER_BINARY " start -d 4 10.0.0.7:4\n\n"

        "\t5.  Add a new remote server to DS running in domain 4 :\n\n"
        "\t    $ " FAST_SERVER_BINARY " add -d 4 10.0.0.7:4\n\n"

        "\t6.  List all servers running locally:\n\n"
        "\t    $ " FAST_SERVER_BINARY " list\n\n"

        "\t7.  Starts a DS in domain 3 pointing to local DS in domain 6:\n\n"
        "\t    $ " FAST_SERVER_BINARY " start -d 3 127.0.0.1:6\n\n"
;

#endif // FASTDDS_CLI_DISCOVERY_PARSER_HPP
