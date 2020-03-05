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
 *
 */

#include <fastdds/rtps/builtin/data/ReaderProxyData.h>
#include <fastdds/rtps/common/CDRMessage_t.h>
#include <fastrtps/log/Log.h>
#include <fastdds/rtps/network/NetworkFactory.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {


ReaderProxyData::ReaderProxyData (
        const size_t max_unicast_locators,
        const size_t max_multicast_locators)
    : m_expectsInlineQos(false)
#if HAVE_SECURITY
    , security_attributes_(0UL)
    , plugin_security_attributes_(0UL)
#endif
    , remote_locators_(max_unicast_locators, max_multicast_locators)
    , m_userDefinedId(0)
    , m_isAlive(true)
    , m_topicKind(NO_KEY)
    , m_type_id(nullptr)
    , m_type(nullptr)
    , m_type_information(nullptr)
{
    // As DDS-XTypes, v1.2 (page 182) document stablishes, local default is ALLOW_TYPE_COERCION,
    // but when remotes doesn't send TypeConsistencyQos, we must assume DISALLOW.
    m_qos.type_consistency.m_kind = DISALLOW_TYPE_COERCION;
}

ReaderProxyData::ReaderProxyData (
        const size_t max_unicast_locators,
        const size_t max_multicast_locators,
        const VariableLengthDataLimits& data_limits)
    : ReaderProxyData(max_unicast_locators, max_multicast_locators)
{
    m_qos.m_userData.set_max_size(static_cast<uint32_t>(data_limits.max_user_data));
    m_qos.m_partition.set_max_size(static_cast<uint32_t>(data_limits.max_partitions));
}

ReaderProxyData::~ReaderProxyData()
{
    delete m_type;
    delete m_type_id;
    delete m_type_information;

    logInfo(RTPS_PROXY_DATA, "ReaderProxyData destructor: " << m_guid; );
}

ReaderProxyData::ReaderProxyData(
        const ReaderProxyData& readerInfo)
    : m_expectsInlineQos(readerInfo.m_expectsInlineQos)
#if HAVE_SECURITY
    , security_attributes_(readerInfo.security_attributes_)
    , plugin_security_attributes_(readerInfo.plugin_security_attributes_)
#endif
    , m_guid(readerInfo.m_guid)
    , remote_locators_(readerInfo.remote_locators_)
    , m_key(readerInfo.m_key)
    , m_RTPSParticipantKey(readerInfo.m_RTPSParticipantKey)
    , m_typeName(readerInfo.m_typeName)
    , m_topicName(readerInfo.m_topicName)
    , m_userDefinedId(readerInfo.m_userDefinedId)
    , m_isAlive(readerInfo.m_isAlive)
    , m_topicKind(readerInfo.m_topicKind)
    , m_type_id(nullptr)
    , m_type(nullptr)
    , m_type_information(nullptr)
{
    if (readerInfo.m_type_id)
    {
        type_id(*readerInfo.m_type_id);
    }

    if (readerInfo.m_type)
    {
        type(*readerInfo.m_type);
    }

    if (readerInfo.m_type_information)
    {
        type_information(*readerInfo.m_type_information);
    }

    m_qos.setQos(readerInfo.m_qos, true);
}

ReaderProxyData& ReaderProxyData::operator =(
        const ReaderProxyData& readerInfo)
{
    m_expectsInlineQos = readerInfo.m_expectsInlineQos;
#if HAVE_SECURITY
    security_attributes_ = readerInfo.security_attributes_;
    plugin_security_attributes_ = readerInfo.plugin_security_attributes_;
#endif
    m_guid = readerInfo.m_guid;
    remote_locators_ = readerInfo.remote_locators_;
    m_key = readerInfo.m_key;
    m_RTPSParticipantKey = readerInfo.m_RTPSParticipantKey;
    m_typeName = readerInfo.m_typeName;
    m_topicName = readerInfo.m_topicName;
    m_userDefinedId = readerInfo.m_userDefinedId;
    m_isAlive = readerInfo.m_isAlive;
    m_expectsInlineQos = readerInfo.m_expectsInlineQos;
    m_topicKind = readerInfo.m_topicKind;
    m_qos.setQos(readerInfo.m_qos, true);

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

    if (readerInfo.m_type_information)
    {
        type_information(*readerInfo.m_type_information);
    }
    else
    {
        delete m_type_information;
        m_type_information = nullptr;
    }

    return *this;
}

uint32_t ReaderProxyData::get_serialized_size(
        bool include_encapsulation) const
{
    uint32_t ret_val = include_encapsulation ? 4 : 0;

    // PID_UNICAST_LOCATOR
    ret_val += static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * remote_locators_.unicast.size());

    // PID_MULTICAST_LOCATOR
    ret_val += static_cast<uint32_t>((4 + PARAMETER_LOCATOR_LENGTH) * remote_locators_.multicast.size());

    // PID_EXPECTS_INLINE_QOS
    ret_val += 4 + PARAMETER_BOOL_LENGTH;

    // PID_PARTICIPANT_GUID
    ret_val += 4 + PARAMETER_GUID_LENGTH;

    // PID_TOPIC_NAME
    ret_val += ParameterString_t::cdr_serialized_size(m_topicName);

    // PID_TYPE_NAME
    ret_val += ParameterString_t::cdr_serialized_size(m_typeName);

    // PID_KEY_HASH
    ret_val += 4 + 16;

    // PID_ENDPOINT_GUID
    ret_val += 4 + PARAMETER_GUID_LENGTH;

    // PID_PROTOCOL_VERSION
    ret_val += 4 + 4;

    // PID_VENDORID
    ret_val += 4 + 4;

    if (m_qos.m_durability.send_always() || m_qos.m_durability.hasChanged)
    {
        ret_val += m_qos.m_durability.cdr_serialized_size();
    }
    if (m_qos.m_durabilityService.send_always() || m_qos.m_durabilityService.hasChanged)
    {
        ret_val += m_qos.m_durabilityService.cdr_serialized_size();
    }
    if (m_qos.m_deadline.send_always() || m_qos.m_deadline.hasChanged)
    {
        ret_val += m_qos.m_deadline.cdr_serialized_size();
    }
    if (m_qos.m_latencyBudget.send_always() || m_qos.m_latencyBudget.hasChanged)
    {
        ret_val += m_qos.m_latencyBudget.cdr_serialized_size();
    }
    if (m_qos.m_liveliness.send_always() || m_qos.m_liveliness.hasChanged)
    {
        ret_val += m_qos.m_liveliness.cdr_serialized_size();
    }
    if (m_qos.m_reliability.send_always() || m_qos.m_reliability.hasChanged)
    {
        ret_val += m_qos.m_reliability.cdr_serialized_size();
    }
    if (m_qos.m_lifespan.send_always() || m_qos.m_lifespan.hasChanged)
    {
        ret_val += m_qos.m_lifespan.cdr_serialized_size();
    }
    if (m_qos.m_userData.send_always() || m_qos.m_userData.hasChanged)
    {
        ret_val += m_qos.m_userData.cdr_serialized_size();
    }
    if (m_qos.m_timeBasedFilter.send_always() || m_qos.m_timeBasedFilter.hasChanged)
    {
        ret_val += m_qos.m_timeBasedFilter.cdr_serialized_size();
    }
    if (m_qos.m_ownership.send_always() || m_qos.m_ownership.hasChanged)
    {
        ret_val += m_qos.m_ownership.cdr_serialized_size();
    }
    if (m_qos.m_destinationOrder.send_always() || m_qos.m_destinationOrder.hasChanged)
    {
        ret_val += m_qos.m_destinationOrder.cdr_serialized_size();
    }
    if (m_qos.m_presentation.send_always() || m_qos.m_presentation.hasChanged)
    {
        ret_val += m_qos.m_presentation.cdr_serialized_size();
    }
    if (m_qos.m_partition.send_always() || m_qos.m_partition.hasChanged)
    {
        ret_val += m_qos.m_partition.cdr_serialized_size();
    }
    if (m_qos.m_topicData.send_always() || m_qos.m_topicData.hasChanged)
    {
        ret_val += m_qos.m_topicData.cdr_serialized_size();
    }
    if (m_qos.m_groupData.send_always() || m_qos.m_groupData.hasChanged)
    {
        ret_val += m_qos.m_groupData.cdr_serialized_size();
    }
    if (m_qos.m_disablePositiveACKs.send_always() || m_qos.m_disablePositiveACKs.hasChanged)
    {
        ret_val += m_qos.m_disablePositiveACKs.cdr_serialized_size();
    }
    if (m_type_id && m_type_id->m_type_identifier._d() != 0)
    {
        ret_val += m_type_id->cdr_serialized_size();
    }
    if (m_type && m_type->m_type_object._d() != 0)
    {
        ret_val += m_type->cdr_serialized_size();
    }
    if (m_type_information && m_type_information->assigned())
    {
        ret_val += m_type_information->cdr_serialized_size();
    }
    if (m_qos.type_consistency.send_always() || m_qos.type_consistency.hasChanged)
    {
        ret_val += m_qos.type_consistency.cdr_serialized_size();
    }

#if HAVE_SECURITY
    if ((this->security_attributes_ != 0UL) || (this->plugin_security_attributes_ != 0UL))
    {
        ret_val += 4 + PARAMETER_ENDPOINT_SECURITY_INFO_LENGTH;
    }
#endif

    // PID_SENTINEL
    return ret_val + 4;
}

bool ReaderProxyData::writeToCDRMessage(
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
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    for (const Locator_t& locator : remote_locators_.multicast)
    {
        ParameterLocator_t p(fastdds::dds::PID_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, locator);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterBool_t p(fastdds::dds::PID_EXPECTS_INLINE_QOS, PARAMETER_BOOL_LENGTH, m_expectsInlineQos);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterGuid_t p(fastdds::dds::PID_PARTICIPANT_GUID, PARAMETER_GUID_LENGTH, m_RTPSParticipantKey);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterString_t p(fastdds::dds::PID_TOPIC_NAME, 0, m_topicName);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterString_t p(fastdds::dds::PID_TYPE_NAME, 0, m_typeName);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterKey_t p(fastdds::dds::PID_KEY_HASH, 16, m_key);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterGuid_t p(fastdds::dds::PID_ENDPOINT_GUID, PARAMETER_GUID_LENGTH, m_guid);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterProtocolVersion_t p(fastdds::dds::PID_PROTOCOL_VERSION, 4);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterVendorId_t p(fastdds::dds::PID_VENDORID, 4);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_durability.send_always() || m_qos.m_durability.hasChanged)
    {
        if (!m_qos.m_durability.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_durabilityService.send_always() || m_qos.m_durabilityService.hasChanged)
    {
        if (!m_qos.m_durabilityService.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_deadline.send_always() || m_qos.m_deadline.hasChanged)
    {
        if (!m_qos.m_deadline.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_latencyBudget.send_always() || m_qos.m_latencyBudget.hasChanged)
    {
        if (!m_qos.m_latencyBudget.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_liveliness.send_always() || m_qos.m_liveliness.hasChanged)
    {
        if (!m_qos.m_liveliness.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_reliability.send_always() || m_qos.m_reliability.hasChanged)
    {
        if (!m_qos.m_reliability.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_lifespan.send_always() || m_qos.m_lifespan.hasChanged)
    {
        if (!m_qos.m_lifespan.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_userData.send_always() || m_qos.m_userData.hasChanged)
    {
        if (!m_qos.m_userData.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_timeBasedFilter.send_always() || m_qos.m_timeBasedFilter.hasChanged)
    {
        if (!m_qos.m_timeBasedFilter.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_ownership.send_always() || m_qos.m_ownership.hasChanged)
    {
        if (!m_qos.m_ownership.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_destinationOrder.send_always() || m_qos.m_destinationOrder.hasChanged)
    {
        if (!m_qos.m_destinationOrder.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_presentation.send_always() || m_qos.m_presentation.hasChanged)
    {
        if (!m_qos.m_presentation.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_partition.send_always() || m_qos.m_partition.hasChanged)
    {
        if (!m_qos.m_partition.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_topicData.send_always() || m_qos.m_topicData.hasChanged)
    {
        if (!m_qos.m_topicData.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_groupData.send_always() || m_qos.m_groupData.hasChanged)
    {
        if (!m_qos.m_groupData.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_disablePositiveACKs.send_always() || m_qos.m_disablePositiveACKs.hasChanged)
    {
        if (!m_qos.m_disablePositiveACKs.addToCDRMessage(msg))
        {
            return false;
        }
    }

    if (m_type_id && m_type_id->m_type_identifier._d() != 0)
    {
        if (!m_type_id->addToCDRMessage(msg))
        {
            return false;
        }
    }

    if (m_type && m_type->m_type_object._d() != 0)
    {
        if (!m_type->addToCDRMessage(msg))
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
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
#endif

    /* TODO - Enable when implement XCDR, XCDR2 and/or XML
       if (m_qos.representation.send_always() || m_qos.representation.hasChanged)
       {
        if (!m_qos.representation.addToCDRMessage(msg)) return false;
       }
     */

    if (m_qos.type_consistency.send_always() || m_qos.type_consistency.hasChanged)
    {
        if (!m_qos.type_consistency.addToCDRMessage(msg))
        {
            return false;
        }
    }

    if (m_type_information && m_type_information->assigned())
    {
        if (!m_type_information->addToCDRMessage(msg))
        {
            return false;
        }
    }

    return CDRMessage::addParameterSentinel(msg);
}

bool ReaderProxyData::readFromCDRMessage(
        CDRMessage_t* msg,
        const NetworkFactory& network)
{
    auto param_process = [this, &network](CDRMessage_t* msg, const ParameterId_t& pid, uint16_t plength)
            {
                switch (pid)
                {
                    case fastdds::dds::PID_DURABILITY:
                    {
                        if (!m_qos.m_durability.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_DURABILITY_SERVICE:
                    {
                        if (!m_qos.m_durabilityService.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_DEADLINE:
                    {
                        if (!m_qos.m_deadline.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_LATENCY_BUDGET:
                    {
                        if (!m_qos.m_latencyBudget.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_LIVELINESS:
                    {
                        if (!m_qos.m_liveliness.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_RELIABILITY:
                    {
                        if (!m_qos.m_reliability.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_LIFESPAN:
                    {
                        if (!m_qos.m_lifespan.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_USER_DATA:
                    {
                        if (!m_qos.m_userData.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TIME_BASED_FILTER:
                    {
                        if (!m_qos.m_timeBasedFilter.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_OWNERSHIP:
                    {
                        if (!m_qos.m_ownership.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_DESTINATION_ORDER:
                    {
                        if (!m_qos.m_destinationOrder.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_PRESENTATION:
                    {
                        if (!m_qos.m_presentation.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_PARTITION:
                    {
                        if (!m_qos.m_partition.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TOPIC_DATA:
                    {
                        if (!m_qos.m_topicData.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_GROUP_DATA:
                    {
                        if (!m_qos.m_groupData.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TOPIC_NAME:
                    {
                        ParameterString_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        m_topicName = p.getName();
                        break;
                    }
                    case fastdds::dds::PID_TYPE_NAME:
                    {
                        ParameterString_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        m_typeName = p.getName();
                        break;
                    }
                    case fastdds::dds::PID_PARTICIPANT_GUID:
                    {
                        ParameterGuid_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
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
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        m_guid = p.guid;
                        memcpy(m_key.value, p.guid.guidPrefix.value, 12);
                        memcpy(m_key.value + 12, p.guid.entityId.value, 4);
                        break;
                    }
                    case fastdds::dds::PID_UNICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        Locator_t temp_locator;
                        if (network.transform_remote_locator(p.locator, temp_locator))
                        {
                            remote_locators_.add_unicast_locator(temp_locator);
                        }
                        break;
                    }
                    case fastdds::dds::PID_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        Locator_t temp_locator;
                        if (network.transform_remote_locator(p.locator, temp_locator))
                        {
                            remote_locators_.add_multicast_locator(temp_locator);
                        }
                        break;
                    }
                    case fastdds::dds::PID_EXPECTS_INLINE_QOS:
                    {
                        ParameterBool_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        m_expectsInlineQos = p.value;
                        break;
                    }
                    case fastdds::dds::PID_KEY_HASH:
                    {
                        ParameterKey_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        m_key = p.key;
                        iHandle2GUID(m_guid, m_key);
                        break;
                    }
                    case fastdds::dds::PID_DATA_REPRESENTATION:
                    {
                        if (!m_qos.representation.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TYPE_CONSISTENCY_ENFORCEMENT:
                    {
                        if (!m_qos.type_consistency.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TYPE_IDV1:
                    {
                        if (!type_id().readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TYPE_OBJECTV1:
                    {
                        if (!type().readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case fastdds::dds::PID_TYPE_INFORMATION:
                    {
                        if (!type_information().readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }

                    case fastdds::dds::PID_DISABLE_POSITIVE_ACKS:
                    {
                        if (!m_qos.m_disablePositiveACKs.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
#if HAVE_SECURITY
                    case fastdds::dds::PID_ENDPOINT_SECURITY_INFO:
                    {
                        ParameterEndpointSecurityInfo_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        security_attributes_ = p.security_attributes;
                        plugin_security_attributes_ = p.plugin_security_attributes;
                        break;
                    }
#endif
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
            if (m_guid.entityId.value[3] == 0x04)
            {
                m_topicKind = NO_KEY;
            }
            else if (m_guid.entityId.value[3] == 0x07)
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

void ReaderProxyData::clear()
{
    m_expectsInlineQos = false;
#if HAVE_SECURITY
    security_attributes_ = 0UL;
    plugin_security_attributes_ = 0UL;
#endif
    m_guid = c_Guid_Unknown;
    remote_locators_.unicast.clear();
    remote_locators_.multicast.clear();
    m_key = InstanceHandle_t();
    m_RTPSParticipantKey = InstanceHandle_t();
    m_typeName = "";
    m_topicName = "";
    m_userDefinedId = 0;
    m_isAlive = true;
    m_topicKind = NO_KEY;
    m_qos.clear();

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

bool ReaderProxyData::is_update_allowed(
        const ReaderProxyData& rdata) const
{
    if ( (m_guid != rdata.m_guid) ||
#if HAVE_SECURITY
            (security_attributes_ != rdata.security_attributes_) ||
            (plugin_security_attributes_ != rdata.security_attributes_) ||
#endif
            (m_typeName != rdata.m_typeName) ||
            (m_topicName != rdata.m_topicName) )
    {
        return false;
    }

    return m_qos.canQosBeUpdated(rdata.m_qos);
}

void ReaderProxyData::update(
        ReaderProxyData* rdata)
{
    remote_locators_ = rdata->remote_locators_;
    m_qos.setQos(rdata->m_qos, false);
    m_isAlive = rdata->m_isAlive;
    m_expectsInlineQos = rdata->m_expectsInlineQos;
}

void ReaderProxyData::copy(
        ReaderProxyData* rdata)
{
    m_guid = rdata->m_guid;
    remote_locators_ = rdata->remote_locators_;
    m_key = rdata->m_key;
    m_RTPSParticipantKey = rdata->m_RTPSParticipantKey;
    m_typeName = rdata->m_typeName;
    m_topicName = rdata->m_topicName;
    m_userDefinedId = rdata->m_userDefinedId;
    m_qos = rdata->m_qos;
    m_expectsInlineQos = rdata->m_expectsInlineQos;
    m_isAlive = rdata->m_isAlive;
    m_topicKind = rdata->m_topicKind;

    if (rdata->m_type_id)
    {
        type_id(*rdata->m_type_id);
    }
    else
    {
        delete m_type_id;
        m_type_id = nullptr;
    }

    if (rdata->m_type)
    {
        type(*rdata->m_type);
    }
    else
    {
        delete m_type;
        m_type = nullptr;
    }

    if (rdata->m_type_information)
    {
        type_information(*rdata->m_type_information);
    }
    else
    {
        delete m_type_information;
        m_type_information = nullptr;
    }
}

void ReaderProxyData::add_unicast_locator(
        const Locator_t& locator)
{
    remote_locators_.add_unicast_locator(locator);
}

void ReaderProxyData::set_announced_unicast_locators(
        const LocatorList_t& locators)
{
    remote_locators_.unicast.clear();
    for (const Locator_t& locator : locators)
    {
        remote_locators_.add_unicast_locator(locator);
    }
}

void ReaderProxyData::set_remote_unicast_locators(
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

void ReaderProxyData::add_multicast_locator(
        const Locator_t& locator)
{
    remote_locators_.add_multicast_locator(locator);
}

void ReaderProxyData::set_multicast_locators(
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

void ReaderProxyData::set_locators(
        const RemoteLocatorList& locators)
{
    remote_locators_ = locators;
}

void ReaderProxyData::set_remote_locators(
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
