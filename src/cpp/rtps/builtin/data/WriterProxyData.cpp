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

#include <fastrtps/rtps/builtin/data/WriterProxyData.h>

#include <fastrtps/rtps/common/CDRMessage_t.h>

#include <fastrtps/log/Log.h>

#include <fastrtps/rtps/network/NetworkFactory.h>

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
#endif
    , m_userDefinedId(0)
    , m_typeMaxSerialized(0)
    , m_topicKind(NO_KEY)
    , m_topicDiscoveryKind(NO_CHECK)
    , m_type_id(nullptr)
    , m_type(nullptr)
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
}

WriterProxyData::WriterProxyData(
        const WriterProxyData& writerInfo)
#if HAVE_SECURITY
    : security_attributes_(writerInfo.security_attributes_)
    , plugin_security_attributes_(writerInfo.plugin_security_attributes_)
    , m_guid(writerInfo.m_guid)
#else
    : m_guid(writerInfo.m_guid)
#endif
    , remote_locators_(writerInfo.remote_locators_)
    , m_key(writerInfo.m_key)
    , m_RTPSParticipantKey(writerInfo.m_RTPSParticipantKey)
    , m_typeName(writerInfo.m_typeName)
    , m_topicName(writerInfo.m_topicName)
    , m_userDefinedId(writerInfo.m_userDefinedId)
    , m_typeMaxSerialized(writerInfo.m_typeMaxSerialized)
    , m_topicKind(writerInfo.m_topicKind)
    , persistence_guid_(writerInfo.persistence_guid_)
    , m_topicDiscoveryKind(writerInfo.m_topicDiscoveryKind)
    , m_type_id(nullptr)
    , m_type(nullptr)
{
    if (writerInfo.m_type_id)
    {
        type_id(*writerInfo.m_type_id);
    }

    if (writerInfo.m_type)
    {
        type(*writerInfo.m_type);
    }

    m_qos.setQos(writerInfo.m_qos, true);
}

WriterProxyData::~WriterProxyData()
{
    delete m_type;
    delete m_type_id;

    logInfo(RTPS_PROXY_DATA, m_guid);
}

WriterProxyData& WriterProxyData::operator =(
        const WriterProxyData& writerInfo)
{
#if HAVE_SECURITY
    security_attributes_ = writerInfo.security_attributes_;
    plugin_security_attributes_ = writerInfo.plugin_security_attributes_;
#endif
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
    m_topicDiscoveryKind = writerInfo.m_topicDiscoveryKind;

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
    ret_val += ParameterString_t::cdr_serialized_size(m_topicName);

    // PID_TYPE_NAME
    ret_val += ParameterString_t::cdr_serialized_size(m_typeName);

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
    if (m_qos.m_durability.sendAlways() || m_qos.m_durability.hasChanged)
    {
        ret_val += m_qos.m_durability.cdr_serialized_size();
    }
    if (m_qos.m_durabilityService.sendAlways() || m_qos.m_durabilityService.hasChanged)
    {
        ret_val += m_qos.m_durabilityService.cdr_serialized_size();
    }
    if (m_qos.m_deadline.sendAlways() || m_qos.m_deadline.hasChanged)
    {
        ret_val += m_qos.m_deadline.cdr_serialized_size();
    }
    if (m_qos.m_latencyBudget.sendAlways() || m_qos.m_latencyBudget.hasChanged)
    {
        ret_val += m_qos.m_latencyBudget.cdr_serialized_size();
    }
    if (m_qos.m_liveliness.sendAlways() || m_qos.m_liveliness.hasChanged)
    {
        ret_val += m_qos.m_liveliness.cdr_serialized_size();
    }
    if (m_qos.m_reliability.sendAlways() || m_qos.m_reliability.hasChanged)
    {
        ret_val += m_qos.m_reliability.cdr_serialized_size();
    }
    if (m_qos.m_lifespan.sendAlways() || m_qos.m_lifespan.hasChanged)
    {
        ret_val += m_qos.m_lifespan.cdr_serialized_size();
    }
    if (m_qos.m_userData.sendAlways() || m_qos.m_userData.hasChanged)
    {
        ret_val += m_qos.m_userData.cdr_serialized_size();
    }
    if (m_qos.m_timeBasedFilter.sendAlways() || m_qos.m_timeBasedFilter.hasChanged)
    {
        ret_val += m_qos.m_timeBasedFilter.cdr_serialized_size();
    }
    if (m_qos.m_ownership.sendAlways() || m_qos.m_ownership.hasChanged)
    {
        ret_val += m_qos.m_ownership.cdr_serialized_size();
    }
    if (m_qos.m_ownershipStrength.sendAlways() || m_qos.m_ownershipStrength.hasChanged)
    {
        ret_val += m_qos.m_ownershipStrength.cdr_serialized_size();
    }
    if (m_qos.m_destinationOrder.sendAlways() || m_qos.m_destinationOrder.hasChanged)
    {
        ret_val += m_qos.m_destinationOrder.cdr_serialized_size();
    }
    if (m_qos.m_presentation.sendAlways() || m_qos.m_presentation.hasChanged)
    {
        ret_val += m_qos.m_presentation.cdr_serialized_size();
    }
    if (m_qos.m_partition.sendAlways() || m_qos.m_partition.hasChanged)
    {
        ret_val += m_qos.m_partition.cdr_serialized_size();
    }
    if (m_qos.m_topicData.sendAlways() || m_qos.m_topicData.hasChanged)
    {
        ret_val += m_qos.m_topicData.cdr_serialized_size();
    }
    if (m_qos.m_disablePositiveACKs.sendAlways() || m_qos.m_disablePositiveACKs.hasChanged)
    {
        ret_val += m_qos.m_disablePositiveACKs.cdr_serialized_size();
    }
    if (m_qos.m_groupData.sendAlways() || m_qos.m_groupData.hasChanged)
    {
        ret_val += m_qos.m_groupData.cdr_serialized_size();
    }

    if (m_topicDiscoveryKind != NO_CHECK)
    {
        if (m_type_id && m_type_id->m_type_identifier._d() != 0)
        {
            ret_val += m_type_id->cdr_serialized_size();
        }

        if (m_type && m_type->m_type_object._d() != 0)
        {
            ret_val += m_type->cdr_serialized_size();
        }
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
        ParameterLocator_t p(PID_UNICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, locator);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    for (const Locator_t& locator : remote_locators_.multicast)
    {
        ParameterLocator_t p(PID_MULTICAST_LOCATOR, PARAMETER_LOCATOR_LENGTH, locator);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterGuid_t p(PID_PARTICIPANT_GUID, PARAMETER_GUID_LENGTH, m_RTPSParticipantKey);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterString_t p(PID_TOPIC_NAME, 0, m_topicName);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterString_t p(PID_TYPE_NAME, 0, m_typeName);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterKey_t p(PID_KEY_HASH, 16, m_key);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterGuid_t p(PID_ENDPOINT_GUID, 16, m_guid);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterPort_t p(PID_TYPE_MAX_SIZE_SERIALIZED, 4, m_typeMaxSerialized);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterProtocolVersion_t p(PID_PROTOCOL_VERSION, 4);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    {
        ParameterVendorId_t p(PID_VENDORID, 4);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (persistence_guid_ != c_Guid_Unknown)
    {
        ParameterGuid_t p(PID_PERSISTENCE_GUID, 16, persistence_guid_);
        if (!p.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_durability.sendAlways() || m_qos.m_durability.hasChanged)
    {
        if (!m_qos.m_durability.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_durabilityService.sendAlways() || m_qos.m_durabilityService.hasChanged)
    {
        if (!m_qos.m_durabilityService.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_deadline.sendAlways() ||  m_qos.m_deadline.hasChanged)
    {
        if (!m_qos.m_deadline.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_latencyBudget.sendAlways() ||  m_qos.m_latencyBudget.hasChanged)
    {
        if (!m_qos.m_latencyBudget.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_liveliness.sendAlways() ||  m_qos.m_liveliness.hasChanged)
    {
        if (!m_qos.m_liveliness.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_reliability.sendAlways() ||  m_qos.m_reliability.hasChanged)
    {
        if (!m_qos.m_reliability.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_lifespan.sendAlways() ||  m_qos.m_lifespan.hasChanged)
    {
        if (!m_qos.m_lifespan.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if ( m_qos.m_userData.sendAlways() || m_qos.m_userData.hasChanged)
    {
        if (!m_qos.m_userData.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_timeBasedFilter.sendAlways() ||  m_qos.m_timeBasedFilter.hasChanged)
    {
        if (!m_qos.m_timeBasedFilter.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_ownership.sendAlways() ||  m_qos.m_ownership.hasChanged)
    {
        if (!m_qos.m_ownership.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_ownershipStrength.sendAlways() ||  m_qos.m_ownershipStrength.hasChanged)
    {
        if (!m_qos.m_ownershipStrength.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_destinationOrder.sendAlways() ||  m_qos.m_destinationOrder.hasChanged)
    {
        if (!m_qos.m_destinationOrder.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_presentation.sendAlways() ||  m_qos.m_presentation.hasChanged)
    {
        if (!m_qos.m_presentation.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_partition.sendAlways() ||  m_qos.m_partition.hasChanged)
    {
        if (!m_qos.m_partition.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_topicData.sendAlways() || m_qos.m_topicData.hasChanged)
    {
        if (!m_qos.m_topicData.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_disablePositiveACKs.sendAlways() || m_qos.m_topicData.hasChanged)
    {
        if (!m_qos.m_disablePositiveACKs.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_groupData.sendAlways() ||  m_qos.m_groupData.hasChanged)
    {
        if (!m_qos.m_groupData.addToCDRMessage(msg))
        {
            return false;
        }
    }

    if (m_topicDiscoveryKind != NO_CHECK)
    {
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

    return CDRMessage::addParameterSentinel(msg);
}

bool WriterProxyData::readFromCDRMessage(
        CDRMessage_t* msg,
        const NetworkFactory& network)
{
    auto param_process = [this, &network](CDRMessage_t* msg, const ParameterId_t& pid, uint16_t plength)
    {
        switch (pid)
        {
            case PID_DURABILITY:
            {
                if (!m_qos.m_durability.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_DURABILITY_SERVICE:
            {
                if (!m_qos.m_durabilityService.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_DEADLINE:
            {
                if (!m_qos.m_deadline.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_LATENCY_BUDGET:
            {
                if (!m_qos.m_latencyBudget.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_LIVELINESS:
            {
                if (!m_qos.m_liveliness.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_RELIABILITY:
            {
                if (!m_qos.m_reliability.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_LIFESPAN:
            {
                if (!m_qos.m_lifespan.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_USER_DATA:
            {
                if (!m_qos.m_userData.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_TIME_BASED_FILTER:
            {
                if (!m_qos.m_timeBasedFilter.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_OWNERSHIP:
            {
                if (!m_qos.m_ownership.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_OWNERSHIP_STRENGTH:
            {
                if (!m_qos.m_ownershipStrength.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_DESTINATION_ORDER:
            {
                if (!m_qos.m_destinationOrder.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_PRESENTATION:
            {
                if (!m_qos.m_presentation.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_PARTITION:
            {
                if (!m_qos.m_partition.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_TOPIC_DATA:
            {
                if (!m_qos.m_topicData.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_GROUP_DATA:
            {
                if (!m_qos.m_groupData.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
            case PID_TOPIC_NAME:
            {
                ParameterString_t p(pid, plength);
                if (!p.readFromCDRMessage(msg, plength))
                {
                    return false;
                }

                m_topicName = p.getName();
                break;
            }
            case PID_TYPE_NAME:
            {
                ParameterString_t p(pid, plength);
                if (!p.readFromCDRMessage(msg, plength))
                {
                    return false;
                }

                m_typeName = p.getName();
                break;
            }
            case PID_PARTICIPANT_GUID:
            {
                ParameterGuid_t p(pid, plength);
                if (!p.readFromCDRMessage(msg, plength))
                {
                    return false;
                }

                for (uint8_t i = 0; i < 16; ++i)
                {
                    if (i < 12)
                    {
                        m_RTPSParticipantKey.value[i] = p.guid.guidPrefix.value[i];
                    }
                    else
                    {
                        m_RTPSParticipantKey.value[i] = p.guid.entityId.value[i - 12];
                    }
                }
                break;
            }
            case PID_ENDPOINT_GUID:
            {
                ParameterGuid_t p(pid, plength);
                if (!p.readFromCDRMessage(msg, plength))
                {
                    return false;
                }

                m_guid = p.guid;
                for (uint8_t i = 0; i < 16; ++i)
                {
                    if (i < 12)
                    {
                        m_key.value[i] = p.guid.guidPrefix.value[i];
                    }
                    else
                    {
                        m_key.value[i] = p.guid.entityId.value[i - 12];
                    }
                }
                break;
            }
            case PID_PERSISTENCE_GUID:
            {
                ParameterGuid_t p(pid, plength);
                if (!p.readFromCDRMessage(msg, plength))
                {
                    return false;
                }

                persistence_guid_ = p.guid;
                break;
            }
            case PID_UNICAST_LOCATOR:
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
            case PID_MULTICAST_LOCATOR:
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
            case PID_KEY_HASH:
            {
                ParameterKey_t p(PID_KEY_HASH, plength);
                if (!p.readFromCDRMessage(msg, plength))
                {
                    return false;
                }

                m_key = p.key;
                iHandle2GUID(m_guid, m_key);
                break;
            }
            case PID_TYPE_IDV1:
            {
                TypeIdV1 p;
                if (!p.readFromCDRMessage(msg, plength))
                {
                    return false;
                }

                type_id(p);
                m_topicDiscoveryKind = MINIMAL;
                if (m_type_id->m_type_identifier._d() == types::EK_COMPLETE)
                {
                    m_topicDiscoveryKind = COMPLETE;
                }
                break;
            }
            case PID_TYPE_OBJECTV1:
            {
                TypeObjectV1 p;
                if (!p.readFromCDRMessage(msg, plength))
                {
                    return false;
                }

                if (m_type == nullptr)
                {
                    m_type = new TypeObjectV1();
                }
                *m_type = p;
                m_topicDiscoveryKind = MINIMAL;
                if (m_type->m_type_object._d() == types::EK_COMPLETE)
                {
                    m_topicDiscoveryKind = COMPLETE;
                }
                break;
            }
            case PID_DISABLE_POSITIVE_ACKS:
            {
                if (!m_qos.m_disablePositiveACKs.readFromCDRMessage(msg, plength))
                {
                    return false;
                }
                break;
            }
#if HAVE_SECURITY
            case PID_ENDPOINT_SECURITY_INFO:
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
    m_topicDiscoveryKind = NO_CHECK;
    if (m_type_id)
    {
        *m_type_id = TypeIdV1();
    }
    if (m_type)
    {
        *m_type = TypeObjectV1();
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
    m_topicDiscoveryKind = wdata->m_topicDiscoveryKind;
    if (m_topicDiscoveryKind != NO_CHECK)
    {
        m_type_id = wdata->m_type_id;
        m_type = wdata->m_type;
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
#endif
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
