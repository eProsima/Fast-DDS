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

#include <fastdds/rtps/builtin/data/ParticipantProxyData.h>

#include <fastdds/dds/log/Log.hpp>

#include <fastdds/rtps/builtin/BuiltinProtocols.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/builtin/discovery/participant/PDPSimple.h>
#include <fastdds/rtps/network/NetworkFactory.h>
#include <fastdds/rtps/resources/TimedEvent.h>
#include <fastrtps/utils/TimeConversion.h>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/core/policy/QosPoliciesSerializer.hpp>
#include <fastrtps_deprecated/participant/ParticipantImpl.h>
#include <rtps/transport/shared_mem/SHMLocator.hpp>

#include "ProxyDataFilters.hpp"
#include "ProxyHashTables.hpp"

#include <mutex>
#include <chrono>

using namespace eprosima::fastrtps;
using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastrtps {
namespace rtps {

ParticipantProxyData::ParticipantProxyData(
        const RTPSParticipantAllocationAttributes& allocation)
    : m_protocolVersion(c_ProtocolVersion)
    , m_VendorId(c_VendorId_Unknown)
    , m_expectsInlineQos(false)
    , m_availableBuiltinEndpoints(0)
    , metatraffic_locators(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
    , default_locators(allocation.locators.max_unicast_locators, allocation.locators.max_multicast_locators)
    , m_manualLivelinessCount ()
#if HAVE_SECURITY
    , security_attributes_(0UL)
    , plugin_security_attributes_(0UL)
#endif
    , isAlive(false)
    , m_properties(static_cast<uint32_t>(allocation.data_limits.max_properties))
    , lease_duration_event(nullptr)
    , should_check_lease_duration(false)
    , m_readers(new ProxyHashTable<ReaderProxyData>(allocation.readers))
    , m_writers(new ProxyHashTable<WriterProxyData>(allocation.writers))
{
    m_userData.set_max_size(static_cast<uint32_t>(allocation.data_limits.max_user_data));
}

ParticipantProxyData::ParticipantProxyData(
        const ParticipantProxyData& pdata)
    : m_protocolVersion(pdata.m_protocolVersion)
    , m_guid(pdata.m_guid)
    , m_VendorId(pdata.m_VendorId)
    , m_expectsInlineQos(pdata.m_expectsInlineQos)
    , m_availableBuiltinEndpoints(pdata.m_availableBuiltinEndpoints)
    , metatraffic_locators(pdata.metatraffic_locators)
    , default_locators(pdata.default_locators)
    , m_manualLivelinessCount ()
    , m_participantName(pdata.m_participantName)
    , m_key(pdata.m_key)
    , m_leaseDuration(pdata.m_leaseDuration)
#if HAVE_SECURITY
    , identity_token_(pdata.identity_token_)
    , permissions_token_(pdata.permissions_token_)
    , security_attributes_(pdata.security_attributes_)
    , plugin_security_attributes_(pdata.plugin_security_attributes_)
#endif
    , isAlive(pdata.isAlive)
    , m_properties(pdata.m_properties)
    , m_userData(pdata.m_userData)
    , lease_duration_event(nullptr)
    , should_check_lease_duration(false)
    // This method is only called when calling the participant discovery listener and the
    // corresponding DiscoveredParticipantInfo struct is created. Only participant info is used,
    // so there is no need to copy m_readers and m_writers
    , m_readers(nullptr)
    , m_writers(nullptr)
    , lease_duration_(pdata.lease_duration_)
{
}

ParticipantProxyData::~ParticipantProxyData()
{
    logInfo(RTPS_PARTICIPANT, m_guid);

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
    ret_val += 4 + 4;

    // PID_VENDORID
    ret_val += 4 + 4;

    if (m_expectsInlineQos)
    {
        // PID_EXPECTS_INLINE_QOS
        ret_val += 4 + PARAMETER_BOOL_LENGTH;
    }

    // PID_PARTICIPANT_GUID
    ret_val += 4 + PARAMETER_GUID_LENGTH;

    // PID_METATRAFFIC_MULTICAST_LOCATOR
    ret_val += static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * metatraffic_locators.multicast.size());

    // PID_METATRAFFIC_UNICAST_LOCATOR
    ret_val += static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * metatraffic_locators.unicast.size());

    // PID_DEFAULT_UNICAST_LOCATOR
    ret_val += static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * default_locators.unicast.size());

    // PID_DEFAULT_MULTICAST_LOCATOR
    ret_val += static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * default_locators.multicast.size());

    // PID_PARTICIPANT_LEASE_DURATION
    ret_val += 4 + PARAMETER_TIME_LENGTH;

    // PID_BUILTIN_ENDPOINT_SET
    ret_val += 4 + PARAMETER_BUILTINENDPOINTSET_LENGTH;

    if (m_participantName.size() > 0)
    {
        // PID_ENTITY_NAME
        ret_val += fastdds::dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(m_participantName);
    }

    if (m_userData.size() > 0)
    {
        // PID_USER_DATA
        ret_val += fastdds::dds::QosPoliciesSerializer<UserDataQosPolicy>::cdr_serialized_size(m_userData);
    }

    if (m_properties.size() > 0)
    {
        // PID_PROPERTY_LIST
        ret_val += fastdds::dds::ParameterSerializer<ParameterPropertyList_t>::cdr_serialized_size(m_properties);
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
        ret_val += fastdds::dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(permissions_token_);
    }

    if ((security_attributes_ != 0UL) || (plugin_security_attributes_ != 0UL))
    {
        // PID_PARTICIPANT_SECURITY_INFO
        ret_val += 4 + PARAMETER_PARTICIPANT_SECURITY_INFO_LENGTH;
    }
#endif

    // PID_SENTINEL
    return ret_val + 4;
}

bool ParticipantProxyData::writeToCDRMessage(
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
        ParameterProtocolVersion_t p(fastdds::dds::PID_PROTOCOL_VERSION, 4);
        p.protocolVersion = this->m_protocolVersion;
        if (!fastdds::dds::ParameterSerializer<ParameterProtocolVersion_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterVendorId_t p(fastdds::dds::PID_VENDORID, 4);
        p.vendorId[0] = this->m_VendorId[0];
        p.vendorId[1] = this->m_VendorId[1];
        if (!fastdds::dds::ParameterSerializer<ParameterVendorId_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    if (this->m_expectsInlineQos)
    {
        ParameterBool_t p(fastdds::dds::PID_EXPECTS_INLINE_QOS, PARAMETER_BOOL_LENGTH, m_expectsInlineQos);
        if (!fastdds::dds::ParameterSerializer<ParameterBool_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterGuid_t p(fastdds::dds::PID_PARTICIPANT_GUID, PARAMETER_GUID_LENGTH, m_guid);
        if (!fastdds::dds::ParameterSerializer<ParameterGuid_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    for (const Locator_t& it : metatraffic_locators.multicast)
    {
        ParameterLocator_t p(fastdds::dds::PID_METATRAFFIC_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, it);
        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    for (const Locator_t& it : metatraffic_locators.unicast)
    {
        ParameterLocator_t p(fastdds::dds::PID_METATRAFFIC_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, it);
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
        p.time = m_leaseDuration;
        if (!fastdds::dds::ParameterSerializer<ParameterTime_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterBuiltinEndpointSet_t p(fastdds::dds::PID_BUILTIN_ENDPOINT_SET, PARAMETER_BUILTINENDPOINTSET_LENGTH);
        p.endpointSet = m_availableBuiltinEndpoints;
        if (!fastdds::dds::ParameterSerializer<ParameterBuiltinEndpointSet_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }

    if (m_participantName.size() > 0)
    {
        ParameterString_t p(fastdds::dds::PID_ENTITY_NAME, 0, m_participantName);
        if (!fastdds::dds::ParameterSerializer<ParameterString_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }

    if (m_userData.size() > 0)
    {
        if (!fastdds::dds::QosPoliciesSerializer<UserDataQosPolicy>::add_to_cdr_message(m_userData,
                msg))
        {
            return false;
        }
    }

    if (m_properties.size() > 0)
    {
        if (!fastdds::dds::ParameterSerializer<ParameterPropertyList_t>::add_to_cdr_message(m_properties, msg))
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
        if (!fastdds::dds::ParameterSerializer<ParameterParticipantSecurityInfo_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
#endif

    return fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(msg);
}

bool ParticipantProxyData::readFromCDRMessage(
        CDRMessage_t* msg,
        bool use_encapsulation,
        const NetworkFactory& network,
        bool is_shm_transport_available)
{
    bool are_shm_metatraffic_locators_present = false;
    bool are_shm_default_locators_present = false;
    bool is_shm_transport_possible = false;

    auto param_process = [this, &network, &is_shm_transport_possible,
                    &are_shm_metatraffic_locators_present,
                    &are_shm_default_locators_present,
                    &is_shm_transport_available](CDRMessage_t* msg, const ParameterId_t& pid, uint16_t plength)
            {
                switch (pid)
                {
                    case fastdds::dds::PID_KEY_HASH:
                    {
                        ParameterKey_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterKey_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        GUID_t guid;
                        iHandle2GUID(guid, p.key);
                        m_guid = guid;
                        m_key = p.key;
                        break;
                    }
                    case fastdds::dds::PID_PROTOCOL_VERSION:
                    {
                        ParameterProtocolVersion_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterProtocolVersion_t>::read_from_cdr_message(p,
                                msg, plength))
                        {
                            return false;
                        }

                        if (p.protocolVersion.m_major < c_ProtocolVersion.m_major)
                        {
                            return false;
                        }
                        m_protocolVersion = p.protocolVersion;
                        break;
                    }
                    case fastdds::dds::PID_VENDORID:
                    {
                        ParameterVendorId_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterVendorId_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        m_VendorId[0] = p.vendorId[0];
                        m_VendorId[1] = p.vendorId[1];
                        is_shm_transport_available &= (m_VendorId == c_VendorId_eProsima);
                        break;
                    }
                    case fastdds::dds::PID_EXPECTS_INLINE_QOS:
                    {
                        ParameterBool_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterBool_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        m_expectsInlineQos = p.value;
                        break;
                    }
                    case fastdds::dds::PID_PARTICIPANT_GUID:
                    {
                        ParameterGuid_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterGuid_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        m_guid = p.guid;
                        m_key = p.guid;
                        break;
                    }
                    case fastdds::dds::PID_METATRAFFIC_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        Locator_t temp_locator;
                        if (network.transform_remote_locator(p.locator, temp_locator))
                        {
                            ProxyDataFilters::filter_locators(
                                is_shm_transport_available,
                                &is_shm_transport_possible,
                                &are_shm_metatraffic_locators_present,
                                &metatraffic_locators,
                                temp_locator,
                                false);
                        }
                        break;
                    }
                    case fastdds::dds::PID_METATRAFFIC_UNICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        Locator_t temp_locator;
                        if (network.transform_remote_locator(p.locator, temp_locator))
                        {
                            ProxyDataFilters::filter_locators(
                                is_shm_transport_available,
                                &is_shm_transport_possible,
                                &are_shm_metatraffic_locators_present,
                                &metatraffic_locators,
                                temp_locator,
                                true);
                        }
                        break;
                    }
                    case fastdds::dds::PID_DEFAULT_UNICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        Locator_t temp_locator;
                        if (network.transform_remote_locator(p.locator, temp_locator))
                        {
                            ProxyDataFilters::filter_locators(
                                is_shm_transport_available,
                                &is_shm_transport_possible,
                                &are_shm_default_locators_present,
                                &default_locators,
                                temp_locator,
                                true);
                        }
                        break;
                    }
                    case fastdds::dds::PID_DEFAULT_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        Locator_t temp_locator;
                        if (network.transform_remote_locator(p.locator, temp_locator))
                        {
                            ProxyDataFilters::filter_locators(
                                is_shm_transport_available,
                                &is_shm_transport_possible,
                                &are_shm_default_locators_present,
                                &default_locators,
                                temp_locator,
                                false);
                        }
                        break;
                    }
                    case fastdds::dds::PID_PARTICIPANT_LEASE_DURATION:
                    {
                        ParameterTime_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterTime_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        m_leaseDuration = p.time.to_duration_t();
                        lease_duration_ =
                                std::chrono::microseconds(TimeConv::Duration_t2MicroSecondsInt64(
                                            m_leaseDuration));
                        break;
                    }
                    case fastdds::dds::PID_BUILTIN_ENDPOINT_SET:
                    {
                        ParameterBuiltinEndpointSet_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterBuiltinEndpointSet_t>::read_from_cdr_message(p,
                                msg, plength))
                        {
                            return false;
                        }

                        m_availableBuiltinEndpoints = p.endpointSet;
                        break;
                    }
                    case fastdds::dds::PID_ENTITY_NAME:
                    {
                        ParameterString_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterString_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        m_participantName = p.getName();
                        break;
                    }
                    case fastdds::dds::PID_PROPERTY_LIST:
                    {
                        if (!fastdds::dds::ParameterSerializer<ParameterPropertyList_t>::read_from_cdr_message(
                                    m_properties, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_USER_DATA:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<UserDataQosPolicy>::read_from_cdr_message(m_userData,
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
                        if (!fastdds::dds::ParameterSerializer<ParameterToken_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        identity_token_ = std::move(p.token);
#else
                        logWarning(RTPS_PARTICIPANT, "Received PID_IDENTITY_TOKEN but security is disabled");
#endif
                        break;
                    }
                    case fastdds::dds::PID_PERMISSIONS_TOKEN:
                    {
#if HAVE_SECURITY
                        ParameterToken_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterToken_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        permissions_token_ = std::move(p.token);
#else
                        logWarning(RTPS_PARTICIPANT, "Received PID_PERMISSIONS_TOKEN but security is disabled");
#endif
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
                        logWarning(RTPS_PARTICIPANT,
                                "Received PID_PARTICIPANT_SECURITY_INFO but security is disabled");
#endif
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
        return ParameterList::readParameterListfromCDRMsg(*msg, param_process, use_encapsulation, qos_size);
    }
    catch (std::bad_alloc& ba)
    {
        std::cerr << "bad_alloc caught: " << ba.what() << '\n';
        return false;
    }
}

void ParticipantProxyData::clear()
{
    m_protocolVersion = ProtocolVersion_t();
    m_guid = GUID_t();
    //set_VendorId_Unknown(m_VendorId);
    m_VendorId = c_VendorId_Unknown;
    m_expectsInlineQos = false;
    m_availableBuiltinEndpoints = 0;
    metatraffic_locators.unicast.clear();
    metatraffic_locators.multicast.clear();
    default_locators.unicast.clear();
    default_locators.multicast.clear();
    m_participantName = "";
    m_key = InstanceHandle_t();
    m_leaseDuration = Duration_t();
    lease_duration_ = std::chrono::microseconds::zero();
    isAlive = true;
#if HAVE_SECURITY
    identity_token_ = IdentityToken();
    permissions_token_ = PermissionsToken();
    security_attributes_ = 0UL;
    plugin_security_attributes_ = 0UL;
#endif
    m_properties.clear();
    m_properties.length = 0;
    m_userData.clear();
    m_userData.length = 0;
}

void ParticipantProxyData::copy(
        const ParticipantProxyData& pdata)
{
    m_protocolVersion = pdata.m_protocolVersion;
    m_guid = pdata.m_guid;
    m_VendorId[0] = pdata.m_VendorId[0];
    m_VendorId[1] = pdata.m_VendorId[1];
    m_availableBuiltinEndpoints = pdata.m_availableBuiltinEndpoints;
    metatraffic_locators = pdata.metatraffic_locators;
    default_locators = pdata.default_locators;
    m_participantName = pdata.m_participantName;
    m_leaseDuration = pdata.m_leaseDuration;
    lease_duration_ = std::chrono::microseconds(TimeConv::Duration_t2MicroSecondsInt64(pdata.m_leaseDuration));
    m_key = pdata.m_key;
    isAlive = pdata.isAlive;
    m_userData = pdata.m_userData;
    m_properties = pdata.m_properties;

    // This method is only called when a new participant is discovered.The destination of the copy
    // will always be a new ParticipantProxyData or one from the pool, so there is no need for
    // m_readers and m_writers to be copied

#if HAVE_SECURITY
    identity_token_ = pdata.identity_token_;
    permissions_token_ = pdata.permissions_token_;
    security_attributes_ = pdata.security_attributes_;
    plugin_security_attributes_ = pdata.plugin_security_attributes_;
#endif
}

bool ParticipantProxyData::updateData(
        ParticipantProxyData& pdata)
{
    metatraffic_locators = pdata.metatraffic_locators;
    default_locators = pdata.default_locators;
    m_leaseDuration = pdata.m_leaseDuration;
    isAlive = true;
    m_userData = pdata.m_userData;
    m_properties = pdata.m_properties;
#if HAVE_SECURITY
    identity_token_ = pdata.identity_token_;
    permissions_token_ = pdata.permissions_token_;
    security_attributes_ = pdata.security_attributes_;
    plugin_security_attributes_ = pdata.plugin_security_attributes_;
#endif
    auto new_lease_duration = std::chrono::microseconds(TimeConv::Duration_t2MicroSecondsInt64(m_leaseDuration));
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
        const GUID_t& guid)
{
    // only valid values
    if (guid == c_Guid_Unknown)
    {
        return;
    }

    // generate pair
    std::pair<std::string, std::string> persistent_guid;
    persistent_guid.first = "PID_PERSISTENCE_GUID";

    std::ostringstream data;
    data << guid;
    persistent_guid.second = data.str();

    // if exists replace
    ParameterPropertyList_t::iterator it = std::find_if(
        m_properties.begin(),
        m_properties.end(),
        [&persistent_guid](const fastdds::dds::ParameterProperty_t& p)
                {
                    return persistent_guid.first == p.first();
                });

    if (it != m_properties.end())
    {
        if (!it->modify(persistent_guid))
        {
            logError(RTPS_PARTICIPANT, "Failed to change property <"
                    << it->first() << " | " << it->second() << "> to <"
                    << persistent_guid.first << " | " << persistent_guid.second << ">");
        }
    }
    else
    {
        // if not exists add
        m_properties.push_back(persistent_guid);
    }
}

GUID_t ParticipantProxyData::get_persistence_guid() const
{
    GUID_t persistent(c_Guid_Unknown);

    ParameterPropertyList_t::const_iterator it = std::find_if(
        m_properties.begin(),
        m_properties.end(),
        [](const fastdds::dds::ParameterProperty_t p)
                {
                    return "PID_PERSISTENCE_GUID" == p.first();
                });

    if (it != m_properties.end())
    {
        std::istringstream in(it->second());
        in >> persistent;
    }

    return persistent;
}

void ParticipantProxyData::set_sample_identity(
        const SampleIdentity& sid)
{
    fastdds::dds::set_proxy_property(sid, "PID_CLIENT_SERVER_KEY", m_properties);
}

SampleIdentity ParticipantProxyData::get_sample_identity() const
{
    return fastdds::dds::get_proxy_property<SampleIdentity>("PID_CLIENT_SERVER_KEY", m_properties);
}

void ParticipantProxyData::set_backup_stamp(const GUID_t& guid)
{
    fastdds::dds::set_proxy_property(guid,"PID_BACKUP_STAMP", m_properties);
}

GUID_t ParticipantProxyData::get_backup_stamp() const
{
    return fastdds::dds::get_proxy_property<GUID_t>("PID_BACKUP_STAMP", m_properties);
}

void ParticipantProxyData::assert_liveliness()
{
    last_received_message_tm_ = std::chrono::steady_clock::now();
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
