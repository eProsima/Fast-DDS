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
 * @file ReaderProxyData.hpp
 */

#ifndef RTPS_BUILTIN_DATA__READERPROXYDATA_HPP
#define RTPS_BUILTIN_DATA__READERPROXYDATA_HPP

#include <gmock/gmock.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>
#if HAVE_SECURITY
#include <fastdds/rtps/attributes/EndpointSecurityAttributes.hpp>
#endif // if HAVE_SECURITY

namespace eprosima {
namespace fastdds {
namespace rtps {

class NetworkFactory;

class ReaderProxyData : public SubscriptionBuiltinTopicData
{
public:

    ReaderProxyData (
            const size_t max_unicast_locators,
            const size_t max_multicast_locators,
            const VariableLengthDataLimits& data_limits,
            const fastdds::rtps::ContentFilterProperty::AllocationConfiguration& content_filterlimits = {})
        : SubscriptionBuiltinTopicData(max_unicast_locators, max_multicast_locators, data_limits, content_filterlimits)
#if HAVE_SECURITY
        , security_attributes_(0UL)
        , plugin_security_attributes_(0UL)
#endif // if HAVE_SECURITY
        , m_is_alive(true)
        , m_user_defined_id(0)
    {

    }

    ReaderProxyData (
            const size_t max_unicast_locators,
            const size_t max_multicast_locators,
            const fastdds::rtps::ContentFilterProperty::AllocationConfiguration& content_filterlimits = {})
        : ReaderProxyData(max_unicast_locators, max_multicast_locators, VariableLengthDataLimits(),
                content_filterlimits)
    {

    }

    ReaderProxyData(
            const VariableLengthDataLimits&,
            const SubscriptionBuiltinTopicData& subscription_data)
        : SubscriptionBuiltinTopicData(subscription_data)
    {

    }

    bool disable_positive_acks_enabled() const
    {
        return false;
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

    bool has_locators() const
    {
        return !remote_locators.unicast.empty() || !remote_locators.multicast.empty();
    }

    void clear ()
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

    bool read_from_cdr_message(
            CDRMessage_t* /*msg*/,
            fastdds::rtps::VendorId_t /*source_vendor_id*/)
    {
        return true;
    }

    void set_multicast_locators(
            const LocatorList_t& /*locator*/,
            const NetworkFactory& /*network*/,
            bool /*from_this_host*/)
    {
    }

    void is_alive (
            bool alive)
    {
        m_is_alive = alive;
    }

    bool is_alive () const
    {
        return m_is_alive;
    }

    bool has_type_information () const
    {
        return type_information.assigned();
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
            const InstanceHandle_t& rtps_participant_key)
    {
        m_rtps_participant_key = rtps_participant_key;
    }

    void rtps_participant_key(
            InstanceHandle_t&& rtps_participant_key)
    {
        m_rtps_participant_key = std::move(rtps_participant_key);
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
            const SubscriptionBuiltinTopicData&,
            bool)
    {
    }

    void set_qos(
            const dds::ReaderQos&,
            bool)
    {
    }

private:

    void init(
            const VariableLengthDataLimits&)
    {
    }

    bool can_qos_be_updated(
            const SubscriptionBuiltinTopicData&) const
    {
        return true;
    }

    bool m_is_alive;
    InstanceHandle_t m_key;
    InstanceHandle_t m_rtps_participant_key;
    uint16_t m_user_defined_id;

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // RTPS_BUILTIN_DATA__READERPROXYDATA_HPP
