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
 * @file WriterProxyData.h
 */

#ifndef _FASTDDS_RTPS_BUILTIN_DATA_WRITERPROXYDATA_H_
#define _FASTDDS_RTPS_BUILTIN_DATA_WRITERPROXYDATA_H_

#include <fastrtps/rtps/common/Guid.h>
#include <fastrtps/rtps/common/RemoteLocators.hpp>
#include <fastrtps/qos/WriterQos.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>

#if HAVE_SECURITY
#include <fastrtps/rtps/security/accesscontrol/EndpointSecurityAttributes.h>
#endif

namespace eprosima {
namespace fastrtps {
namespace rtps {

class NetworkFactory;

class WriterProxyData
{
    public:

        WriterProxyData(
                size_t max_unicast_locators,
                size_t max_multicast_locators)
            : remote_locators_(max_unicast_locators, max_multicast_locators)
        {
            m_qos.m_userData.set_max_size(0);
            m_qos.m_partition.set_max_size(0);
        }

        WriterProxyData(
                size_t max_unicast_locators,
                size_t max_multicast_locators,
                const VariableLengthDataLimits& data_limits)
            : remote_locators_(max_unicast_locators, max_multicast_locators)
        {
            m_qos.m_userData.set_max_size((uint32_t)data_limits.max_user_data);
            m_qos.m_partition.set_max_size((uint32_t)data_limits.max_partitions);
        }

        const GUID_t& guid() const { return m_guid; }

        GUID_t& guid() { return m_guid; }

        void guid (const GUID_t& guid) { m_guid = guid; }

        void clear() { }

        void persistence_guid(const GUID_t& /*guid*/) { }

        void set_announced_unicast_locators(const LocatorList_t& /*locators*/) { }

        void set_remote_unicast_locators(const LocatorList_t& /*locators*/, const NetworkFactory& /*network*/) { }

        void set_remote_locators(
                const RemoteLocatorList& /*locators*/,
                const NetworkFactory& /*network*/,
                bool /*use_multicast*/) { }

        void topicKind (int /*kind*/) { }

        const RemoteLocatorList& remote_locators() const
        {
            return remote_locators_;
        }

#if HAVE_SECURITY
        security::EndpointSecurityAttributesMask security_attributes_ = 0UL;
        security::PluginEndpointSecurityAttributesMask plugin_security_attributes_ = 0UL;
#endif

        WriterQos m_qos;

    private:

        GUID_t m_guid;
        RemoteLocatorList remote_locators_;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_BUILTIN_DATA_WRITERPROXYDATA_H_

