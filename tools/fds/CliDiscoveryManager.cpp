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

#include "CliDiscoveryManager.hpp"
#include "CliDiscoveryParser.hpp"

#include <chrono>
#include <condition_variable>
#include <csignal>
#include <fstream>
#include <iostream>
#include <mutex>
#include <regex>
#include <sstream>
#include <stdlib.h>
#include <thread>

#if __APPLE__
#include <signal.h>
#endif // if __APPLE__

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/utils/IPLocator.hpp>
#include <rtps/attributes/ServerAttributes.hpp>


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

#ifndef _WIN32
static std::string read_servers_from_file(
        const std::string& file)
{
    std::ifstream ifs(file);
    if (!ifs.is_open())
    {
        EPROSIMA_LOG_ERROR(CLI, "Error opening file: " << file);
        return "";
    }
    std::string line;
    std::getline(ifs, line);
    ifs.close();
    return line;
}

static bool write_servers_to_file(
        const std::string& file,
        const std::string& servers)
{
    std::ofstream ofs(file);
    if (!ofs.is_open())
    {
        EPROSIMA_LOG_ERROR(CLI, "Error opening file: " << file);
        return false;
    }
    ofs << servers;
    ofs.close();
    return true;
}

#endif // ifndef _WIN32

namespace eprosima {
namespace fastdds {
namespace dds {

CliDiscoveryManager::CliDiscoveryManager()
    : pServer(nullptr)
{
    try
    {
        intraprocess_dir_ = get_default_shared_dir();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error getting default shared directory: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

CliDiscoveryManager::~CliDiscoveryManager()
{
    Log::Flush();
    std::cout.flush();
    std::cerr.flush();
}

std::string CliDiscoveryManager::get_default_shared_dir()
{
    std::string shm_path;
#ifdef _WIN32
    shm_path = "c:\\programdata\\eprosima\\fastdds_interprocess\\";
#elif __APPLE__
    shm_path = "/private/tmp/boost_interprocess/";
#elif __linux__
    shm_path = "/dev/shm/";
#else
    throw std::runtime_error(std::string("Platform not supported"));
#endif // ifdef _WIN32
    return shm_path;
}

bool CliDiscoveryManager::initial_options_fail(
        const std::vector<option::Option>& options,
        option::Parser& parse,
        bool check_nonOpts)
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
    if (check_nonOpts)
    {
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
    }
    return false;
}

uint16_t CliDiscoveryManager::get_discovery_server_port(
        const eprosima::option::Option* portArg)
{
    uint16_t port = 0;
    if (nullptr != portArg)
    {
        std::stringstream port_stream;

        port_stream << portArg->arg;
        port_stream >> port;
        if (!port_stream.eof())
        {
            EPROSIMA_LOG_WARNING(CLI, "Invalid listening locator port specified:" << port);
            return 0;
        }
    }
    return port;
}

#ifndef _WIN32
uint16_t CliDiscoveryManager::get_discovery_server_port(
        const uint32_t& domainId)
{
    if (domainId > 232)
    {
        EPROSIMA_LOG_WARNING(CLI, "Domain ID " << domainId << " is too high and cannot run in an unreachable port.");
        return 0;
    }
    uint16_t port = port_params_.get_discovery_server_port(domainId);

    return port;
}

DomainId_t CliDiscoveryManager::get_domain_id(
        const eprosima::option::Option* domain_id)
{
    DomainId_t id = 0;

    // Python parser always adds the domain_id option, so it is always present
    if (domain_id != nullptr)
    {
        std::stringstream domain_stream;

        domain_stream << domain_id->arg;
        domain_stream >> id;
    }

    return id;
}

std::string CliDiscoveryManager::get_remote_servers(
        option::Parser& parse,
        int numServs)
{
    std::string servers;
    if (numServs)
    {
        servers = parse.nonOption(0);
    }
    return servers;
}

std::string CliDiscoveryManager::exec_command(
        const std::string& command)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, int (*)(
        FILE*)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe)
    {
        EPROSIMA_LOG_ERROR(CLI, "Error processing command:" << command);
        return "";
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

std::vector<uint16_t> CliDiscoveryManager::get_listening_ports()
{
    std::vector<uint16_t> ports;
    std::string command;
#ifdef __APPLE__
    command = "netstat -an | grep udp4";
    std::regex port_regex(R"(\*.([\d+]+))");
#else
    command = "ss -lun";
    std::regex port_regex(R"(0\.0\.0\.0:(\d+))");
#endif // ifdef __APPLE__
    std::string result = exec_command(command);
    std::smatch match;
    std::string line;
    std::istringstream result_stream(result);
    while (std::getline(result_stream, line))
    {
        if (std::regex_search(line, match, port_regex))
        {
            try
            {
                ports.push_back(static_cast<uint16_t>(std::stoi(match[1].str())));
            }
            catch (const std::invalid_argument& e)
            {
                EPROSIMA_LOG_ERROR(CLI, "Error getting port from line: " << line);
            }
        }
    }
    std::sort(ports.begin(), ports.end());
    return ports;
}

std::vector<MetaInfo_DS> CliDiscoveryManager::get_local_servers()
{
    std::vector<MetaInfo_DS> servers;
    std::vector<uint16_t> ports = get_listening_ports();
    for (const uint16_t& port : ports)
    {
        // Check if the port is a Discovery Server port
        double domain = double(port - port_params_.portBase - port_params_.offsetd4) / port_params_.domainIDGain;
        if (domain >= 0 && std::floor(domain) == domain)
        {
            servers.push_back(MetaInfo_DS(static_cast<uint32_t>(domain), port));
        }
    }
    return servers;
}

bool CliDiscoveryManager::is_server_running(
        const DomainId_t& domain)
{
    std::vector<MetaInfo_DS> servers = get_local_servers();
    for (const MetaInfo_DS& server : servers)
    {
        if (server.domain_id == domain)
        {
            return true;
        }
    }
    return false;
}

pid_t CliDiscoveryManager::get_pid_of_server(
        const uint16_t& port)
{
    std::string command = "lsof -i :";
    command += std::to_string(port);
    command += " | grep UDP | awk '{print $2}'";
    std::string result = exec_command(command);
    if (result.empty())
    {
        EPROSIMA_LOG_ERROR(CLI, "Error getting PID. No server found on port " << port);
        return 0;
    }
    pid_t ret_value = 0;
    try
    {
        ret_value = std::stoi(result);
    }
    catch (const std::invalid_argument& e)
    {
        EPROSIMA_LOG_ERROR(CLI, "Error getting PID from: " << result);
    }
    return ret_value;
}

void CliDiscoveryManager::start_server_auto_mode(
        const uint16_t& port,
        const DomainId_t& domain)
{
    set_server_qos(port);

    // Create the server in the child process
    pServer = DomainParticipantFactory::get_instance()->create_participant(0, serverQos);

    if (nullptr == pServer)
    {
        EPROSIMA_LOG_ERROR(CLI, "Server creation for Domain ID [" << domain << "] failed with the given settings.");
        return;
    }

    std::cout << "Server for Domain ID [" << domain << "] started on port " << port << std::endl;

    std::unique_lock<std::mutex> lock(g_signal_mutex);
    // Handle signal SIGINT for every thread
    signal(SIGUSR1, sigint_handler);
    signal(SIGUSR2, sigint_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    signal(SIGQUIT, sigint_handler);
    signal(SIGHUP, sigint_handler);


    bool should_break = false;
    while (!should_break)
    {
        g_signal_cv.wait(lock, []
                {
                    return 0 != g_signal_status;
                });
        if (SIGUSR1 == g_signal_status)
        {
            // Update Qos
            std::stringstream file_name;
            file_name << intraprocess_dir_ << "/" << port << "_servers.txt";
            rtps::LocatorList_t serverList;
            load_environment_server_info(read_servers_from_file(file_name.str()), serverList);
            for (rtps::Locator_t& locator : serverList)
            {
                locator.kind = LOCATOR_KIND_UDPv4;
            }
            pServer->get_qos(serverQos);
            serverQos.wire_protocol().builtin.discovery_config.m_DiscoveryServers = serverList;
            pServer->set_qos(serverQos);
            g_signal_status = 0;
        }
        else if (SIGUSR2 == g_signal_status)
        {
            DomainParticipantFactory::get_instance()->delete_participant(pServer);
            std::stringstream file_name;
            file_name << intraprocess_dir_ << "/" << port << "_servers.txt";
            rtps::LocatorList_t serverList;
            load_environment_server_info(read_servers_from_file(file_name.str()), serverList);
            for (rtps::Locator_t& locator : serverList)
            {
                locator.kind = LOCATOR_KIND_UDPv4;
            }
            serverQos.wire_protocol().builtin.discovery_config.m_DiscoveryServers = serverList;
            pServer = DomainParticipantFactory::get_instance()->create_participant(0, serverQos);
            g_signal_status = 0;
        }
        else
        {
            should_break = true;
        }
    }
    DomainParticipantFactory::get_instance()->delete_participant(pServer);
    exit(0);
    return;
}

#endif // ifndef _WIN32

void CliDiscoveryManager::set_server_qos(
        const uint16_t port)
{
    rtps::Locator_t udp_locator;
    udp_locator.kind = LOCATOR_KIND_UDPv4;
    udp_locator.port = port;
    // Initialize to the wan interface
    IPLocator::setIPv4(udp_locator, "0.0.0.0");
    serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(udp_locator);
    auto udp_descriptor = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
    serverQos.transport().user_transports.push_back(udp_descriptor);
    serverQos.transport().use_builtin_transports = false;
    serverQos.wire_protocol().builtin.discovery_config.discoveryProtocol = rtps::DiscoveryProtocol::SERVER;
    serverQos.name("DiscoveryServerAuto");
}

bool CliDiscoveryManager::load_XML_file(
        const eprosima::option::Option* xmlArg)
{
    constexpr const char* delimiter = "@";
    std::string sXMLConfigFile = "";
    std::string profile = "";

    sXMLConfigFile = xmlArg->arg;
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
            EPROSIMA_LOG_ERROR(CLI, "Cannot open XML file " << sXMLConfigFile << ". Please, check the path of this "
                                                            << "XML file.");
            return false;
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
                EPROSIMA_LOG_ERROR(CLI, "Error setting environment variables: " << errmsg);
                return false;
            }
#else
            if (0 != unsetenv("FASTDDS_DEFAULT_PROFILES_FILE") ||
                    0 != setenv("SKIP_DEFAULT_XML_FILE", "1", 1))
            {
                EPROSIMA_LOG_ERROR(CLI, "Error setting environment variables: " << std::strerror(errno));
                return false;
            }
#endif // ifdef _WIN32
            // Set default participant QoS from XML file
            if (RETCODE_OK != DomainParticipantFactory::get_instance()->load_profiles())
            {
                EPROSIMA_LOG_ERROR(CLI, "Error setting default DomainParticipantQos from XML default profile.");
                return false;
            }
            serverQos = DomainParticipantFactory::get_instance()->get_default_participant_qos();
        }
        else
        {
            if (RETCODE_OK !=
                    DomainParticipantFactory::get_instance()->get_participant_qos_from_profile(
                        profile, serverQos))
            {
                EPROSIMA_LOG_ERROR(CLI, "Error loading specified profile from XML file.");
                return false;
            }
        }
    }
    return true;
}

bool CliDiscoveryManager::add_udp_servers()
{
    if (udp_ports_.size() < udp_ips_.size() && udp_ips_.size() != 1)
    {
        EPROSIMA_LOG_WARNING(CLI,
                "The number of specified ports doesn't match the ip addresses provided. Locators might share their port number.");
    }
    auto it_p = udp_ports_.begin();
    auto it_i = udp_ips_.begin();
    while (it_p != udp_ports_.end() || it_i != udp_ips_.end())
    {
        uint16_t port = (it_p != udp_ports_.end()) ? *it_p : rtps::DEFAULT_ROS2_SERVER_PORT;
        std::string ip = (it_i != udp_ips_.end()) ? *it_i : "";

        Locator_t loc(port);
        if (!set_address_and_kind(ip, loc, false))
        {
            return false;
        }
        serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(loc);

        if (it_p != udp_ports_.end())
        {
            ++it_p;
        }
        if (it_i != udp_ips_.end())
        {
            ++it_i;
        }
    }
    return true;
}

bool CliDiscoveryManager::add_tcp_servers()
{
    if (tcp_ports_.size() < tcp_ips_.size() && tcp_ips_.size() != 1)
    {
        EPROSIMA_LOG_ERROR(CLI,
                "The number of specified TCP ports is lower than the ip addresses provided. TCP transports cannot share listening port.");
        return false;
    }
    auto it_p = tcp_ports_.begin();
    auto it_i = tcp_ips_.begin();
    while (it_p != tcp_ports_.end() || it_i != tcp_ips_.end())
    {
        uint16_t port = (it_p != tcp_ports_.end()) ? *it_p : rtps::DEFAULT_TCP_SERVER_PORT;
        std::string ip = (it_i != tcp_ips_.end()) ? *it_i : "";

        Locator_t loc(port);
        IPLocator::setLogicalPort(loc, port);
        if (!set_address_and_kind(ip, loc, true))
        {
            return false;
        }
        serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(loc);

        if (it_p != tcp_ports_.end())
        {
            ++it_p;
        }
        if (it_i != tcp_ips_.end())
        {
            ++it_i;
        }
    }
    return true;
}

void CliDiscoveryManager::configure_transports()
{
    if (!serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList.has_kind<LOCATOR_KIND_UDPv4>())
    {
        serverQos.transport().use_builtin_transports = false;
        std::shared_ptr<rtps::SharedMemTransportDescriptor> shm_transport =
                std::make_shared<rtps::SharedMemTransportDescriptor>();
        serverQos.transport().user_transports.push_back(shm_transport);
    }
    // Add UDPv6 transport if required
    if (serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList.has_kind<LOCATOR_KIND_UDPv6>())
    {
        auto descriptor = std::make_shared<fastdds::rtps::UDPv6TransportDescriptor>();
        descriptor->sendBufferSize = serverQos.transport().send_socket_buffer_size;
        descriptor->receiveBufferSize = serverQos.transport().listen_socket_buffer_size;
        serverQos.transport().user_transports.push_back(std::move(descriptor));
    }
    // Add new transport for each TCP listening port
    for (auto& loc : serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList)
    {
        if ( LOCATOR_KIND_TCPv4 == loc.kind )
        {
            auto tcp_descriptor = std::make_shared<eprosima::fastdds::rtps::TCPv4TransportDescriptor>();
            tcp_descriptor->non_blocking_send = true;
            tcp_descriptor->add_listener_port(static_cast<uint16_t>(loc.port));
            serverQos.transport().user_transports.push_back(tcp_descriptor);
        }
        if ( LOCATOR_KIND_TCPv6 == loc.kind )
        {
            auto tcp_descriptor = std::make_shared<eprosima::fastdds::rtps::TCPv6TransportDescriptor>();
            tcp_descriptor->non_blocking_send = true;
            tcp_descriptor->add_listener_port(static_cast<uint16_t>(loc.port));
            serverQos.transport().user_transports.push_back(tcp_descriptor);
        }
    }
}

void CliDiscoveryManager::get_cli_ports_and_ips(
        const eprosima::option::Option* udpPort,
        const eprosima::option::Option* udpIp,
        const eprosima::option::Option* tcpPort,
        const eprosima::option::Option* tcpIp)
{
    while (udpPort)
    {
        udp_ports_.push_back(get_discovery_server_port(udpPort));
        udpPort = udpPort->next();
    }
    while (udpIp)
    {
        if (udpIp->arg == nullptr)
        {
            // Listen on 'any' interface by default
            udp_ips_.push_back("0.0.0.0");
        }
        else
        {
            udp_ips_.push_back(std::string(udpIp->arg));
        }
        udpIp = udpIp->next();
    }
    while (tcpPort)
    {
        tcp_ports_.push_back(get_discovery_server_port(tcpPort));
        tcpPort = tcpPort->next();
    }
    while (tcpIp)
    {
        if (tcpIp->arg == nullptr)
        {
            // Listen on 'any' interface by default
            tcp_ips_.push_back("0.0.0.0");
        }
        else
        {
            tcp_ips_.push_back(std::string(tcpIp->arg));
        }
        tcpIp = tcpIp->next();
    }
}

bool CliDiscoveryManager::set_address_and_kind(
        const std::string& address,
        Locator_t& locator,
        bool is_tcp)
{
    int type = LOCATOR_PORT_INVALID;
    if (address.empty())
    {
        IPLocator::setIPv4(locator, default_ip);
        type = LOCATOR_KIND_UDPv4;
    }
    else
    {
        std::string loc_address = address;
        // Trial order IPv4, IPv6 & DNS
        Locator_t loc_v6(locator); // Test locator to check validity of IPv6 address
        loc_v6.kind = LOCATOR_KIND_UDPv6;
        if (IPLocator::isIPv4(loc_address) && IPLocator::setIPv4(locator, loc_address))
        {
            type = LOCATOR_KIND_UDPv4;
        }
        else if (IPLocator::isIPv6(loc_address) && IPLocator::setIPv6(loc_v6, loc_address))
        {
            type = LOCATOR_KIND_UDPv6;
            locator = loc_v6;
        }
        else
        {
            auto response = IPLocator::resolveNameDNS(loc_address);
            // Add the first valid IPv4 address that we can find
            if (response.first.size() > 0)
            {
                loc_address = response.first.begin()->data();
                if (IPLocator::setIPv4(locator, loc_address))
                {
                    type = LOCATOR_KIND_UDPv4;
                }
            }
            else if (response.second.size() > 0)
            {
                loc_address = response.second.begin()->data();
                if (IPLocator::setIPv6(locator, loc_address))
                {
                    type = LOCATOR_KIND_UDPv6;
                }
            }
        }
    }
    if (type == LOCATOR_PORT_INVALID)
    {
        EPROSIMA_LOG_ERROR(CLI, "Invalid listening locator address specified: " << address);
        return false;
    }
    if (is_tcp)
    {
        type *= 4;
    }
    locator.kind = type;
    return true;
}

int CliDiscoveryManager::fastdds_discovery_server(
        const std::vector<option::Option>& options,
        option::Parser& parse)
{
    // Convenience aliases
    using Locator = fastdds::rtps::Locator_t;
    using DiscoveryProtocol = fastdds::rtps::DiscoveryProtocol;
    using IPLocator = fastdds::rtps::IPLocator;

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

    if (options[DOMAIN_OPT])
    {
        EPROSIMA_LOG_WARNING(CLI,
                "Argument '-d' is only used in AUTO mode. Use -h to see more information.");
    }

    //////////////////////////////////
    /// Load XML file if specified ///
    //////////////////////////////////
    if (nullptr != options[XML_FILE])
    {
        if (!load_XML_file(options[XML_FILE]))
        {
            return 1;
        }
    }

    // Retrieve server ID: is optional and only specified once
    // Note there is a specific cast to pointer if the Option is valid
    const option::Option* pOp = options[SERVERID];
    int server_id = -1;
    std::stringstream server_stream;

    if (nullptr == pOp)
    {
        fastdds::rtps::GuidPrefix_t prefix_cero;
        if (nullptr != options[XML_FILE] && !(serverQos.wire_protocol().builtin.discovery_config.discoveryProtocol ==
                eprosima::fastdds::rtps::DiscoveryProtocol::SERVER ||
                serverQos.wire_protocol().builtin.discovery_config.discoveryProtocol ==
                eprosima::fastdds::rtps::DiscoveryProtocol::BACKUP))
        {
            // Discovery protocol specified in XML file is not SERVER nor BACKUP
            EPROSIMA_LOG_ERROR(CLI,
                    "The provided configuration is not valid. Participant must be either SERVER or BACKUP.");
            return 1;
        }
        else if (serverQos.wire_protocol().prefix == prefix_cero &&
                serverQos.wire_protocol().builtin.discovery_config.discoveryProtocol ==
                eprosima::fastdds::rtps::DiscoveryProtocol::BACKUP)
        {
            // Discovery protocol specified in XML is BACKUP, but no GUID was specified
            EPROSIMA_LOG_ERROR(CLI, "Specifying a GUID prefix is mandatory for BACKUP Discovery Servers." <<
                    "Update the XML file or use the -i argument.");
        }
    }
    else if (pOp->count() != 1)
    {
        EPROSIMA_LOG_ERROR(CLI,
                "Only one server participant can be created, thus, only one server id can be specified.");
        return 1;
    }
    else
    {
        // Cast of option to int has already been checked
        server_stream << pOp->arg;
        server_stream >> server_id;
        if (!eprosima::fastdds::rtps::get_server_client_default_guidPrefix(server_id,
                serverQos.wire_protocol().prefix))
        {
            EPROSIMA_LOG_ERROR(CLI, "Failed to set the GUID with the server identifier provided.");
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
    serverQos.name(server_stream.str().c_str());

    // Choose the kind of server to create
    pOp = options[BACKUP];
    if (nullptr != pOp)
    {
        fastdds::rtps::GuidPrefix_t prefix_cero;
        if (serverQos.wire_protocol().prefix == prefix_cero)
        {
            // BACKUP argument used, but no GUID was specified either in the XML nor in the CLI
            EPROSIMA_LOG_ERROR(CLI,
                    "Specifying a GUID prefix is mandatory for BACKUP Discovery Servers. Use the -i argument.");
            return 1;
        }
        serverQos.wire_protocol().builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::BACKUP;
    }
    else if (nullptr == options[XML_FILE])
    {
        serverQos.wire_protocol().builtin.discovery_config.discoveryProtocol = DiscoveryProtocol::SERVER;
    }

    get_cli_ports_and_ips(options[UDP_PORT], options[UDPADDRESS], options[TCP_PORT], options[TCPADDRESS]);
    if (udp_ports_.empty() && udp_ips_.empty() && tcp_ports_.empty() && tcp_ips_.empty())
    {
        if (options[XML_FILE] == nullptr)
        {
            // Add default UDP server
            Locator locator(rtps::DEFAULT_ROS2_SERVER_PORT);
            IPLocator::setIPv4(locator, 0, 0, 0, 0);
            serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);
        }
    }
    else
    {
        serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList.clear();
        if (!add_udp_servers())
        {
            EPROSIMA_LOG_ERROR(CLI, "Error creating UDP server.");
            return 1;
        }
        if (!add_tcp_servers())
        {
            EPROSIMA_LOG_ERROR(CLI, "Error creating TCP server.");
            return 1;
        }
    }

    configure_transports();

    // Create the server
    int return_value = 0;
    pServer = DomainParticipantFactory::get_instance()->create_participant(0, serverQos);

    if (nullptr == pServer)
    {
        EPROSIMA_LOG_ERROR(CLI, "Server creation failed with the given settings. Please review locators setup.");
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

        // Print running server attributes
        std::cout << "#### ";
        if (serverQos.wire_protocol().builtin.discovery_config.discoveryProtocol == DiscoveryProtocol::BACKUP)
        {
            std::cout << "Backup ";
        }
        std::cout << "Server started ####" << std::endl;
        std::cout << "  GUID prefix: " << pServer->guid().guidPrefix << std::endl;
        std::cout << "  Running on:  ";
        for (auto locator_it = serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList.begin();
                locator_it != serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList.end();)
        {
            std::cout << *locator_it;
            if (++locator_it != serverQos.wire_protocol().builtin.metatrafficUnicastLocatorList.end())
            {
                std::cout << std::endl << "               ";
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

#ifndef _WIN32
int CliDiscoveryManager::fastdds_discovery_auto_start(
        const std::vector<option::Option>& options,
        option::Parser& parse)
{
    // Check initial options allowing unknown args
    if (initial_options_fail(options, parse, false))
    {
        return 1;
    }

    int return_value = 0;
    // Servers are added directly to the CLI with no arg associated
    int numServerArgs = parse.nonOptionsCount();
    if (numServerArgs > 1)
    {
        EPROSIMA_LOG_ERROR(CLI_START,
                "Too many arguments specified. Expected format is: start -d <domain> <ip:domain;ip:domain...>");
        return 1;
    }

    // Check if there is a server running in the domain specified
    const option::Option* pOp = options[DOMAIN_OPT];
    DomainId_t id = get_domain_id(pOp);
    if (is_server_running(id))
    {
        std::cout << "Server for Domain ID [" << id << "] is already running." << std::endl;
        return return_value;
    }

    // Create a server for the domain specified
    uint16_t port = get_discovery_server_port(id);
    if (port == 0)
    {
        return 1;
    }
    std::string servers = get_remote_servers(parse, numServerArgs);
    rtps::LocatorList_t serverList;
    if (!load_environment_server_info(servers, serverList))
    {
        EPROSIMA_LOG_ERROR(CLI_START,
                "Error parsing servers. Expected format is: <ip:domain;ip:domain...>");
        return 1;
    }
    for (rtps::Locator_t& locator : serverList)
    {
        locator.kind = LOCATOR_KIND_UDPv4;
    }
    serverQos.wire_protocol().builtin.discovery_config.m_DiscoveryServers = serverList;
    start_server_auto_mode(port, id);

    return return_value;
}

int CliDiscoveryManager::fastdds_discovery_stop(
        const std::vector<option::Option>& options,
        option::Parser& parse)
{
    if (initial_options_fail(options, parse))
    {
        return 1;
    }

    // Stop a server for the domain specified
    int return_value = 0;

    const option::Option* pOp = options[DOMAIN_OPT];
    DomainId_t id = get_domain_id(pOp);

    if (!is_server_running(id))
    {
        std::cout << "There is no running Server for Domain ID [" << id << "]." << std::endl;
        return return_value;
    }

    // Create a server for the domain specified
    uint16_t port = get_discovery_server_port(id);
    pid_t server_pid = get_pid_of_server(port);
    if (server_pid == 0)
    {
        return 1;
    }
    if (kill(server_pid, SIGINT) == 0)
    {
        std::cout << "Server for Domain ID [" << id << "] stopped." << std::endl;
    }
    else
    {
        EPROSIMA_LOG_ERROR(CLI_STOP, "Could not stop server for Domain ID [" << id << "].");
        return_value = 1;
    }

    return return_value;
}

int CliDiscoveryManager::fastdds_discovery_add(
        const std::vector<option::Option>& options,
        option::Parser& parse)
{
    if (initial_options_fail(options, parse, false))
    {
        return 1;
    }

    // Add new remote servers for the domain specified
    int return_value = 0;
    // Servers are added directly to the CLI with no arg associated
    int numServs = parse.nonOptionsCount();
    if (numServs > 1)
    {
        EPROSIMA_LOG_ERROR(CLI_ADD,
                "Too many arguments specified. Expected format is: add -d <domain> <ip:domain;ip:domain...>");
        return 1;
    }

    const option::Option* pOp = options[DOMAIN_OPT];
    DomainId_t id = get_domain_id(pOp);
    if (!is_server_running(id))
    {
        std::cout << "There is no running Server for Domain ID [" << id << "]." << std::endl;
        return return_value;
    }

    uint16_t port = get_discovery_server_port(id);
    pid_t server_pid = get_pid_of_server(port);
    if (server_pid == 0)
    {
        return 1;
    }
    std::stringstream file_name;
    file_name << intraprocess_dir_ << "/" << port << "_servers.txt";
    std::string servers = get_remote_servers(parse, numServs);
    if (servers.empty())
    {
        EPROSIMA_LOG_ERROR(CLI_ADD,
                "No servers specified to add. Use ROS_STATIC_PEERS or format: add -d <domain> <ip:domain;ip:domain...>");
        return 1;
    }
    rtps::LocatorList_t serverList;
    if (!load_environment_server_info(servers, serverList))
    {
        EPROSIMA_LOG_ERROR(CLI_ADD,
                "Error parsing servers. Expected format is: <ip:domain;ip:domain...>");
        return 1;
    }
    std::cout << "Signaling Server for Domain ID [" << id << "] to add: " << servers << std::endl;
    write_servers_to_file(file_name.str(), servers);
    kill(server_pid, SIGUSR1);

    return return_value;
}

int CliDiscoveryManager::fastdds_discovery_set(
        const std::vector<option::Option>& options,
        option::Parser& parse)
{
    if (initial_options_fail(options, parse, false))
    {
        return 1;
    }

    // Set a server for the domain specified
    int return_value = 0;
    // Servers are added directly to the CLI with no arg associated
    int numServs = parse.nonOptionsCount();
    if (numServs > 1)
    {
        EPROSIMA_LOG_ERROR(CLI_SET,
                "Too many arguments specified. Expected format is: set -d <domain> <ip:domain;ip:domain...>");
        return 1;
    }

    const option::Option* pOp = options[DOMAIN_OPT];
    DomainId_t id = get_domain_id(pOp);
    if (!is_server_running(id))
    {
        std::cout << "There is no running Server for Domain ID [" << id << "]." << std::endl;
        return return_value;
    }

    uint16_t port = get_discovery_server_port(id);
    pid_t server_pid = get_pid_of_server(port);
    if (server_pid == 0)
    {
        return 1;
    }
    std::stringstream file_name;
    file_name << intraprocess_dir_ << "/" << port << "_servers.txt";
    std::string servers = get_remote_servers(parse, numServs);
    if (servers.empty())
    {
        EPROSIMA_LOG_ERROR(CLI_ADD,
                "No servers specified to add. Use ROS_STATIC_PEERS or format: add -d <domain> <ip:domain;ip:domain...>");
        return 1;
    }
    rtps::LocatorList_t serverList;
    if (!load_environment_server_info(servers, serverList))
    {
        EPROSIMA_LOG_ERROR(CLI_ADD,
                "Error parsing servers. Expected format is: <ip:domain;ip:domain...>");
        return 1;
    }
    std::cout << "Signaling Server for Domain ID [" << id << "] to set: " << servers << std::endl;
    write_servers_to_file(file_name.str(), servers);
    kill(server_pid, SIGUSR2);

    return return_value;
}

int CliDiscoveryManager::fastdds_discovery_list(
        const std::vector<option::Option>& options,
        option::Parser& parse)
{
    if (initial_options_fail(options, parse))
    {
        return 1;
    }

    // List all servers
    std::vector<MetaInfo_DS> servers = get_local_servers();

    std::cout << "### Servers running ###" << std::endl;
    for (const MetaInfo_DS& server : servers)
    {
        std::cout << server << std::endl;
    }
    std::cout << "#######################" << std::endl;

    return 0;
}

int CliDiscoveryManager::fastdds_discovery_info(
        const std::vector<option::Option>& options,
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

#endif // ifndef _WIN32

} // namespace dds
} // namespace fastdds
} // namespace eprosima
