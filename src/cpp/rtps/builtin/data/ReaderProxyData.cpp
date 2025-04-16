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
 * @file ReaderProxyData.cpp
 */

#include <rtps/builtin/data/ReaderProxyData.hpp>

#include <fastdds/core/policy/ParameterList.hpp>
#include <fastdds/core/policy/QosPoliciesSerializer.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/VendorId_t.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/network/NetworkFactory.hpp>
#include <utils/BuiltinTopicKeyConversions.hpp>
#include <utils/SystemInfo.hpp>

#include "ProxyDataFilters.hpp"

using ParameterList = eprosima::fastdds::dds::ParameterList;
using ContentFilterProperty = eprosima::fastdds::rtps::ContentFilterProperty;

namespace eprosima {
namespace fastdds {
namespace rtps {

using ::operator <<;

ReaderProxyData::ReaderProxyData (
        const size_t max_unicast_locators,
        const size_t max_multicast_locators,
        const VariableLengthDataLimits& data_limits,
        const fastdds::rtps::ContentFilterProperty::AllocationConfiguration& content_filter_limits)
    : SubscriptionBuiltinTopicData(max_unicast_locators, max_multicast_locators, data_limits, content_filter_limits)
{
    init(data_limits);
}

ReaderProxyData::ReaderProxyData (
        const size_t max_unicast_locators,
        const size_t max_multicast_locators,
        const fastdds::rtps::ContentFilterProperty::AllocationConfiguration& content_filter_limits)
    : ReaderProxyData(max_unicast_locators, max_multicast_locators, VariableLengthDataLimits(), content_filter_limits)
{

}

ReaderProxyData::ReaderProxyData(
        const VariableLengthDataLimits& data_limits,
        const SubscriptionBuiltinTopicData& subscription_data)
    : SubscriptionBuiltinTopicData(subscription_data)
{
    init(data_limits);
}

ReaderProxyData::~ReaderProxyData()
{

    delete m_type;
    delete m_type_id;

    EPROSIMA_LOG_INFO(RTPS_PROXY_DATA, "ReaderProxyData destructor: " << guid; );
}

ReaderProxyData::ReaderProxyData(
        const ReaderProxyData& readerInfo)
    : SubscriptionBuiltinTopicData(readerInfo)
#if HAVE_SECURITY
    , security_attributes_(readerInfo.security_attributes_)
    , plugin_security_attributes_(readerInfo.plugin_security_attributes_)
#endif // if HAVE_SECURITY
    , m_network_configuration(readerInfo.m_network_configuration)
    , m_key(readerInfo.m_key)
    , m_rtps_participant_key(readerInfo.m_rtps_participant_key)
    , m_user_defined_id(readerInfo.m_user_defined_id)
    , m_is_alive(readerInfo.m_is_alive)
    , m_type_id(nullptr)
    , m_type(nullptr)
{
    if (readerInfo.m_type_id)
    {
        type_id(*readerInfo.m_type_id);
    }

    if (readerInfo.m_type)
    {
        type(*readerInfo.m_type);
    }

    if (readerInfo.has_type_information())
    {
        type_information = readerInfo.type_information;
    }
}

ReaderProxyData& ReaderProxyData::operator =(
        const ReaderProxyData& readerInfo)
{
    SubscriptionBuiltinTopicData::key = readerInfo.SubscriptionBuiltinTopicData::key;
    participant_key = readerInfo.participant_key;
    type_name = readerInfo.type_name;
    topic_name = readerInfo.topic_name;
    topic_kind = readerInfo.topic_kind;

    set_qos(readerInfo, true);

    type_information = readerInfo.type_information;

    if (readerInfo.history)
    {
        history = readerInfo.history;
    }
    if (readerInfo.resource_limits)
    {
        resource_limits = readerInfo.resource_limits;
    }
    if (readerInfo.reader_data_lifecycle)
    {
        reader_data_lifecycle = readerInfo.reader_data_lifecycle;
    }
    if (readerInfo.rtps_reliable_reader)
    {
        rtps_reliable_reader = readerInfo.rtps_reliable_reader;
    }
    if (readerInfo.endpoint)
    {
        endpoint = readerInfo.endpoint;
    }
    if (readerInfo.reader_resource_limits)
    {
        reader_resource_limits = readerInfo.reader_resource_limits;
    }

    content_filter = readerInfo.content_filter;
    guid = readerInfo.guid;
    participant_guid = readerInfo.participant_guid;
    remote_locators = readerInfo.remote_locators;
    loopback_transformation = readerInfo.loopback_transformation;
    expects_inline_qos = readerInfo.expects_inline_qos;
    properties = readerInfo.properties;

    m_network_configuration = readerInfo.m_network_configuration;
    m_key = readerInfo.m_key;
    m_rtps_participant_key = readerInfo.m_rtps_participant_key;
    m_user_defined_id = readerInfo.m_user_defined_id;
    m_is_alive = readerInfo.m_is_alive;

#if HAVE_SECURITY
    security_attributes_ = readerInfo.security_attributes_;
    plugin_security_attributes_ = readerInfo.plugin_security_attributes_;
#endif // if HAVE_SECURITY

    if (readerInfo.m_type_id)
    {
        type_id(*readerInfo.m_type_id);
    }
    else
    {
        delete m_type_id;
        m_type_id = nullptr;
    }

    if (readerInfo.m_type)
    {
        type(*readerInfo.m_type);
    }
    else
    {
        delete m_type;
        m_type = nullptr;
    }

    return *this;
}

void ReaderProxyData::init(
        const VariableLengthDataLimits& data_limits)
{
#if HAVE_SECURITY
    security_attributes_ = 0UL;
    plugin_security_attributes_ = 0UL;
#endif // if HAVE_SECURITY
    m_network_configuration = 0;
    m_user_defined_id = 0;
    m_is_alive = true;
    m_type_id = nullptr;
    m_type = nullptr;

    properties.set_max_size(static_cast<uint32_t>(data_limits.max_properties));

    // As DDS-XTypes, v1.2 (page 182) document stablishes, local default is ALLOW_TYPE_COERCION,
    // but when remotes doesn't send TypeConsistencyQos, we must assume DISALLOW.
    type_consistency.m_kind = dds::DISALLOW_TYPE_COERCION;
}

uint32_t ReaderProxyData::get_serialized_size(
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

    // PID_EXPECTS_INLINE_QOS
    ret_val += 4 + PARAMETER_BOOL_LENGTH;

    // PID_PARTICIPANT_GUID
    ret_val += 4 + PARAMETER_GUID_LENGTH;

    // PID_TOPIC_NAME
    ret_val += dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(topic_name);

    // PID_TYPE_NAME
    ret_val += dds::ParameterSerializer<Parameter_t>::cdr_serialized_size(type_name);

    // PID_KEY_HASH
    ret_val += 4 + 16;

    // PID_PROTOCOL_VERSION
    ret_val += 4 + 4;

    // PID_VENDORID
    ret_val += 4 + 4;

    if (durability.send_always() || durability.hasChanged)
    {
        ret_val +=
                dds::QosPoliciesSerializer<dds::DurabilityQosPolicy>::cdr_serialized_size(durability);
    }
    if (deadline.send_always() || deadline.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::DeadlineQosPolicy>::cdr_serialized_size(deadline);
    }
    if (latency_budget.send_always() || latency_budget.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::LatencyBudgetQosPolicy>::cdr_serialized_size(
            latency_budget);
    }
    if (liveliness.send_always() || liveliness.hasChanged)
    {
        ret_val +=
                dds::QosPoliciesSerializer<dds::LivelinessQosPolicy>::cdr_serialized_size(liveliness);
    }
    if (reliability.send_always() || reliability.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::ReliabilityQosPolicy>::cdr_serialized_size(
            reliability);
    }
    if (lifespan.send_always() || lifespan.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::LifespanQosPolicy>::cdr_serialized_size(lifespan);
    }
    if (user_data.send_always() || user_data.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::UserDataQosPolicy>::cdr_serialized_size(user_data);
    }
    if (time_based_filter.send_always() || time_based_filter.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::TimeBasedFilterQosPolicy>::cdr_serialized_size(
            time_based_filter);
    }
    if (ownership.send_always() || ownership.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::OwnershipQosPolicy>::cdr_serialized_size(ownership);
    }
    if (destination_order.send_always() || destination_order.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::DestinationOrderQosPolicy>::cdr_serialized_size(
            destination_order);
    }
    if (presentation.send_always() || presentation.hasChanged)
    {
        ret_val +=
                dds::QosPoliciesSerializer<dds::PresentationQosPolicy>::cdr_serialized_size(
            presentation);
    }
    if (partition.send_always() || partition.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::PartitionQosPolicy>::cdr_serialized_size(partition);
    }
    if (topic_data.send_always() || topic_data.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::TopicDataQosPolicy>::cdr_serialized_size(topic_data);
    }
    if (group_data.send_always() || group_data.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::GroupDataQosPolicy>::cdr_serialized_size(group_data);
    }
    if (disable_positive_acks.send_always() || disable_positive_acks.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::DisablePositiveACKsQosPolicy>::cdr_serialized_size(
            disable_positive_acks);
    }
    if (type_information.assigned())
    {
        ret_val +=
                dds::QosPoliciesSerializer<dds::xtypes::TypeInformationParameter>::cdr_serialized_size(
            type_information);
    }
    if (type_consistency.send_always() || type_consistency.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::TypeConsistencyEnforcementQosPolicy>::cdr_serialized_size(
            type_consistency);
    }
    if ((data_sharing.send_always() || data_sharing.hasChanged) &&
            data_sharing.kind() != fastdds::dds::OFF)
    {
        ret_val += dds::QosPoliciesSerializer<dds::DataSharingQosPolicy>::cdr_serialized_size(
            data_sharing);
    }

    if (properties.size() > 0)
    {
        // PID_PROPERTY_LIST
        ret_val += dds::ParameterSerializer<dds::ParameterPropertyList_t>::cdr_serialized_size(properties);
    }

    // PID_CONTENT_FILTER_PROPERTY
    // Take into count only when filter_class_name and filter_expression are not empty.
    if (0 < content_filter.filter_class_name.size() && 0 < content_filter.filter_expression.size())
    {
        ret_val += dds::ParameterSerializer<ContentFilterProperty>::cdr_serialized_size(content_filter);
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

    if (type_consistency.send_always() || type_consistency.hasChanged)
    {
        ret_val += dds::QosPoliciesSerializer<dds::TypeConsistencyEnforcementQosPolicy>::cdr_serialized_size(
            type_consistency);
    }
    if (type_information.assigned())
    {
        ret_val +=
                dds::QosPoliciesSerializer<dds::xtypes::TypeInformationParameter>::cdr_serialized_size(
            type_information);
    }

    // PID_SENTINEL
    return ret_val + 4;
}

bool ReaderProxyData::write_to_cdr_message(
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
        ParameterGuid_t p(fastdds::dds::PID_ENDPOINT_GUID, PARAMETER_GUID_LENGTH, guid);
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
        ParameterBool_t p(fastdds::dds::PID_EXPECTS_INLINE_QOS, PARAMETER_BOOL_LENGTH, expects_inline_qos);
        if (!dds::ParameterSerializer<ParameterBool_t>::add_to_cdr_message(p, msg))
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
    if (durability.send_always() || durability.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::DurabilityQosPolicy>::add_to_cdr_message(durability, msg))
        {
            return false;
        }
    }
    if (deadline.send_always() || deadline.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::DeadlineQosPolicy>::add_to_cdr_message(deadline, msg))
        {
            return false;
        }
    }
    if (latency_budget.send_always() || latency_budget.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::LatencyBudgetQosPolicy>::add_to_cdr_message(
                    latency_budget, msg))
        {
            return false;
        }
    }
    if (liveliness.send_always() || liveliness.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::LivelinessQosPolicy>::add_to_cdr_message(liveliness, msg))
        {
            return false;
        }
    }
    if (reliability.send_always() || reliability.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::ReliabilityQosPolicy>::add_to_cdr_message(
                    reliability, msg))
        {
            return false;
        }
    }
    if (lifespan.send_always() || lifespan.hasChanged)
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
    if (time_based_filter.send_always() || time_based_filter.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::TimeBasedFilterQosPolicy>::add_to_cdr_message(
                    time_based_filter, msg))
        {
            return false;
        }
    }
    if (ownership.send_always() || ownership.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::OwnershipQosPolicy>::add_to_cdr_message(ownership, msg))
        {
            return false;
        }
    }
    if (destination_order.send_always() || destination_order.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::DestinationOrderQosPolicy>::add_to_cdr_message(
                    destination_order, msg))
        {
            return false;
        }
    }
    if (presentation.send_always() || presentation.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::PresentationQosPolicy>::add_to_cdr_message(
                    presentation, msg))
        {
            return false;
        }
    }
    if (partition.send_always() || partition.hasChanged)
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
    if (group_data.send_always() || group_data.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::GroupDataQosPolicy>::add_to_cdr_message(group_data, msg))
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
        if (!dds::QosPoliciesSerializer<dds::DataSharingQosPolicy>::add_to_cdr_message(
                    data_sharing, msg))
        {
            return false;
        }
    }

    if (m_type_id && m_type_id->m_type_identifier._d() != fastdds::dds::xtypes::TK_NONE)
    {
        if (!dds::QosPoliciesSerializer<dds::xtypes::TypeInformationParameter>::add_to_cdr_message(
                    type_information, msg))
        {
            return false;
        }
    }
    if (properties.size() > 0)
    {
        if (!dds::ParameterSerializer<dds::ParameterPropertyList_t>::add_to_cdr_message(properties, msg))
        {
            return false;
        }
    }

    // Serialize ContentFilterProperty only when filter_class_name and filter_expression are not empty.
    if (0 < content_filter.filter_class_name.size() && 0 < content_filter.filter_expression.size())
    {
        if (!dds::ParameterSerializer<ContentFilterProperty>::add_to_cdr_message(content_filter, msg))
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

    if (type_consistency.send_always() || type_consistency.hasChanged)
    {
        if (!dds::QosPoliciesSerializer<dds::TypeConsistencyEnforcementQosPolicy>::add_to_cdr_message(
                    type_consistency, msg))
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

    return dds::ParameterSerializer<Parameter_t>::add_parameter_sentinel(msg);
}

bool ReaderProxyData::read_from_cdr_message(
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
                    case fastdds::dds::PID_TIME_BASED_FILTER:
                    {
                        if (!dds::QosPoliciesSerializer<dds::TimeBasedFilterQosPolicy>::read_from_cdr_message(
                                    time_based_filter, msg, plength))
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
                        if (!dds::ParameterSerializer<ParameterString_t>::read_from_cdr_message(
                                    p, msg, plength))
                        {
                            return false;
                        }

                        topic_name = p.getName();
                        break;
                    }
                    case fastdds::dds::PID_TYPE_NAME:
                    {
                        ParameterString_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterString_t>::read_from_cdr_message(
                                    p, msg, plength))
                        {
                            return false;
                        }

                        type_name = p.getName();
                        break;
                    }
                    case fastdds::dds::PID_PARTICIPANT_GUID:
                    {
                        ParameterGuid_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterGuid_t>::read_from_cdr_message(
                                    p, msg, plength))
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
                        if (!dds::ParameterSerializer<ParameterGuid_t>::read_from_cdr_message(
                                    p, msg, plength))
                        {
                            return false;
                        }

                        guid = p.guid;
                        m_key = p.guid;
                        from_entity_id_to_topic_key(guid.entityId, SubscriptionBuiltinTopicData::key.value);
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
                        if (!dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(
                                    p, msg, plength))
                        {
                            return false;
                        }

                        remote_locators.add_unicast_locator(p.locator);
                        break;
                    }
                    case fastdds::dds::PID_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterLocator_t>::read_from_cdr_message(
                                    p, msg, plength))
                        {
                            return false;
                        }

                        remote_locators.add_unicast_locator(p.locator);
                        break;
                    }
                    case fastdds::dds::PID_EXPECTS_INLINE_QOS:
                    {
                        ParameterBool_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterBool_t>::read_from_cdr_message(
                                    p, msg, plength))
                        {
                            return false;
                        }

                        expects_inline_qos = p.value;
                        break;
                    }
                    case fastdds::dds::PID_KEY_HASH:
                    {
                        ParameterKey_t p(pid, plength);
                        if (!dds::ParameterSerializer<ParameterKey_t>::read_from_cdr_message(
                                    p, msg, plength))
                        {
                            return false;
                        }

                        m_key = p.key;
                        iHandle2GUID(guid, m_key);
                        break;
                    }
                    case fastdds::dds::PID_DATA_REPRESENTATION:
                    {
                        if (!dds::QosPoliciesSerializer<dds::DataRepresentationQosPolicy>::read_from_cdr_message(
                                    representation, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TYPE_CONSISTENCY_ENFORCEMENT:
                    {
                        if (!dds::QosPoliciesSerializer<dds::TypeConsistencyEnforcementQosPolicy>::
                                read_from_cdr_message(type_consistency, msg, plength))
                        {
                            return false;
                        }
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

                        if (!dds::QosPoliciesSerializer<dds::xtypes::TypeInformationParameter>::read_from_cdr_message(
                                    type_information, msg, plength))
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
                                read_from_cdr_message(
                                    disable_positive_acks, msg, plength))
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
                    case fastdds::dds::PID_PROPERTY_LIST:
                    {
                        if (!dds::ParameterSerializer<dds::ParameterPropertyList_t>::read_from_cdr_message(
                                    properties, msg, plength))
                        {
                            return false;
                        }
                        break;
                    }

                    case fastdds::dds::PID_CONTENT_FILTER_PROPERTY:
                    {
                        if (!dds::ParameterSerializer<ContentFilterProperty>::read_from_cdr_message(
                                    content_filter, msg, plength))
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
                            EPROSIMA_LOG_ERROR(RTPS_READER_PROXY_DATA,
                                    "Received with error.");
                            return false;
                        }

                        break;
                    }

                    case fastdds::dds::PID_HISTORY:
                    {
                        if (!history)
                        {
                            history.reset(true);
                        }

                        if (!dds::QosPoliciesSerializer<dds::HistoryQosPolicy>::read_from_cdr_message(
                                    history.value(), msg, plength))
                        {
                            EPROSIMA_LOG_ERROR(RTPS_READER_PROXY_DATA,
                                    "Received with error.");
                            return false;
                        }
                        break;
                    }

                    case fastdds::dds::PID_RESOURCE_LIMITS:
                    {
                        if (!resource_limits)
                        {
                            resource_limits.reset(true);
                        }

                        if (!dds::QosPoliciesSerializer<dds::ResourceLimitsQosPolicy>::read_from_cdr_message(
                                    resource_limits.value(), msg, plength))
                        {
                            EPROSIMA_LOG_ERROR(RTPS_READER_PROXY_DATA,
                                    "Received with error.");
                            return false;
                        }
                        break;
                    }

                    case fastdds::dds::PID_READER_DATA_LIFECYCLE:
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

                        if (!reader_data_lifecycle)
                        {
                            reader_data_lifecycle.reset(true);
                        }

                        if (!dds::QosPoliciesSerializer<dds::ReaderDataLifecycleQosPolicy>::read_from_cdr_message(
                                    reader_data_lifecycle.value(), msg, plength))
                        {
                            EPROSIMA_LOG_ERROR(RTPS_READER_PROXY_DATA,
                                    "Received with error.");
                            return false;
                        }
                        break;
                    }

                    case fastdds::dds::PID_RTPS_RELIABLE_READER:
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

                        if (!rtps_reliable_reader)
                        {
                            rtps_reliable_reader.reset(true);
                        }

                        if (!dds::QosPoliciesSerializer<dds::RTPSReliableReaderQos>::read_from_cdr_message(
                                    rtps_reliable_reader.value(), msg, plength))
                        {
                            EPROSIMA_LOG_ERROR(RTPS_READER_PROXY_DATA,
                                    "Received with error.");
                            return false;
                        }
                        break;
                    }

                    case fastdds::dds::PID_RTPS_ENDPOINT:
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

                        if (!endpoint)
                        {
                            endpoint.reset(true);
                        }

                        if (!dds::QosPoliciesSerializer<dds::RTPSEndpointQos>::read_from_cdr_message(
                                    endpoint.value(), msg, plength))
                        {
                            EPROSIMA_LOG_ERROR(RTPS_READER_PROXY_DATA,
                                    "Received with error.");
                            return false;
                        }
                        break;
                    }

                    case fastdds::dds::PID_READER_RESOURCE_LIMITS:
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

                        if (!reader_resource_limits)
                        {
                            reader_resource_limits.reset(true);
                        }

                        if (!dds::QosPoliciesSerializer<dds::ReaderResourceLimitsQos>::read_from_cdr_message(
                                    reader_resource_limits.value(), msg, plength))
                        {
                            EPROSIMA_LOG_ERROR(RTPS_READER_PROXY_DATA,
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
            if (guid.entityId.value[3] == 0x04)
            {
                topic_kind = NO_KEY;
            }
            else if (guid.entityId.value[3] == 0x07)
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

void ReaderProxyData::setup_locators(
        const ReaderProxyData& rdata,
        NetworkFactory& network,
        const ParticipantProxyData& participant_data)
{
    if (this == &rdata)
    {
        return;
    }

    bool from_this_host = participant_data.is_from_this_host();

    if (rdata.has_locators())
    {
        // Get the transformed remote locators for the ReaderProxyData received
        remote_locators.unicast.clear();
        remote_locators.multicast.clear();
        for (const Locator_t& locator : rdata.remote_locators.unicast)
        {
            Locator_t temp_locator;
            if (network.transform_remote_locator(locator, temp_locator, m_network_configuration, from_this_host))
            {
                ProxyDataFilters::filter_locators(network, remote_locators, temp_locator, true);
            }
        }
        for (const Locator_t& locator : rdata.remote_locators.multicast)
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

void ReaderProxyData::clear()
{
    //Clear SubscriptionBuiltinTopicData
    SubscriptionBuiltinTopicData::key = BuiltinTopicKey_t{{0, 0, 0}};
    participant_key = BuiltinTopicKey_t{{0, 0, 0}};
    type_name = "";
    topic_name = "";
    topic_kind = NO_KEY;

    durability.clear();
    deadline.clear();
    latency_budget.clear();
    liveliness.clear();
    reliability.clear();
    ownership.clear();
    destination_order.clear();
    user_data.clear();
    time_based_filter.clear();
    presentation.clear();
    partition.clear();
    topic_data.clear();
    group_data.clear();
    lifespan.clear();
    disable_positive_acks.clear();
    representation.clear();
    type_consistency.clear();
    type_information.clear();
    data_sharing.clear();
    if (history)
    {
        history->clear();
    }
    if (resource_limits)
    {
        resource_limits->clear();
    }
    if (reader_data_lifecycle)
    {
        reader_data_lifecycle->clear();
    }
    if (rtps_reliable_reader)
    {
        rtps_reliable_reader->clear();
    }
    if (endpoint)
    {
        endpoint->clear();
    }
    if (reader_resource_limits)
    {
        reader_resource_limits->clear();
    }

    content_filter.filter_class_name = "";
    content_filter.content_filtered_topic_name = "";
    content_filter.related_topic_name = "";
    content_filter.filter_expression = "";
    content_filter.expression_parameters.clear();
    guid = c_Guid_Unknown;
    participant_guid = c_Guid_Unknown;
    remote_locators.unicast.clear();
    remote_locators.multicast.clear();
    loopback_transformation = NetworkConfigSet_t();
    expects_inline_qos = false;
    properties.clear();
    properties.length = 0;

    //Clear ReaderProxyData
    m_network_configuration = 0;
    m_key = InstanceHandle_t();
    m_rtps_participant_key = InstanceHandle_t();
    m_user_defined_id = 0;
    m_is_alive = true;

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

bool ReaderProxyData::is_update_allowed(
        const ReaderProxyData& rdata) const
{
    if ((guid != rdata.guid) ||
#if HAVE_SECURITY
            (security_attributes_ != rdata.security_attributes_) ||
            (plugin_security_attributes_ != rdata.plugin_security_attributes_) ||
#endif // if HAVE_SECURITY
            (type_name != rdata.type_name) ||
            (topic_name != rdata.topic_name))
    {
        return false;
    }

    return can_qos_be_updated(rdata);
}

void ReaderProxyData::update(
        ReaderProxyData* rdata)
{
    remote_locators = rdata->remote_locators;
    set_qos(*rdata, false);
    m_is_alive = rdata->m_is_alive;
    expects_inline_qos = rdata->expects_inline_qos;
    content_filter = rdata->content_filter;
}

void ReaderProxyData::set_qos(
        const SubscriptionBuiltinTopicData& qos,
        bool first_time)
{
    if (first_time)
    {
        durability = qos.durability;
        durability.hasChanged = true;
    }
    if (first_time || deadline.period != qos.deadline.period)
    {
        deadline = qos.deadline;
        deadline.hasChanged = true;
    }
    if (latency_budget.duration != qos.latency_budget.duration)
    {
        latency_budget = qos.latency_budget;
        latency_budget.hasChanged = true;
    }
    if (first_time)
    {
        liveliness = qos.liveliness;
        liveliness.hasChanged = true;
    }
    if (first_time)
    {
        reliability = qos.reliability;
        reliability.hasChanged = true;
    }
    if (first_time)
    {
        ownership = qos.ownership;
        ownership.hasChanged = true;
    }
    if (destination_order.kind != qos.destination_order.kind)
    {
        destination_order = qos.destination_order;
        destination_order.hasChanged = true;
    }
    if (first_time || user_data.data_vec() != qos.user_data.data_vec())
    {
        user_data = qos.user_data;
        user_data.hasChanged = true;
    }
    if (time_based_filter.minimum_separation != qos.time_based_filter.minimum_separation )
    {
        time_based_filter = qos.time_based_filter;
        time_based_filter.hasChanged = true;
    }
    if (first_time || presentation.access_scope != qos.presentation.access_scope ||
            presentation.coherent_access != qos.presentation.coherent_access ||
            presentation.ordered_access != qos.presentation.ordered_access)
    {
        presentation = qos.presentation;
        presentation.hasChanged = true;
    }
    if (first_time || qos.partition.names() != partition.names())
    {
        partition = qos.partition;
        partition.hasChanged = true;
    }
    if (first_time || topic_data.getValue() != qos.topic_data.getValue())
    {
        topic_data = qos.topic_data;
        topic_data.hasChanged = true;
    }
    if (first_time || group_data.getValue() != qos.group_data.getValue())
    {
        group_data = qos.group_data;
        group_data.hasChanged = true;
    }
    if (lifespan.duration != qos.lifespan.duration )
    {
        lifespan = qos.lifespan;
        lifespan.hasChanged = true;
    }
    if (first_time)
    {
        disable_positive_acks = qos.disable_positive_acks;
        disable_positive_acks.hasChanged = true;
    }

    if (representation.m_value != qos.representation.m_value)
    {
        representation = qos.representation;
        representation.hasChanged = true;
    }

    if (first_time ||
            type_consistency.m_kind != qos.type_consistency.m_kind ||
            type_consistency.m_ignore_member_names != qos.type_consistency.m_ignore_member_names ||
            type_consistency.m_ignore_string_bounds != qos.type_consistency.m_ignore_string_bounds ||
            type_consistency.m_ignore_sequence_bounds != qos.type_consistency.m_ignore_sequence_bounds ||
            type_consistency.m_force_type_validation != qos.type_consistency.m_force_type_validation ||
            type_consistency.m_prevent_type_widening != qos.type_consistency.m_prevent_type_widening)
    {
        type_consistency = qos.type_consistency;
        type_consistency.hasChanged = true;
    }

    if (!(data_sharing == qos.data_sharing))
    {
        data_sharing = qos.data_sharing;
        data_sharing.hasChanged = true;
    }
}

void ReaderProxyData::set_qos(
        const dds::ReaderQos& qos,
        bool first_time)
{
    if (first_time)
    {
        durability = qos.m_durability;
        durability.hasChanged = true;
    }
    if (first_time || deadline.period != qos.m_deadline.period)
    {
        deadline = qos.m_deadline;
        deadline.hasChanged = true;
    }
    if (latency_budget.duration != qos.m_latencyBudget.duration)
    {
        latency_budget = qos.m_latencyBudget;
        latency_budget.hasChanged = true;
    }
    if (first_time)
    {
        liveliness = qos.m_liveliness;
        liveliness.hasChanged = true;
    }
    if (first_time)
    {
        reliability = qos.m_reliability;
        reliability.hasChanged = true;
    }
    if (first_time)
    {
        ownership = qos.m_ownership;
        ownership.hasChanged = true;
    }
    if (destination_order.kind != qos.m_destinationOrder.kind)
    {
        destination_order = qos.m_destinationOrder;
        destination_order.hasChanged = true;
    }
    if (first_time || user_data.data_vec() != qos.m_userData.data_vec())
    {
        user_data = qos.m_userData;
        user_data.hasChanged = true;
    }
    if (time_based_filter.minimum_separation != qos.m_timeBasedFilter.minimum_separation )
    {
        time_based_filter = qos.m_timeBasedFilter;
        time_based_filter.hasChanged = true;
    }
    if (first_time || presentation.access_scope != qos.m_presentation.access_scope ||
            presentation.coherent_access != qos.m_presentation.coherent_access ||
            presentation.ordered_access != qos.m_presentation.ordered_access)
    {
        presentation = qos.m_presentation;
        presentation.hasChanged = true;
    }
    if (first_time || qos.m_partition.names() != partition.names())
    {
        partition = qos.m_partition;
        partition.hasChanged = true;
    }
    if (first_time || topic_data.getValue() != qos.m_topicData.getValue())
    {
        topic_data = qos.m_topicData;
        topic_data.hasChanged = true;
    }
    if (first_time || group_data.getValue() != qos.m_groupData.getValue())
    {
        group_data = qos.m_groupData;
        group_data.hasChanged = true;
    }
    if (lifespan.duration != qos.m_lifespan.duration )
    {
        lifespan = qos.m_lifespan;
        lifespan.hasChanged = true;
    }
    if (first_time)
    {
        disable_positive_acks = qos.m_disablePositiveACKs;
        disable_positive_acks.hasChanged = true;
    }

    if (representation.m_value != qos.representation.m_value)
    {
        representation = qos.representation;
        representation.hasChanged = true;
    }

    if (first_time ||
            type_consistency.m_kind != qos.type_consistency.m_kind ||
            type_consistency.m_ignore_member_names != qos.type_consistency.m_ignore_member_names ||
            type_consistency.m_ignore_string_bounds != qos.type_consistency.m_ignore_string_bounds ||
            type_consistency.m_ignore_sequence_bounds != qos.type_consistency.m_ignore_sequence_bounds ||
            type_consistency.m_force_type_validation != qos.type_consistency.m_force_type_validation ||
            type_consistency.m_prevent_type_widening != qos.type_consistency.m_prevent_type_widening)
    {
        type_consistency = qos.type_consistency;
        type_consistency.hasChanged = true;
    }

    if (!(data_sharing == qos.data_sharing))
    {
        data_sharing = qos.data_sharing;
        data_sharing.hasChanged = true;
    }
}

bool ReaderProxyData::can_qos_be_updated(
        const SubscriptionBuiltinTopicData& qos) const
{
    bool updatable = true;
    if ( durability.kind != qos.durability.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Durability kind cannot be changed after the creation of a subscriber.");
    }

    if (liveliness.kind != qos.liveliness.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Liveliness Kind cannot be changed after the creation of a subscriber.");
    }

    if (liveliness.lease_duration != qos.liveliness.lease_duration)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Liveliness lease duration cannot be changed after the creation of a subscriber.");
    }

    if (liveliness.announcement_period != qos.liveliness.announcement_period)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Liveliness announcement cannot be changed after the creation of a subscriber.");
    }

    if (reliability.kind != qos.reliability.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Reliability Kind cannot be changed after the creation of a subscriber.");
    }
    if (ownership.kind != qos.ownership.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK, "Ownership Kind cannot be changed after the creation of a subscriber.");
    }
    if (destination_order.kind != qos.destination_order.kind)
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Destination order Kind cannot be changed after the creation of a subscriber.");
    }
    if (data_sharing.kind() != qos.data_sharing.kind() ||
            data_sharing.domain_ids() != qos.data_sharing.domain_ids())
    {
        updatable = false;
        EPROSIMA_LOG_WARNING(RTPS_QOS_CHECK,
                "Data sharing configuration cannot be changed after the creation of a subscriber.");
    }
    return updatable;
}

void ReaderProxyData::add_unicast_locator(
        const Locator_t& locator)
{
    remote_locators.add_unicast_locator(locator);
}

void ReaderProxyData::set_announced_unicast_locators(
        const LocatorList_t& locators)
{
    remote_locators.unicast.clear();
    for (const Locator_t& locator : locators)
    {
        remote_locators.add_unicast_locator(locator);
    }
}

void ReaderProxyData::set_remote_unicast_locators(
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

void ReaderProxyData::add_multicast_locator(
        const Locator_t& locator)
{
    remote_locators.add_multicast_locator(locator);
}

void ReaderProxyData::set_multicast_locators(
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

void ReaderProxyData::set_locators(
        const RemoteLocatorList& locators)
{
    remote_locators = locators;
}

void ReaderProxyData::set_remote_locators(
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
