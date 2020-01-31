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

#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/common/CDRMessage_t.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/rtps/network/NetworkFactory.h>

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
    , m_topicDiscoveryKind(NO_CHECK)
    , m_type_id(nullptr)
    , m_type(nullptr)
{
}

ReaderProxyData::ReaderProxyData (
        const size_t max_unicast_locators,
        const size_t max_multicast_locators,
        const VariableLengthDataLimits& data_limits)
    : ReaderProxyData(max_unicast_locators, max_multicast_locators)
{
    m_qos.m_userData.max_size((uint32_t)data_limits.max_user_data);
    m_qos.m_partition.max_size((uint32_t)data_limits.max_partitions);
}

ReaderProxyData::~ReaderProxyData()
{
    delete m_type;
    delete m_type_id;

    logInfo(RTPS_PROXY_DATA, "ReaderProxyData destructor: " << this->m_guid; );
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
    , m_topicDiscoveryKind(readerInfo.m_topicDiscoveryKind)
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
    m_topicDiscoveryKind = readerInfo.m_topicDiscoveryKind;

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

bool ReaderProxyData::writeToCDRMessage(
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
        ParameterBool_t p(PID_EXPECTS_INLINE_QOS, PARAMETER_BOOL_LENGTH, m_expectsInlineQos);
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
    if (m_qos.m_deadline.sendAlways() || m_qos.m_deadline.hasChanged)
    {
        if (!m_qos.m_deadline.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_latencyBudget.sendAlways() || m_qos.m_latencyBudget.hasChanged)
    {
        if (!m_qos.m_latencyBudget.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_liveliness.sendAlways() || m_qos.m_liveliness.hasChanged)
    {
        if (!m_qos.m_liveliness.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_reliability.sendAlways() || m_qos.m_reliability.hasChanged)
    {
        if (!m_qos.m_reliability.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_lifespan.sendAlways() || m_qos.m_lifespan.hasChanged)
    {
        if (!m_qos.m_lifespan.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_userData.sendAlways() || m_qos.m_userData.hasChanged)
    {
        if (!m_qos.m_userData.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_timeBasedFilter.sendAlways() || m_qos.m_timeBasedFilter.hasChanged)
    {
        if (!m_qos.m_timeBasedFilter.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_ownership.sendAlways() || m_qos.m_ownership.hasChanged)
    {
        if (!m_qos.m_ownership.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_destinationOrder.sendAlways() || m_qos.m_destinationOrder.hasChanged)
    {
        if (!m_qos.m_destinationOrder.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_presentation.sendAlways() || m_qos.m_presentation.hasChanged)
    {
        if (!m_qos.m_presentation.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_partition.sendAlways() || m_qos.m_partition.hasChanged)
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
    if (m_qos.m_groupData.sendAlways() || m_qos.m_groupData.hasChanged)
    {
        if (!m_qos.m_groupData.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_timeBasedFilter.sendAlways() || m_qos.m_timeBasedFilter.hasChanged)
    {
        if (!m_qos.m_timeBasedFilter.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if (m_qos.m_disablePositiveACKs.sendAlways() || m_qos.m_disablePositiveACKs.hasChanged)
    {
        if (!m_qos.m_disablePositiveACKs.addToCDRMessage(msg))
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
    if ((this->security_attributes_ != 0UL) || (this->plugin_security_attributes_ != 0UL))
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

bool ReaderProxyData::readFromCDRMessage(
        CDRMessage_t* msg,
        const NetworkFactory& network)
{
    auto param_process = [this, &network](CDRMessage_t* msg, const ParameterId_t &pid, uint16_t plength)
        {
            try
            {
                switch (pid)
                {
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
                    case PID_EXPECTS_INLINE_QOS:
                    {
                        ParameterBool_t p(pid, plength);
                        if (!p.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }

                        m_expectsInlineQos = p.value;
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
                    case PID_KEY_HASH:
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
                    case PID_DURABILITY:
                    {
                        if (!m_qos.m_durability.readFromCDRMessage(msg, plength))
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
                    case PID_OWNERSHIP:
                    {
                        if (!m_qos.m_ownership.readFromCDRMessage(msg, plength))
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
                    case PID_DESTINATION_ORDER:
                    {
                        if (!m_qos.m_destinationOrder.readFromCDRMessage(msg, plength))
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
                    case PID_DURABILITY_SERVICE:
                    {
                        if (!m_qos.m_durabilityService.readFromCDRMessage(msg, plength))
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
                    case PID_DATA_REPRESENTATION:
                    {
                        if (!m_qos.m_dataRepresentation.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
                        break;
                    }
                    case PID_TYPE_CONSISTENCY_ENFORCEMENT:
                    {
                        if (!m_qos.m_typeConsistency.readFromCDRMessage(msg, plength))
                        {
                            return false;
                        }
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

                        type(p);
                        m_topicDiscoveryKind = MINIMAL;
                        if (m_type->m_type_object._d() == types::EK_COMPLETE)
                        {
                            m_topicDiscoveryKind = COMPLETE;
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
                    case PID_DISABLE_POSITIVE_ACKS:
                    {
                        if (!m_qos.m_disablePositiveACKs.readFromCDRMessage(msg, plength))
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
            }
            catch (std::bad_alloc& ba)
            {
                std::cerr << "bad_alloc caught: " << ba.what() << '\n';
                return false;
            }

            return true;
        };

    uint32_t qos_size;
    clear();
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
    //clear user data but keep max size on qos
    uint32_t max_user_data = (uint32_t)m_qos.m_userData.max_size();
    uint32_t max_partitions = (uint32_t)m_qos.m_partition.max_size();
    m_qos = ReaderQos();
    m_qos.m_userData.max_size(max_user_data);
    m_qos.m_partition.max_size(max_partitions);
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
    //cout << "COPYING DATA: expects inlineQOS : " << rdata->m_expectsInlineQos << endl;
    m_expectsInlineQos = rdata->m_expectsInlineQos;
    m_isAlive = rdata->m_isAlive;
    m_topicKind = rdata->m_topicKind;
    m_topicDiscoveryKind = rdata->m_topicDiscoveryKind;
    if (m_topicDiscoveryKind != NO_CHECK)
    {
        m_type_id = rdata->m_type_id;
        m_type = rdata->m_type;
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
