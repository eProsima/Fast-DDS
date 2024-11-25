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

#ifndef FASTDDS_CLI_DISCOVERY_HPP
#define FASTDDS_CLI_DISCOVERY_HPP

#include <string>
#include <vector>

#include <optionparser.hpp>
#include <fastdds/rtps/common/PortParameters.hpp>
#include <fastdds/dds/core/Types.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

constexpr const char* domain_env_var = "ROS_DOMAIN_ID";
constexpr const char* remote_servers_env_var = "ROS_STATIC_PEERS";
constexpr const char* default_ip = "0.0.0.0";

namespace fds {
enum ToolCommand : uint16_t
{
    AUTO = 0,
    START = 1,
    STOP = 2,
    ADD = 3,
    SET = 4,
    LIST = 5,
    INFO = 6,
    SERVER = 42
};
} // namespace fds

namespace eprosima {
namespace fastdds {
namespace dds {

struct MetaInfo_DS
{
    uint32_t domain_id;
    uint16_t port;
    std::string address;

    MetaInfo_DS()
        : domain_id(0)
        , port(0)
        , address("0.0.0.0")
    {
    }

    MetaInfo_DS(
            uint32_t domain_id,
            uint32_t port)
        : domain_id(domain_id)
        , port(port)
        , address("0.0.0.0")
    {
    }

    friend std::ostream& operator <<(
            std::ostream& os,
            const MetaInfo_DS& meta)
    {
        os << "- Server:\n"
           << "   Domain ID: " << meta.domain_id
           << "\n   Port: " << meta.port
           << "\n   Address: " << meta.address << std::endl;
        return os;
    }

};

class CliDiscoveryManager
{
public:

    using Locator_t = fastdds::rtps::Locator_t;
    using IPLocator = fastdds::rtps::IPLocator;

    /**
     * @brief Constructor
     */
    CliDiscoveryManager();

    /**
     * @brief Get the default shared directory used to communicate servers' info between processes.
     * @return The default shared directory
     */
    std::string get_default_shared_dir();

    /**
     * @brief Get the domain id from the environment variable or the CLI argument. If none of the two is
     * provided, the default domain id is 0.
     * @param domain_id The domain id argument
     * @return The domain id
     */
    DomainId_t get_domain_id(
            const eprosima::option::Option* domain_id);

    /**
     * @brief Set the remote servers list from the environment variable.
     * Previously set servers are not cleared.
     * @param target_list The list to be set
     */
    void addRemoteServersFromEnv(
            rtps::LocatorList_t& target_list);

    /**
     * @brief Load the remote servers information from the CLI or the environment variable.
     * @param parse The parser object to be used
     * @param numServs Number of nonOpts, which are meant to be servers
     * @return The servers locators in string format
     */
    std::string getRemoteServers(
            option::Parser& parse,
            int numServs);

    /**
     * @brief Check if the options received by the CLI are free of errors.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     * @param check_nonOpts True if the nonOpts should be checked, false otherwise
     * @return True if the options are incorrect, false otherwise
     */
    bool initial_options_fail(
            const std::vector<option::Option>& options,
            option::Parser& parse,
            bool check_nonOpts = true);

    /**
     * @brief Get the port of the Discovery Server from the CLI.
     * @param portArg The port argument received by the CLI
     * @return The port of the Discovery Server
     */
    uint16_t getDiscoveryServerPort(
            const eprosima::option::Option* portArg);

    /**
     * @brief Get the port of the Discovery Server running in the specified domain.
     * @param domainId The domain id of the Discovery Server
     * @return The port of the Discovery Server
     */
    uint16_t getDiscoveryServerPort(
            const uint32_t& domainId);

    /**
     * @brief Execute the command provided by the user.
     * @param command The command to be executed
     * @return The output of the command
     */
    std::string execCommand(
            const std::string& command);

    /**
     * @brief Get the listening TCP ports of the machine.
     * @return An ordered vector with the listening ports
     */
    std::vector<uint16_t> getListeningPorts();

    /**
     * @brief Get the local Discovery Servers running in the machine.
     * @return A vector with the info of local servers
     */
    std::vector<MetaInfo_DS> getLocalServers();

    /**
     * @brief Check if a Discovery Server is running in the specified domain.
     * @param domain The domain id of the Discovery Server
     * @return True if the server is running, false otherwise
     */
    bool isServerRunning(
            const DomainId_t& domain);

    /**
     * @brief Get the PID of the Discovery Server running in the specified port.
     * @param port The port of the Discovery Server
     * @return The PID of the Discovery Server
     */
    pid_t getPidOfServer(
            const uint16_t& port);

    /**
     * @brief Starts a new Discovery Server in the specified @c port running
     * in the background.
     * @param port The port of the Discovery Server
     * @param domain The domain id of the Discovery Server
     * @param use_env_var True if the environment variable should be used, false otherwise
     * @return The PID of the Discovery Server in the background
     */
    pid_t startServerInBackground(
            const uint16_t& port,
            const DomainId_t& domain,
            bool use_env_var);

    /**
     * @brief Set the QoS of the Discovery Server.
     * @param port The port of the Discovery Server
     */
    void setServerQos(
            const uint16_t port);

    /**
     * @brief Load the XML configuration into the serverQos.
     * @param xmlArg The XML file argument
     * @return True if the XML file is loaded, false otherwise
     */
    bool loadXMLFile(
            const eprosima::option::Option* xmlArg);

    /**
     * @brief Get the UDP and TCP ports and addresses from the CLI arguments.
     * @param udpPort The UDP port argument
     * @param udpIp The UDP address argument
     * @param tcpPort The TCP port argument
     * @param tcpIp The TCP address argument
     */
    void getCliPortsAndIps(
            const eprosima::option::Option* udpPort,
            const eprosima::option::Option* udpIp,
            const eprosima::option::Option* tcpPort,
            const eprosima::option::Option* tcpIp);

    /**
     * @brief Set the address and kind of the locator.
     * @param [in]      address The address of the locator
     * @param [in, out] locator The locator to be set
     * @param [in]      is_tcp True if the locator is TCP, false otherwise
     * @return True if the address and kind are set, false otherwise
     */
    bool setAddressAndKind(
            const std::string& address,
            Locator_t& locator,
            bool is_tcp);

    /**
     * @brief Set the metatrafficUnicastLocatorList for UDP servers.
     * @return True if the metatrafficUnicastLocatorList is set, false otherwise
     */
    bool addUdpServers();

    /**
     * @brief Set the metatrafficUnicastLocatorList for TCP servers.
     * @return True if the metatrafficUnicastLocatorList is set, false otherwise
     */
    bool addTcpServers();

    /**
     * @brief Configure the transports of the Discovery Server participant. If UDPv6 is enabled, it
     * adds an UDPv6 transport. If TCPv4 or TCPv6 are enabled, it adds a TCPv4 or TCPv6
     * transport for each listening port provided. Builtin transports are disabled if no UDPv4 locator
     * is provided.
     */
    void configureTransports();

    /**
     * @brief Create a Discovery Server with the configuration options received.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_server(
            const std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Launch the AUTO mode of the CLI. It checks if a new Discovery Server exists in the
     * specified domain. It it does not exist, it creates a new one. If it exists, it does nothing.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_auto(
            const std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Starts a new Discovery Server in the specified domain if there is not an active server
     * already running. It does nothing if there is an active server.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_start(
            const std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Stops the active Discovery Server running in the specified domain.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_stop(
            const std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Adds new remote servers to the Discovery Server running in the specified domain.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_add(
            const std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Repalce the remote servers list of the Discovery Server running in the specified domain
     * with a new list.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_set(
            const std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Lists all the local running Discovery Servers.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_list(
            const std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Provides detailed information of the Discovery Server running in the specified domain.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_info(
            const std::vector<option::Option>& options,
            option::Parser& parse);

protected:

    //! QoS of the Discovery Server that will be initialized
    DomainParticipantQos serverQos;
    //! DomainParticipant of the Discovery Server
    DomainParticipant* pServer;
    //! Default port parameters used to calculate Discovery Server ports
    rtps::PortParameters port_params_;
    //! UDP ports received from the CLI
    std::list<uint16_t> udp_ports_;
    //! UDP addresses received from the CLI
    std::list<std::string> udp_ips_;
    //! TCP ports received from the CLI
    std::list<uint16_t> tcp_ports_;
    //! TCP addresses received from the CLI
    std::list<std::string> tcp_ips_;
    //! Endpoint QoS to get data_sharing directory
    std::string intraprocess_dir_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_CLI_DISCOVERY_HPP





