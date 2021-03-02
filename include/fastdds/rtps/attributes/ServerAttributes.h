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

/**
 * @file ServerAttributes.h
 *
 */

#ifndef _FASTDDS_SERVERATTRIBUTES_H_
#define _FASTDDS_SERVERATTRIBUTES_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/Locator.h>

#include <list>

namespace eprosima {

namespace fastrtps {
namespace rtps {
class ParticipantProxyData;
} // fastrtps
} // rtps

namespace fastdds {
namespace rtps {

/**
 * Class RemoteServerAttributes, to define the attributes of the Discovery Server Protocol.
 * @ingroup RTPS_ATTRIBUTES_MODULE
 */

class RemoteServerAttributes
{
public:

    RTPS_DllAPI inline bool operator ==(
            const RemoteServerAttributes& r) const
    {
        return guidPrefix == r.guidPrefix
               && metatrafficUnicastLocatorList == r.metatrafficUnicastLocatorList
               && metatrafficMulticastLocatorList == r.metatrafficMulticastLocatorList;
        //     && proxy == r.proxy;
    }

    RTPS_DllAPI void clear()
    {
        guidPrefix = fastrtps::rtps::GuidPrefix_t::unknown();
        metatrafficUnicastLocatorList.clear();
        metatrafficMulticastLocatorList.clear();
        proxy = nullptr;
    }

    RTPS_DllAPI fastrtps::rtps::GUID_t GetParticipant() const;

    RTPS_DllAPI fastrtps::rtps::GUID_t GetPDPReader() const;
    RTPS_DllAPI fastrtps::rtps::GUID_t GetPDPWriter() const;


    FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastrtps::rtps:GetEDPPublicationsReader()",
            "Not implemented nor used functions.")
    RTPS_DllAPI fastrtps::rtps::GUID_t GetEDPPublicationsReader() const;
    FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastrtps::rtps:GetEDPSubscriptionsWriter()",
            "Not implemented nor used functions.")
    RTPS_DllAPI fastrtps::rtps::GUID_t GetEDPSubscriptionsWriter() const;

    FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastrtps::rtps:GetEDPPublicationsWriter()",
            "Not implemented nor used functions.")
    RTPS_DllAPI fastrtps::rtps::GUID_t GetEDPPublicationsWriter() const;
    FASTDDS_DEPRECATED_UNTIL(3, "eprosima::fastrtps::rtps:GetEDPSubscriptionsReader()",
            "Not implemented nor used functions.")
    RTPS_DllAPI fastrtps::rtps::GUID_t GetEDPSubscriptionsReader() const;

    RTPS_DllAPI inline bool ReadguidPrefix(
            const char* pfx)
    {
        return bool(std::istringstream(pfx) >> guidPrefix);
    }

    //!Metatraffic Unicast Locator List
    LocatorList metatrafficUnicastLocatorList;
    //!Metatraffic Multicast Locator List.
    LocatorList metatrafficMulticastLocatorList;

    //!Guid prefix
    fastrtps::rtps::GuidPrefix_t guidPrefix;

    // Live participant proxy reference
    const fastrtps::rtps::ParticipantProxyData* proxy{};
};

typedef std::list<RemoteServerAttributes> RemoteServerList_t;

// port use if the ros environment variable doesn't specified one
constexpr uint16_t DEFAULT_ROS2_SERVER_PORT = 11811;
// default server base guidPrefix
const char* const DEFAULT_ROS2_SERVER_GUIDPREFIX = "44.53.00.5f.45.50.52.4f.53.49.4d.41";

/* Environment variable to specify a semicolon-separated list of UDPv4 locators (ip:port) that define remote server
 * locators.
 * The position in the list is used as a "server id" to extrapolate the server's GUID prefix.
 * For the variable to take any effect, the following pre-conditions must be met:
 *    1. The server's GUID prefix must be compliant with the schema
 *       "44.53.<server_id_in_hex>.5f.45.50.52.4f.53.49.4d.41", which is the schema followed by the prefixes generated
 *        when creating server using fastdds cli, being DEFAULT_ROS2_SERVER_GUIDPREFIX the prefix for ID=0.
 *    1. The discovery protocol must be either SIMPLE or SERVER.
 *       1. In the case of SIMPLE, the participant is created as a CLIENT instead.
 *       1. In the case of SERVER, the participant is created as a SERVER, using the DEFAULT_ROS2_MASTER_URI list to
 *          expand the list of remote servers.
 */
const char* const DEFAULT_ROS2_MASTER_URI = "ROS_DISCOVERY_SERVER";

/**
 * Retrieves a semicolon-separated list of locators from a string, and
 * populates a RemoteServerList_t mapping list position to default guid.
 * @param[in] list servers listening locator list.
 * @param[out] attributes reference to a RemoteServerList_t to populate.
 * @return true if parsing succeeds, false otherwise (or if the list is empty)
 */
RTPS_DllAPI bool load_environment_server_info(
        std::string list,
        RemoteServerList_t& attributes);

/**
 * Retrieves a semicolon-separated list of locators from DEFAULT_ROS2_MASTER_URI environment variable, and
 * populates a RemoteServerList_t mapping list position to default guid.
 * @param[out] attributes reference to a RemoteServerList_t to populate.
 * @return true if parsing succeeds, false otherwise
 */
RTPS_DllAPI bool load_environment_server_info(
        RemoteServerList_t& attributes);

/**
 * Get the value of environment variable DEFAULT_ROS2_MASTER_URI
 * @return The value of environment variable DEFAULT_ROS2_MASTER_URI. Empty string if the variable is not defined.
 */
RTPS_DllAPI const std::string& ros_discovery_server_env();

/**
 * Returns the guidPrefix associated to the given server id
 * @param[in] id of the default server whose guidPrefix we want to retrieve
 * @param[out] guid reference to the guidPrefix to modify
 * @return true if the server guid can be delivered
 */
RTPS_DllAPI bool get_server_client_default_guidPrefix(
        int id,
        fastrtps::rtps::GuidPrefix_t& guid);

} // namespace rtps
} // namespace fastdds

// keep former namespace references available
namespace fastrtps {
namespace rtps {

using fastdds::rtps::RemoteServerAttributes;
using fastdds::rtps::RemoteServerList_t;
using fastdds::rtps::DEFAULT_ROS2_SERVER_PORT;
using fastdds::rtps::DEFAULT_ROS2_SERVER_GUIDPREFIX;
using fastdds::rtps::DEFAULT_ROS2_MASTER_URI;
using fastdds::rtps::load_environment_server_info;
using fastdds::rtps::ros_discovery_server_env;
using fastdds::rtps::get_server_client_default_guidPrefix;

} // fastrtps
} // rtps

} // namespace eprosima

#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_SERVERATTRIBUTES_H_ */
