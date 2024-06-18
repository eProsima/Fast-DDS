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

#include <fastcdr/cdr/fixed_size_string.hpp>

#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/attributes/TopicAttributes.h>
#include <fastdds/rtps/common/LocatorList.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>
#if HAVE_SECURITY
#include <fastdds/rtps/attributes/EndpointSecurityAttributes.h>
#endif // if HAVE_SECURITY


namespace eprosima {
namespace fastdds {
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

    FASTDDS_EXPORTED_API WriterProxyData(
            const size_t max_unicast_locators,
            const size_t max_multicast_locators);

    FASTDDS_EXPORTED_API WriterProxyData(
            const size_t max_unicast_locators,
            const size_t max_multicast_locators,
            const VariableLengthDataLimits& data_limits);

    virtual FASTDDS_EXPORTED_API ~WriterProxyData();

    FASTDDS_EXPORTED_API WriterProxyData(
            const WriterProxyData& writerInfo);

    FASTDDS_EXPORTED_API WriterProxyData& operator =(
            const WriterProxyData& writerInfo);

    FASTDDS_EXPORTED_API void guid(
            const GUID_t& guid)
    {
        m_guid = guid;
    }

    FASTDDS_EXPORTED_API void guid(
            GUID_t&& guid)
    {
        m_guid = std::move(guid);
    }

    FASTDDS_EXPORTED_API const GUID_t& guid() const
    {
        return m_guid;
    }

    FASTDDS_EXPORTED_API GUID_t& guid()
    {
        return m_guid;
    }

    FASTDDS_EXPORTED_API void networkConfiguration(
            const NetworkConfigSet_t& networkConfiguration)
    {
        m_networkConfiguration = networkConfiguration;
    }

    FASTDDS_EXPORTED_API void networkConfiguration(
            NetworkConfigSet_t&& networkConfiguration)
    {
        m_networkConfiguration = std::move(networkConfiguration);
    }

    FASTDDS_EXPORTED_API const NetworkConfigSet_t& networkConfiguration() const
    {
        return m_networkConfiguration;
    }

    FASTDDS_EXPORTED_API NetworkConfigSet_t& networkConfiguration()
    {
        return m_networkConfiguration;
    }

    FASTDDS_EXPORTED_API void persistence_guid(
            const GUID_t& guid)
    {
        persistence_guid_ = guid;
    }

    FASTDDS_EXPORTED_API void persistence_guid(
            GUID_t&& guid)
    {
        persistence_guid_ = std::move(guid);
    }

    FASTDDS_EXPORTED_API GUID_t persistence_guid() const
    {
        return persistence_guid_;
    }

    FASTDDS_EXPORTED_API GUID_t& persistence_guid()
    {
        return persistence_guid_;
    }

    FASTDDS_EXPORTED_API void set_persistence_entity_id(
            const EntityId_t& nid)
    {
        persistence_guid_.entityId = persistence_guid_.guidPrefix != c_GuidPrefix_Unknown ? nid : c_EntityId_Unknown;
    }

    FASTDDS_EXPORTED_API bool has_locators() const
    {
        return !remote_locators_.unicast.empty() || !remote_locators_.multicast.empty();
    }

    FASTDDS_EXPORTED_API const RemoteLocatorList& remote_locators() const
    {
        return remote_locators_;
    }

    FASTDDS_EXPORTED_API void add_unicast_locator(
            const Locator_t& locator);

    void set_announced_unicast_locators(
            const LocatorList_t& locators);

    void set_remote_unicast_locators(
            const LocatorList_t& locators,
            const NetworkFactory& network);

    FASTDDS_EXPORTED_API void add_multicast_locator(
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

    FASTDDS_EXPORTED_API void key(
            const InstanceHandle_t& key)
    {
        m_key = key;
    }

    FASTDDS_EXPORTED_API void key(
            InstanceHandle_t&& key)
    {
        m_key = std::move(key);
    }

    FASTDDS_EXPORTED_API InstanceHandle_t key() const
    {
        return m_key;
    }

    FASTDDS_EXPORTED_API InstanceHandle_t& key()
    {
        return m_key;
    }

    FASTDDS_EXPORTED_API void RTPSParticipantKey(
            const InstanceHandle_t& RTPSParticipantKey)
    {
        m_RTPSParticipantKey = RTPSParticipantKey;
    }

    FASTDDS_EXPORTED_API void RTPSParticipantKey(
            InstanceHandle_t&& RTPSParticipantKey)
    {
        m_RTPSParticipantKey = std::move(RTPSParticipantKey);
    }

    FASTDDS_EXPORTED_API InstanceHandle_t RTPSParticipantKey() const
    {
        return m_RTPSParticipantKey;
    }

    FASTDDS_EXPORTED_API InstanceHandle_t& RTPSParticipantKey()
    {
        return m_RTPSParticipantKey;
    }

    FASTDDS_EXPORTED_API void typeName(
            const fastcdr::string_255& typeName)
    {
        m_typeName = typeName;
    }

    FASTDDS_EXPORTED_API void typeName(
            fastcdr::string_255&& typeName)
    {
        m_typeName = std::move(typeName);
    }

    FASTDDS_EXPORTED_API const fastcdr::string_255& typeName() const
    {
        return m_typeName;
    }

    FASTDDS_EXPORTED_API fastcdr::string_255& typeName()
    {
        return m_typeName;
    }

    FASTDDS_EXPORTED_API void topicName(
            const fastcdr::string_255& topicName)
    {
        m_topicName = topicName;
    }

    FASTDDS_EXPORTED_API void topicName(
            fastcdr::string_255&& topicName)
    {
        m_topicName = std::move(topicName);
    }

    FASTDDS_EXPORTED_API const fastcdr::string_255& topicName() const
    {
        return m_topicName;
    }

    FASTDDS_EXPORTED_API fastcdr::string_255& topicName()
    {
        return m_topicName;
    }

    FASTDDS_EXPORTED_API void userDefinedId(
            uint16_t userDefinedId)
    {
        m_userDefinedId = userDefinedId;
    }

    FASTDDS_EXPORTED_API uint16_t userDefinedId() const
    {
        return m_userDefinedId;
    }

    FASTDDS_EXPORTED_API uint16_t& userDefinedId()
    {
        return m_userDefinedId;
    }

    FASTDDS_EXPORTED_API void typeMaxSerialized(
            uint32_t typeMaxSerialized)
    {
        m_typeMaxSerialized = typeMaxSerialized;
    }

    FASTDDS_EXPORTED_API uint32_t typeMaxSerialized() const
    {
        return m_typeMaxSerialized;
    }

    FASTDDS_EXPORTED_API uint32_t& typeMaxSerialized()
    {
        return m_typeMaxSerialized;
    }

    FASTDDS_EXPORTED_API void topicKind(
            TopicKind_t topicKind)
    {
        m_topicKind = topicKind;
    }

    FASTDDS_EXPORTED_API TopicKind_t topicKind() const
    {
        return m_topicKind;
    }

    FASTDDS_EXPORTED_API TopicKind_t& topicKind()
    {
        return m_topicKind;
    }

    FASTDDS_EXPORTED_API void type_id(
            const TypeIdV1& other_type_id)
    {
        type_id() = other_type_id;
    }

    FASTDDS_EXPORTED_API const TypeIdV1& type_id() const
    {
        assert(m_type_id != nullptr);
        return *m_type_id;
    }

    FASTDDS_EXPORTED_API TypeIdV1& type_id()
    {
        if (m_type_id == nullptr)
        {
            m_type_id = new TypeIdV1();
        }
        return *m_type_id;
    }

    FASTDDS_EXPORTED_API bool has_type_id() const
    {
        return m_type_id != nullptr;
    }

    FASTDDS_EXPORTED_API void type(
            const TypeObjectV1& other_type)
    {
        type() = other_type;
    }

    FASTDDS_EXPORTED_API const TypeObjectV1& type() const
    {
        assert(m_type != nullptr);
        return *m_type;
    }

    FASTDDS_EXPORTED_API TypeObjectV1& type()
    {
        if (m_type == nullptr)
        {
            m_type = new TypeObjectV1();
        }
        return *m_type;
    }

    FASTDDS_EXPORTED_API bool has_type() const
    {
        return m_type != nullptr;
    }

    FASTDDS_EXPORTED_API void type_information(
            const xtypes::TypeInformationParameter& other_type_information)
    {
        type_information() = other_type_information;
    }

    FASTDDS_EXPORTED_API const xtypes::TypeInformationParameter& type_information() const
    {
        assert(m_type_information != nullptr);
        return *m_type_information;
    }

    FASTDDS_EXPORTED_API xtypes::TypeInformationParameter& type_information()
    {
        if (m_type_information == nullptr)
        {
            m_type_information = new xtypes::TypeInformationParameter();
        }
        return *m_type_information;
    }

    FASTDDS_EXPORTED_API bool has_type_information() const
    {
        return m_type_information != nullptr;
    }

    //!WriterQOS
    fastdds::dds::WriterQos m_qos;

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
    bool readFromCDRMessage(
            CDRMessage_t* msg,
            const NetworkFactory& network,
            bool is_shm_transport_possible,
            bool should_filter_locators,
            fastdds::rtps::VendorId_t source_vendor_id = c_VendorId_eProsima);

private:

    //!GUID
    GUID_t m_guid;

    //!Network configuration
    NetworkConfigSet_t m_networkConfiguration;

    //!Holds locator information
    RemoteLocatorList remote_locators_;

    //!GUID_t of the Writer converted to InstanceHandle_t
    InstanceHandle_t m_key;

    //!GUID_t of the participant converted to InstanceHandle
    InstanceHandle_t m_RTPSParticipantKey;

    //!Type name
    fastcdr::string_255 m_typeName;

    //!Topic name
    fastcdr::string_255 m_topicName;

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
    xtypes::TypeInformationParameter* m_type_information;

    //!
    ParameterPropertyList_t m_properties;
};

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif // _FASTDDS_RTPS_BUILTIN_DATA_WRITERPROXYDATA_H_
