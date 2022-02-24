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

#include <condition_variable>
#include <csignal>
#include <iostream>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "optionparser.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/attributes/ServerAttributes.h>
#include <fastdds/rtps/common/Locator.h>
#include <fastrtps/utils/IPLocator.h>

volatile sig_atomic_t g_signal_status = 0;
std::mutex g_signal_mutex;
std::condition_variable g_signal_cv;

void sigint_handler(
        int signum)
{
    // Locking here is counterproductive
    g_signal_status = signum;
    g_signal_cv.notify_one();
}

namespace eprosima {
namespace fastdds {
namespace dds {

int fastdds_discovery_server(
        int argc,
        char* argv[])
{
    // Convenience aliases
    using Locator = fastrtps::rtps::Locator_t;
    using DiscoveryProtocol = fastrtps::rtps::DiscoveryProtocol_t;
    using IPLocator = fastrtps::rtps::IPLocator;

    // Skip program name argv[0] if present
    argc -= (argc > 0);
    argv += (argc > 0);
    option::Stats stats(usage, argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);

    // Check the command line options
    if (parse.error())
    {
        return 1;
    }

    // No arguments beyond options
    int noopts = parse.nonOptionsCount();
    if (noopts)
    {
        std::string sep(noopts == 1 ? "argument: " : "arguments: ");
        std::cout << "Unknown ";

        while (noopts--)
        {
            std::cout << sep << parse.nonOption(noopts);
            sep = ", ";
        }

        std::endl(std::cout);
        return 1;
    }

    // Show help if asked to
    if (options[HELP] || argc == 0)
    {
        option::printUsage(std::cout, usage);
        return 0;
    }

    DomainParticipantQos participantQos;

    // Retrieve server Id: is mandatory and only specified once
    // Note there is a specific cast to pointer if the Option is valid
    option::Option* pOp = options[SERVERID];
    int server_id;

    if (nullptr == pOp)
    {
        std::cout << "Specify server id is mandatory: use -i or --server-id option." << std::endl;
        return 1;
    }
    else if (pOp->count() != 1)
    {
        std::cout << "Only one server can be created, thus, only one server id can be specified." << std::endl;
        return 1;
    }
    else
    {
        std::stringstream is;
        is << pOp->arg;

        // Validation has been already done
        // Name the server according with the identifier
        if (!(is >> server_id
                && eprosima::fastdds::rtps::get_server_client_default_guidPrefix(server_id,
                participantQos.wire_protocol().prefix)))
        {
            std::cout << "The provided server identifier is not valid" << std::endl;
            return 1;
        }

        // Clear the std::stringstream state and reset it to an empty string
        is.clear();
        is.str("");

        // Set Participant Name
        is << "eProsima Default Server number " << server_id;
        participantQos.name(is.str().c_str());
    }

    // Choose the kind of server to create
    participantQos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            options[BACKUP] ? DiscoveryProtocol::BACKUP : DiscoveryProtocol::SERVER;

    // Set up listening locators.
    // If the number of specify ports doesn't match the number of IPs the last port is used.
    // If at least one port specified replace the default one
    Locator locator(rtps::DEFAULT_ROS2_SERVER_PORT);

    // Retrieve first UDP port
    option::Option* pO_port = options[PORT];
    if (nullptr != pO_port)
    {
        std::stringstream is;
        is << pO_port->arg;
        uint16_t id;

        if (!(is >> id
                && is.eof()
                && IPLocator::setPhysicalPort(locator, id)))
        {
            std::cout << "Invalid listening locator port specified:" << id << std::endl;
            return 1;
        }
    }

    IPLocator::setIPv4(locator, 0, 0, 0, 0);

    // Retrieve first IP address
    pOp = options[IPADDRESS];
    if (nullptr == pOp)
    {
        // Add default locator
        participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);
    }
    else
    {
        while (pOp)
        {
            // Get next address
            std::string address = std::string(pOp->arg);

            // Check whether the address is IPv4
            if (!IPLocator::isIPv4(address))
            {
                auto response = IPLocator::resolveNameDNS(address);

                // Add the first valid IPv4 address that we can find
                if (response.first.size() > 0)
                {
                    address = response.first.begin()->data();
                }
            }

            // Update locator address
            if (!IPLocator::setIPv4(locator, address))
            {
                std::cout << "Invalid listening locator address specified:" << address << std::endl;
                return 1;
            }

            // Update UDP port
            if (nullptr != pO_port)
            {
                std::stringstream is;
                is << pO_port->arg;
                uint16_t id;

                if (!(is >> id
                        && is.eof()
                        && IPLocator::setPhysicalPort(locator, id)))
                {
                    std::cout << "Invalid listening locator port specified:" << id << std::endl;
                    return 1;
                }
            }

            // Add the locator
            participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);

            pOp = pOp->next();
            if (pO_port)
            {
                pO_port = pO_port->next();
            }
            else
            {
                std::cout << "Warning: the number of specified ports doesn't match the ip" << std::endl
                          << "         addresses provided. Locators share its port number." << std::endl;
            }
        }
    }

    // Create the server
    int return_value = 0;
    DomainParticipant* pServer = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

    if (nullptr == pServer)
    {
        std::cout << "Server creation failed with the given settings. Please review locators setup." << std::endl;
        return_value = 1;
    }
    else
    {
        std::unique_lock<std::mutex> lock(g_signal_mutex);

        // Handle signal SIGINT for every thread
        signal(SIGINT, sigint_handler);

        // Print running server attributes
        std::cout << "### Server is running ###" << std::endl;
        std::cout << "  Participant Type:   " <<
            participantQos.wire_protocol().builtin.discovery_config.discoveryProtocol <<
            std::endl;
        std::cout << "  Server ID:          " << server_id << std::endl;
        std::cout << "  Server GUID prefix: " << pServer->guid().guidPrefix << std::endl;
        std::cout << "  Server Addresses:   ";
        for (auto locator_it = participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.begin();
                locator_it != participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.end();)
        {
            std::cout << *locator_it;
            if (++locator_it != participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.end())
            {
                std::cout << std::endl << "                      ";
            }
        }
        std::cout << std::endl;

        g_signal_cv.wait(lock, []
                {
                    return 0 != g_signal_status;
                });

        std::cout << std::endl << "### Server shut down ###" << std::endl;
        DomainParticipantFactory::get_instance()->delete_participant(pServer);
    }

    Log::Flush();
    std::cout.flush();
    return return_value;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main (
        int argc,
        char* argv[])
{
    return eprosima::fastdds::dds::fastdds_discovery_server(argc, argv);
}

// Argument validation function definitions
/* Static */
option::ArgStatus Arg::check_server_id(
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
        std::cout << "Option '" << option.name
                  << "' is mandatory. Should be a key identifier between 0 and 255." << std::endl;
    }

    return option::ARG_ILLEGAL;
}

/* Static */
option::ArgStatus Arg::required(
        const option::Option& option,
        bool msg)
{
    if (nullptr != option.arg)
    {
        return option::ARG_OK;
    }

    if (msg)
    {
        std::cout << "Option '" << option << "' requires an argument" << std::endl;
    }
    return option::ARG_ILLEGAL;
}

/* Static */
option::ArgStatus Arg::check_udp_port(
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
        std::cout << "Option '" << option.name
                  << "' value should be an UDP port between 1025 and 65535." << std::endl;
    }

    return option::ARG_ILLEGAL;
}

#endif // FASTDDS_SERVER_SERVER_CPP_
