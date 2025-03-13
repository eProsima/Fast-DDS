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

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/core/policy/QosPoliciesSerializer.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/network/NetworkFactory.hpp>
#include <utils/BuiltinTopicKeyConversions.hpp>
#include <utils/SystemInfo.hpp>

#include "ProxyDataFilters.hpp"

using ParameterList = eprosima::fastdds::dds::ParameterList;

namespace eprosima {
namespace fastdds {
namespace rtps {

using ::operator <<;

WriterProxyData::WriterProxyData(
        const size_t max_unicast_locators,
        const size_t max_multicast_locators,
        const VariableLengthDataLimits& data_limits)
    : PublicationBuiltinTopicData(max_unicast_locators, max_multicast_locators, data_limits)
{
    init(data_limits);
}

WriterProxyData::WriterProxyData(
        const size_t max_unicast_locators,
        const size_t max_multicast_locators)
    : WriterProxyData(max_unicast_locators, max_multicast_locators, VariableLengthDataLimits())
{
}

WriterProxyData::WriterProxyData(
        const VariableLengthDataLimits& data_limits,
        const PublicationBuiltinTopicData& publication_data)
    : PublicationBuiltinTopicData(publication_data)
{
    init(data_limits);
}

WriterProxyData::WriterProxyData(
        const WriterProxyData& writerInfo)
    : PublicationBuiltinTopicData(writerInfo)
#if HAVE_SECURITY
    , security_attributes_(writerInfo.security_attributes_)
    , plugin_security_attributes_(writerInfo.plugin_security_attributes_)
#endif // if HAVE_SECURITY
    , m_network_configuration(writerInfo.m_network_configuration)
    , m_key(writerInfo.m_key)
    , m_rtps_participant_key(writerInfo.m_rtps_participant_key)
    , m_user_defined_id(writerInfo.m_user_defined_id)
    , m_type_id(nullptr)
    , m_type(nullptr)
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

    type_information = writerInfo.type_information;
}

WriterProxyData::~WriterProxyData()
{
    if (nullptr != m_type)
    {
        delete m_type;
    }
    if (nullptr != m_type_id)
    {
        delete m_type_id;
    }

    EPROSIMA_LOG_INFO(RTPS_PROXY_DATA, PublicationBuiltinTopicData::guid);
}

WriterProxyData& WriterProxyData::operator =(
        const WriterProxyData& writerInfo)
{
    PublicationBuiltinTopicData::key = writerInfo.PublicationBuiltinTopicData::key;
    participant_key = writerInfo.participant_key;
    type_name = writerInfo.type_name;
    topic_name = writerInfo.topic_name;
    topic_kind = writerInfo.topic_kind;

    set_qos(writerInfo, true);

    if (writerInfo.has_type_information())
    {
        type_information = writerInfo.type_information;
    }

    guid = writerInfo.guid;
    participant_guid = writerInfo.participant_guid;
    persistence_guid = writerInfo.persistence_guid;
    loopback_transformation = writerInfo.loopback_transformation;
    remote_locators = writerInfo.remote_locators;
    max_serialized_size = writerInfo.max_serialized_size;

    m_network_configuration = writerInfo.m_network_configuration;
    m_key = writerInfo.m_key;
    m_rtps_participant_key = writerInfo.m_rtps_participant_key;
    m_user_defined_id = writerInfo.m_user_defined_id;
    m_properties = writerInfo.m_properties;

#if HAVE_SECURITY
    security_attributes_ = writerInfo.security_attributes_;
    plugin_security_attributes_ = writerInfo.plugin_security_attributes_;
#endif // if HAVE_SECURITY

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

    if (writerInfo.type_information.assigned())
    {
        type_information = writerInfo.type_information;
    }

    return *this;
}

void WriterProxyData::init(
    const VariableLengthDataLimits& data_limits)
{
#if HAVE_SECURITY
    security_attributes_ = 0;
    plugin_security_attributes_ = 0;
#endif // if HAVE_SECURITY
    m_network_configuration = 0;
    m_user_defined_id = 0;
    m_type_id = nullptr;
    m_type = nullptr;

    m_properties.set_max_size(static_cast<uint32_t>(data_limits.max_properties));
}

uint32_t WriterProxyData::get_serialized_size(
        bool include_encapsulation) const
{
    uint32_t ret_val = include_encapsulation ? 4 : 0;

    // PID_ENDPOINT_GUID
    ret_val += 4 + PARAMETER_GUID_LENGTH;

    // PID_NETWORK_CONFIGURATION_SET
    ret_val += 4 + PARAMETER_NETWORKCONFIGSET_LENGTH;

    // PID_UNICAST_LOCATOR
    ret_val += static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * remote_locators.unicast.size());

    // PID_MULTICAST_LOCATOR
    ret_val += static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * remote_locators.multicast.size());

    // PID_PARTICIPANT_GUID
    ret_val += 4 + PARAMETER_GUID_LENGTH;

    // PID_TOPIC_NAME
    ret_val += dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(topic_name);

    // PID_TYPE_NAME
    ret_val += dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(type_name);

    // PID_KEY_HASH
    ret_val += 4 + 16;

    // PID_TYPE_MAX_SIZE_SERIALIZED
    ret_val += 4 + 4;

    // PID_PROTOCOL_VERSION
    ret_val += 4 + 4;

    // PID_VENDORID
    ret_val += 4 + 4;

    if (persistence_guid != c_Guid_Unknown)
    {
        // PID_PERSISTENCE_GUID
        ret_val += 4 + PARAMETER_GUID_LENGTH;
    }
    if (durability.send_always() || durability.hasChanged)
    {
        ret_val +=
                dds::QosPoliciesSerializer<dds::DurabilityQosPolicy>::cdr_serialized_size(durability);
    }
    if (durability_service.send_always() || durability_service.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::DurabilityServiceQosPolicy>::cdr_serialized_size(
            durability_service);
    }
    if (deadline.send_always() || deadline.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::DeadlineQosPolicy>::cdr_serialized_size(deadline);
    }
    if (latency_budget.send_always() || latency_budget.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::LatencyBudgetQosPolicy>::cdr_serialized_size(latency_budget);
    }
    if (liveliness.send_always() || liveliness.hasChanged)
    {
        ret_val +=
                dds::QosPoliciesSerializer<dds::LivelinessQosPolicy>::cdr_serialized_size(liveliness);
    }
    if (reliability.send_always() || reliability.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::ReliabilityQosPolicy>::cdr_serialized_size(reliability);
    }
    if (lifespan.send_always() || lifespan.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::LifespanQosPolicy>::cdr_serialized_size(lifespan);
    }
    if (user_data.send_always() || user_data.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::UserDataQosPolicy>::cdr_serialized_size(user_data);
    }
    if (ownership.send_always() || ownership.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::OwnershipQosPolicy>::cdr_serialized_size(ownership);
    }
    if (ownership_strength.send_always() || ownership_strength.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::OwnershipStrengthQosPolicy>::cdr_serialized_size(
            ownership_strength);
    }
    if (destination_order.send_always() || destination_order.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::DestinationOrderQosPolicy>::cdr_serialized_size(
            destination_order);
    }
    if (presentation.send_always() || presentation.hasChanged)
    {
        ret_val +=
                dds::QosPoliciesSerializer<dds::PresentationQosPolicy>::cdr_serialized_size(presentation);
    }
    if (partition.send_always() || partition.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::PartitionQosPolicy>::cdr_serialized_size(partition);
    }
    if (topic_data.send_always() || topic_data.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::TopicDataQosPolicy>::cdr_serialized_size(topic_data);
    }
    if (disable_positive_acks.send_always() || disable_positive_acks.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::DisablePositiveACKsQosPolicy>::cdr_serialized_size(
            disable_positive_acks);
    }
    if ((data_sharing.send_always() || data_sharing.hasChanged) &&
            data_sharing.kind() != fastdds::dds::OFF)
    {
        ret_val += dds::QosPoliciesSerializer<dds::DataSharingQosPolicy>::cdr_serialized_size(data_sharing);
    }
    if (group_data.send_always() || group_data.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::GroupDataQosPolicy>::cdr_serialized_size(group_data);
    }
    if (m_type_id && m_type_id->m_type_identifier._d() != fastdds::dds::xtypes::TK_NONE)
    {
        ret_val += dds::QosPoliciesSerializer<dds::TypeIdV1>::cdr_serialized_size(*m_type_id);
    }
    if (m_type && m_type->m_type_object._d() != fastdds::dds::xtypes::TK_NONE)
    {
        ret_val += dds::QosPoliciesSerializer<dds::TypeObjectV1>::cdr_serialized_size(*m_type);
    }

    if (m_properties.size() > 0)
    {
        // PID_PROPERTY_LIST
        ret_val += dds::ParameterSerializer<dds::ParameterPropertyList_t>::cdr_serialized_size(m_properties);
    }

#if HAVE_SECURITY
    if ((this->security_attributes_ != 0UL) || (this->plugin_security_attributes_ != 0UL))
    {
        ret_val += 4 + PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH;
    }
#endif // if HAVE_SECURITY

    if (representation.send_always() || representation.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::DataRepresentationQosPolicy>::cdr_serialized_size(
            representation);
    }

    if (type_information.assigned())
    {
        ret_val += dds::QosPoliciesSerializer<dds::xtypes::TypeInformationParameter>::cdr_serialized_size(
            type_information);
    }

    // PID_SENTINEL
    return ret_val + 4;
}

bool WriterProxyData::write_to_cdr_message(
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

    {
        ParameterGuid_t p(fastdds::dds::PID_ENDPOINT_GUID, 16, guid);
        if (!dds::ParameterSerializer<ParameterGuid_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }

    {
        ParameterNetworkConfigSet_t p(fastdds::dds::PID_NETWORK_CONFIGURATION_SET, PARAMETER_NETWORKCONFIGSET_LENGTH);
        p.netconfigSet = m_network_configuration;
        if (!dds::ParameterSerializer<ParameterNetworkConfigSet_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }

    for (const Locator_t& locator : remote_locators.unicast)
    {
        ParameterLocator_t p(fastdds::dds::PID_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, locator);
        if (!dds::ParameterSerializer<ParameterLocator_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    for (const Locator_t& locator : remote_locators.multicast)
    {
        ParameterLocator_t p(fastdds::dds::PID_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, locator);
        if (!dds::ParameterSerializer<ParameterLocator_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterGuid_t p(fastdds::dds::PID_PARTICIPANT_GUID, PARAMETER_GUID_LENGTH, m_rtps_participant_key);
        if (!dds::ParameterSerializer<ParameterGuid_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterString_t p(fastdds::dds::PID_TOPIC_NAME, 0, topic_name);
        if (!dds::ParameterSerializer<ParameterString_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterString_t p(fastdds::dds::PID_TYPE_NAME, 0, type_name);
        if (!dds::ParameterSerializer<ParameterString_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterKey_t p(fastdds::dds::PID_KEY_HASH, 16, m_key);
        if (!dds::ParameterSerializer<ParameterKey_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterPort_t p(fastdds::dds::PID_TYPE_MAX_SIZE_SERIALIZED, 4, max_serialized_size);
        if (!dds::ParameterSerializer<ParameterPort_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterProtocolVersion_t p(fastdds::dds::PID_PROTOCOL_VERSION, 4);
        if (!dds::ParameterSerializer<ParameterProtocolVersion_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    {
        ParameterVendorId_t p(fastdds::dds::PID_VENDORID, 4);
        if (!dds::ParameterSerializer<ParameterVendorId_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    if (persistence_guid != c_Guid_Unknown)
    {
        ParameterGuid_t p(fastdds::dds::PID_PERSISTENCE_GUID, 16, persistence_guid);
        if (!dds::ParameterSerializer<ParameterGuid_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
    if (durability.send_always() || durability.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::DurabilityQosPolicy>::add_to_cdr_message(durability, msg))
        {
            return false;
        }
    }
    if (durability_service.send_always() || durability_service.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::DurabilityServiceQosPolicy>::add_to_cdr_message(
                    durability_service, msg))
        {
            return false;
        }
    }
    if (deadline.send_always() ||  deadline.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::DeadlineQosPolicy>::add_to_cdr_message(deadline, msg))
        {
            return false;
        }
    }
    if (latency_budget.send_always() ||  latency_budget.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::LatencyBudgetQosPolicy>::add_to_cdr_message(latency_budget, msg))
        {
            return false;
        }
    }
    if (liveliness.send_always() ||  liveliness.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::LivelinessQosPolicy>::add_to_cdr_message(liveliness, msg))
        {
            return false;
        }
    }
    if (reliability.send_always() ||  reliability.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::ReliabilityQosPolicy>::add_to_cdr_message(reliability, msg))
        {
            return false;
        }
    }
    if (lifespan.send_always() ||  lifespan.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::LifespanQosPolicy>::add_to_cdr_message(lifespan, msg))
        {
            return false;
        }
    }
    if (user_data.send_always() || user_data.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::UserDataQosPolicy>::add_to_cdr_message(user_data, msg))
        {
            return false;
        }
    }
    if (ownership.send_always() ||  ownership.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::OwnershipQosPolicy>::add_to_cdr_message(ownership, msg))
        {
            return false;
        }
    }
    if (ownership_strength.send_always() ||  ownership_strength.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::OwnershipStrengthQosPolicy>::add_to_cdr_message(
                    ownership_strength, msg))
        {
            return false;
        }
    }
    if (destination_order.send_always() ||  destination_order.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::DestinationOrderQosPolicy>::add_to_cdr_message(
                    destination_order, msg))
        {
            return false;
        }
    }
    if (presentation.send_always() ||  presentation.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::PresentationQosPolicy>::add_to_cdr_message(presentation, msg))
        {
            return false;
        }
    }
    if (partition.send_always() ||  partition.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::PartitionQosPolicy>::add_to_cdr_message(partition, msg))
        {
            return false;
        }
    }
    if (topic_data.send_always() || topic_data.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::TopicDataQosPolicy>::add_to_cdr_message(topic_data, msg))
        {
            return false;
        }
    }
    if ((disable_positive_acks.send_always() || disable_positive_acks.hasChanged) &&
            disable_positive_acks.enabled)
    {
        if (!dds::QosPoliciesSerializer<dds::DisablePositiveACKsQosPolicy>::add_to_cdr_message(
                    disable_positive_acks, msg))
        {
            return false;
        }
    }
    if ((data_sharing.send_always() || data_sharing.hasChanged) &&
            data_sharing.kind() != fastdds::dds::OFF)
    {
        if (!dds::QosPoliciesSerializer<dds::DataSharingQosPolicy>::add_to_cdr_message(data_sharing, msg))
        {
            return false;
        }
    }
    if (group_data.send_always() ||  group_data.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::GroupDataQosPolicy>::add_to_cdr_message(group_data, msg))
        {
            return false;
        }
    }
    if (type_information.assigned())
    {
        if (!dds::QosPoliciesSerializer<dds::xtypes::TypeInformationParameter>::add_to_cdr_message(
                    type_information, msg))
        {
            return false;
        }
    }
    if (m_properties.size() > 0)
    {
        if (!dds::ParameterSerializer<ParameterPropertyList_t>::add_to_cdr_message(m_properties, msg))
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
        if (!dds::ParameterSerializer<ParameterEndpointSecurityInfo_t>::add_to_cdr_message(p, msg))
        {
            return false;
        }
    }
#endif // if HAVE_SECURITY

    if (representation.send_always() || representation.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::DataRepresentationQosPolicy>::add_to_cdr_message(
                    representation, msg))
        {
            return false;
        }
    }



    return dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(msg);
}

bool WriterProxyData::read_from_cdr_message(
        CDRMessage_t* msg,
        fastdds::rtps::VendorId_t source_vendor_id)
{
    auto param_process = [this, source_vendor_id](
        CDRMessage_t* msg, const ParameterId_t& pid, uint16_t plength)
            {
                VendorId_t vendor_id = source_vendor_id;

                switch (pid)
                {
                    case fastdds::dds::PID_VENDORID:
                    {
                        ParameterVendorId_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterVendorId_t>::read_from_cdr_message(
                                    p, msg, plength))
                        {
                            return false;
                        }

                        vendor_id = p.vendorId;
                        break;
                    }
                    case fastdds::dds::PID_DURABILITY:
                    {
                        if (!dds::QosPoliciesSerializer<dds::DurabilityQosPolicy>::read_from_cdr_message(
                                    durability, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_DURABILITY_SERVICE:
                    {
                        if (!dds::QosPoliciesSerializer<dds::DurabilityServiceQosPolicy>::read_from_cdr_message(
                                    durability_service, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_DEADLINE:
                    {
                        if (!dds::QosPoliciesSerializer<dds::DeadlineQosPolicy>::read_from_cdr_message(
                                    deadline, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_LATENCY_BUDGET:
                    {
                        if (!dds::QosPoliciesSerializer<dds::LatencyBudgetQosPolicy>::read_from_cdr_message(
                                    latency_budget, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_LIVELINESS:
                    {
                        if (!dds::QosPoliciesSerializer<dds::LivelinessQosPolicy>::read_from_cdr_message(
                                    liveliness, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_RELIABILITY:
                    {
                        if (!dds::QosPoliciesSerializer<dds::ReliabilityQosPolicy>::read_from_cdr_message(
                                    reliability, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_LIFESPAN:
                    {
                        if (!dds::QosPoliciesSerializer<dds::LifespanQosPolicy>::read_from_cdr_message(
                                    lifespan, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_USER_DATA:
                    {
                        if (!dds::QosPoliciesSerializer<dds::UserDataQosPolicy>::read_from_cdr_message(
                                    user_data, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_OWNERSHIP:
                    {
                        if (!dds::QosPoliciesSerializer<dds::OwnershipQosPolicy>::read_from_cdr_message(
                                    ownership, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_OWNERSHIP_STRENGTH:
                    {
                        if (!dds::QosPoliciesSerializer<dds::OwnershipStrengthQosPolicy>::read_from_cdr_message(
                                    ownership_strength, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_DESTINATION_ORDER:
                    {
                        if (!dds::QosPoliciesSerializer<dds::DestinationOrderQosPolicy>::read_from_cdr_message(
                                    destination_order, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_PRESENTATION:
                    {
                        if (!dds::QosPoliciesSerializer<dds::PresentationQosPolicy>::read_from_cdr_message(
                                    presentation, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_PARTITION:
                    {
                        if (!dds::QosPoliciesSerializer<dds::PartitionQosPolicy>::read_from_cdr_message(
                                    partition, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TOPIC_DATA:
                    {
                        if (!dds::QosPoliciesSerializer<dds::TopicDataQosPolicy>::read_from_cdr_message(
                                    topic_data, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_GROUP_DATA:
                    {
                        if (!dds::QosPoliciesSerializer<dds::GroupDataQosPolicy>::read_from_cdr_message(
                                    group_data, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TOPIC_NAME:
                    {
                        ParameterString_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterString_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        topic_name = p.getName();
                        break;
                    }
                    case fastdds::dds::PID_TYPE_NAME:
                    {
                        ParameterString_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterString_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        type_name = p.getName();
                        break;
                    }
                    case fastdds::dds::PID_PARTICIPANT_GUID:
                    {
                        ParameterGuid_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterGuid_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        m_rtps_participant_key = p.guid;
                        participant_guid = p.guid;
                        from_guid_prefix_to_topic_key(participant_guid.guidPrefix, participant_key.value);
                        break;
                    }
                    case fastdds::dds::PID_ENDPOINT_GUID:
                    {
                        ParameterGuid_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterGuid_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        guid = p.guid;
                        m_key = p.guid;
                        from_entity_id_to_topic_key(guid.entityId, PublicationBuiltinTopicData::key.value);
                        break;
                    }
                    case fastdds::dds::PID_PERSISTENCE_GUID:
                    {
                        VendorId_t local_vendor_id = source_vendor_id;
                        if (c_VendorId_Unknown == local_vendor_id)
                        {
                            local_vendor_id = ((c_VendorId_Unknown == vendor_id) ? c_VendorId_eProsima : vendor_id);
                        }

                        // Ignore custom PID when coming from other vendors except RTI Connext
                        if ((c_VendorId_eProsima != local_vendor_id) &&
                                (fastdds::rtps::c_VendorId_rti_connext != local_vendor_id))
                        {
                            EPROSIMA_LOG_INFO(RTPS_PROXY_DATA,
                                    "Ignoring custom PID" << pid << " from vendor " << local_vendor_id);
                            return true;
                        }

                        ParameterGuid_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterGuid_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        persistence_guid = p.guid;
                        break;
                    }
                    case fastdds::dds::PID_NETWORK_CONFIGURATION_SET:
                    {
                        VendorId_t local_vendor_id = source_vendor_id;
                        if (c_VendorId_Unknown == local_vendor_id)
                        {
                            local_vendor_id = ((c_VendorId_Unknown == vendor_id) ? c_VendorId_eProsima : vendor_id);
                        }

                        // Ignore custom PID when coming from other vendors
                        if (c_VendorId_eProsima != local_vendor_id)
                        {
                            EPROSIMA_LOG_INFO(RTPS_PROXY_DATA,
                                    "Ignoring custom PID" << pid << " from vendor " << local_vendor_id);
                            return true;
                        }

                        ParameterNetworkConfigSet_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterNetworkConfigSet_t>::read_from_cdr_message(
                                    p, msg, plength))
                        {
                            return false;
                        }

                        m_network_configuration = p.netconfigSet;
                        break;
                    }
                    case fastdds::dds::PID_UNICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        remote_locators.add_unicast_locator(p.locator);
                        break;
                    }
                    case fastdds::dds::PID_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        remote_locators.add_multicast_locator(p.locator);
                        break;
                    }
                    case fastdds::dds::PID_KEY_HASH:
                    {
                        ParameterKey_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterKey_t>::read_from_cdr_message(p, msg, plength))
                        {
                            return false;
                        }

                        m_key = p.key;
                        iHandle2GUID(guid, m_key);
                        break;
                    }
                    case fastdds::dds::PID_TYPE_IDV1:
                    {
                        EPROSIMA_LOG_WARNING(RTPS_PROXY_DATA,
                                "Reception of TypeIdentifiers is not supported. They will be ignored.");
                        break;
                    }
                    case fastdds::dds::PID_TYPE_OBJECTV1:
                    {
                        EPROSIMA_LOG_WARNING(RTPS_PROXY_DATA,
                                "Reception of TypeObjects is not supported. They will be ignored.");
                        break;
                    }
                    case fastdds::dds::PID_TYPE_INFORMATION:
                    {
                        VendorId_t local_vendor_id = source_vendor_id;
                        if (c_VendorId_Unknown == local_vendor_id)
                        {
                            local_vendor_id = ((c_VendorId_Unknown == vendor_id) ? c_VendorId_eProsima : vendor_id);
                        }

                        // Ignore this PID when coming from other vendors
                        if (c_VendorId_eProsima != local_vendor_id)
                        {
                            EPROSIMA_LOG_INFO(RTPS_PROXY_DATA,
                                    "Ignoring PID" << pid << " from vendor " << local_vendor_id);
                            return true;
                        }

                        if (!dds::QosPoliciesSerializer<dds::xtypes::TypeInformationParameter>::
                                read_from_cdr_message(type_information, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_DISABLE_POSITIVE_ACKS:
                    {
                        VendorId_t local_vendor_id = source_vendor_id;
                        if (c_VendorId_Unknown == local_vendor_id)
                        {
                            local_vendor_id = ((c_VendorId_Unknown == vendor_id) ? c_VendorId_eProsima : vendor_id);
                        }

                        // Ignore custom PID when coming from other vendors except RTI Connext
                        if ((c_VendorId_eProsima != local_vendor_id) &&
                                (fastdds::rtps::c_VendorId_rti_connext != local_vendor_id))
                        {
                            EPROSIMA_LOG_INFO(RTPS_PROXY_DATA,
                                    "Ignoring custom PID" << pid << " from vendor " << local_vendor_id);
                            return true;
                        }

                        if (!dds::QosPoliciesSerializer<dds::DisablePositiveACKsQosPolicy>::
                                read_from_cdr_message(disable_positive_acks, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
#if HAVE_SECURITY
                    case fastdds::dds::PID_ENDPOINT_SECURITY_INFO:
                    {
                        ParameterEndpointSecurityInfo_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterEndpointSecurityInfo_t>::read_from_cdr_message(
                                    p, msg, plength))
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
                        if (!dds::QosPoliciesSerializer<dds::DataRepresentationQosPolicy>::
                                read_from_cdr_message(representation, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TYPE_CONSISTENCY_ENFORCEMENT:
                    {
                        EPROSIMA_LOG_ERROR(RTPS_PROXY_DATA,
                                "Received TypeConsistencyEnforcementQos from a writer, but they haven't.");
                        break;
                    }

                    case fastdds::dds::PID_PROPERTY_LIST:
                    {
                        if (!dds::ParameterSerializer<ParameterPropertyList_t>::read_from_cdr_message(
                                    m_properties, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }

                    case fastdds::dds::PID_DATASHARING:
                    {
                        VendorId_t local_vendor_id = source_vendor_id;
                        if (c_VendorId_Unknown == local_vendor_id)
                        {
                            local_vendor_id = ((c_VendorId_Unknown == vendor_id) ? c_VendorId_eProsima : vendor_id);
                        }

                        // Ignore custom PID when coming from other vendors
                        if (c_VendorId_eProsima != local_vendor_id)
                        {
                            EPROSIMA_LOG_INFO(RTPS_PROXY_DATA,
                                    "Ignoring custom PID" << pid << " from vendor " << local_vendor_id);
                            return true;
                        }

                        if (!dds::QosPoliciesSerializer<dds::DataSharingQosPolicy>::read_from_cdr_message(
                                    data_sharing, msg, plength))
                        {
                            EPROSIMA_LOG_ERROR(RTPS_WRITER_PROXY_DATA,
                                    "Received with error.");
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
    data_sharing.off();
    try
    {
        if (ParameterList::readParameterListfromCDRMsg(*msg, param_process, true, qos_size))
        {
            if (0x03 == (guid.entityId.value[3] & 0x0F))
            {
                topic_kind = NO_KEY;
            }
            else if (0x02 == (guid.entityId.value[3] & 0x0F))
            {
                topic_kind = WITH_KEY;
            }

            /* Some vendors (i.e. CycloneDDS) do not follow DDSI-RTPS and omit PID_PARTICIPANT_GUID
             * In that case we use a default value relying on the prefix from m_guid and the default
             * participant entity id
             */
            if (!m_rtps_participant_key.isDefined())
            {
                GUID_t tmp_guid = guid;
                tmp_guid.entityId = c_EntityId_RTPSParticipant;
                m_rtps_participant_key = tmp_guid;
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

void WriterProxyData::setup_locators(
        const WriterProxyData& wdata,
        NetworkFactory& network,
        const ParticipantProxyData& participant_data)
{
    if (this == &wdata)
    {
        return;
    }

    bool from_this_host = participant_data.is_from_this_host();

    if (wdata.has_locators())
    {
        // Get the transformed remote locators for the WriterProxyData received
        remote_locators.unicast.clear();
        remote_locators.multicast.clear();
        for (const Locator_t& locator : wdata.remote_locators.unicast)
        {
            Locator_t temp_locator;
            if (network.transform_remote_locator(locator, temp_locator, m_network_configuration, from_this_host))
            {
                ProxyDataFilters::filter_locators(network, remote_locators, temp_locator, true);
            }
        }
        for (const Locator_t& locator : wdata.remote_locators.multicast)
        {
            Locator_t temp_locator;
            if (network.transform_remote_locator(locator, temp_locator, m_network_configuration, from_this_host))
            {
                ProxyDataFilters::filter_locators(network, remote_locators, temp_locator, false);
            }
        }
    }
    else
    {
        // Get the remote locators from the participant_data
        set_remote_locators(participant_data.default_locators, network, true, from_this_host);
    }
}

void WriterProxyData::clear()
{
    PublicationBuiltinTopicData::clear();

    guid = c_Guid_Unknown;
    persistence_guid = c_Guid_Unknown;
    participant_guid = c_Guid_Unknown;
    remote_locators.unicast.clear();
    remote_locators.multicast.clear();
    type_name = "";
    topic_name = "";
    topic_kind = NO_KEY;
    max_serialized_size = 0;

    m_network_configuration = 0;
    m_user_defined_id = 0;
    m_key = InstanceHandle_t();
    m_rtps_participant_key = InstanceHandle_t();
    m_properties.clear();
    m_properties.length = 0;

#if HAVE_SECURITY
    security_attributes_ = 0UL;
    plugin_security_attributes_ = 0UL;
#endif // if HAVE_SECURITY

    if (m_type_id)
    {
        *m_type_id = dds::TypeIdV1();
    }
    if (m_type)
    {
        *m_type = dds::TypeObjectV1();
    }
}

bool WriterProxyData::is_update_allowed(
        const WriterProxyData& wdata) const
{
    if ((guid != wdata.guid) ||
            (persistence_guid != wdata.persistence_guid) ||
#if HAVE_SECURITY
            (security_attributes_ != wdata.security_attributes_) ||
            (plugin_security_attributes_ != wdata.plugin_security_attributes_) ||
#endif // if HAVE_SECURITY
            (type_name != wdata.type_name) ||
            (topic_name != wdata.topic_name))
    {
        return false;
    }

    return can_qos_be_updated(wdata);
}

void WriterProxyData::update(
        WriterProxyData* wdata)
{
    // m_network_configuration = wdata->m_network_configuration; // TODO: update?
    remote_locators = wdata->remote_locators;
    set_qos(*wdata, false);
}

void WriterProxyData::add_unicast_locator(
        const Locator_t& locator)
{
    remote_locators.add_unicast_locator(locator);
}

void WriterProxyData::set_announced_unicast_locators(
        const LocatorList_t& locators)
{
    remote_locators.unicast.clear();
    for (const Locator_t& locator : locators)
    {
        remote_locators.add_unicast_locator(locator);
    }
}

void WriterProxyData::set_remote_unicast_locators(
        const LocatorList_t& locators,
        const NetworkFactory& network,
        bool from_this_host)
{
    remote_locators.unicast.clear();
    for (const Locator_t& locator : locators)
    {
        if (network.is_locator_remote_or_allowed(locator, from_this_host))
        {
            remote_locators.add_unicast_locator(locator);
        }
    }
}

void WriterProxyData::add_multicast_locator(
        const Locator_t& locator)
{
    remote_locators.add_multicast_locator(locator);
}

void WriterProxyData::set_multicast_locators(
        const LocatorList_t& locators,
        const NetworkFactory& network,
        bool from_this_host)
{
    remote_locators.multicast.clear();
    for (const Locator_t& locator : locators)
    {
        if (network.is_locator_remote_or_allowed(locator, from_this_host))
        {
            remote_locators.add_multicast_locator(locator);
        }
    }
}

void WriterProxyData::set_locators(
        const RemoteLocatorList& locators)
{
    remote_locators = locators;
}

void WriterProxyData::set_remote_locators(
        const RemoteLocatorList& locators,
        const NetworkFactory& network,
        bool use_multicast_locators,
        bool from_this_host)
{
    remote_locators.unicast.clear();
    remote_locators.multicast.clear();

    for (const Locator_t& locator : locators.unicast)
    {
        if (network.is_locator_remote_or_allowed(locator, from_this_host))
        {
            remote_locators.add_unicast_locator(locator);
        }
    }

    if (use_multicast_locators)
    {
        for (const Locator_t& locator : locators.multicast)
        {
            if (network.is_locator_remote_or_allowed(locator, from_this_host))
            {
                remote_locators.add_multicast_locator(locator);
            }
        }
    }
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
