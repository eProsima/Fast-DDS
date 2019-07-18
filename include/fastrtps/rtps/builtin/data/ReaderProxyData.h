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
 * @file ReaderProxyData.h
 *
 */

#ifndef _RTPS_BUILTIN_DATA_READERPROXYDATA_H_
#define _RTPS_BUILTIN_DATA_READERPROXYDATA_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "../../../attributes/TopicAttributes.h"
#include "../../../qos/ParameterList.h"
#include "../../../qos/ReaderQos.h"

#include "../../attributes/WriterAttributes.h"

#if HAVE_SECURITY
#include "../../security/accesscontrol/EndpointSecurityAttributes.h"
#endif

#include "../../common/RemoteLocators.hpp"

namespace eprosima {
namespace fastrtps{
namespace rtps {

struct CDRMessage_t;
class NetworkFactory;

/**
 * Class ReaderProxyData, used to represent all the information on a Reader (both local and remote) with the purpose of
 * implementing the discovery.
 * *@ingroup BUILTIN_MODULE
 */
class ReaderProxyData
{
    public:

        RTPS_DllAPI ReaderProxyData(
                const size_t max_unicast_locators,
                const size_t max_multicast_locators);

        RTPS_DllAPI virtual ~ReaderProxyData();

        RTPS_DllAPI ReaderProxyData(const ReaderProxyData& readerInfo);

        RTPS_DllAPI ReaderProxyData& operator=(const ReaderProxyData& readerInfo);

        RTPS_DllAPI void guid(const GUID_t& guid)
        {
            m_guid = guid;
        }

        RTPS_DllAPI void guid(GUID_t&& guid)
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

        RTPS_DllAPI bool has_locators() const
        {
            return !remote_locators_.unicast.empty() || !remote_locators_.multicast.empty();
        }

        RTPS_DllAPI const RemoteLocatorList& remote_locators() const
        { 
            return remote_locators_;
        }

        RTPS_DllAPI void add_unicast_locator(const Locator_t& locator);

        void set_announced_unicast_locators(
                const LocatorList_t& locators);

        void set_remote_unicast_locators(
                const LocatorList_t& locators,
                const NetworkFactory& network);

        RTPS_DllAPI void add_multicast_locator(const Locator_t& locator);

        void set_multicast_locators(
                const LocatorList_t& locators,
                const NetworkFactory& network);

        void set_locators(
                const RemoteLocatorList& locators);

        void set_remote_locators(
                const RemoteLocatorList& locators,
                const NetworkFactory& network,
                bool use_multicast_locators);

        RTPS_DllAPI void key(const InstanceHandle_t& key)
        {
            m_key = key;
        }

        RTPS_DllAPI void key(InstanceHandle_t&& key)
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

        RTPS_DllAPI void RTPSParticipantKey(const InstanceHandle_t& RTPSParticipantKey)
        {
            m_RTPSParticipantKey = RTPSParticipantKey;
        }

        RTPS_DllAPI void RTPSParticipantKey(InstanceHandle_t&& RTPSParticipantKey)
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

        RTPS_DllAPI void typeName(const string_255& typeName)
        {
            m_typeName = typeName;
        }

        RTPS_DllAPI void typeName(string_255&& typeName)
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

        RTPS_DllAPI void topicName(const string_255& topicName)
        {
            m_topicName = topicName;
        }

        RTPS_DllAPI void topicName(string_255&& topicName)
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

        RTPS_DllAPI void userDefinedId(uint16_t userDefinedId)
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

        RTPS_DllAPI void isAlive(bool isAlive)
        {
            m_isAlive = isAlive;
        }

        RTPS_DllAPI bool isAlive() const
        {
            return m_isAlive;
        }

        RTPS_DllAPI bool& isAlive()
        {
            return m_isAlive;
        }

        RTPS_DllAPI void topicKind(TopicKind_t topicKind)
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

        RTPS_DllAPI void type_id(TypeIdV1 type_id)
        {
            m_type_id = type_id;
        }

        RTPS_DllAPI TypeIdV1 type_id() const
        {
            return m_type_id;
        }

        RTPS_DllAPI TypeIdV1& type_id()
        {
            return m_type_id;
        }

        RTPS_DllAPI void type(TypeObjectV1 type)
        {
            m_type = type;
        }

        RTPS_DllAPI TypeObjectV1 type() const
        {
            return m_type;
        }

        RTPS_DllAPI TypeObjectV1& type()
        {
            return m_type;
        }

        RTPS_DllAPI void topicDiscoveryKind(TopicDiscoveryKind_t topicDiscoveryKind)
        {
            m_topicDiscoveryKind = topicDiscoveryKind;
        }

        RTPS_DllAPI TopicDiscoveryKind_t topicDiscoveryKind() const
        {
            return m_topicDiscoveryKind;
        }

        RTPS_DllAPI TopicDiscoveryKind_t& topicDiscoveryKind()
        {
            return m_topicDiscoveryKind;
        }

        inline bool disable_positive_acks() const
        {
            return m_qos.m_disablePositiveACKs.enabled;
        }

        /**
         * Write as a parameter list on a CDRMessage_t
         * @return True on success
         */
        bool writeToCDRMessage(CDRMessage_t* msg, bool write_encapsulation);

        /**
         *  Read the information from a CDRMessage_t. The position of hte message must be in the beggining on the parameter list.
         * @param msg Pointer to the message.
         * @param network Reference to network factory for locator validation and transformation
         * @return true on success
         */
        RTPS_DllAPI bool readFromCDRMessage(
                CDRMessage_t* msg,
                const NetworkFactory& network);

        //!
        bool m_expectsInlineQos;
        //!Reader Qos
        ReaderQos m_qos;

#if HAVE_SECURITY
        //!EndpointSecurityInfo.endpoint_security_attributes
        security::EndpointSecurityAttributesMask security_attributes_;

        //!EndpointSecurityInfo.plugin_endpoint_security_attributes
        security::PluginEndpointSecurityAttributesMask plugin_security_attributes_;
#endif

        /**
         * Clear (put to default) the information.
         */
        void clear();

        /**
         * Check if this object can be updated with the information on another object.
         * @param rdata ReaderProxyData object to be checked.
         * @return true if this object can be updated with the information on rdata.
         */
        bool is_update_allowed(const ReaderProxyData& rdata) const;

        /**
         * Update the information (only certain fields will be updated).
         * @param rdata Poitner to the object from which we are going to update.
         */
        void update(ReaderProxyData* rdata);

        /**
         * Copy ALL the information from another object.
         * @param rdata Pointer to the object from where the information must be copied.
         */
        void copy(ReaderProxyData* rdata);

    private:

        //!GUID
        GUID_t m_guid;
        //!Holds locator information
        RemoteLocatorList remote_locators_;
        //!GUID_t of the Reader converted to InstanceHandle_t
        InstanceHandle_t m_key;
        //!GUID_t of the participant converted to InstanceHandle
        InstanceHandle_t m_RTPSParticipantKey;
        //!Type name
        string_255 m_typeName;
        //!Topic name
        string_255 m_topicName;
        //!User defined ID
        uint16_t m_userDefinedId;
        //!Field to indicate if the Reader is Alive.
        bool m_isAlive;
        //!Topic kind
        TopicKind_t m_topicKind;
        //!Topic Discovery Kind
        TopicDiscoveryKind_t m_topicDiscoveryKind;
        //!Type Identifier
        TypeIdV1 m_type_id;
        //!Type Object
        TypeObjectV1 m_type;
};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif // _RTPS_BUILTIN_DATA_READERPROXYDATA_H_
