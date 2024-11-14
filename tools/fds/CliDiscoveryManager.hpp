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

constexpr const char* domain_env_var = "ROS_DOMAIN_ID";

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
};

class CliDiscoveryManager
{
public:

    /**
     * @brief Get the domain id from the environment variable or the CLI argument. If none of the two is
     * provided, the default domain id is 0.
     * @param domain_id The domain id argument
     * @return The domain id
     */
    eprosima::fastdds::dds::DomainId_t get_domain_id(
            const eprosima::option::Option* domain_id);

    /**
     * @brief Check if the options received by the CLI are free of errors.
     * @param options The options received by the CLI
     * @param parse The parser object to be used
     * @return True if the options are incorrect, false otherwise
     */
    bool initial_options_fail(
            std::vector<option::Option>& options,
            option::Parser& parse);

    inline uint32_t getDiscoveryServerPort(
            const uint32_t domainId);

    /**
     * @brief Execute the command provided by the user.
     * @param command The command to be executed
     * @return The output of the command
     */
    std::string execCommand(
            const std::string& command);

    std::vector<uint16_t> getListeningPorts();

    /**
     * @brief Create a Discovery Server with the configuration options received.
     * @param options The options received by the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_server(
            std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Launch the AUTO mode of the CLI. It checks if a new Discovery Server exists in the
     * specified domain. It it does not exist, it creates a new one. If it exists, it does nothing.
     * @param options The options received by the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_auto(
            std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Starts a new Discovery Server in the specified domain if there is not an active server
     * already running. It does nothing if there is an active server.
     * @param options The options received by the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_start(
            std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Stops the active Discovery Server running in the specified domain.
     * @param options The options received by the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_stop(
            std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Adds new remote servers to the Discovery Server running in the specified domain.
     * @param options The options received by the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_add(
            std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Repalce the remote servers list of the Discovery Server running in the specified domain
     * with a new list.
     * @param options The options received by the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_set(
            std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Lists all the local running Discovery Servers.
     * @param options The options received by the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_list(
            std::vector<option::Option>& options,
            option::Parser& parse);

    /**
     * @brief Provides detailed information of the Discovery Server running in the specified domain.
     * @param options The options received by the CLI
     * @param parse The parser object to be used
     */
    int fastdds_discovery_info(
            std::vector<option::Option>& options,
            option::Parser& parse);

private:
    //! Default port parameters used to calculate Discovery Server ports
    rtps::PortParameters port_params_;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_CLI_DISCOVERY_HPP





