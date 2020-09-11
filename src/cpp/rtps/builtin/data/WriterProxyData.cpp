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
 * @file WriterProxyData.cpp
 *
 */

#include <fastdds/rtps/builtin/data/WriterProxyData.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/network/NetworkFactory.h>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/core/policy/QosPoliciesSerializer.hpp>

#include "ProxyDataFilters.hpp"

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastrtps {
namespace rtps {

WriterProxyData::WriterProxyData(
        const size_t max_unicast_locators,
        const size_t max_multicast_locators)
#if HAVE_SECURITY
    : security_attributes_(0)
    , plugin_security_attributes_(0)
    , remote_locators_(max_unicast_locators, max_multicast_locators)
#else
    : remote_locators_(max_unicast_locators, max_multicast_locators)
#endif // if HAVE_SECURITY
    , m_userDefinedId(0)
    , m_typeMaxSerialized(0)
    , m_topicKind(NO_KEY)
    , m_type_id(nullptr)
    , m_type(nullptr)
    , m_type_information(nullptr)
{
}

WriterProxyData::WriterProxyData(
        const size_t max_unicast_locators,
        const size_t max_multicast_locators,
        const VariableLengthDataLimits& data_limits)
    : WriterProxyData(max_unicast_locators, max_multicast_locators)
{
    m_qos.m_userData.set_max_size(static_cast<uint32_t>(data_limits.max_user_data));
    m_qos.m_partition.set_max_size(static_cast<uint32_t>(data_limits.max_partitions));
    m_properties.set_max_size(static_cast<uint32_t>(data_limits.max_properties));
}

WriterProxyData::WriterProxyData(
        const WriterProxyData& writerInfo)
#if HAVE_SECURITY
    : security_attributes_(writerInfo.security_attributes_)
    , plugin_security_attributes_(writerInfo.plugin_security_attributes_)
    , m_guid(writerInfo.m_guid)
#else
    : m_guid(writerInfo.m_guid)
#endif // if HAVE_SECURITY
    , remote_locators_(writerInfo.remote_locators_)
    , m_key(writerInfo.m_key)
    , m_RTPSParticipantKey(writerInfo.m_RTPSParticipantKey)
    , m_typeName(writerInfo.m_typeName)
    , m_topicName(writerInfo.m_topicName)
    , m_userDefinedId(writerInfo.m_userDefinedId)
    , m_typeMaxSerialized(writerInfo.m_typeMaxSerialized)
    , m_topicKind(writerInfo.m_topicKind)
    , persistence_guid_(writerInfo.persistence_guid_)
    , m_type_id(nullptr)
    , m_type(nullptr)
    , m_type_information(nullptr)
    , m_properties(writerInfo.m_properties)
{
    if (writerInfo.m_type_id)
    {
        type_id(*writerInfo.m_type_id);
    }

    if (writerInfo.m_type)
    {
        type(*writerInfo.m_type);
    }

    if (writerInfo.m_type_information)
    {
        type_information(*writerInfo.m_type_information);
    }

    m_qos.setQos(writerInfo.m_qos, true);
}

WriterProxyData::~WriterProxyData()
{
    delete m_type;
    delete m_type_id;
    delete m_type_information;

    logInfo(RTPS_PROXY_DATA, m_guid);
}

WriterProxyData& WriterProxyData::operator =(
        const WriterProxyData& writerInfo)
{
#if HAVE_SECURITY
    security_attributes_ = writerInfo.security_attributes_;
    plugin_security_attributes_ = writerInfo.plugin_security_attributes_;
#endif // if HAVE_SECURITY
    m_guid = writerInfo.m_guid;
    remote_locators_ = writerInfo.remote_locators_;
    m_key = writerInfo.m_key;
    m_RTPSParticipantKey = writerInfo.m_RTPSParticipantKey;
    m_typeName = writerInfo.m_typeName;
    m_topicName = writerInfo.m_topicName;
    m_userDefinedId = writerInfo.m_userDefinedId;
    m_typeMaxSerialized = writerInfo.m_typeMaxSerialized;
    m_topicKind = writerInfo.m_topicKind;
    persistence_guid_ = writerInfo.persistence_guid_;
    m_qos.setQos(writerInfo.m_qos, true);
    m_properties = writerInfo.m_properties;

    if (writerInfo.m_type_id)
    {
        type_id(*writerInfo.m_type_id);
    }
    else
    {
        delete m_type_id;
        m_type_id = nullptr;
    }

    if (writerInfo.m_type)
    {
        type(*writerInfo.m_type);
    }
    else
    {
        delete m_type;
        m_type = nullptr;
    }

    if (writerInfo.m_type_information)
    {
        type_information(*writerInfo.m_type_information);
    }
    else
    {
        delete m_type_information;
        m_type_information = nullptr;
    }

    return *this;
}

uint32_t WriterProxyData::get_serialized_size(
        bool include_encapsulation) const
{
    uint32_t ret_val = include_encapsulation ? 4 : 0;

    // PID_UNICAST_LOCATOR
    ret_val += static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * remote_locators_.unicast.size());

    // PID_MULTICAST_LOCATOR
    ret_val += static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * remote_locators_.multicast.size());

    // PID_PARTICIPANT_GUID
    ret_val += 4 + PARAMETER_GUID_LENGTH;

    // PID_TOPIC_NAME
    ret_val += fastdds::dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(m_topicName);

    // PID_TYPE_NAME
    ret_val += fastdds::dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(m_typeName);

    // PID_KEY_HASH
    ret_val += 4 + 16;

    // PID_ENDPOINT_GUID
    ret_val += 4 + PARAMETER_GUID_LENGTH;

    // PID_TYPE_MAX_SIZE_SERIALIZED
    ret_val += 4 + 4;

    // PID_PROTOCOL_VERSION
    ret_val += 4 + 4;

    // PID_VENDORID
    ret_val += 4 + 4;

    if (persistence_guid_ != c_Guid_Unknown)
    {
        // PID_PERSISTENCE_GUID
        ret_val += 4 + PARAMETER_GUID_LENGTH;
    }
    if (m_qos.m_durability.send_always() || m_qos.m_durability.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<DurabilityQosPolicy>::cdr_serialized_size(m_qos.m_durability);
    }
    if (m_qos.m_durabilityService.send_always() || m_qos.m_durabilityService.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<DurabilityServiceQosPolicy>::cdr_serialized_size(
            m_qos.m_durabilityService);
    }
    if (m_qos.m_deadline.send_always() || m_qos.m_deadline.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<DeadlineQosPolicy>::cdr_serialized_size(m_qos.m_deadline);
    }
    if (m_qos.m_latencyBudget.send_always() || m_qos.m_latencyBudget.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<LatencyBudgetQosPolicy>::cdr_serialized_size(
            m_qos.m_latencyBudget);
    }
    if (m_qos.m_liveliness.send_always() || m_qos.m_liveliness.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<LivelinessQosPolicy>::cdr_serialized_size(m_qos.m_liveliness);
    }
    if (m_qos.m_reliability.send_always() || m_qos.m_reliability.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<ReliabilityQosPolicy>::cdr_serialized_size(m_qos.m_reliability);
    }
    if (m_qos.m_lifespan.send_always() || m_qos.m_lifespan.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<LifespanQosPolicy>::cdr_serialized_size(m_qos.m_lifespan);
    }
    if (m_qos.m_userData.send_always() || m_qos.m_userData.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<UserDataQosPolicy>::cdr_serialized_size(m_qos.m_userData);
    }
    if (m_qos.m_timeBasedFilter.send_always() || m_qos.m_timeBasedFilter.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<TimeBasedFilterQosPolicy>::cdr_serialized_size(
            m_qos.m_timeBasedFilter);
    }
    if (m_qos.m_ownership.send_always() || m_qos.m_ownership.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<OwnershipQosPolicy>::cdr_serialized_size(m_qos.m_ownership);
    }
    if (m_qos.m_ownershipStrength.send_always() || m_qos.m_ownershipStrength.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<OwnershipStrengthQosPolicy>::cdr_serialized_size(
            m_qos.m_ownershipStrength);
    }
    if (m_qos.m_destinationOrder.send_always() || m_qos.m_destinationOrder.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<DestinationOrderQosPolicy>::cdr_serialized_size(
            m_qos.m_destinationOrder);
    }
    if (m_qos.m_presentation.send_always() || m_qos.m_presentation.hasChanged)
    {
        ret_val +=
                fastdds::dds::QosPoliciesSerializer<PresentationQosPolicy>::cdr_serialized_size(m_qos.m_presentation);
    }
    if (m_qos.m_partition.send_always() || m_qos.m_partition.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<PartitionQosPolicy>::cdr_serialized_size(m_qos.m_partition);
    }
    if (m_qos.m_topicData.send_always() || m_qos.m_topicData.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<TopicDataQosPolicy>::cdr_serialized_size(m_qos.m_topicData);
    }
    if (m_qos.m_disablePositiveACKs.send_always() || m_qos.m_disablePositiveACKs.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<DisablePositiveACKsQosPolicy>::cdr_serialized_size(
            m_qos.m_disablePositiveACKs);
    }
    if (m_qos.m_groupData.send_always() || m_qos.m_groupData.hasChanged)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<GroupDataQosPolicy>::cdr_serialized_size(m_qos.m_groupData);
    }
    if (m_type_id && m_type_id->m_type_identifier._d() != 0)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<TypeIdV1>::cdr_serialized_size(*m_type_id);
    }
    if (m_type && m_type->m_type_object._d() != 0)
    {
        ret_val += fastdds::dds::QosPoliciesSerializer<TypeObjectV1>::cdr_serialized_size(*m_type);
    }
    if (m_type_information && m_type_information->assigned())
    {
        ret_val +=
                fastdds::dds::QosPoliciesSerializer<xtypes::TypeInformation>::cdr_serialized_size(*m_type_information);
    }

    if (m_properties.size() > 0)
    {
        // PID_PROPERTY_LIST
        ret_val += fastdds::dds::ParameterSerializer<ParameterPropertyList_t>::cdr_serialized_size(m_properties);
    }

#if HAVE_SECURITY
    if ((this->security_attributes_ != 0UL) || (this->plugin_security_attributes_ != 0UL))
    {
        ret_val += 4 + PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH;
    }
#endif // if HAVE_SECURITY

    // PID_SENTINEL
    return ret_val + 4;
}

bool WriterProxyData::writeToCDRMessage(
        CDRMessage_t* msg,
        bool write_encapsulation) const
{
    if (write_encapsulation)
    {
        if (!ParameterList::writeEncapsulationToCDRMsg(msg))
        {
            return false;
        }
    }

    for (const Locator_t& locator : remote_locators_.unicast)
    {
        ParameterLocator_t p(fastdds::dds::PID_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, locator);
        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    for (const Locator_t& locator : remote_locators_.multicast)
    {
        ParameterLocator_t p(fastdds::dds::PID_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, locator);
        if (!fastdds::dds::ParameterSerializer<ParameterLocator_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterGuid_t p(fastdds::dds::PID_PARTICIPANT_GUID, PARAMETER_GUID_LENGTH, m_RTPSParticipantKey);
        if (!fastdds::dds::ParameterSerializer<ParameterGuid_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterString_t p(fastdds::dds::PID_TOPIC_NAME, 0, m_topicName);
        if (!fastdds::dds::ParameterSerializer<ParameterString_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterString_t p(fastdds::dds::PID_TYPE_NAME, 0, m_typeName);
        if (!fastdds::dds::ParameterSerializer<ParameterString_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterKey_t p(fastdds::dds::PID_KEY_HASH, 16, m_key);
        if (!fastdds::dds::ParameterSerializer<ParameterKey_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterGuid_t p(fastdds::dds::PID_ENDPOINT_GUID, 16, m_guid);
        if (!fastdds::dds::ParameterSerializer<ParameterGuid_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterPort_t p(fastdds::dds::PID_TYPE_MAX_SIZE_SERIALIZED, 4, m_typeMaxSerialized);
        if (!fastdds::dds::ParameterSerializer<ParameterPort_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterProtocolVersion_t p(fastdds::dds::PID_PROTOCOL_VERSION, 4);
        if (!fastdds::dds::ParameterSerializer<ParameterProtocolVersion_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterVendorId_t p(fastdds::dds::PID_VENDORID, 4);
        if (!fastdds::dds::ParameterSerializer<ParameterVendorId_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    if (persistence_guid_ != c_Guid_Unknown)
    {
        ParameterGuid_t p(fastdds::dds::PID_PERSISTENCE_GUID, 16, persistence_guid_);
        if (!fastdds::dds::ParameterSerializer<ParameterGuid_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    if ( m_qos.m_durability.send_always() || m_qos.m_durability.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<DurabilityQosPolicy>::add_to_cdr_message(m_qos.m_durability, msg))
        {
            return false;
        }
    }
    if (m_qos.m_durabilityService.send_always() || m_qos.m_durabilityService.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<DurabilityServiceQosPolicy>::add_to_cdr_message(m_qos.
                m_durabilityService, msg))
        {
            return false;
        }
    }
    if (m_qos.m_deadline.send_always() ||  m_qos.m_deadline.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<DeadlineQosPolicy>::add_to_cdr_message(m_qos.m_deadline, msg))
        {
            return false;
        }
    }
    if (m_qos.m_latencyBudget.send_always() ||  m_qos.m_latencyBudget.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<LatencyBudgetQosPolicy>::add_to_cdr_message(m_qos.m_latencyBudget,
                msg))
        {
            return false;
        }
    }
    if (m_qos.m_liveliness.send_always() ||  m_qos.m_liveliness.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<LivelinessQosPolicy>::add_to_cdr_message(m_qos.m_liveliness, msg))
        {
            return false;
        }
    }
    if (m_qos.m_reliability.send_always() ||  m_qos.m_reliability.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<ReliabilityQosPolicy>::add_to_cdr_message(m_qos.m_reliability, msg))
        {
            return false;
        }
    }
    if (m_qos.m_lifespan.send_always() ||  m_qos.m_lifespan.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<LifespanQosPolicy>::add_to_cdr_message(m_qos.m_lifespan, msg))
        {
            return false;
        }
    }
    if ( m_qos.m_userData.send_always() || m_qos.m_userData.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<UserDataQosPolicy>::add_to_cdr_message(m_qos.m_userData, msg))
        {
            return false;
        }
    }
    if (m_qos.m_timeBasedFilter.send_always() ||  m_qos.m_timeBasedFilter.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<TimeBasedFilterQosPolicy>::add_to_cdr_message(m_qos.m_timeBasedFilter,
                msg))
        {
            return false;
        }
    }
    if (m_qos.m_ownership.send_always() ||  m_qos.m_ownership.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<OwnershipQosPolicy>::add_to_cdr_message(m_qos.m_ownership, msg))
        {
            return false;
        }
    }
    if (m_qos.m_ownershipStrength.send_always() ||  m_qos.m_ownershipStrength.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<OwnershipStrengthQosPolicy>::add_to_cdr_message(m_qos.
                m_ownershipStrength, msg))
        {
            return false;
        }
    }
    if (m_qos.m_destinationOrder.send_always() ||  m_qos.m_destinationOrder.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<DestinationOrderQosPolicy>::add_to_cdr_message(m_qos.m_destinationOrder,
                msg))
        {
            return false;
        }
    }
    if (m_qos.m_presentation.send_always() ||  m_qos.m_presentation.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<PresentationQosPolicy>::add_to_cdr_message(m_qos.m_presentation, msg))
        {
            return false;
        }
    }
    if (m_qos.m_partition.send_always() ||  m_qos.m_partition.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<PartitionQosPolicy>::add_to_cdr_message(m_qos.m_partition, msg))
        {
            return false;
        }
    }
    if (m_qos.m_topicData.send_always() || m_qos.m_topicData.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<TopicDataQosPolicy>::add_to_cdr_message(m_qos.m_topicData, msg))
        {
            return false;
        }
    }
    if ((m_qos.m_disablePositiveACKs.send_always() || m_qos.m_topicData.hasChanged) &&
            m_qos.m_disablePositiveACKs.enabled)
    {
        if (!fastdds::dds::QosPoliciesSerializer<DisablePositiveACKsQosPolicy>::add_to_cdr_message(m_qos.
                m_disablePositiveACKs, msg))
        {
            return false;
        }
    }
    if (m_qos.m_groupData.send_always() ||  m_qos.m_groupData.hasChanged)
    {
        if (!fastdds::dds::QosPoliciesSerializer<GroupDataQosPolicy>::add_to_cdr_message(m_qos.m_groupData, msg))
        {
            return false;
        }
    }

    if (m_type_id && m_type_id->m_type_identifier._d() != 0)
    {
        if (!fastdds::dds::QosPoliciesSerializer<TypeIdV1>::add_to_cdr_message(*m_type_id, msg))
        {
            return false;
        }
    }

    if (m_type && m_type->m_type_object._d() != 0)
    {
        if (!fastdds::dds::QosPoliciesSerializer<TypeObjectV1>::add_to_cdr_message(*m_type, msg))
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
    if ((security_attributes_ != 0UL) || (plugin_security_attributes_ != 0UL))
    {
        ParameterEndpointSecurityInfo_t p;
        p.security_attributes = security_attributes_;
        p.plugin_security_attributes = plugin_security_attributes_;
        if (!fastdds::dds::ParameterSerializer<ParameterEndpointSecurityInfo_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
#endif // if HAVE_SECURITY

    /* TODO - Enable when implement XCDR, XCDR2 and/or XML
       if (m_qos.representation.send_always() || m_qos.representation.hasChanged)
       {
        if (!m_qos.representation.addToCDRMessage(msg)) return false;
       }
     */

    if (m_type_information && m_type_information->assigned())
    {
        if (!fastdds::dds::QosPoliciesSerializer<xtypes::TypeInformation>::add_to_cdr_message(*m_type_information, msg))
        {
            return false;
        }
    }

    return fastdds::dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(msg);
}

bool WriterProxyData::readFromCDRMessage(
        CDRMessage_t* msg,
        const NetworkFactory& network,
        bool is_shm_transport_available)
{
    bool are_shm_default_locators_present = false;
    bool is_shm_transport_possible = false;

    auto param_process = [this, &network,
                    &is_shm_transport_available,
                    &is_shm_transport_possible,
                    &are_shm_default_locators_present](CDRMessage_t* msg, const ParameterId_t& pid, uint16_t plength)
            {
                switch (pid)
                {
                    case fastdds::dds::PID_VENDORID:
                    {
                        ParameterVendorId_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterVendorId_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        is_shm_transport_available &= (p.vendorId == c_VendorId_eProsima);
                        break;
                    }
                    case fastdds::dds::PID_DURABILITY:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<DurabilityQosPolicy>::read_from_cdr_message(m_qos.
                                m_durability, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_DURABILITY_SERVICE:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<DurabilityServiceQosPolicy>::read_from_cdr_message(
                                    m_qos.m_durabilityService, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_DEADLINE:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<DeadlineQosPolicy>::read_from_cdr_message(m_qos.
                                m_deadline, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_LATENCY_BUDGET:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<LatencyBudgetQosPolicy>::read_from_cdr_message(m_qos.
                                m_latencyBudget, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_LIVELINESS:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<LivelinessQosPolicy>::read_from_cdr_message(m_qos.
                                m_liveliness, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_RELIABILITY:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<ReliabilityQosPolicy>::read_from_cdr_message(m_qos.
                                m_reliability, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_LIFESPAN:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<LifespanQosPolicy>::read_from_cdr_message(m_qos.
                                m_lifespan, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_USER_DATA:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<UserDataQosPolicy>::read_from_cdr_message(m_qos.
                                m_userData, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TIME_BASED_FILTER:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<TimeBasedFilterQosPolicy>::read_from_cdr_message(m_qos.
                                m_timeBasedFilter, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_OWNERSHIP:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<OwnershipQosPolicy>::read_from_cdr_message(m_qos.
                                m_ownership, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_OWNERSHIP_STRENGTH:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<OwnershipStrengthQosPolicy>::read_from_cdr_message(
                                    m_qos.m_ownershipStrength, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_DESTINATION_ORDER:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<DestinationOrderQosPolicy>::read_from_cdr_message(m_qos
                                .m_destinationOrder, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_PRESENTATION:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<PresentationQosPolicy>::read_from_cdr_message(m_qos.
                                m_presentation, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_PARTITION:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<PartitionQosPolicy>::read_from_cdr_message(m_qos.
                                m_partition, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TOPIC_DATA:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<TopicDataQosPolicy>::read_from_cdr_message(
                                    m_qos.m_topicData, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_GROUP_DATA:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<GroupDataQosPolicy>::read_from_cdr_message(m_qos.
                                m_groupData, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TOPIC_NAME:
                    {
                        ParameterString_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterString_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        m_topicName = p.getName();
                        break;
                    }
                    case fastdds::dds::PID_TYPE_NAME:
                    {
                        ParameterString_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterString_t>::read_from_cdr_message(p, msg,
                                plength))
                        {
                            return false;
                        }

                        m_typeName = p.getName();
                        break;
                    }
                    case fastdds::dds::PID_PARTICIPANT_GUID:
                    {
                        ParameterGuid_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterGuid_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        memcpy(m_RTPSParticipantKey.value, p.guid.guidPrefix.value, 12);
                        memcpy(m_RTPSParticipantKey.value + 12, p.guid.entityId.value, 4);
                        break;
                    }
                    case fastdds::dds::PID_ENDPOINT_GUID:
                    {
                        ParameterGuid_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterGuid_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        m_guid = p.guid;
                        memcpy(m_key.value, p.guid.guidPrefix.value, 12);
                        memcpy(m_key.value + 12, p.guid.entityId.value, 4);
                        break;
                    }
                    case fastdds::dds::PID_PERSISTENCE_GUID:
                    {
                        ParameterGuid_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterGuid_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        persistence_guid_ = p.guid;
                        break;
                    }
                    case fastdds::dds::PID_UNICAST_LOCATOR:
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
                                &remote_locators_,
                                temp_locator,
                                true);
                        }
                        break;
                    }
                    case fastdds::dds::PID_MULTICAST_LOCATOR:
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
                                &remote_locators_,
                                temp_locator,
                                false);
                        }
                        break;
                    }
                    case fastdds::dds::PID_KEY_HASH:
                    {
                        ParameterKey_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterKey_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        m_key = p.key;
                        iHandle2GUID(m_guid, m_key);
                        break;
                    }
                    case fastdds::dds::PID_TYPE_IDV1:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<TypeIdV1>::read_from_cdr_message(type_id(), msg,
                                plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TYPE_OBJECTV1:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<TypeObjectV1>::read_from_cdr_message(type(), msg,
                                plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TYPE_INFORMATION:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<xtypes::TypeInformation>::read_from_cdr_message(
                                    type_information(), msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_DISABLE_POSITIVE_ACKS:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<DisablePositiveACKsQosPolicy>::read_from_cdr_message(
                                    m_qos.m_disablePositiveACKs, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
#if HAVE_SECURITY
                    case fastdds::dds::PID_ENDPOINT_SECURITY_INFO:
                    {
                        ParameterEndpointSecurityInfo_t p(pid, plength);
                        if (!fastdds::dds::ParameterSerializer<ParameterEndpointSecurityInfo_t>::read_from_cdr_message(p,
                                msg, plength))
                        {
                            return false;
                        }

                        security_attributes_ = p.security_attributes;
                        plugin_security_attributes_ = p.plugin_security_attributes;
                        break;
                    }
#endif // if HAVE_SECURITY
                    case fastdds::dds::PID_DATA_REPRESENTATION:
                    {
                        if (!fastdds::dds::QosPoliciesSerializer<DataRepresentationQosPolicy>::read_from_cdr_message(
                                    m_qos.representation, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TYPE_CONSISTENCY_ENFORCEMENT:
                    {
                        logError(RTPS_PROXY_DATA,
                                "Received TypeConsistencyEnforcementQos from a writer, but they haven't.");
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
        if (ParameterList::readParameterListfromCDRMsg(*msg, param_process, true, qos_size))
        {
            if (m_guid.entityId.value[3] == 0x03)
            {
                m_topicKind = NO_KEY;
            }
            else if (m_guid.entityId.value[3] == 0x02)
            {
                m_topicKind = WITH_KEY;
            }
            return true;
        }
    }
    catch (std::bad_alloc& ba)
    {
        std::cerr << "bad_alloc caught: " << ba.what() << '\n';
    }

    return false;
}

void WriterProxyData::clear()
{
    m_guid = c_Guid_Unknown;
    remote_locators_.unicast.clear();
    remote_locators_.multicast.clear();
    m_key = InstanceHandle_t();
    m_RTPSParticipantKey = InstanceHandle_t();
    m_typeName = "";
    m_topicName = "";
    m_userDefinedId = 0;
    m_qos.clear();
    m_typeMaxSerialized = 0;
    m_topicKind = NO_KEY;
    persistence_guid_ = c_Guid_Unknown;
    m_properties.clear();
    m_properties.length = 0;

    if (m_type_id)
    {
        *m_type_id = TypeIdV1();
    }
    if (m_type)
    {
        *m_type = TypeObjectV1();
    }
    if (m_type_information)
    {
        *m_type_information = xtypes::TypeInformation();
    }
}

void WriterProxyData::copy(
        WriterProxyData* wdata)
{
    m_guid = wdata->m_guid;
    remote_locators_ = wdata->remote_locators_;
    m_key = wdata->m_key;
    m_RTPSParticipantKey = wdata->m_RTPSParticipantKey;
    m_typeName = wdata->m_typeName;
    m_topicName = wdata->m_topicName;
    m_userDefinedId = wdata->m_userDefinedId;
    m_qos = wdata->m_qos;
    m_typeMaxSerialized = wdata->m_typeMaxSerialized;
    m_topicKind = wdata->m_topicKind;
    persistence_guid_ = wdata->persistence_guid_;
    m_properties = wdata->m_properties;

    if (wdata->m_type_id)
    {
        type_id(*wdata->m_type_id);
    }
    else
    {
        delete m_type_id;
        m_type_id = nullptr;
    }

    if (wdata->m_type)
    {
        type(*wdata->m_type);
    }
    else
    {
        delete m_type;
        m_type = nullptr;
    }

    if (wdata->m_type_information)
    {
        type_information(*wdata->m_type_information);
    }
    else
    {
        delete m_type_information;
        m_type_information = nullptr;
    }
}

bool WriterProxyData::is_update_allowed(
        const WriterProxyData& wdata) const
{
    if ((m_guid != wdata.m_guid) ||
            (persistence_guid_ != wdata.persistence_guid_) ||
#if HAVE_SECURITY
            (security_attributes_ != wdata.security_attributes_) ||
            (plugin_security_attributes_ != wdata.security_attributes_) ||
#endif // if HAVE_SECURITY
            (m_typeName != wdata.m_typeName) ||
            (m_topicName != wdata.m_topicName))
    {
        return false;
    }

    return m_qos.canQosBeUpdated(wdata.m_qos);
}

void WriterProxyData::update(
        WriterProxyData* wdata)
{
    remote_locators_ = wdata->remote_locators_;
    m_qos.setQos(wdata->m_qos, false);
}

void WriterProxyData::add_unicast_locator(
        const Locator_t& locator)
{
    remote_locators_.add_unicast_locator(locator);
}

void WriterProxyData::set_announced_unicast_locators(
        const LocatorList_t& locators)
{
    remote_locators_.unicast.clear();
    for (const Locator_t& locator : locators)
    {
        remote_locators_.add_unicast_locator(locator);
    }
}

void WriterProxyData::set_remote_unicast_locators(
        const LocatorList_t& locators,
        const NetworkFactory& network)
{
    Locator_t local_locator;
    remote_locators_.unicast.clear();
    for (const Locator_t& locator : locators)
    {
        if (network.transform_remote_locator(locator, local_locator))
        {
            remote_locators_.add_unicast_locator(local_locator);
        }
    }
}

void WriterProxyData::add_multicast_locator(
        const Locator_t& locator)
{
    remote_locators_.add_multicast_locator(locator);
}

void WriterProxyData::set_multicast_locators(
        const LocatorList_t& locators,
        const NetworkFactory& network)
{
    Locator_t local_locator;
    remote_locators_.multicast.clear();
    for (const Locator_t& locator : locators)
    {
        if (network.transform_remote_locator(locator, local_locator))
        {
            remote_locators_.add_multicast_locator(locator);
        }
    }
}

void WriterProxyData::set_locators(
        const RemoteLocatorList& locators)
{
    remote_locators_ = locators;
}

void WriterProxyData::set_remote_locators(
        const RemoteLocatorList& locators,
        const NetworkFactory& network,
        bool use_multicast_locators)
{
    Locator_t local_locator;
    remote_locators_.unicast.clear();
    remote_locators_.multicast.clear();

    for (const Locator_t& locator : locators.unicast)
    {
        if (network.transform_remote_locator(locator, local_locator))
        {
            remote_locators_.add_unicast_locator(local_locator);
        }
    }

    if (use_multicast_locators)
    {
        for (const Locator_t& locator : locators.multicast)
        {
            if (network.transform_remote_locator(locator, local_locator))
            {
                remote_locators_.add_multicast_locator(locator);
            }
        }
    }
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */
