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
namespace fastrtps{
namespace rtps {

    class ParticipantProxyData;

    /**
     * Class RemoteServerAttributes, to define the attributes of the Discovery Server Protocol.
     * @ingroup RTPS_ATTRIBUTES_MODULE
     */

    class RemoteServerAttributes
    {
    public:
        RTPS_DllAPI inline bool operator==(const RemoteServerAttributes& r) const
        { return guidPrefix == r.guidPrefix; }

        RTPS_DllAPI GUID_t GetParticipant() const;

        RTPS_DllAPI GUID_t GetPDPReader() const;
        RTPS_DllAPI GUID_t GetPDPWriter() const;

        RTPS_DllAPI GUID_t GetEDPPublicationsReader() const;
        RTPS_DllAPI GUID_t GetEDPSubscriptionsWriter() const;

        RTPS_DllAPI GUID_t GetEDPPublicationsWriter() const;
        RTPS_DllAPI GUID_t GetEDPSubscriptionsReader() const;

        RTPS_DllAPI inline bool ReadguidPrefix(const char * pfx)
        {
            return bool(std::istringstream(pfx) >> guidPrefix);
        }

        //!Metatraffic Unicast Locator List
        LocatorList_t metatrafficUnicastLocatorList;
        //!Metatraffic Multicast Locator List.
        LocatorList_t metatrafficMulticastLocatorList;

        //!Guid prefix
        GuidPrefix_t guidPrefix;

        // Live participant proxy reference
        const ParticipantProxyData * proxy{};
    };

    typedef std::list<RemoteServerAttributes> RemoteServerList_t;

}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif /* _FASTDDS_SERVERATTRIBUTES_H_ */
