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

#ifndef FASTDDS_SERVER_SERVER_CPP_
#define FASTDDS_SERVER_SERVER_CPP_

#include "server.h"

#include <iostream>
#include <sstream>
#include <regex>
#include <vector>

#include <mutex>
#include <condition_variable>
#include <csignal>

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima;
using namespace fastrtps;
using namespace std;

volatile sig_atomic_t g_signal_status = 0;
mutex g_signal_mutex;
condition_variable g_signal_cv;

void sigint_handler(
        int signum)
{
    //  locking here is counterproductive
    //  unique_lock<mutex> lock(g_signal_mutex);
    g_signal_status = signum;
    g_signal_cv.notify_one();
}

int main (
        int argc,
        char* argv[])
{
    // skip program name argv[0] if present
    argc -= (argc > 0);
    argv += (argc > 0);
    option::Stats stats(usage, argc, argv);
    vector<option::Option> options(stats.options_max);
    vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

    // check the command line options
    if (parse.error())
    {
        return 1;
    }

    // no arguments beyond options
    int noopts = parse.nonOptionsCount();
    if ( noopts )
    {
        string sep( noopts == 1 ? "argument: " : "arguments: " );

        cout << "Unknown ";

        while ( noopts-- )
        {
            cout << sep << parse.nonOption(noopts);
            sep = ", ";
        }

        endl(cout);

        return 1;
    }

    // show help if asked to
    if (options[HELP] || argc == 0)
    {
        option::printUsage(std::cout, usage);

        return 0;
    }

    // auto att = make_unique<ParticipantAttributes>();
    // C++11 constrains
    unique_ptr<ParticipantAttributes> att(new ParticipantAttributes());
    rtps::RTPSParticipantAttributes& rtps = att->rtps;

    // Retrieve server Id: is mandatory and only specified once
    // Note there is a specific cast to pointer if the Option is valid
    option::Option* pOp = options[SERVERID];
    int server_id;

    if ( nullptr == pOp )
    {
        cout << "Specify server id is mandatory: use -i or --server-id option." << endl;
        return 1;
    }
    else if ( pOp->count() != 1)
    {
        cout << "Only one server can be created, thus, only one server id can be specified." << endl;
        return 1;
    }
    else
    {
        stringstream is;
        is << pOp->arg;

        // validation have been already done
        // Name the server according with the identifier
        if ( !( is >> server_id
                && eprosima::fastdds::rtps::get_server_client_default_guidPrefix(server_id, rtps.prefix)))
        {
            cout << "The provided server identifier is not valid" << endl;
            return 1;
        }

        // Set Participant Name
        is << "eProsima Default Server number " << server_id;
        rtps.setName(is.str().c_str());
    }

    // Choose the kind of server to create
    rtps.builtin.discovery_config.discoveryProtocol =
            options[BACKUP] ? rtps::DiscoveryProtocol_t::BACKUP : rtps::DiscoveryProtocol_t::SERVER;

    // Set up listening locators.
    // If the number of specify ports doesn't match the number of IPs the last port is used.
    // If at least one port specified replace the default one
    rtps::Locator_t locator(eprosima::fastdds::rtps::DEFAULT_ROS2_SERVER_PORT);

    // retrieve first UDP port
    option::Option* pO_port = options[PORT];
    if ( nullptr != pO_port )
    {
        stringstream is;
        is << pO_port->arg;
        uint16_t id;

        if ( !(is >> id
                && is.eof()
                && rtps::IPLocator::setPhysicalPort(locator, id)))
        {
            cout << "Invalid listening locator port specified:" << id << endl;
            return 1;
        }
    }

    rtps::IPLocator::setIPv4(locator, 0, 0, 0, 0);

    // retrieve first IP address
    pOp = options[IPADDRESS];
    if ( nullptr == pOp )
    {
        // add default locator
        rtps.builtin.metatrafficUnicastLocatorList.push_back(locator);
    }
    else
    {
        while ( pOp )
        {
            // Update locator address
            if (!rtps::IPLocator::setIPv4(locator, string(pOp->arg)))
            {
                cout << "Invalid listening locator address specified:" << pOp->arg << endl;
                return 1;
            }

            // Update UDP port
            if ( nullptr != pO_port )
            {
                stringstream is;
                is << pO_port->arg;
                uint16_t id;

                if ( !(is >> id
                        && is.eof()
                        && rtps::IPLocator::setPhysicalPort(locator, id)))
                {
                    cout << "Invalid listening locator port specified:" << id << endl;
                    return 1;
                }
            }

            // add the locator
            rtps.builtin.metatrafficUnicastLocatorList.push_back(locator);

            pOp = pOp->next();
            if (pO_port)
            {
                pO_port = pO_port->next();
            }
            else
            {
                cout << "warning: the number of specified ports doesn't match the ip" << endl
                     << "         addresses provided. Locators share its port number." << endl;
            }
        }
    }

    // Create the server
    int return_value = 0;
    Participant* pServer = Domain::createParticipant(*att, nullptr);

    if ( nullptr == pServer )
    {
        cout << "Server creation failed with the given settings. Please review locators setup." << endl;
        return_value = 1;
    }
    else
    {
        unique_lock<mutex> lock(g_signal_mutex);

        // handle signal SIGINT for every thread
        signal(SIGINT, sigint_handler);

        // Print running server attributes
        cout << "### Server is running ###" << endl;
        cout << "  Server ID:          " << server_id << endl;
        cout << "  Server GUID prefix: " << pServer->getGuid().guidPrefix << endl;
        cout << "  Server Addresses:   ";
        for (auto locator_it = att->rtps.builtin.metatrafficUnicastLocatorList.begin();
                locator_it != att->rtps.builtin.metatrafficUnicastLocatorList.end();)
        {
            cout << *locator_it;
            if (++locator_it != att->rtps.builtin.metatrafficUnicastLocatorList.end())
            {
                cout << std::endl << "                      ";
            }
        }
        cout << std::endl;

        g_signal_cv.wait(lock, []
                {
                    return 0 != g_signal_status;
                });

        cout << endl << "### Server shut down ###" << endl;
    }

    att.reset();
    fastdds::dds::Log::Flush();
    cout.flush();
    Domain::stopAll();

    return return_value;
}

// argument validation function definitions
/*static*/
option::ArgStatus Arg::check_server_id(
        const option::Option& option,
        bool msg)
{
    // the argument is required
    if ( nullptr != option.arg )
    {
        // It must be a number within 0 and 255
        stringstream is;
        is << option.arg;
        int id;

        if ( is >> id
                && is.eof()
                && id >= 0
                && id <  256 )
        {
            return option::ARG_OK;
        }
    }

    if (msg)
    {
        cout << "Option '" << option.name
             << "' is mandatory. Should be a key indentifier between 0 and 255." << endl;
    }

    return option::ARG_ILLEGAL;
}

/*static*/
option::ArgStatus Arg::check_server_ipv4(
        const option::Option& option,
        bool msg)
{
    static const std::regex ipv4(R"(^(?:[0-9]{1,3}\.){3}[0-9]{1,3}$)");

    // the argument is required
    if ( nullptr != option.arg )
    {
        // we must check if its a proper ip address
        if ( std::regex_match(option.arg, ipv4))
        {
            return option::ARG_OK;
        }
    }

    if (msg)
    {
        cout << "Option '" << option.name
             << "' should be a proper IPv4 address." << endl;
    }

    return option::ARG_ILLEGAL;
}

/*static*/
option::ArgStatus Arg::check_udp_port(
        const option::Option& option,
        bool msg)
{
    // the argument is required
    if ( nullptr != option.arg )
    {
        // It must be in an ephemeral port range
        stringstream is;
        is << option.arg;
        int id;

        if ( is >> id
                && is.eof()
                && id > 1024
                && id < 65536 )
        {
            return option::ARG_OK;
        }
    }

    if (msg)
    {
        cout << "Option '" << option.name
             << "' value should be an UDP port between 1025 and 65535." << endl;
    }

    return option::ARG_ILLEGAL;
}

#endif // FASTDDS_SERVER_SERVER_CPP_
