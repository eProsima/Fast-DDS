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
 * @file WriterProxyData.hpp
 */

#ifndef FASTDDS_RTPS_BUILTIN_DATA__WRITERPROXYDATA_HPP
#define FASTDDS_RTPS_BUILTIN_DATA__WRITERPROXYDATA_HPP

#include <gmock/gmock.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>

#if HAVE_SECURITY
#include <fastdds/rtps/attributes/EndpointSecurityAttributes.hpp>
#endif // if HAVE_SECURITY

namespace eprosima {
namespace fastdds {
namespace rtps {

class NetworkFactory;

class WriterProxyData : public PublicationBuiltinTopicData
{
public:

    WriterProxyData(
            const size_t max_unicast_locators,
            const size_t max_multicast_locators,
            const VariableLengthDataLimits& data_limits)
        : PublicationBuiltinTopicData(max_unicast_locators, max_multicast_locators, data_limits)
#if HAVE_SECURITY
        , security_attributes_(0)
        , plugin_security_attributes_(0)
#endif // if HAVE_SECURITY
        , m_user_defined_id(0)
    {

    }

    WriterProxyData(
            const size_t max_unicast_locators,
            const size_t max_multicast_locators)
        : WriterProxyData(max_unicast_locators, max_multicast_locators, VariableLengthDataLimits())
    {
    }

    WriterProxyData(
            const VariableLengthDataLimits&,
            const PublicationBuiltinTopicData& publication_data)
        : PublicationBuiltinTopicData(publication_data)
    {
    }

    void clear()
    {
    }

    void set_announced_unicast_locators(
            const LocatorList_t& /*locators*/)
    {
    }

    void set_remote_unicast_locators(
            const LocatorList_t& /*locators*/,
            const NetworkFactory& /*network*/)
    {
    }

    void set_remote_locators(
            const RemoteLocatorList& /*locators*/,
            const NetworkFactory& /*network*/,
            bool /*use_multicast*/,
            bool /*from_this_host*/)
    {
    }

    void add_unicast_locator(
            const Locator_t& locator)
    {
        remote_locators.add_unicast_locator(locator);
    }

    void add_multicast_locator(
            const Locator_t& locator)
    {
        remote_locators.add_multicast_locator(locator);
    }

    void set_multicast_locators(
            const LocatorList_t& /*locators*/,
            const NetworkFactory& /*network*/,
            bool /*from_this_host*/)
    {

    }

    bool has_locators() const
    {
        return !remote_locators.unicast.empty() || !remote_locators.multicast.empty();
    }

    bool has_type_information () const
    {
        return type_information.assigned();
    }

    void type_max_serialized(
            uint32_t typeMaxSerialized)
    {
        max_serialized_size = typeMaxSerialized;
    }

    uint32_t type_max_serialized() const
    {
        return max_serialized_size;
    }

    uint32_t& type_max_serialized()
    {
        return max_serialized_size;
    }

    void key(
            const InstanceHandle_t& key)
    {
        m_key = key;
    }

    void key(
            InstanceHandle_t&& key)
    {
        m_key = std::move(key);
    }

    InstanceHandle_t key() const
    {
        return m_key;
    }

    InstanceHandle_t& key()
    {
        return m_key;
    }

    void rtps_participant_key(
            const InstanceHandle_t& RTPSParticipantKey)
    {
        m_rtps_participant_key = RTPSParticipantKey;
    }

    void rtps_participant_key(
            InstanceHandle_t&& RTPSParticipantKey)
    {
        m_rtps_participant_key = std::move(RTPSParticipantKey);
    }

    InstanceHandle_t rtps_participant_key() const
    {
        return m_rtps_participant_key;
    }

    InstanceHandle_t& rtps_participant_key()
    {
        return m_rtps_participant_key;
    }

    void set_locators(
            const RemoteLocatorList& /*locators*/)
    {

    }

    void user_defined_id(
            uint16_t user_defined_id)
    {
        m_user_defined_id = user_defined_id;
    }

    uint16_t user_defined_id() const
    {
        return m_user_defined_id;
    }

    uint16_t& user_defined_id()
    {
        return m_user_defined_id;
    }

#if HAVE_SECURITY
    security::EndpointSecurityAttributesMask security_attributes_ = 0UL;
    security::PluginEndpointSecurityAttributesMask plugin_security_attributes_ = 0UL;
#endif // if HAVE_SECURITY

    void set_qos(
            const PublicationBuiltinTopicData&,
            bool)
    {
    }

    void set_qos(
            const dds::WriterQos&,
            bool)
    {
    }

private:

    void init(
            const VariableLengthDataLimits&)
    {
    }

    bool can_qos_be_updated(
            const PublicationBuiltinTopicData&) const
    {
        return true;
    }

    InstanceHandle_t m_key;
    InstanceHandle_t m_rtps_participant_key;
    uint16_t m_user_defined_id;
};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_BUILTIN_DATA__WRITERPROXYDATA_HPP

