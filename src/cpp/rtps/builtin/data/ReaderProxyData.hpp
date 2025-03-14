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

#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#if HAVE_SECURITY
#include <fastdds/rtps/attributes/EndpointSecurityAttributes.hpp>
#endif // if HAVE_SECURITY
#include <fastdds/rtps/attributes/RTPSParticipantAllocationAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/builtin/data/ContentFilterProperty.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct CDRMessage_t;
class NetworkFactory;
class ParticipantProxyData;

/**
 * Class ReaderProxyData, used to represent all the information on a Reader (both local and remote) with the purpose of
 * implementing the discovery.
 * *@ingroup BUILTIN_MODULE
 */
class ReaderProxyData : public SubscriptionBuiltinTopicData
{
public:

    ReaderProxyData(
            const size_t max_unicast_locators,
            const size_t max_multicast_locators,
            const fastdds::rtps::ContentFilterProperty::AllocationConfiguration& content_filter_limits = {});

    ReaderProxyData(
            const size_t max_unicast_locators,
            const size_t max_multicast_locators,
            const VariableLengthDataLimits& data_limits,
            const fastdds::rtps::ContentFilterProperty::AllocationConfiguration& content_filter_limits = {});

    ReaderProxyData(
            const VariableLengthDataLimits& data_limits,
            const SubscriptionBuiltinTopicData& subscription_data);

    virtual ~ReaderProxyData();

    ReaderProxyData(
            const ReaderProxyData& readerInfo);

    ReaderProxyData& operator =(
            const ReaderProxyData& readerInfo);

    void network_configuration(
            const NetworkConfigSet_t& network_configuration)
    {
        m_network_configuration = network_configuration;
    }

    void network_configuration(
            NetworkConfigSet_t&& network_configuration)
    {
        m_network_configuration = std::move(network_configuration);
    }

    const NetworkConfigSet_t& network_configuration() const
    {
        return m_network_configuration;
    }

    NetworkConfigSet_t& network_configuration()
    {
        return m_network_configuration;
    }

    bool has_locators() const
    {
        return !remote_locators.unicast.empty() || !remote_locators.multicast.empty();
    }

    void add_unicast_locator(
            const Locator_t& locator);

    void set_announced_unicast_locators(
            const LocatorList_t& locators);

    /**
     * Set the remote unicast locators from @param locators.
     * @param locators List of locators to be used
     * @param network NetworkFactory to check if the locators are allowed
     * @param from_this_host Whether the server is from this host or not
     */
    void set_remote_unicast_locators(
            const LocatorList_t& locators,
            const NetworkFactory& network,
            bool from_this_host);

    void add_multicast_locator(
            const Locator_t& locator);

    /**
     * Set the remote multicast locators from @param locators.
     * @param locators List of locators to be used
     * @param network NetworkFactory to check if the locators are allowed
     * @param from_this_host Whether the server is from this host or not
     */
    void set_multicast_locators(
            const LocatorList_t& locators,
            const NetworkFactory& network,
            bool from_this_host);

    void set_locators(
            const RemoteLocatorList& locators);

    /**
     * Set the remote multicast and unicast locators from @param locators.
     * @param locators List of locators to be used
     * @param network NetworkFactory to check if the locators are allowed
     * @param use_multicast_locators Whether to set multicast locators or not
     * @param from_this_host Whether the server is from this host or not
     */
    void set_remote_locators(
            const RemoteLocatorList& locators,
            const NetworkFactory& network,
            bool use_multicast_locators,
            bool from_this_host);

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

    void is_alive(
            bool is_alive)
    {
        m_is_alive = is_alive;
    }

    bool is_alive() const
    {
        return m_is_alive;
    }

    bool& is_alive()
    {
        return m_is_alive;
    }

    void type_id(
            const dds::TypeIdV1& other_type_id)
    {
        type_id() = other_type_id;
    }

    const dds::TypeIdV1& type_id() const
    {
        assert(m_type_id != nullptr);
        return *m_type_id;
    }

    dds::TypeIdV1& type_id()
    {
        if (m_type_id == nullptr)
        {
            m_type_id = new dds::TypeIdV1();
        }
        return *m_type_id;
    }

    bool has_type_id() const
    {
        return m_type_id != nullptr;
    }

    void type(
            const dds::TypeObjectV1& other_type)
    {
        type() = other_type;
    }

    const dds::TypeObjectV1& type() const
    {
        assert(m_type != nullptr);
        return *m_type;
    }

    dds::TypeObjectV1& type()
    {
        if (m_type == nullptr)
        {
            m_type = new dds::TypeObjectV1();
        }
        return *m_type;
    }

    bool has_type() const
    {
        return m_type != nullptr;
    }

    bool has_type_information() const
    {
        return type_information.assigned();
    }

    inline bool disable_positive_acks_enabled() const
    {
        return disable_positive_acks.enabled;
    }

    /**
     * Set participant client server sample identity
     * @param sid valid SampleIdentity
     */
    void set_sample_identity(
            const SampleIdentity& sid)
    {
        fastdds::dds::set_proxy_property(sid, "PID_CLIENT_SERVER_KEY", properties);
    }

    /**
     * Retrieve participant SampleIdentity
     * @return SampleIdentity
     */
    SampleIdentity get_sample_identity() const
    {
        return fastdds::dds::get_proxy_property<SampleIdentity>("PID_CLIENT_SERVER_KEY", properties);
    }

    /**
     * Get the size in bytes of the CDR serialization of this object.
     * @param include_encapsulation Whether to include the size of the encapsulation info.
     * @return size in bytes of the CDR serialization.
     */
    uint32_t get_serialized_size(
            bool include_encapsulation) const;

    /**
     * Write as a parameter list on a CDRMessage_t
     * @return True on success
     */
    bool write_to_cdr_message(
            CDRMessage_t* msg,
            bool write_encapsulation) const;

    /**
     * Read the information from a CDRMessage_t. The position of the message must be in the beginning on the
     * parameter list.
     * @param msg Pointer to the message.
     * @param source_vendor_id VendorId of the source participant from which the message was received
     * @return true on success
     */
    bool read_from_cdr_message(
            CDRMessage_t* msg,
            fastdds::rtps::VendorId_t source_vendor_id = c_VendorId_eProsima);

    /**
     * Transform and set the remote locators from the remote_locators of another ReaderProxyData.
     * If the received ReaderProxyData has no locators, remote locators will be extracted from the
     * ParticipantProxyData.
     * @param rdata ReaderProxyData to get the locators from
     * @param network NetworkFactory to transform locators
     * @param participant_data ParticipantProxyData to get the locators from
     */
    void setup_locators(
            const ReaderProxyData& rdata,
            NetworkFactory& network,
            const ParticipantProxyData& participant_data);

#if HAVE_SECURITY
    //!EndpointSecurityInfo.endpoint_security_attributes
    security::EndpointSecurityAttributesMask security_attributes_;

    //!EndpointSecurityInfo.plugin_endpoint_security_attributes
    security::PluginEndpointSecurityAttributesMask plugin_security_attributes_;
#endif // if HAVE_SECURITY

    /**
     * Clear (put to default) the information.
     */
    void clear();

    /**
     * Check if this object can be updated with the information on another object.
     * @param rdata ReaderProxyData object to be checked.
     * @return true if this object can be updated with the information on rdata.
     */
    bool is_update_allowed(
            const ReaderProxyData& rdata) const;

    /**
     * Update the information (only certain fields will be updated).
     * @param rdata Pointer to the object from which we are going to update.
     */
    void update(
            ReaderProxyData* rdata);

    /**
     * Set qos parameters
     * (only certain qos from SubscriptionBuiltinTopicData will be set).
     *
     * @param qos Const reference to the SubscriptionBuiltinTopicData object.
     * @param first_time Boolean indicating whether is the first time (true) or not (false).
     */
    void set_qos(
            const SubscriptionBuiltinTopicData& qos,
            bool first_time);

    /**
     * Set qos parameters from a ReaderQos structure.
     * (only certain qos from SubscriptionBuiltinTopicData will be set).
     *
     * @param qos Const reference to the ReaderQos object.
     * @param first_time Boolean indicating whether is the first time (true) or not (false).
     */
    void set_qos(
            const dds::ReaderQos& qos,
            bool first_time);

private:

    /**
     * Initialize the common attributes of the ReaderProxyData.
     */
    void init(
            const VariableLengthDataLimits& data_limits);

    /**
     * Checks whether the QoS can be updated with the provided QoS.
     *
     * @param qos The QoS to check.
     * @return true if the QoS can be updated, false otherwise.
     */
    bool can_qos_be_updated(
            const SubscriptionBuiltinTopicData& qos) const;


    //!Network configuration
    NetworkConfigSet_t m_network_configuration;
    //!GUID_t of the Reader converted to InstanceHandle_t
    InstanceHandle_t m_key;
    //!GUID_t of the participant converted to InstanceHandle
    InstanceHandle_t m_rtps_participant_key;
    //!User defined ID
    uint16_t m_user_defined_id;
    //!Field to indicate if the Reader is Alive.
    bool m_is_alive;
    //!Type Identifier
    dds::TypeIdV1* m_type_id;
    //!Type Object
    dds::TypeObjectV1* m_type;
};

} // namespace rtps
} // namespace rtps
} // namespace eprosima

#endif // RTPS_BUILTIN_DATA__READERPROXYDATA_HPP
