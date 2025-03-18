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

/**
 * @file ServerAttributes.hpp
 *
 */

#ifndef FASTDDS_RTPS_ATTRIBUTES__SERVERATTRIBUTES_HPP
#define FASTDDS_RTPS_ATTRIBUTES__SERVERATTRIBUTES_HPP
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>

#include <algorithm>
#include <iterator>
#include <list>

namespace eprosima {
namespace fastdds {
namespace rtps {

class ParticipantProxyData;

/**
 * Class RemoteServerAttributes, to define the attributes of the Discovery Server Protocol.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */

class RemoteServerAttributes
{
public:

    inline bool operator ==(
            const RemoteServerAttributes& r) const
    {
        return guidPrefix == r.guidPrefix
               && metatrafficUnicastLocatorList == r.metatrafficUnicastLocatorList
               && metatrafficMulticastLocatorList == r.metatrafficMulticastLocatorList;
    }

    void clear()
    {
        guidPrefix = fastdds::rtps::GuidPrefix_t::unknown();
        metatrafficUnicastLocatorList.clear();
        metatrafficMulticastLocatorList.clear();
    }

    fastdds::rtps::GUID_t GetParticipant() const;

    fastdds::rtps::GUID_t GetPDPReader() const;
    fastdds::rtps::GUID_t GetPDPWriter() const;

    inline bool ReadguidPrefix(
            const char* pfx)
    {
        return bool(std::istringstream(pfx) >> guidPrefix);
    }

    //!Metatraffic Unicast Locator List
    LocatorList metatrafficUnicastLocatorList;
    //!Metatraffic Multicast Locator List.
    LocatorList metatrafficMulticastLocatorList;

    //!Guid prefix
    fastdds::rtps::GuidPrefix_t guidPrefix;

    // Check if there are specific transport locators associated
    // the template parameter is the locator kind (e.g. LOCATOR_KIND_UDPv4)
    template<int kind> bool requires_transport() const
    {
        return metatrafficUnicastLocatorList.has_kind<kind>() ||
               metatrafficMulticastLocatorList.has_kind<kind>();
    }

};

typedef std::list<RemoteServerAttributes> RemoteServerList_t;

template<class charT>
struct server_ostream_separators
{
    static const charT* list_separator;
    static const charT* locator_separator;
};

#ifndef _MSC_VER
template<> const char* server_ostream_separators<char>::list_separator;
template<> const wchar_t* server_ostream_separators<wchar_t>::list_separator;

template<> const char* server_ostream_separators<char>::locator_separator;
template<> const wchar_t* server_ostream_separators<wchar_t>::locator_separator;
#endif // _MSC_VER

template<class charT>
std::basic_ostream<charT>& operator <<(
        std::basic_ostream<charT>& output,
        const RemoteServerAttributes& sa)
{
    typename std::basic_ostream<charT>::sentry s(output);
    output << sa.guidPrefix;
    if (!sa.metatrafficUnicastLocatorList.empty())
    {
        output << server_ostream_separators<charT>::locator_separator << sa.metatrafficUnicastLocatorList;
    }
    if (!sa.metatrafficMulticastLocatorList.empty())
    {
        output << server_ostream_separators<charT>::locator_separator << sa.metatrafficMulticastLocatorList;
    }
    return output;
}

template<class charT>
std::basic_ostream<charT>& operator <<(
        std::basic_ostream<charT>& output,
        const RemoteServerList_t& list)
{
    typename std::basic_ostream<charT>::sentry s(output);
    std::ostream_iterator<RemoteServerAttributes> os_iterator(output, server_ostream_separators<charT>::list_separator);
    std::copy(list.begin(), list.end(), os_iterator);
    return output;
}

// Default server base guidPrefix
const char* const DEFAULT_ROS2_SERVER_GUIDPREFIX = "44.53.00.5f.45.50.52.4f.53.49.4d.41";

/* Environment variable that can either serve to:
 * - Specify the Discovery Server auto mode by setting its value to AUTO.
 * - Specify a semicolon-separated list of locators ([transport]ip:port) that define remote server
 *   locators. The [transport] specification is optional. The default transport is UDPv4.
 *   For the variable to take any effect, the following pre-condition must be met:
 *      - The discovery protocol must be either SIMPLE or SERVER.
 *         a. In the case of SIMPLE, the participant is created as a CLIENT instead.
 *         b. In the case of SERVER, the participant is created as a SERVER, using the DEFAULT_ROS2_MASTER_URI list to
 *            expand the list of remote servers.
 */
const char* const DEFAULT_ROS2_MASTER_URI = "ROS_DISCOVERY_SERVER";

/* Environment variable that:
 * - Will spawn a background Discovery Server in the current domain (if there were not).
 * - Specify an external ip address to connect the background Discovery Server (the port is deduced from the domain).
 * - Set the transports to TCP and SHM.
 * - Make the participant a SUPER_CLIENT.
 */
const char* const ROS2_EASY_MODE_URI = "ROS2_EASY_MODE";

/* Environment variable to transform a SIMPLE participant in a SUPER CLIENT.
 * If the participant is not SIMPLE, the variable doesn't have any effects.
 * The variable can assume the following values:
 *    - FALSE, false, False, 0
 *    - TRUE, true, True, 1
 * If the variable is not set, the program will behave like the variable is set to false.
 */
const char* const ROS_SUPER_CLIENT = "ROS_SUPER_CLIENT";

/**
 * Retrieves a semicolon-separated list of locators from a string, and
 * populates a LocatorList_t.
 * @param [in] list servers listening locator list.
 * @param [out] servers_list reference to a LocatorList_t to populate.
 * @return true if parsing succeeds, false otherwise (or if the list is empty)
 */
bool load_environment_server_info(
        const std::string& list,
        LocatorList& servers_list);

/**
 * Retrieves a semicolon-separated list of locators from DEFAULT_ROS2_MASTER_URI environment variable, and
 * populates a LocatorList_t.
 *
 * The environment variable can be read from an environment file (which allows runtime modification of the remote
 * servers list) or directly from the environment.
 * The value contained in the file takes precedence over the environment value (if both are set).
 *
 * @param [out] servers_list reference to a LocatorList_t to populate.
 * @return true if parsing succeeds, false otherwise
 */
bool load_environment_server_info(
        LocatorList& servers_list);

/**
 * Get the value of environment variable DEFAULT_ROS2_MASTER_URI
 * @return The value of environment variable DEFAULT_ROS2_MASTER_URI. Empty string if the variable is not defined.
 */
const std::string& ros_discovery_server_env();

/**
 * Get the value of environment variable ROS2_EASY_MODE_URI
 * @return The value of environment variable ROS2_EASY_MODE_URI.
 * Empty string if the variable is not defined or does not have a valid IPv4 format.
 */
const std::string& ros_easy_mode_env();

/**
 * Get the value of environment variable ROS_SUPER_CLIENT
 * @return The value of environment variable ROS_SUPER_CLIENT. False if the variable is not defined.
 */
bool ros_super_client_env();

} // rtps
} // fastdds
} // namespace eprosima

#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // FASTDDS_RTPS_ATTRIBUTES__SERVERATTRIBUTES_HPP
