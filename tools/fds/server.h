// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_SERVER_SERVER_H_
#define FASTDDS_SERVER_SERVER_H_

// Parsing setup
#include "optionparser.h"

enum  optionIndex
{
    UNKNOWN,
    HELP,
    SERVERID,
    IPADDRESS,
    PORT,
    BACKUP,
};

struct Arg : public option::Arg
{
    static option::ArgStatus check_server_id(
            const option::Option& option,
            bool msg);

    static option::ArgStatus check_server_ipv4(
            const option::Option& option,
            bool msg);

    static option::ArgStatus check_udp_port(
            const option::Option& option,
            bool msg);
};

const option::Descriptor usage[] = {

    { UNKNOWN,   0, "",   "",             Arg::None,
      "\neProsima Server-Client discovery auxiliary generator tool version " FAST_SERVER_VERSION "\n"
      "\nUsage: " FAST_SERVER_BINARY " -i {0-255} [optional parameters] \nGeneral options:" },

    { HELP,      0, "h",  "help",         Arg::None,
      "  -h  \t--help       Produce help message.\n" },

    { SERVERID,  0, "i", "server-id",    Arg::check_server_id,
      "  -i \t--server-id  Mandatory unique server identifier. Specifies zero based\n"
      "\t             server position in ROS_DISCOVERY_SERVER environment variable.\n" },

    { IPADDRESS, 0, "l", "ip-address",   Arg::check_server_ipv4,
      "  -l \t--ip-address Server interface chosen to listen the clients. Defaults\n"
      "\t             to any (0.0.0.0)\n" },

    { PORT,      0, "p",  "port",         Arg::check_udp_port,
      "  -p  \t--port       UDP port chosen to listen the clients. Defaults to 11811\n" },

    { BACKUP,    0, "b",  "backup",       Arg::None,
      "  -b  \t--backup     Creates a server with a backup file associated.\n" },

    { UNKNOWN,   0, "",  "",              Arg::None,
      "Examples:\n"

      "\t1. Launch a default server with id 0 (first on ROS_DISCOVERY_SERVER)\n"
      "\t   listening on all available interfaces on UDP port 11811. Only one\n"
      "\t   server can use default values per machine.\n\n"
      "\t$ " FAST_SERVER_BINARY " -i 0\n\n"

      "\t2. Launch a default server with id 1 (second on ROS_DISCOVERY_SERVER)\n"
      "\t   listening on localhost with UDP port 14520. Only localhost clients\n"
      "\t   can reach the server using as ROS_DISCOVERY_SERVER=;127.0.0.1:14520\n\n"
      "\t$ " FAST_SERVER_BINARY " -i 1 -l 127.0.0.1 -p 14520\n\n"

      "\t3. Launch a default server with id 3 (third on ROS_DISCOVERY_SERVER)\n"
      "\t   listening on Wi-Fi (192.168.36.34) and Ethernet (172.20.96.1) local\n"
      "\t   interfaces with UDP ports 8783 and 51083 respectively\n"
      "\t   (addresses and ports are made up for the example).\n\n"
      "\t$ " FAST_SERVER_BINARY " -i 1 -l 192.168.36.34 -p 14520 -l 172.20.96.1 -p 51083\n\n"

      "\t4. Launch a default server with id 4 (fourth on ROS_DISCOVERY_SERVER)\n"
      "\t   listening on 172.30.144.1 with UDP port 12345 and provided with a\n"
      "\t   backup file. If the server crashes it will automatically restore its\n"
      "\t   previous state when reenacted.\n\n"
      "\t$ " FAST_SERVER_BINARY " -i 1 -l 172.30.144.1 -p 12345 -b" },

    { 0, 0, 0, 0, 0, 0 }
};

#endif // FASTDDS_SERVER_SERVER_H_
