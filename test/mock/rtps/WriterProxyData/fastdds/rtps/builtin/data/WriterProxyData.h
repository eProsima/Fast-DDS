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
#endif // if HAVE_SECURITY

#include <gmock/gmock.h>

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
        , topic_kind_(NO_KEY)
        , is_alive_(true)
        , type_id_()
        , type_()
        , type_info_()
        , m_typeMaxSerialized(0)
        , m_userDefinedId(0)
    {
        m_qos.m_userData.set_max_size(0);
        m_qos.m_partition.set_max_size(0);
    }

    WriterProxyData(
            size_t max_unicast_locators,
            size_t max_multicast_locators,
            const VariableLengthDataLimits& data_limits)
        : remote_locators_(max_unicast_locators, max_multicast_locators)
        , topic_kind_(NO_KEY)
        , is_alive_(true)
        , type_id_()
        , type_()
        , type_info_()
        , m_typeMaxSerialized(0)
        , m_userDefinedId(0)
    {
        m_qos.m_userData.set_max_size((uint32_t)data_limits.max_user_data);
        m_qos.m_partition.set_max_size((uint32_t)data_limits.max_partitions);
    }

    const GUID_t& guid() const
    {
        return m_guid;
    }

    GUID_t& guid()
    {
        return m_guid;
    }

    void guid (
            const GUID_t& guid)
    {
        m_guid = guid;
    }

    void clear()
    {
    }

    const GUID_t& persistence_guid() const
    {
        return m_guid;
    }

    void persistence_guid(
            const GUID_t& /*guid*/)
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
            bool /*use_multicast*/)
    {
    }

    void set_multicast_locators(
            const LocatorList_t& /*locators*/,
            const NetworkFactory& /*network*/)
    {

    }

    const RemoteLocatorList& remote_locators() const
    {
        return remote_locators_;
    }

    void typeName(
            const string_255& typeName)
    {
        type_name_ = typeName;
    }

    string_255 typeName() const
    {
        return type_name_;
    }

    void topicName(
            const string_255& topicName)
    {
        topic_name_ = topicName;
    }

    string_255 topicName() const
    {
        return topic_name_;
    }

    void topicKind(
            TopicKind_t topicKind)
    {
        topic_kind_ = topicKind;
    }

    TopicKind_t topicKind() const
    {
        return topic_kind_;
    }

    void isAlive (
            bool alive)
    {
        is_alive_ = alive;
    }

    bool isAlive () const
    {
        return is_alive_;
    }

    bool has_type_id () const
    {
        return false;
    }

    void type_id(
            const TypeIdV1& other_type_id)
    {
        type_id_ = other_type_id;
    }

    const TypeIdV1& type_id() const
    {
        return type_id_;
    }

    TypeIdV1& type_id()
    {
        return type_id_;
    }

    bool has_type () const
    {
        return false;
    }

    void type(
            const TypeObjectV1& other_type)
    {
        type_ = other_type;
    }

    const TypeObjectV1& type() const
    {
        return type_;
    }

    TypeObjectV1& type()
    {
        return type_;
    }

    bool has_type_information () const
    {
        return false;
    }

    void type_information(
            const xtypes::TypeInformation& other_type_info)
    {
        type_info_ = other_type_info;
    }

    const xtypes::TypeInformation& type_information() const
    {
        return type_info_;
    }

    xtypes::TypeInformation& type_information()
    {
        return type_info_;
    }

    void typeMaxSerialized(
            uint32_t typeMaxSerialized)
    {
        m_typeMaxSerialized = typeMaxSerialized;
    }

    uint32_t typeMaxSerialized() const
    {
        return m_typeMaxSerialized;
    }

    uint32_t& typeMaxSerialized()
    {
        return m_typeMaxSerialized;
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

    void RTPSParticipantKey(
            const InstanceHandle_t& RTPSParticipantKey)
    {
        m_RTPSParticipantKey = RTPSParticipantKey;
    }

    void RTPSParticipantKey(
            InstanceHandle_t&& RTPSParticipantKey)
    {
        m_RTPSParticipantKey = std::move(RTPSParticipantKey);
    }

    InstanceHandle_t RTPSParticipantKey() const
    {
        return m_RTPSParticipantKey;
    }

    InstanceHandle_t& RTPSParticipantKey()
    {
        return m_RTPSParticipantKey;
    }

    void set_locators(
            const RemoteLocatorList& /*locators*/)
    {

    }

    void userDefinedId(
            uint16_t userDefinedId)
    {
        m_userDefinedId = userDefinedId;
    }

    uint16_t userDefinedId() const
    {
        return m_userDefinedId;
    }

    uint16_t& userDefinedId()
    {
        return m_userDefinedId;
    }

#if HAVE_SECURITY
    security::EndpointSecurityAttributesMask security_attributes_ = 0UL;
    security::PluginEndpointSecurityAttributesMask plugin_security_attributes_ = 0UL;
#endif // if HAVE_SECURITY

    WriterQos m_qos;

private:

    GUID_t m_guid;
    RemoteLocatorList remote_locators_;
    std::string topic_name_;
    std::string type_name_;
    TopicKind_t topic_kind_;
    bool is_alive_;
    TypeIdV1 type_id_;
    TypeObjectV1 type_;
    xtypes::TypeInformation type_info_;
    uint32_t m_typeMaxSerialized;
    InstanceHandle_t m_key;
    InstanceHandle_t m_RTPSParticipantKey;
    uint16_t m_userDefinedId;
};

} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // _FASTDDS_RTPS_BUILTIN_DATA_WRITERPROXYDATA_H_

