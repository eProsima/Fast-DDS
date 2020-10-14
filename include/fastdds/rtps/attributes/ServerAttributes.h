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

// To be eventually removed together with eprosima::fastrtps
namespace aux = ::eprosima::fastrtps::rtps;

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
        guidPrefix = aux::GuidPrefix_t::unknown();
        metatrafficUnicastLocatorList.clear();
        metatrafficMulticastLocatorList.clear();
        proxy = nullptr;
    }

    RTPS_DllAPI aux::GUID_t GetParticipant() const;

    RTPS_DllAPI aux::GUID_t GetPDPReader() const;
    RTPS_DllAPI aux::GUID_t GetPDPWriter() const;

    RTPS_DllAPI aux::GUID_t GetEDPPublicationsReader() const;
    RTPS_DllAPI aux::GUID_t GetEDPSubscriptionsWriter() const;

    RTPS_DllAPI aux::GUID_t GetEDPPublicationsWriter() const;
    RTPS_DllAPI aux::GUID_t GetEDPSubscriptionsReader() const;

    RTPS_DllAPI inline bool ReadguidPrefix(
            const char* pfx)
    {
        return bool(std::istringstream(pfx) >> guidPrefix);
    }

    //!Metatraffic Unicast Locator List
    aux::LocatorList_t metatrafficUnicastLocatorList;
    //!Metatraffic Multicast Locator List.
    aux::LocatorList_t metatrafficMulticastLocatorList;

    //!Guid prefix
    aux::GuidPrefix_t guidPrefix;

    // Live participant proxy reference
    const aux::ParticipantProxyData* proxy{};
};

typedef std::list<RemoteServerAttributes> RemoteServerList_t;

// port use if the ros environment variable doesn't specified one
constexpr uint16_t DEFAULT_ROS2_SERVER_PORT = 11811;
// default server base guidPrefix
const char* const DEFAULT_ROS2_SERVER_GUIDPREFIX = "44.49.53.43.53.45.52.56.45.52.5F.30";

/**
 * Retrieves a ; separated list of locators from an environment variable and
 * populates a RemoteServerList_t mapping list position to default guid.
 * @param list servers listening locator list provided.
 * @param attributes referenct to a RemoteServerList_t to populate.
 * @return true if parsing succeeds
 */
RTPS_DllAPI bool load_environment_server_info(
        std::string list,
        RemoteServerList_t& attributes);
/**
 * returns the guidPrefix associated to the default server id given
 * @param id of the default server whose guidPrefix we want to retrieve
 * @param guid reference to the guidPrefix to modify
 * @return true if the server guid can be delivered
 */
RTPS_DllAPI bool get_server_client_default_guidPrefix(
        int id,
        aux::GuidPrefix_t& guid);

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_SERVERATTRIBUTES_H_ */
