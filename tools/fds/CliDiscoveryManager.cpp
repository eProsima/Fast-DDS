// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "CliDiscoveryManager.hpp"
#include "CliDiscoveryParser.hpp"

#include <condition_variable>
#include <csignal>
#include <iostream>
#include <mutex>
#include <regex>
#include <sstream>
#include <stdlib.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/utils/IPLocator.hpp>
#include <utils/SystemInfo.hpp>


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

eprosima::fastdds::dds::DomainId_t CliDiscoveryManager::get_domain_id(
        const eprosima::option::Option* domain_id)
{
    eprosima::fastdds::dds::DomainId_t id = 0;

    // Domain provided by CLI has priority over environment variable
    if (domain_id != nullptr)
    {
        std::stringstream domain_stream;

        domain_stream << domain_id->arg;
        domain_stream >> id;
    }
    else
    {
        // Retrieve domain from environment variable
        std::string env_value;
        if (eprosima::SystemInfo::get_env(domain_env_var, env_value) == eprosima::fastdds::dds::RETCODE_OK)
        {
            std::stringstream domain_stream;

            domain_stream << env_value;

            if (domain_stream >> id
                    && domain_stream.eof()
                    && id <  256 )
            {
                domain_stream >> id;
            }
            else
            {
                std::cout << "Found Invalid Domain ID in environment variable: " << env_value << std::endl;
            }
        }
    }

    return id;
}

bool CliDiscoveryManager::initial_options_fail(
        std::vector<option::Option>& options,
        option::Parser& parse)
{
    // Check the command line options
    if (parse.error())
    {
        option::printUsage(std::cout, usage);
        return true;
    }

    if (options[UNKNOWN])
    {
        EPROSIMA_LOG_ERROR(CLI, "Unknown option: " << options[UNKNOWN].name);
        option::printUsage(std::cout, usage);
        return true;
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

        std::cout << std::endl;
        return true;
    }
    return false;
}

inline uint32_t CliDiscoveryManager::getDiscoveryServerPort(
            const uint32_t domainId)
{
    uint32_t port = port_params_.portBase + port_params_.domainIDGain * domainId + port_params_.offsetd4;

    if (port > 65535)
    {
        std::cout << "Discovery server port is too high. Domain ID is too high: " << domainId << std::endl;
    }
    return port;
}

std::string CliDiscoveryManager::execCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe)
    {
        std::cerr << "Error processing command:" << command << std::endl;
        return "";
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

std::vector<uint16_t> CliDiscoveryManager::getListeningPorts()
{
    std::vector<uint16_t> ports;
    const std::string& command = "netstat -at | grep LISTEN";
    std::string result = execCommand(command);
    std::regex port_regex(R"(0\.0\.0\.0:(\d+))");
    std::smatch match;
    std::string line;
    std::istringstream result_stream(result);
    while (std::getline(result_stream, line))
    {
        if (std::regex_search(line, match, port_regex))
        {
            ports.push_back(static_cast<uint16_t>(std::stoi(match[1].str())));
        }
    }
    std::sort(ports.begin(), ports.end());
    return ports;
}

int CliDiscoveryManager::fastdds_discovery_server(
        std::vector<option::Option>& options,
        option::Parser& parse)
{
    // Convenience aliases
    using Locator = fastdds::rtps::Locator_t;
    using DiscoveryProtocol = fastdds::rtps::DiscoveryProtocol;
    using IPLocator = fastdds::rtps::IPLocator;

    constexpr const char* delimiter = "@";
    std::string sXMLConfigFile = "";
    std::string profile = "";

    if (initial_options_fail(options, parse))
    {
        return 1;
    }

    // Show help if asked to
    if (options[HELP])
    {
        option::printUsage(std::cout, usage);
        return 0;
    }

    if (options[EXAMPLES_OPT])
    {
        std::cout << EXAMPLES << std::endl;
        return 0;
    }

    // Show version if asked to
    if (options[VERSION])
    {
        std::cout << "Fast DDS version: " << FASTDDS_VERSION_STR << std::endl;
        return 0;
    }

    DomainParticipantQos participantQos;

    if (nullptr != options[XML_FILE])
    {
        sXMLConfigFile = options[XML_FILE].arg;
        if (sXMLConfigFile.length() > 0)
        {
            size_t delimiter_pos = sXMLConfigFile.find(delimiter);
            if (std::string::npos != delimiter_pos)
            {
                profile = sXMLConfigFile.substr(0, delimiter_pos);
                sXMLConfigFile = sXMLConfigFile.substr(delimiter_pos + 1, sXMLConfigFile.length());
            }

            if (RETCODE_OK != DomainParticipantFactory::get_instance()->load_XML_profiles_file(
                        sXMLConfigFile))
            {
                std::cout << "Cannot open XML file " << sXMLConfigFile << ". Please, check the path of this "
                          << "XML file." << std::endl;
                return 1;
            }
            if (profile.empty())
            {
                // Set environment variables to prevent loading the default XML file
#ifdef _WIN32
                if (0 != _putenv_s("FASTDDS_DEFAULT_PROFILES_FILE", "") ||
                        0 != _putenv_s("SKIP_DEFAULT_XML_FILE", "1"))
                {
                    char errmsg[1024];
                    strerror_s(errmsg, sizeof(errmsg), errno);
                    std::cout << "Error setting environment variables: " << errmsg << std::endl;
                    return 1;
                }
#else
                if (0 != unsetenv("FASTDDS_DEFAULT_PROFILES_FILE") ||
                        0 != setenv("SKIP_DEFAULT_XML_FILE", "1", 1))
                {
                    std::cout << "Error setting environment variables: " << std::strerror(errno) << std::endl;
                    return 1;
                }
#endif // ifdef _WIN32
                // Set default participant QoS from XML file
                if (RETCODE_OK != DomainParticipantFactory::get_instance()->load_profiles())
                {
                    std::cout << "Error setting default DomainParticipantQos from XML default profile." << std::endl;
                    return 1;
                }
                participantQos = DomainParticipantFactory::get_instance()->get_default_participant_qos();
            }
            else
            {
                if (RETCODE_OK !=
                        DomainParticipantFactory::get_instance()->get_participant_qos_from_profile(
                            profile, participantQos))
                {
                    std::cout << "Error loading specified profile from XML file." << std::endl;
                    return 1;
                }
            }
        }

    }

    // Retrieve server ID: is optional and only specified once
    // Note there is a specific cast to pointer if the Option is valid
    option::Option* pOp = options[SERVERID];
    int server_id = -1;
    std::stringstream server_stream;

    if (nullptr == pOp)
    {
        fastdds::rtps::GuidPrefix_t prefix_cero;
        if (!(participantQos.wire_protocol().builtin.discovery_config.discoveryProtocol ==
                eprosima::fastdds::rtps::DiscoveryProtocol::SERVER ||
                participantQos.wire_protocol().builtin.discovery_config.discoveryProtocol ==
                eprosima::fastdds::rtps::DiscoveryProtocol::BACKUP) && nullptr != options[XML_FILE])
        {
            // Discovery protocol specified in XML file is not SERVER nor BACKUP
            std::cout << "The provided configuration is not valid. Participant must be either SERVER or BACKUP. " <<
                std::endl;
            return 1;
        }
        else if (participantQos.wire_protocol().prefix == prefix_cero &&
                participantQos.wire_protocol().builtin.discovery_config.discoveryProtocol ==
                eprosima::fastdds::rtps::DiscoveryProtocol::BACKUP)
        {
            // Discovery protocol specified in XML is BACKUP, but no GUID was specified
            std::cout << "Specifying a GUID prefix is mandatory for BACKUP Discovery Servers." <<
                "Update the XML file or use the -i argument." << std::endl;
        }
    }
    else if (pOp->count() != 1)
    {
        std::cout << "Only one server participant can be created, thus, only one server id can be specified." << std::endl;
        return 1;
    }
    else
    {
        // Cast of option to int has already been checked
        server_stream << pOp->arg;
        server_stream >> server_id;
        if (!eprosima::fastdds::rtps::get_server_client_default_guidPrefix(server_id,
                participantQos.wire_protocol().prefix))
        {
            std::cout << "Failed to set the GUID with the server identifier provided." << std::endl;
            return 1;
        }
    }

    // Clear the std::stringstream state and reset it to an empty string
    server_stream.clear();
    server_stream.str("");

    // Set Participant Name
    std::string server_name =
            (server_id == -1) ? "eProsima Guidless Server" : "eProsima Default Server" + std::to_string(
        server_id);
    server_stream << server_name;
    participantQos.name(server_stream.str().c_str());

    // Choose the kind of server to create
    pOp = options[BACKUP];
    if (nullptr != pOp)
    {
        fastdds::rtps::GuidPrefix_t prefix_cero;
        if (participantQos.wire_protocol().prefix == prefix_cero && server_id == -1)
        {
            // BACKUP argument used, but no GUID was specified either in the XML nor in the CLI
            std::cout << "Specifying a GUID prefix is mandatory for BACKUP Discovery Servers. Use the -i argument." <<
                std::endl;
            return 1;
        }
        participantQos.wire_protocol().builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::BACKUP;
    }
    else if (nullptr == options[XML_FILE])
    {
        participantQos.wire_protocol().builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SERVER;
    }

    // Set up listening UDP locators.
    /**
     * The metatraffic unicast locator list can be defined:
     *    1. By means of the CLI specifying a locator address (pOp != nullptr) and port (pO_port != nullptr)
     *          Locator: UDPAddres:port
     *    2. By means of the CLI specifying only the locator address (pOp != nullptr)
     *          Locator: UDPAddress:11811
     *    3. By means of the CLI specifying only the port number (pO_port != nullptr)
     *          Locator: UDPv4:[0.0.0.0]:port
     *    4. By means of the XML configuration file (options[XML_FILE] != nullptr)
     *    5. No information provided.
     *          Locator: UDPv4:[0.0.0.0]:11811
     *
     * The UDP CLI has priority over the XML file configuration.
     */

    // If the number of specified ports doesn't match the number of IPs the last port is used.
    // If at least one port is specified, it will replace the default one
    Locator locator4(rtps::DEFAULT_ROS2_SERVER_PORT);
    Locator locator6(LOCATOR_KIND_UDPv6, rtps::DEFAULT_ROS2_SERVER_PORT);

    // Retrieve first UDP port
    option::Option* pO_port = options[UDP_PORT];
    if (nullptr != pO_port)
    {
        std::stringstream is;
        is << pO_port->arg;
        uint16_t id;

        if (!(is >> id
                && is.eof()
                && IPLocator::setPhysicalPort(locator4, id)
                && IPLocator::setPhysicalPort(locator6, id)))
        {
            std::cout << "Invalid listening locator port specified:" << id << std::endl;
            return 1;
        }
    }

    IPLocator::setIPv4(locator4, 0, 0, 0, 0);
    IPLocator::setIPv6(locator6, 0, 0, 0, 0, 0, 0, 0, 0);

    // Retrieve first IP address
    pOp = options[UDPADDRESS];
    if (pOp != nullptr && pOp->arg == nullptr)
    {
        pOp->arg = "0.0.0.0";
    }
    option::Option* pO_tcp = options[TCPADDRESS];
    if (pO_tcp != nullptr && pO_tcp->arg == nullptr)
    {
        pO_tcp->arg = "0.0.0.0";
    }
    // Retrieve first TCP port
    option::Option* pO_tcp_port = options[TCP_PORT];

    bool udp_server_initialized = (pOp != nullptr) || (pO_port != nullptr);

    /**
     * A locator has been initialized previously in [0.0.0.0] address using either the DEFAULT_ROS2_SERVER_PORT or the
     * port number set in the CLI. This locator must be used:
     *     a) If there is no IP address defined in the CLI (pOp == nullptr) but the port has been defined
     *        (pO_port != nullptr) and there is no TCP address nor port provided by the CLI
     *     b) If there is no locator information provided either by CLI (UDP and TCP) or XML file
     *        (options[XML_FILE] == nullptr)
     */
    if (nullptr == pOp && (nullptr == options[XML_FILE] || nullptr != pO_port) &&
            (nullptr == pO_tcp && nullptr == pO_tcp_port))
    {
        // Add default locator in cases a) and b)
        participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.clear();
        participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator4);
        udp_server_initialized = true;
    }
    else if (nullptr == pOp && nullptr != pO_port)
    {
        // UDP port AND TCP port/address has been specified without specifying UDP address
        participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.clear();
        participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator4);
    }
    else if (nullptr != pOp)
    {
        // UDP address has been specified
        participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.clear();
        while (pOp)
        {
            // Get next address
            std::string address = std::string(pOp->arg);
            int type = LOCATOR_PORT_INVALID;

            // Trial order IPv4, IPv6 & DNS
            if (IPLocator::isIPv4(address) && IPLocator::setIPv4(locator4, address))
            {
                type = LOCATOR_KIND_UDPv4;
            }
            else if (IPLocator::isIPv6(address) && IPLocator::setIPv6(locator6, address))
            {
                type = LOCATOR_KIND_UDPv6;
            }
            else
            {
                auto response = IPLocator::resolveNameDNS(address);

                // Add the first valid IPv4 address that we can find
                if (response.first.size() > 0)
                {
                    address = response.first.begin()->data();
                    if (IPLocator::setIPv4(locator4, address))
                    {
                        type = LOCATOR_KIND_UDPv4;
                    }
                }
                else if (response.second.size() > 0)
                {
                    address = response.second.begin()->data();
                    if (IPLocator::setIPv6(locator6, address))
                    {
                        type = LOCATOR_KIND_UDPv6;
                    }
                }
            }

            // On failure report error
            if ( LOCATOR_PORT_INVALID == type )
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
                        && IPLocator::setPhysicalPort(locator4, id)
                        && IPLocator::setPhysicalPort(locator6, id)))
                {
                    std::cout << "Invalid listening locator port specified:" << id << std::endl;
                    return 1;
                }
            }

            // Add the locator
            participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList
                    .push_back( LOCATOR_KIND_UDPv4 == type ? locator4 : locator6 );

            pOp = pOp->next();
            if (pO_port)
            {
                pO_port = pO_port->next();
            }
            else
            {
                std::cout << "Warning: the number of specified ports doesn't match the ip" << std::endl
                          << "         addresses provided. Locators share their port number." << std::endl;
            }
        }

        // add UDPv6 transport if required
        if (participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.has_kind<LOCATOR_KIND_UDPv6>())
        {
            // extend builtin transports with the UDPv6 transport
            auto descriptor = std::make_shared<fastdds::rtps::UDPv6TransportDescriptor>();
            descriptor->sendBufferSize = participantQos.transport().send_socket_buffer_size;
            descriptor->receiveBufferSize = participantQos.transport().listen_socket_buffer_size;
            participantQos.transport().user_transports.push_back(std::move(descriptor));
        }
    }

    // Add TCP default locators addresses
    Locator locator_tcp_4(LOCATOR_KIND_TCPv4, rtps::DEFAULT_TCP_SERVER_PORT);
    Locator locator_tcp_6(LOCATOR_KIND_TCPv6, rtps::DEFAULT_TCP_SERVER_PORT);
    bool default_port = true;

    // Manage TCP port
    if (nullptr != pO_tcp_port)
    {
        std::stringstream is;
        is << pO_tcp_port->arg;
        uint16_t id;
        default_port = false;

        if (!(is >> id
                && is.eof()
                && IPLocator::setPhysicalPort(locator_tcp_4, id)
                && IPLocator::setLogicalPort(locator_tcp_4, id)
                && IPLocator::setPhysicalPort(locator_tcp_6, id)
                && IPLocator::setLogicalPort(locator_tcp_6, id)))
        {
            std::cout << "Invalid listening locator port specified:" << id << std::endl;
            return 1;
        }
    }
    else
    {
        IPLocator::setPhysicalPort(locator_tcp_4, rtps::DEFAULT_TCP_SERVER_PORT);
        IPLocator::setLogicalPort(locator_tcp_4, rtps::DEFAULT_TCP_SERVER_PORT);
        IPLocator::setPhysicalPort(locator_tcp_6, rtps::DEFAULT_TCP_SERVER_PORT);
        IPLocator::setLogicalPort(locator_tcp_6, rtps::DEFAULT_TCP_SERVER_PORT);
    }

    if (nullptr != pO_tcp || nullptr != pO_tcp_port)
    {
        if (nullptr == pO_tcp)
        {
            // Only the TCP port has been specified. Use "any" as interface for TCP to listen on all interfaces
            IPLocator::setIPv4(locator_tcp_4, "0.0.0.0");
            participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator_tcp_4);
            auto tcp_descriptor = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
            tcp_descriptor->add_listener_port(static_cast<uint16_t>(locator_tcp_4.port));
            participantQos.transport().user_transports.push_back(tcp_descriptor);
        }
        else if (nullptr != pO_tcp)
        {
            // Add tcp locator address
            while (pO_tcp)
            {
                // Get next address
                std::string address = std::string(pO_tcp->arg);
                int type = LOCATOR_PORT_INVALID;

                // Trial order IPv4, IPv6 & DNS
                if (IPLocator::isIPv4(address) && IPLocator::setIPv4(locator_tcp_4, address))
                {
                    type = LOCATOR_KIND_TCPv4;
                }
                else if (IPLocator::isIPv6(address) && IPLocator::setIPv6(locator_tcp_6, address))
                {
                    type = LOCATOR_KIND_TCPv6;
                }
                else
                {
                    auto response = IPLocator::resolveNameDNS(address);

                    // Add the first valid IPv4 address that we can find
                    if (response.first.size() > 0)
                    {
                        address = response.first.begin()->data();
                        if (IPLocator::setIPv4(locator_tcp_4, address))
                        {
                            type = LOCATOR_KIND_TCPv4;
                        }
                    }
                    else if (response.second.size() > 0)
                    {
                        address = response.second.begin()->data();
                        if (IPLocator::setIPv6(locator_tcp_6, address))
                        {
                            type = LOCATOR_KIND_TCPv6;
                        }
                    }
                }

                // On failure report error
                if ( LOCATOR_PORT_INVALID == type )
                {
                    std::cout << "Invalid listening locator address specified:" << address << std::endl;
                    return 1;
                }

                // Update TCP port
                if (nullptr != pO_tcp_port)
                {
                    std::stringstream is;
                    is << pO_tcp_port->arg;
                    uint16_t id;

                    if (!(is >> id
                            && is.eof()
                            && IPLocator::setPhysicalPort(locator_tcp_4, id)
                            && IPLocator::setLogicalPort(locator_tcp_4, id)
                            && IPLocator::setPhysicalPort(locator_tcp_6, id)
                            && IPLocator::setLogicalPort(locator_tcp_6, id)))
                    {
                        std::cout << "Invalid listening locator port specified:" << id << std::endl;
                        return 1;
                    }
                }

                // Add the locator
                participantQos.wire_protocol().builtin.metatrafficUnicastLocatorList
                        .push_back( LOCATOR_KIND_TCPv4 == type ? locator_tcp_4 : locator_tcp_6 );

                // Create user transport
                if (type == LOCATOR_KIND_TCPv4)
                {
                    auto tcp_descriptor = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
                    tcp_descriptor->add_listener_port(static_cast<uint16_t>(locator_tcp_4.port));
                    participantQos.transport().user_transports.push_back(tcp_descriptor);
                }
                else
                {
                    auto tcp_descriptor = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
                    tcp_descriptor->add_listener_port(static_cast<uint16_t>(locator_tcp_6.port));
                    participantQos.transport().user_transports.push_back(tcp_descriptor);
                }

                pO_tcp = pO_tcp->next();
                if (pO_tcp_port)
                {
                    pO_tcp_port = pO_tcp_port->next();
                    default_port = false;
                }
                else
                {
                    if (!default_port)
                    {
                        std::cout << "Error: the number of specified TCP ports doesn't match the ip addresses" <<
                            std::endl
                                  << "       provided. TCP transports cannot share their port number." << std::endl;
                        return 1;
                    }
                    // One default port has already been used
                    default_port = false;
                }
            }
        }
    }

    fastdds::rtps::GuidPrefix_t guid_prefix = participantQos.wire_protocol().prefix;
    participantQos.transport().use_builtin_transports = udp_server_initialized || options[XML_FILE] != nullptr;

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
        signal(SIGTERM, sigint_handler);
#ifndef _WIN32
        signal(SIGQUIT, sigint_handler);
        signal(SIGHUP, sigint_handler);
#endif // ifndef _WIN32

        bool has_security = false;
        if (guid_prefix != pServer->guid().guidPrefix)
        {
            has_security = true;
        }

        // Print running server attributes
        std::cout << "### Server is running ###" << std::endl;
        std::cout << "  Participant Type:   " <<
            participantQos.wire_protocol().builtin.discovery_config.discoveryProtocol <<
            std::endl;
        std::cout << "  Security:           " << (has_security ? "YES" : "NO") << std::endl;
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

int CliDiscoveryManager::fastdds_discovery_auto(
    std::vector<option::Option>& options,
    option::Parser& parse)
{
    if (initial_options_fail(options, parse))
    {
        return 1;
    }

    // Auto mode should check if there is a server created for the domain specified, if not, create one.
    int return_value = 0;

    option::Option* pOp = options[DOMAIN];
    DomainId_t id = get_domain_id(pOp);
    uint32_t port = getDiscoveryServerPort(id);


    std::cout << "Port for Domain ID [" << id << "] is: " << port << std::endl;

    // Where do we parse the environment variable ROS_STATIC_PEERS?

    std::cout << "Auto mode not implemented yet." << std::endl;

    return return_value;
}

int CliDiscoveryManager::fastdds_discovery_start(
    std::vector<option::Option>& options,
    option::Parser& parse)
{
    if (initial_options_fail(options, parse))
    {
        return 1;
    }
    // Start a server for the domain specified
    int return_value = 0;
    std::cout << "Start mode not implemented yet." << std::endl;

    return return_value;
}

int CliDiscoveryManager::fastdds_discovery_stop(
    std::vector<option::Option>& options,
    option::Parser& parse)
{
    if (initial_options_fail(options, parse))
    {
        return 1;
    }

    // Stop a server for the domain specified
    int return_value = 0;
    std::cout << "Stop mode not implemented yet." << std::endl;

    return return_value;
}

int CliDiscoveryManager::fastdds_discovery_add(
    std::vector<option::Option>& options,
    option::Parser& parse)
{
    if (initial_options_fail(options, parse))
    {
        return 1;
    }

    // Add a server for the domain specified
    int return_value = 0;
    std::cout << "Add mode not implemented yet." << std::endl;

    return return_value;
}

int CliDiscoveryManager::fastdds_discovery_set(
    std::vector<option::Option>& options,
    option::Parser& parse)
{
    if (initial_options_fail(options, parse))
    {
        return 1;
    }

    // Set a server for the domain specified
    int return_value = 0;
    std::cout << "Set mode not implemented yet." << std::endl;

    return return_value;
}

int CliDiscoveryManager::fastdds_discovery_list(
    std::vector<option::Option>& options,
    option::Parser& parse)
{
    if (initial_options_fail(options, parse))
    {
        return 1;
    }

    // List all servers
    int return_value = 0;

    option::Option* pOp = options[DOMAIN];
    DomainId_t id = get_domain_id(pOp);
    rtps::PortParameters port_default;
    uint32_t port = getDiscoveryServerPort(id, port_default);

    std::cout << "Port for Domain ID [" << id << "] is: " << port << std::endl;
    std::cout << "List mode not implemented yet." << std::endl;

    return return_value;
}

int CliDiscoveryManager::fastdds_discovery_info(
    std::vector<option::Option>& options,
    option::Parser& parse)
{
    if (initial_options_fail(options, parse))
    {
        return 1;
    }

    // Get info from a server for the domain specified
    int return_value = 0;
    std::cout << "Info mode not implemented yet." << std::endl;

    return return_value;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
