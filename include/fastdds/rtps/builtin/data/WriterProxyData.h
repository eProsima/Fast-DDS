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
 *
 */

#ifndef _FASTDDS_RTPS_BUILTIN_DATA_WRITERPROXYDATA_H_
#define _FASTDDS_RTPS_BUILTIN_DATA_WRITERPROXYDATA_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastrtps/qos/WriterQos.h>
#include <fastrtps/attributes/TopicAttributes.h>

#include <fastrtps/utils/fixed_size_string.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>

#if HAVE_SECURITY
#include <fastdds/rtps/security/accesscontrol/EndpointSecurityAttributes.h>
#endif // if HAVE_SECURITY

#include <fastdds/rtps/common/RemoteLocators.hpp>

namespace eprosima {
namespace fastrtps {
namespace rtps {

struct CDRMessage_t;
class NetworkFactory;
class ParticipantProxyData;

/**
 **@ingroup BUILTIN_MODULE
 */
class WriterProxyData
{
public:

    RTPS_DllAPI WriterProxyData(
            const size_t max_unicast_locators,
            const size_t max_multicast_locators);

    RTPS_DllAPI WriterProxyData(
            const size_t max_unicast_locators,
            const size_t max_multicast_locators,
            const VariableLengthDataLimits& data_limits);

    virtual RTPS_DllAPI ~WriterProxyData();

    RTPS_DllAPI WriterProxyData(
            const WriterProxyData& writerInfo);

    RTPS_DllAPI WriterProxyData& operator =(
            const WriterProxyData& writerInfo);

    RTPS_DllAPI void guid(
            const GUID_t& guid)
    {
        m_guid = guid;
    }

    RTPS_DllAPI void guid(
            GUID_t&& guid)
    {
        m_guid = std::move(guid);
    }

    RTPS_DllAPI const GUID_t& guid() const
    {
        return m_guid;
    }

    RTPS_DllAPI GUID_t& guid()
    {
        return m_guid;
    }

    RTPS_DllAPI void persistence_guid(
            const GUID_t& guid)
    {
        persistence_guid_ = guid;
    }

    RTPS_DllAPI void persistence_guid(
            GUID_t&& guid)
    {
        persistence_guid_ = std::move(guid);
    }

    RTPS_DllAPI GUID_t persistence_guid() const
    {
        return persistence_guid_;
    }

    RTPS_DllAPI GUID_t& persistence_guid()
    {
        return persistence_guid_;
    }

    RTPS_DllAPI void set_persistence_entity_id(
            const EntityId_t& nid)
    {
        persistence_guid_.entityId = persistence_guid_.guidPrefix != c_GuidPrefix_Unknown ? nid : c_EntityId_Unknown;
    }

    RTPS_DllAPI bool has_locators() const
    {
        return !remote_locators_.unicast.empty() || !remote_locators_.multicast.empty();
    }

    RTPS_DllAPI const RemoteLocatorList& remote_locators() const
    {
        return remote_locators_;
    }

    RTPS_DllAPI void add_unicast_locator(
            const Locator_t& locator);

    void set_announced_unicast_locators(
            const LocatorList_t& locators);

    void set_remote_unicast_locators(
            const LocatorList_t& locators,
            const NetworkFactory& network);

    RTPS_DllAPI void add_multicast_locator(
            const Locator_t& locator);

    void set_multicast_locators(
            const LocatorList_t& locators,
            const NetworkFactory& network);

    void set_locators(
            const RemoteLocatorList& locators);

    void set_remote_locators(
            const RemoteLocatorList& remote_locators,
            const NetworkFactory& network,
            bool use_multicast_locators);

    RTPS_DllAPI void key(
            const InstanceHandle_t& key)
    {
        m_key = key;
    }

    RTPS_DllAPI void key(
            InstanceHandle_t&& key)
    {
        m_key = std::move(key);
    }

    RTPS_DllAPI InstanceHandle_t key() const
    {
        return m_key;
    }

    RTPS_DllAPI InstanceHandle_t& key()
    {
        return m_key;
    }

    RTPS_DllAPI void RTPSParticipantKey(
            const InstanceHandle_t& RTPSParticipantKey)
    {
        m_RTPSParticipantKey = RTPSParticipantKey;
    }

    RTPS_DllAPI void RTPSParticipantKey(
            InstanceHandle_t&& RTPSParticipantKey)
    {
        m_RTPSParticipantKey = std::move(RTPSParticipantKey);
    }

    RTPS_DllAPI InstanceHandle_t RTPSParticipantKey() const
    {
        return m_RTPSParticipantKey;
    }

    RTPS_DllAPI InstanceHandle_t& RTPSParticipantKey()
    {
        return m_RTPSParticipantKey;
    }

    RTPS_DllAPI void typeName(
            const string_255& typeName)
    {
        m_typeName = typeName;
    }

    RTPS_DllAPI void typeName(
            string_255&& typeName)
    {
        m_typeName = std::move(typeName);
    }

    RTPS_DllAPI const string_255& typeName() const
    {
        return m_typeName;
    }

    RTPS_DllAPI string_255& typeName()
    {
        return m_typeName;
    }

    RTPS_DllAPI void topicName(
            const string_255& topicName)
    {
        m_topicName = topicName;
    }

    RTPS_DllAPI void topicName(
            string_255&& topicName)
    {
        m_topicName = std::move(topicName);
    }

    RTPS_DllAPI const string_255& topicName() const
    {
        return m_topicName;
    }

    RTPS_DllAPI string_255& topicName()
    {
        return m_topicName;
    }

    RTPS_DllAPI void userDefinedId(
            uint16_t userDefinedId)
    {
        m_userDefinedId = userDefinedId;
    }

    RTPS_DllAPI uint16_t userDefinedId() const
    {
        return m_userDefinedId;
    }

    RTPS_DllAPI uint16_t& userDefinedId()
    {
        return m_userDefinedId;
    }

    RTPS_DllAPI void typeMaxSerialized(
            uint32_t typeMaxSerialized)
    {
        m_typeMaxSerialized = typeMaxSerialized;
    }

    RTPS_DllAPI uint32_t typeMaxSerialized() const
    {
        return m_typeMaxSerialized;
    }

    RTPS_DllAPI uint32_t& typeMaxSerialized()
    {
        return m_typeMaxSerialized;
    }

    RTPS_DllAPI void topicKind(
            TopicKind_t topicKind)
    {
        m_topicKind = topicKind;
    }

    RTPS_DllAPI TopicKind_t topicKind() const
    {
        return m_topicKind;
    }

    RTPS_DllAPI TopicKind_t& topicKind()
    {
        return m_topicKind;
    }

    RTPS_DllAPI void type_id(
            const TypeIdV1& other_type_id)
    {
        type_id() = other_type_id;
    }

    RTPS_DllAPI const TypeIdV1& type_id() const
    {
        assert(m_type_id != nullptr);
        return *m_type_id;
    }

    RTPS_DllAPI TypeIdV1& type_id()
    {
        if (m_type_id == nullptr)
        {
            m_type_id = new TypeIdV1();
        }
        return *m_type_id;
    }

    RTPS_DllAPI bool has_type_id() const
    {
        return m_type_id != nullptr;
    }

    RTPS_DllAPI void type(
            const TypeObjectV1& other_type)
    {
        type() = other_type;
    }

    RTPS_DllAPI const TypeObjectV1& type() const
    {
        assert(m_type != nullptr);
        return *m_type;
    }

    RTPS_DllAPI TypeObjectV1& type()
    {
        if (m_type == nullptr)
        {
            m_type = new TypeObjectV1();
        }
        return *m_type;
    }

    RTPS_DllAPI bool has_type() const
    {
        return m_type != nullptr;
    }

    RTPS_DllAPI void type_information(
            const xtypes::TypeInformation& other_type_information)
    {
        type_information() = other_type_information;
    }

    RTPS_DllAPI const xtypes::TypeInformation& type_information() const
    {
        assert(m_type_information != nullptr);
        return *m_type_information;
    }

    RTPS_DllAPI xtypes::TypeInformation& type_information()
    {
        if (m_type_information == nullptr)
        {
            m_type_information = new xtypes::TypeInformation();
        }
        return *m_type_information;
    }

    RTPS_DllAPI bool has_type_information() const
    {
        return m_type_information != nullptr;
    }

    //!WriterQOS
    WriterQos m_qos;

    /**
     * Set participant client server sample identity
     * @param sid valid SampleIdentity
     */
    void set_sample_identity(
            const SampleIdentity& sid)
    {
        fastdds::dds::set_proxy_property(sid, "PID_CLIENT_SERVER_KEY", m_properties);
    }

    /**
     * Retrieve participant SampleIdentity
     * @return SampleIdentity
     */
    SampleIdentity get_sample_identity() const
    {
        return fastdds::dds::get_proxy_property<SampleIdentity>("PID_CLIENT_SERVER_KEY", m_properties);
    }

#if HAVE_SECURITY
    //!EndpointSecurityInfo.endpoint_security_attributes
    security::EndpointSecurityAttributesMask security_attributes_;

    //!EndpointSecurityInfo.plugin_endpoint_security_attributes
    security::PluginEndpointSecurityAttributesMask plugin_security_attributes_;
#endif // if HAVE_SECURITY

    //!Clear the information and return the object to the default state.
    void clear();

    /**
     * Check if this object can be updated with the information on another object.
     * @param wdata WriterProxyData object to be checked.
     * @return true if this object can be updated with the information on wdata.
     */
    bool is_update_allowed(
            const WriterProxyData& wdata) const;

    /**
     * Update certain parameters from another object.
     * @param wdata pointer to object with new information.
     */
    void update(
            WriterProxyData* wdata);

    //!Copy all information from another object.
    void copy(
            WriterProxyData* wdata);

    /**
     * Get the size in bytes of the CDR serialization of this object.
     * @param include_encapsulation Whether to include the size of the encapsulation info.
     * @return size in bytes of the CDR serialization.
     */
    uint32_t get_serialized_size(
            bool include_encapsulation) const;

    //!Write as a parameter list on a CDRMessage_t
    bool writeToCDRMessage(
            CDRMessage_t* msg,
            bool write_encapsulation) const;

    //!Read a parameter list from a CDRMessage_t.
    RTPS_DllAPI bool readFromCDRMessage(
            CDRMessage_t* msg,
            const NetworkFactory& network,
            bool is_shm_transport_possible);

private:

    //!GUID
    GUID_t m_guid;

    //!Holds locator information
    RemoteLocatorList remote_locators_;

    //!GUID_t of the Writer converted to InstanceHandle_t
    InstanceHandle_t m_key;

    //!GUID_t of the participant converted to InstanceHandle
    InstanceHandle_t m_RTPSParticipantKey;

    //!Type name
    string_255 m_typeName;

    //!Topic name
    string_255 m_topicName;

    //!User defined ID
    uint16_t m_userDefinedId;

    //!Maximum size of the type associated with this Wrtiter, serialized.
    uint32_t m_typeMaxSerialized;

    //!Topic kind
    TopicKind_t m_topicKind;

    //!Persistence GUID
    GUID_t persistence_guid_;

    //!Type Identifier
    TypeIdV1* m_type_id;

    //!Type Object
    TypeObjectV1* m_type;

    //!Type Information
    xtypes::TypeInformation* m_type_information;

    //!
    ParameterPropertyList_t m_properties;
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // _FASTDDS_RTPS_BUILTIN_DATA_WRITERPROXYDATA_H_
