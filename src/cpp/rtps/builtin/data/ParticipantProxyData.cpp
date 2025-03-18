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
 * @file ParticipantProxyData.cpp
 *
 */

#include <rtps/builtin/data/ParticipantProxyData.hpp>

#include <chrono>
#include <mutex>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/core/policy/QosPoliciesSerializer.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/ProductVersion_t.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>

#include <rtps/builtin/BuiltinProtocols.h>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/participant/PDPSimple.h>
#include <rtps/network/NetworkFactory.hpp>
#include <rtps/resources/TimedEvent.h>
#include <rtps/transport/shared_mem/SHMLocator.hpp>
#include <utils/BuiltinTopicKeyConversions.hpp>
#include <utils/SystemInfo.hpp>
#include <utils/TimeConversion.hpp>

#include "ProxyDataFilters.hpp"
#include "ProxyHashTables.hpp"

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastdds {
namespace rtps {

using ::operator <<;

ParticipantProxyData::ParticipantProxyData(
        const RTPSParticipantAllocationAttributes& allocation)
    : ParticipantBuiltinTopicData(
        c_VendorId_Unknown,
        fastdds::dds::DOMAIN_ID_UNKNOWN,
        allocation
        )
    , m_protocol_version(c_ProtocolVersion)
    , m_expects_inline_qos(false)
    , m_available_builtin_endpoints(0)
    , m_network_configuration(0)
    , m_manual_liveliness_count()
#if HAVE_SECURITY
    , security_attributes_(0UL)
    , plugin_security_attributes_(0UL)
#endif // if HAVE_SECURITY
    , is_alive(false)
    , lease_duration_event(nullptr)
    , should_check_lease_duration(false)
    , m_readers(new ProxyHashTable<ReaderProxyData>(allocation.readers))
    , m_writers(new ProxyHashTable<WriterProxyData>(allocation.writers))
    , m_sample_identity()
{
    user_data.set_max_size(static_cast<uint32_t>(allocation.data_limits.max_user_data));
}

ParticipantProxyData::ParticipantProxyData(
        const ParticipantProxyData& pdata)
    : ParticipantBuiltinTopicData(pdata)
    , m_protocol_version(pdata.m_protocol_version)
    , machine_id(pdata.machine_id)
    , m_expects_inline_qos(pdata.m_expects_inline_qos)
    , m_available_builtin_endpoints(pdata.m_available_builtin_endpoints)
    , m_network_configuration(pdata.m_network_configuration)
    , m_manual_liveliness_count()
    , m_key(pdata.m_key)
#if HAVE_SECURITY
    , identity_token_(pdata.identity_token_)
    , permissions_token_(pdata.permissions_token_)
    , security_attributes_(pdata.security_attributes_)
    , plugin_security_attributes_(pdata.plugin_security_attributes_)
#endif // if HAVE_SECURITY
    , is_alive(pdata.is_alive)
    , lease_duration_event(nullptr)
    , should_check_lease_duration(false)
    // This method is only called when calling the participant discovery listener and the
    // corresponding DiscoveredParticipantInfo struct is created. Only participant info is used,
    // so there is no need to copy m_readers and m_writers
    , m_readers(nullptr)
    , m_writers(nullptr)
    , m_sample_identity(pdata.m_sample_identity)
    , lease_duration_(pdata.lease_duration_)
{
}

ParticipantProxyData::~ParticipantProxyData()
{
    EPROSIMA_LOG_INFO(RTPS_PARTICIPANT, guid);

    // delete all reader proxies
    if (m_readers)
    {
        for (ProxyHashTable<ReaderProxyData>::value_type val : *m_readers)
        {
            delete val.second;
        }

        delete m_readers;
    }

    // delete all writers proxies
    if (m_writers)
    {
        for (ProxyHashTable<WriterProxyData>::value_type val : *m_writers)
        {
            delete val.second;
        }

        delete m_writers;
    }

    if (lease_duration_event != nullptr)
    {
        delete lease_duration_event;
    }
}

uint32_t ParticipantProxyData::get_serialized_size(
        bool include_encapsulation) const
{
    uint32_t ret_val = include_encapsulation ? 4 : 0;

    // PID_PROTOCOL_VERSION
    ret_val += 4 + PARAMETER_PROTOCOL_LENGTH;

    // PID_VENDORID
    ret_val += 4 + PARAMETER_VENDOR_LENGTH;

    // PID_PRODUCT_VERSION
    ret_val += 4 + PARAMETER_PRODUCT_VERSION_LENGTH;

    // PID_DOMAIN_ID
    ret_val += 4 + PARAMETER_DOMAINID_LENGTH;

    if (m_expects_inline_qos)
    {
        // PID_EXPECTS_INLINE_QOS
        ret_val += 4 + PARAMETER_BOOL_LENGTH;
    }

    // PID_PARTICIPANT_GUID
    ret_val += 4 + PARAMETER_GUID_LENGTH;

    // PID_NETWORK_CONFIGURATION_SET
    ret_val += 4 + PARAMETER_NETWORKCONFIGSET_LENGTH;

    if (machine_id.size() > 0)
    {
        // PID_MACHINE_ID
        ret_val +=
                fastdds::dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(machine_id);
    }

    // PID_METATRAFFIC_MULTICAST_LOCATOR
    ret_val +=
            static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) *
            metatraffic_locators.multicast.size());

    // PID_METATRAFFIC_UNICAST_LOCATOR
    ret_val +=
            static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) *
            metatraffic_locators.unicast.size());

    // PID_DEFAULT_UNICAST_LOCATOR
    ret_val +=
            static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * default_locators.unicast.size());

    // PID_DEFAULT_MULTICAST_LOCATOR
    ret_val +=
            static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * default_locators.multicast.size());

    // PID_PARTICIPANT_LEASE_DURATION
    ret_val += 4 + PARAMETER_TIME_LENGTH;

    // PID_BUILTIN_ENDPOINT_SET
    ret_val += 4 + PARAMETER_BUILTINENDPOINTSET_LENGTH;

    if (participant_name.size() > 0)
    {
        // PID_ENTITY_NAME
        ret_val +=
                fastdds::dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(participant_name);
    }

    if (user_data.size() > 0)
    {
        // PID_USER_DATA
        ret_val += fastdds::dds::QosPoliciesSerializer<dds::UserDataQosPolicy>::cdr_serialized_size(
            user_data);
    }

    if (properties.size() > 0)
    {
        // PID_PROPERTY_LIST
        ret_val += fastdds::dds::ParameterSerializer<ParameterPropertyList_t>::cdr_serialized_size(
            properties);
    }

#if HAVE_SECURITY
    if (!identity_token_.class_id().empty())
    {
        // PID_IDENTITY_TOKEN
        ret_val += fastdds::dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(identity_token_);
    }

    if (!permissions_token_.class_id().empty())
    {
        // PID_PERMISSIONS_TOKEN
        ret_val += fastdds::dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(
            permissions_token_);
    }

    if ((security_attributes_ != 0UL) || (plugin_security_attributes_ != 0UL))
    {
        // PID_PARTICIPANT_SECURITY_INFO
        ret_val += 4 + PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH;
    }
#endif // if HAVE_SECURITY

    // PID_SENTINEL
    return ret_val + 4;
}

bool ParticipantProxyData::write_to_cdr_message(
        CDRMessage_t* msg,
        bool write_encapsulation)
{
    if (write_encapsulation)
    {
        if (!ParameterList::writeEncapsulationToCDRMsg(msg))
        {
            return false;
        }
    }

    {
        ParameterProtocolVersion_t p(fastdds::dds::PID_PROTOCOL_VERSION, PARAMETER_PROTOCOL_LENGTH);
        p.protocolVersion = this->m_protocol_version;
        if (!fastdds::dds::ParameterSerializer<ParameterProtocolVersion_t>::add_to_cdr_message(
                    p,
                    msg))
        {
            return false;
        }
    }
    {
        ParameterVendorId_t p(fastdds::dds::PID_VENDORID, PARAMETER_VENDOR_LENGTH);
        p.vendorId[0] = this->vendor_id[0];
        p.vendorId[1] = this->vendor_id[1];
        if (!fastdds::dds::ParameterSerializer<ParameterVendorId_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterProductVersion_t p(fastdds::dds::PID_PRODUCT_VERSION,
                PARAMETER_PRODUCT_VERSION_LENGTH);
        p.version.major = this->product_version.major;
        p.version.minor = this->product_version.minor;
        p.version.patch = this->product_version.patch;
        p.version.tweak = this->product_version.tweak;
        if (!fastdds::dds::ParameterSerializer<ParameterProductVersion_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterDomainId_t p(fastdds::dds::PID_DOMAIN_ID, 4);
        p.domain_id = this->domain_id;
        if (!fastdds::dds::ParameterSerializer<ParameterDomainId_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    if (this->m_expects_inline_qos)
    {
        ParameterBool_t p(fastdds::dds::PID_EXPECTS_INLINE_QOS, PARAMETER_BOOL_LENGTH,
                m_expects_inline_qos);
        if (!fastdds::dds::ParameterSerializer<ParameterBool_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterGuid_t p(fastdds::dds::PID_PARTICIPANT_GUID, PARAMETER_GUID_LENGTH, guid);
        if (!fastdds::dds::ParameterSerializer<ParameterGuid_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterNetworkConfigSet_t p(fastdds::dds::PID_NETWORK_CONFIGURATION_SET,
                PARAMETER_NETWORKCONFIGSET_LENGTH);
        p.netconfigSet = m_network_configuration;
        if (!fastdds::dds::ParameterSerializer<ParameterNetworkConfigSet_t>::add_to_cdr_message(
                    p,
                    msg))
        {
            return false;
        }
    }
    if (machine_id.size() > 0)
    {
        ParameterString_t p(fastdds::dds::PID_MACHINE_ID, 0, machine_id);
        if (!fastdds::dds::ParameterSerializer<ParameterString_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    for (const Locator_t& it : metatraffic_locators.multicast)
    {
        ParameterLocator_t p(fastdds::dds::PID_METATRAFFIC_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH,
                it);
        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    for (const Locator_t& it : metatraffic_locators.unicast)
    {
        ParameterLocator_t p(fastdds::dds::PID_METATRAFFIC_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH,
                it);
        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    for (const Locator_t& it : default_locators.unicast)
    {
        ParameterLocator_t p(fastdds::dds::PID_DEFAULT_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, it);
        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    for (const Locator_t& it : default_locators.multicast)
    {
        ParameterLocator_t p(fastdds::dds::PID_DEFAULT_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, it);
        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterTime_t p(fastdds::dds::PID_PARTICIPANT_LEASE_DURATION, PARAMETER_TIME_LENGTH);
        p.time = lease_duration;
        if (!fastdds::dds::ParameterSerializer<ParameterTime_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterBuiltinEndpointSet_t p(fastdds::dds::PID_BUILTIN_ENDPOINT_SET,
                PARAMETER_BUILTINENDPOINTSET_LENGTH);
        p.endpointSet = m_available_builtin_endpoints;
        if (!fastdds::dds::ParameterSerializer<ParameterBuiltinEndpointSet_t>::add_to_cdr_message(
                    p,
                    msg))
        {
            return false;
        }
    }

    if (participant_name.size() > 0)
    {
        ParameterString_t p(fastdds::dds::PID_ENTITY_NAME, 0, participant_name);
        if (!fastdds::dds::ParameterSerializer<ParameterString_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }

    if (user_data.size() > 0)
    {
        if (!fastdds::dds::QosPoliciesSerializer<dds::UserDataQosPolicy>::add_to_cdr_message(
                    user_data,
                    msg))
        {
            return false;
        }
    }

    if (properties.size() > 0)
    {
        if (!fastdds::dds::ParameterSerializer<ParameterPropertyList_t>::add_to_cdr_message(
                    properties,
                    msg))
        {
            return false;
        }
    }

#if HAVE_SECURITY
    if (!identity_token_.class_id().empty())
    {
        ParameterToken_t p(fastdds::dds::PID_IDENTITY_TOKEN, 0);
        p.token = identity_token_;
        if (!fastdds::dds::ParameterSerializer<ParameterToken_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }

    if (!permissions_token_.class_id().empty())
    {
        ParameterToken_t p(fastdds::dds::PID_PERMISSIONS_TOKEN, 0);
        p.token = permissions_token_;
        if (!fastdds::dds::ParameterSerializer<ParameterToken_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }

    if ((security_attributes_ != 0UL) || (plugin_security_attributes_ != 0UL))
    {
        ParameterParticipantSecurityInfo_t p;
        p.security_attributes = security_attributes_;
        p.plugin_security_attributes = plugin_security_attributes_;
        if (!fastdds::dds::ParameterSerializer<ParameterParticipantSecurityInfo_t>::add_to_cdr_message(
                    p,
                    msg))
        {
            return false;
        }
    }
#endif // if HAVE_SECURITY

    return fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(msg);
}

bool ParticipantProxyData::read_from_cdr_message(
        CDRMessage_t* msg,
        bool use_encapsulation,
        NetworkFactory& network,
        bool should_filter_locators,
        fastdds::rtps::VendorId_t source_vendor_id)
{
    auto param_process =
            [this, &network, &should_filter_locators, source_vendor_id](
        CDRMessage_t* msg, const ParameterId_t& pid, uint16_t plength)
            {
                vendor_id = source_vendor_id;
                switch (pid){
                    case fastdds::dds::PID_KEY_HASH:
                    {
                        ParameterKey_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterKey_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        iHandle2GUID(guid, p.key);
                        m_key = p.key;
                        break;
                    }
                    case fastdds::dds::PID_PROTOCOL_VERSION:
                    {
                        ParameterProtocolVersion_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterProtocolVersion_t>::
                                read_from_cdr_message(
                                    p,
                                    msg, plength))
                        {
                            return false;
                        }

                        if (p.protocolVersion.m_major < c_ProtocolVersion.m_major)
                        {
                            return false;
                        }
                        m_protocol_version = p.protocolVersion;
                        break;
                    }
                    case fastdds::dds::PID_VENDORID:
                    {
                        ParameterVendorId_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterVendorId_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        vendor_id[0] = p.vendorId[0];
                        vendor_id[1] = p.vendorId[1];
                        break;
                    }
                    case fastdds::dds::PID_PRODUCT_VERSION:
                    {
                        // Ignore custom PID when coming from other vendors
                        if (c_VendorId_eProsima != vendor_id)
                        {
                            EPROSIMA_LOG_INFO(
                                RTPS_PROXY_DATA,
                                "Ignoring custom PID" << pid << " from vendor " << source_vendor_id);
                            return true;
                        }

                        ParameterProductVersion_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterProductVersion_t>::read_from_cdr_message(
                                    p,
                                    msg, plength))
                        {
                            return false;
                        }

                        product_version = p.version;
                        break;
                    }
                    case fastdds::dds::PID_DOMAIN_ID:
                    {
                        ParameterDomainId_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterDomainId_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        domain_id = p.domain_id;
                        break;
                    }
                    case fastdds::dds::PID_EXPECTS_INLINE_QOS:
                    {
                        ParameterBool_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterBool_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        m_expects_inline_qos = p.value;
                        break;
                    }
                    case fastdds::dds::PID_PARTICIPANT_GUID:
                    {
                        ParameterGuid_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterGuid_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        guid = p.guid;
                        m_key = p.guid;
                        from_guid_prefix_to_topic_key(guid.guidPrefix, key.value);
                        break;
                    }
                    case fastdds::dds::PID_NETWORK_CONFIGURATION_SET:
                    {
                        // Ignore custom PID when coming from other vendors
                        if (c_VendorId_eProsima != vendor_id)
                        {
                            EPROSIMA_LOG_INFO(
                                RTPS_PROXY_DATA,
                                "Ignoring custom PID" << pid << " from vendor " << source_vendor_id);
                            return true;
                        }

                        ParameterNetworkConfigSet_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterNetworkConfigSet_t>::
                                read_from_cdr_message(
                                    p,
                                    msg, plength))
                        {
                            return false;
                        }

                        m_network_configuration = p.netconfigSet;
                        break;
                    }
                    case fastdds::dds::PID_MACHINE_ID:
                    {
                        // Ignore custom PID when coming from other vendors
                        if (c_VendorId_eProsima != vendor_id)
                        {
                            EPROSIMA_LOG_INFO(
                                RTPS_PROXY_DATA,
                                "Ignoring custom PID" << pid << " from vendor " << source_vendor_id);
                            return true;
                        }

                        ParameterString_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterString_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        machine_id = p.getName();
                        break;
                    }
                    case fastdds::dds::PID_METATRAFFIC_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        if (!should_filter_locators)
                        {
                            metatraffic_locators.add_multicast_locator(p.locator);
                        }
                        else
                        {
                            Locator_t temp_locator;
                            if (network.transform_remote_locator(
                                        p.locator, temp_locator, m_network_configuration,
                                        is_from_this_host()))
                            {
                                ProxyDataFilters::filter_locators(
                                    network,
                                    metatraffic_locators,
                                    temp_locator,
                                    false);
                            }
                        }
                        break;
                    }
                    case fastdds::dds::PID_METATRAFFIC_UNICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        if (!should_filter_locators)
                        {
                            metatraffic_locators.add_unicast_locator(p.locator);
                        }
                        else
                        {
                            Locator_t temp_locator;
                            if (network.transform_remote_locator(
                                        p.locator, temp_locator, m_network_configuration,
                                        is_from_this_host()))
                            {
                                ProxyDataFilters::filter_locators(
                                    network,
                                    metatraffic_locators,
                                    temp_locator,
                                    true);
                            }
                        }
                        break;
                    }
                    case fastdds::dds::PID_DEFAULT_UNICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        if (!should_filter_locators)
                        {
                            default_locators.add_unicast_locator(p.locator);
                        }
                        else
                        {
                            Locator_t temp_locator;
                            if (network.transform_remote_locator(
                                        p.locator, temp_locator, m_network_configuration,
                                        is_from_this_host()))
                            {
                                ProxyDataFilters::filter_locators(
                                    network,
                                    default_locators,
                                    temp_locator,
                                    true);
                            }
                        }
                        break;
                    }
                    case fastdds::dds::PID_DEFAULT_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        if (!should_filter_locators)
                        {
                            default_locators.add_multicast_locator(p.locator);
                        }
                        else
                        {
                            Locator_t temp_locator;
                            if (network.transform_remote_locator(
                                        p.locator, temp_locator, m_network_configuration,
                                        is_from_this_host()))
                            {
                                ProxyDataFilters::filter_locators(
                                    network,
                                    default_locators,
                                    temp_locator,
                                    false);
                            }
                        }
                        break;
                    }
                    case fastdds::dds::PID_PARTICIPANT_LEASE_DURATION:
                    {
                        ParameterTime_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterTime_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        lease_duration = p.time.to_duration_t();
                        lease_duration_ =
                                std::chrono::microseconds(
                            fastdds::rtps::TimeConv::Duration_t2MicroSecondsInt64(
                                lease_duration));
                        break;
                    }
                    case fastdds::dds::PID_BUILTIN_ENDPOINT_SET:
                    {
                        ParameterBuiltinEndpointSet_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterBuiltinEndpointSet_t>::
                                read_from_cdr_message(
                                    p,
                                    msg, plength))
                        {
                            return false;
                        }

                        m_available_builtin_endpoints = p.endpointSet;
                        break;
                    }
                    case fastdds::dds::PID_ENTITY_NAME:
                    {
                        ParameterString_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterString_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        participant_name = p.getName();
                        break;
                    }
                    case fastdds::dds::PID_PROPERTY_LIST:
                    {
                        if (!fastdds::dds::ParameterSerializer<ParameterPropertyList_t>::read_from_cdr_message(
                                    properties, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_USER_DATA:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<dds::UserDataQosPolicy>::read_from_cdr_message(
                                    user_data,
                                    msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_IDENTITY_TOKEN:
                    {
#if HAVE_SECURITY
                        ParameterToken_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterToken_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        identity_token_ = std::move(p.token);
#else
                        EPROSIMA_LOG_WARNING(
                            RTPS_PARTICIPANT,
                            "Received PID_IDENTITY_TOKEN but security is disabled");
#endif // if HAVE_SECURITY
                        break;
                    }
                    case fastdds::dds::PID_PERMISSIONS_TOKEN:
                    {
#if HAVE_SECURITY
                        ParameterToken_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterToken_t>::read_from_cdr_message(
                                    p, msg,
                                    plength))
                        {
                            return false;
                        }

                        permissions_token_ = std::move(p.token);
#else
                        EPROSIMA_LOG_WARNING(
                            RTPS_PARTICIPANT,
                            "Received PID_PERMISSIONS_TOKEN but security is disabled");
#endif // if HAVE_SECURITY
                        break;
                    }

                    case fastdds::dds::PID_PARTICIPANT_SECURITY_INFO:
                    {
#if HAVE_SECURITY
                        ParameterParticipantSecurityInfo_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterParticipantSecurityInfo_t>::
                                read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        security_attributes_ = p.security_attributes;
                        plugin_security_attributes_ = p.plugin_security_attributes;
#else
                        EPROSIMA_LOG_WARNING(
                            RTPS_PARTICIPANT,
                            "Received PID_PARTICIPANT_SECURITY_INFO but security is disabled");
#endif // if HAVE_SECURITY
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }

                return true;
            };

    uint32_t qos_size;
    clear();
    try
    {
        return ParameterList::readParameterListfromCDRMsg(
            *msg, param_process, use_encapsulation,
            qos_size);
    }
    catch (std::bad_alloc& ba)
    {
        std::cerr << "bad_alloc caught: " << ba.what() << '\n';
        return false;
    }
}

bool ParticipantProxyData::is_from_this_host() const
{
    bool same_host = false;
    if (machine_id.size() > 0)
    {
        same_host = machine_id == SystemInfo::instance().machine_id();
    }
    else
    {
        same_host = guid.is_from_this_host();
    }
    return same_host;
}

void ParticipantProxyData::clear()
{
    m_protocol_version = ProtocolVersion_t();
    key = BuiltinTopicKey_t();
    guid = GUID_t();
    machine_id = "";
    //set_VendorId_Unknown(vendor_id);
    vendor_id = c_VendorId_Unknown;
    product_version = {};
    domain_id = fastdds::dds::DOMAIN_ID_UNKNOWN;
    m_expects_inline_qos = false;
    m_available_builtin_endpoints = 0;
    m_network_configuration = 0;
    metatraffic_locators.unicast.clear();
    metatraffic_locators.multicast.clear();
    default_locators.unicast.clear();
    default_locators.multicast.clear();
    participant_name = "";
    m_key = InstanceHandle_t();
    lease_duration = dds::Duration_t();
    lease_duration_ = std::chrono::microseconds::zero();
    is_alive = true;
#if HAVE_SECURITY
    identity_token_ = IdentityToken();
    permissions_token_ = PermissionsToken();
    security_attributes_ = 0UL;
    plugin_security_attributes_ = 0UL;
#endif // if HAVE_SECURITY
    properties.clear();
    properties.length = 0;
    user_data.clear();
    user_data.length = 0;
    if (wire_protocol)
    {
        wire_protocol->clear();
    }
}

void ParticipantProxyData::copy(
        const ParticipantProxyData& pdata)
{
    m_protocol_version = pdata.m_protocol_version;
    guid = pdata.guid;
    machine_id = pdata.machine_id;
    vendor_id[0] = pdata.vendor_id[0];
    vendor_id[1] = pdata.vendor_id[1];
    product_version = pdata.product_version;
    domain_id = pdata.domain_id;
    m_available_builtin_endpoints = pdata.m_available_builtin_endpoints;
    m_network_configuration = pdata.m_network_configuration;
    metatraffic_locators = pdata.metatraffic_locators;
    default_locators = pdata.default_locators;
    participant_name = pdata.participant_name;
    lease_duration = pdata.lease_duration;
    lease_duration_ =
            std::chrono::microseconds(
        fastdds::rtps::TimeConv::Duration_t2MicroSecondsInt64(
            pdata.lease_duration));
    m_key = pdata.m_key;
    is_alive = pdata.is_alive;
    user_data = pdata.user_data;
    properties = pdata.properties;
    m_sample_identity = pdata.m_sample_identity;

    if (pdata.wire_protocol)
    {
        wire_protocol = pdata.wire_protocol;
    }

    // This method is only called when a new participant is discovered.The destination of the copy
    // will always be a new ParticipantProxyData or one from the pool, so there is no need for
    // m_readers and m_writers to be copied

#if HAVE_SECURITY
    identity_token_ = pdata.identity_token_;
    permissions_token_ = pdata.permissions_token_;
    security_attributes_ = pdata.security_attributes_;
    plugin_security_attributes_ = pdata.plugin_security_attributes_;
#endif // if HAVE_SECURITY
}

bool ParticipantProxyData::update_data(
        ParticipantProxyData& pdata)
{
    metatraffic_locators = pdata.metatraffic_locators;
    default_locators = pdata.default_locators;
    lease_duration = pdata.lease_duration;
    is_alive = true;
    user_data = pdata.user_data;
    properties = pdata.properties;
#if HAVE_SECURITY
    identity_token_ = pdata.identity_token_;
    permissions_token_ = pdata.permissions_token_;
    security_attributes_ = pdata.security_attributes_;
    plugin_security_attributes_ = pdata.plugin_security_attributes_;
#endif // if HAVE_SECURITY
    auto new_lease_duration =
            std::chrono::microseconds(fastdds::rtps::TimeConv::Duration_t2MicroSecondsInt64(lease_duration));
    if (lease_duration_event != nullptr)
    {
        if (new_lease_duration < lease_duration_)
        {
            // Calculate next trigger.
            auto real_lease_tm = last_received_message_tm_ + new_lease_duration;
            auto next_trigger = real_lease_tm - std::chrono::steady_clock::now();
            lease_duration_event->cancel_timer();
            lease_duration_event->update_interval_millisec(
                (double)std::chrono::duration_cast<std::chrono::milliseconds>(next_trigger).count());
            lease_duration_event->restart_timer();
        }
    }
    lease_duration_ = new_lease_duration;
    return true;
}

void ParticipantProxyData::set_persistence_guid(
        const GUID_t& ps_guid)
{
    // only valid values
    if (ps_guid == c_Guid_Unknown)
    {
        return;
    }

    // generate pair
    std::pair<std::string, std::string> persistent_guid;
    persistent_guid.first = fastdds::dds::parameter_property_persistence_guid;

    std::ostringstream data;
    data << ps_guid;
    persistent_guid.second = data.str();

    // if exists replace
    ParameterPropertyList_t::iterator it = std::find_if(
        properties.begin(),
        properties.end(),
        [&persistent_guid](const fastdds::dds::ParameterProperty_t& p)
        {
            return persistent_guid.first == p.first();
        });

    if (it != properties.end())
    {
        if (!it->modify(persistent_guid))
        {
            EPROSIMA_LOG_ERROR(
                RTPS_PARTICIPANT, "Failed to change property <"
                    << it->first() << " | " << it->second() << "> to <"
                    << persistent_guid.first << " | " << persistent_guid.second << ">");
        }
    }
    else
    {
        // if not exists add
        properties.push_back(persistent_guid.first, persistent_guid.second);
    }
}

GUID_t ParticipantProxyData::get_persistence_guid() const
{
    GUID_t persistent(c_Guid_Unknown);

    ParameterPropertyList_t::const_iterator it = std::find_if(
        properties.begin(),
        properties.end(),
        [](const fastdds::dds::ParameterProperty_t p)
        {
            return fastdds::dds::parameter_property_persistence_guid == p.first();
        });

    if (it != properties.end())
    {
        std::istringstream in(it->second());
        in >> persistent;
    }

    return persistent;
}

void ParticipantProxyData::set_sample_identity(
        const SampleIdentity& sid)
{
    fastdds::dds::set_proxy_property(sid, "PID_CLIENT_SERVER_KEY", properties);
}

SampleIdentity ParticipantProxyData::get_sample_identity() const
{
    return fastdds::dds::get_proxy_property<SampleIdentity>("PID_CLIENT_SERVER_KEY", properties);
}

void ParticipantProxyData::set_backup_stamp(
        const GUID_t& guid_)
{
    fastdds::dds::set_proxy_property(guid_, "PID_BACKUP_STAMP", properties);
}

GUID_t ParticipantProxyData::get_backup_stamp() const
{
    return fastdds::dds::get_proxy_property<GUID_t>("PID_BACKUP_STAMP", properties);
}

void ParticipantProxyData::assert_liveliness()
{
    last_received_message_tm_ = std::chrono::steady_clock::now();
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
