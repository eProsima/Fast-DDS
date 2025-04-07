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

#ifndef FASTDDS_TOOLS_FDS_CLI_DISCOVERY_MANAGER_HPP
#define FASTDDS_TOOLS_FDS_CLI_DISCOVERY_MANAGER_HPP

#include <string>
#include <vector>

#include <optionparser.hpp>
#include <fastdds/rtps/common/PortParameters.hpp>
#include <fastdds/dds/core/Types.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

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
            uint16_t port)
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
           << "    Domain ID: " << meta.domain_id
           << "\n    Port: " << meta.port
           << "\n    Address: " << meta.address << std::endl;
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
     * @brief Destructor
     */
    ~CliDiscoveryManager();

    /**
     * @brief Get the default shared directory used to communicate servers' info between processes.
     * @return The default shared directory
     */
    std::string get_default_shared_dir();

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
    uint16_t get_discovery_server_port(
            const eprosima::option::Option* portArg);

#ifndef _WIN32
    /**
     * @brief Get the port of the Discovery Server running in the specified domain.
     * @param domainId The domain id of the Discovery Server
     * @return The port of the Discovery Server
     */
    uint16_t get_discovery_server_port(
            const uint32_t& domainId);

    /**
     * @brief Get the domain id from the CLI argument. If not provided, the default domain id is 0.
     * @param domain_id The domain id argument
     * @return The domain id
     */
    DomainId_t get_domain_id(
            const eprosima::option::Option* domain_id);

    /**
     * @brief Load the remote servers information from the CLI or the environment variable.
     * @param parse The parser object to be used
     * @param numServs Number of nonOpts, which are meant to be servers
     * @return The servers locators in string format
     */
    std::string get_remote_servers(
            option::Parser& parse,
            int numServs);

    /**
     * @brief Execute the command provided by the user.
     * @param command The command to be executed
     * @return The output of the command
     */
    std::string exec_command(
            const std::string& command);

    /**
     * @brief Get the listening UDP ports of the machine.
     * @return An ordered vector with the listening ports
     */
    virtual std::vector<uint16_t> get_listening_ports();

    /**
     * @brief Get the local Discovery Servers running in the machine.
     * @return A vector with the info of local servers
     */
    std::vector<MetaInfo_DS> get_local_servers();

    /**
     * @brief Check if a Discovery Server is running in the specified domain.
     * @param domain The domain id of the Discovery Server
     * @return True if the server is running, false otherwise
     */
    bool is_server_running(
            const DomainId_t& domain);

    /**
     * @brief Get the PID of the Discovery Server running in the specified port.
     * @param port The port of the Discovery Server
     * @return The PID of the Discovery Server
     */
    pid_t get_pid_of_server(
            const uint16_t& port);

    /**
     * @brief Starts a new Discovery Server in the specified @c port .
     * @param port The port of the Discovery Server
     * @param domain The domain id of the Discovery Server
     * @param use_env_var True if the environment variable should be used, false otherwise
     */
    void start_server_auto_mode(
            const uint16_t& port,
            const DomainId_t& domain);
#endif // ifndef _WIN32

    /**
     * @brief Set the QoS of the Discovery Server.
     * @param port The port of the Discovery Server
     */
    void set_server_qos(
            const uint16_t port);

    /**
     * @brief Load the XML configuration into the serverQos.
     * @param xmlArg The XML file argument
     * @return True if the XML file is loaded, false otherwise
     */
    bool load_XML_file(
            const eprosima::option::Option* xmlArg);

    /**
     * @brief Get the UDP and TCP ports and addresses from the CLI arguments.
     * @param udpPort The UDP port argument
     * @param udpIp The UDP address argument
     * @param tcpPort The TCP port argument
     * @param tcpIp The TCP address argument
     */
    void get_cli_ports_and_ips(
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
    bool set_address_and_kind(
            const std::string& address,
            Locator_t& locator,
            bool is_tcp);

    /**
     * @brief Set the metatrafficUnicastLocatorList for UDP servers.
     * @return True if the metatrafficUnicastLocatorList is set, false otherwise
     */
    bool add_udp_servers();

    /**
     * @brief Set the metatrafficUnicastLocatorList for TCP servers.
     * @return True if the metatrafficUnicastLocatorList is set, false otherwise
     */
    bool add_tcp_servers();

    /**
     * @brief Configure the transports of the Discovery Server participant. If UDPv6 is enabled, it
     * adds an UDPv6 transport. If TCPv4 or TCPv6 are enabled, it adds a TCPv4 or TCPv6
     * transport for each listening port provided. Builtin transports are disabled if no UDPv4 locator
     * is provided.
     */
    void configure_transports();

    /**
     * @brief Create a Discovery Server with the configuration options received.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_server(
            const std::vector<option::Option>& options,
            option::Parser& parse);

#ifndef _WIN32
    /**
     * @brief Launch the AUTO mode of the CLI. It checks if a new Discovery Server exists in the
     * specified domain. It it does not exist, it creates a new one. If it exists, it does nothing.
     * Remote server is added directly from the argument passed to the CLI, which is mandatory.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_auto_start(
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
     * @brief Add new remote servers to the Discovery Server running in the specified domain.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_add(
            const std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Replace the remote servers list of the Discovery Server running in the specified domain
     * with a new list.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_set(
            const std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief List all the local running Discovery Servers.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_list(
            const std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Provide detailed information of the Discovery Server running in the specified domain.
     * @param options The options received from the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_info(
            const std::vector<option::Option>& options,
            option::Parser& parse);
#endif // ifndef _WIN32

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

#endif // FASTDDS_TOOLS_FDS_CLI_DISCOVERY_MANAGER_HPP





