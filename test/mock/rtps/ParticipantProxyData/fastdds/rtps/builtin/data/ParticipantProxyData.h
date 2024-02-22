// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file ParticipantProxyData.h
 */
#ifndef _FASTDDS_RTPS_BUILTIN_DATA_PARTICIPANTPROXYDATA_H_
#define _FASTDDS_RTPS_BUILTIN_DATA_PARTICIPANTPROXYDATA_H_

#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/rtps/common/Token.h>

#if HAVE_SECURITY
#include <fastdds/rtps/security/accesscontrol/ParticipantSecurityAttributes.h>
#endif // if HAVE_SECURITY

namespace eprosima {
namespace fastrtps {
namespace rtps {

class ParticipantProxyData
{
public:

    ParticipantProxyData(
            const RTPSParticipantAllocationAttributes& allocation = c_default_RTPSParticipantAllocationAttributes)
        : m_availableBuiltinEndpoints(0)
        , metatraffic_locators(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
        , default_locators(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
        , m_VendorId(c_VendorId_Unknown)
    {
    }

    ~ParticipantProxyData()
    {
    }

    bool writeToCDRMessage(
            CDRMessage_t* /*msg*/,
            bool /*write_encapsulation*/)
    {
        return true;
    }

    bool readFromCDRMessage(
            CDRMessage_t* /*msg*/)
    {
        return true;
    }

    GUID_t m_guid;
    uint32_t m_availableBuiltinEndpoints;
    RemoteLocatorList metatraffic_locators;
    RemoteLocatorList default_locators;
    VendorId_t m_VendorId;
#if HAVE_SECURITY
    IdentityToken identity_token_;
    PermissionsToken permissions_token_;
    security::ParticipantSecurityAttributesMask security_attributes_ = 0UL;
    security::PluginParticipantSecurityAttributesMask plugin_security_attributes_ = 0UL;
#endif // if HAVE_SECURITY
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_BUILTIN_DATA_PARTICIPANTPROXYDATA_H_
