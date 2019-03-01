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

namespace eprosima {
namespace fastrtps{
namespace rtps {


ReaderProxyData::ReaderProxyData()
    : m_expectsInlineQos(false)
#if HAVE_SECURITY
    , security_attributes_(0UL)
    , plugin_security_attributes_(0UL)
#endif
    , m_userDefinedId(0)
    , m_isAlive(true)
    , m_topicKind(NO_KEY)
    , m_topicDiscoveryKind(NO_CHECK)
    {

    }

ReaderProxyData::~ReaderProxyData()
{
    logInfo(RTPS_PROXY_DATA,"ReaderProxyData destructor: "<< this->m_guid;);
}

ReaderProxyData::ReaderProxyData(const ReaderProxyData& readerInfo)
    : m_expectsInlineQos(readerInfo.m_expectsInlineQos)
#if HAVE_SECURITY
    , security_attributes_(readerInfo.security_attributes_)
    , plugin_security_attributes_(readerInfo.plugin_security_attributes_)
#endif
    , m_guid(readerInfo.m_guid)
    , m_unicastLocatorList(readerInfo.m_unicastLocatorList)
    , m_multicastLocatorList(readerInfo.m_multicastLocatorList)
    , m_key(readerInfo.m_key)
    , m_RTPSParticipantKey(readerInfo.m_RTPSParticipantKey)
    , m_typeName(readerInfo.m_typeName)
    , m_topicName(readerInfo.m_topicName)
    , m_userDefinedId(readerInfo.m_userDefinedId)
    , m_isAlive(readerInfo.m_isAlive)
    , m_topicKind(readerInfo.m_topicKind)
    , m_topicDiscoveryKind(readerInfo.m_topicDiscoveryKind)
    , m_type_id(readerInfo.m_type_id)
    , m_type(readerInfo.m_type)
{
    m_qos.setQos(readerInfo.m_qos, true);
}

ReaderProxyData& ReaderProxyData::operator=(const ReaderProxyData& readerInfo)
{
    m_expectsInlineQos = readerInfo.m_expectsInlineQos;
#if HAVE_SECURITY
    security_attributes_ = readerInfo.security_attributes_;
    plugin_security_attributes_ = readerInfo.plugin_security_attributes_;
#endif
    m_guid = readerInfo.m_guid;
    m_unicastLocatorList = readerInfo.m_unicastLocatorList;
    m_multicastLocatorList = readerInfo.m_multicastLocatorList;
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
    m_type_id = readerInfo.m_type_id;
    m_type = readerInfo.m_type;

    return *this;
}

bool ReaderProxyData::writeToCDRMessage(CDRMessage_t* msg, bool write_encapsulation)
{
    if (write_encapsulation)
    {
        if (!ParameterList::writeEncapsulationToCDRMsg(msg)) return false;
    }

    for(LocatorListIterator lit = m_unicastLocatorList.begin();
            lit!=m_unicastLocatorList.end();++lit)
    {
        ParameterLocator_t p(PID_UNICAST_LOCATOR,PARAMETER_LOCATOR_LENGTH,*lit);
        if (!p.addToCDRMessage(msg)) return false;
    }
    for(LocatorListIterator lit = m_multicastLocatorList.begin();
            lit!=m_multicastLocatorList.end();++lit)
    {
        ParameterLocator_t p(PID_MULTICAST_LOCATOR,PARAMETER_LOCATOR_LENGTH,*lit);
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterBool_t p(PID_EXPECTS_INLINE_QOS,PARAMETER_BOOL_LENGTH,m_expectsInlineQos);
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterGuid_t p(PID_PARTICIPANT_GUID,PARAMETER_GUID_LENGTH,m_RTPSParticipantKey);
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterString_t p(PID_TOPIC_NAME, 0, m_topicName);
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterString_t p(PID_TYPE_NAME,0,m_typeName);
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterKey_t p(PID_KEY_HASH,16,m_key);
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterGuid_t p(PID_ENDPOINT_GUID,16,m_guid);
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterProtocolVersion_t p(PID_PROTOCOL_VERSION,4);
        if (!p.addToCDRMessage(msg)) return false;
    }
    {
        ParameterVendorId_t p(PID_VENDORID,4);
        if (!p.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_durability.sendAlways() || m_qos.m_durability.hasChanged)
    {
        if (!m_qos.m_durability.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_durabilityService.sendAlways() || m_qos.m_durabilityService.hasChanged)
    {
        if (!m_qos.m_durabilityService.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_deadline.sendAlways() || m_qos.m_deadline.hasChanged)
    {
        if (!m_qos.m_deadline.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_latencyBudget.sendAlways() || m_qos.m_latencyBudget.hasChanged)
    {
        if (!m_qos.m_latencyBudget.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_liveliness.sendAlways() || m_qos.m_liveliness.hasChanged)
    {
        if (!m_qos.m_liveliness.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_reliability.sendAlways() || m_qos.m_reliability.hasChanged)
    {
        if (!m_qos.m_reliability.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_lifespan.sendAlways() || m_qos.m_lifespan.hasChanged)
    {
        if (!m_qos.m_lifespan.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_userData.sendAlways() || m_qos.m_userData.hasChanged)
    {
        if (!m_qos.m_userData.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_timeBasedFilter.sendAlways() || m_qos.m_timeBasedFilter.hasChanged)
    {
        if (!m_qos.m_timeBasedFilter.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_ownership.sendAlways() || m_qos.m_ownership.hasChanged)
    {
        if (!m_qos.m_ownership.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_destinationOrder.sendAlways() || m_qos.m_destinationOrder.hasChanged)
    {
        if (!m_qos.m_destinationOrder.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_presentation.sendAlways() || m_qos.m_presentation.hasChanged)
    {
        if (!m_qos.m_presentation.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_partition.sendAlways() || m_qos.m_partition.hasChanged)
    {
        if (!m_qos.m_partition.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_topicData.sendAlways() || m_qos.m_topicData.hasChanged)
    {
        if (!m_qos.m_topicData.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_groupData.sendAlways() || m_qos.m_groupData.hasChanged)
    {
        if (!m_qos.m_groupData.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_timeBasedFilter.sendAlways() || m_qos.m_timeBasedFilter.hasChanged)
    {
        if (!m_qos.m_timeBasedFilter.addToCDRMessage(msg)) return false;
    }

    if (m_topicDiscoveryKind != NO_CHECK)
    {
        if (m_type_id.m_type_identifier._d() != 0)
        {
            if (!m_type_id.addToCDRMessage(msg)) return false;
        }

        if (m_type.m_type_object._d() != 0)
        {
            if (!m_type.addToCDRMessage(msg)) return false;
        }
    }
#if HAVE_SECURITY
    if ((this->security_attributes_ != 0UL) || (this->plugin_security_attributes_ != 0UL))
    {
        ParameterEndpointSecurityInfo_t p;
        p.security_attributes = security_attributes_;
        p.plugin_security_attributes = plugin_security_attributes_;
        if (!p.addToCDRMessage(msg)) return false;
    }
#endif

    return CDRMessage::addParameterSentinel(msg);
}

bool ReaderProxyData::readFromCDRMessage(CDRMessage_t* msg)
{
    auto param_process = [this](const Parameter_t* param)
    {
        switch (param->Pid)
        {
            case PID_DURABILITY:
            {
                const DurabilityQosPolicy* p = dynamic_cast<const DurabilityQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_durability = *p;
                break;
            }
            case PID_DURABILITY_SERVICE:
            {
                const DurabilityServiceQosPolicy* p = dynamic_cast<const DurabilityServiceQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_durabilityService = *p;
                break;
            }
            case PID_DEADLINE:
            {
                const DeadlineQosPolicy* p = dynamic_cast<const DeadlineQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_deadline = *p;
                break;
            }
            case PID_LATENCY_BUDGET:
            {
                const LatencyBudgetQosPolicy* p = dynamic_cast<const LatencyBudgetQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_latencyBudget = *p;
                break;
            }
            case PID_LIVELINESS:
            {
                const LivelinessQosPolicy* p = dynamic_cast<const LivelinessQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_liveliness = *p;
                break;
            }
            case PID_RELIABILITY:
            {
                const ReliabilityQosPolicy* p = dynamic_cast<const ReliabilityQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_reliability = *p;
                break;
            }
            case PID_LIFESPAN:
            {
                const LifespanQosPolicy* p = dynamic_cast<const LifespanQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_lifespan = *p;
                break;
            }
            case PID_USER_DATA:
            {
                const UserDataQosPolicy* p = dynamic_cast<const UserDataQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_userData = *p;
                break;
            }
            case PID_TIME_BASED_FILTER:
            {
                const TimeBasedFilterQosPolicy* p = dynamic_cast<const TimeBasedFilterQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_timeBasedFilter = *p;
                break;
            }
            case PID_OWNERSHIP:
            {
                const OwnershipQosPolicy* p = dynamic_cast<const OwnershipQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_ownership = *p;
                break;
            }
            case PID_DESTINATION_ORDER:
            {
                const DestinationOrderQosPolicy* p = dynamic_cast<const DestinationOrderQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_destinationOrder = *p;
                break;
            }

            case PID_PRESENTATION:
            {
                const PresentationQosPolicy* p = dynamic_cast<const PresentationQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_presentation = *p;
                break;
            }
            case PID_PARTITION:
            {
                const PartitionQosPolicy* p = dynamic_cast<const PartitionQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_partition = *p;
                break;
            }
            case PID_TOPIC_DATA:
            {
                const TopicDataQosPolicy* p = dynamic_cast<const TopicDataQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_topicData = *p;
                break;
            }
            case PID_GROUP_DATA:
            {
                const GroupDataQosPolicy* p = dynamic_cast<const GroupDataQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_groupData = *p;
                break;
            }
            case PID_TOPIC_NAME:
            {
                const ParameterString_t* p = dynamic_cast<const ParameterString_t*>(param);
                assert(p != nullptr);
                m_topicName = p->getName();
                break;
            }
            case PID_TYPE_NAME:
            {
                const ParameterString_t* p = dynamic_cast<const ParameterString_t*>(param);
                assert(p != nullptr);
                m_typeName = p->getName();
                break;
            }
            case PID_PARTICIPANT_GUID:
            {
                const ParameterGuid_t* p = dynamic_cast<const ParameterGuid_t*>(param);
                assert(p != nullptr);
                for (uint8_t i = 0; i < 16; ++i)
                {
                    if (i < 12)
                        m_RTPSParticipantKey.value[i] = p->guid.guidPrefix.value[i];
                    else
                        m_RTPSParticipantKey.value[i] = p->guid.entityId.value[i - 12];
                }
                break;
            }
            case PID_ENDPOINT_GUID:
            {
                const ParameterGuid_t* p = dynamic_cast<const ParameterGuid_t*>(param);
                assert(p != nullptr);
                m_guid = p->guid;
                for (uint8_t i = 0; i<16; ++i)
                {
                    if (i<12)
                        m_key.value[i] = p->guid.guidPrefix.value[i];
                    else
                        m_key.value[i] = p->guid.entityId.value[i - 12];
                }
                break;
            }
            case PID_UNICAST_LOCATOR:
            {
                const ParameterLocator_t* p = dynamic_cast<const ParameterLocator_t*>(param);
                assert(p != nullptr);
                m_unicastLocatorList.push_back(p->locator);
                break;
            }
            case PID_MULTICAST_LOCATOR:
            {
                const ParameterLocator_t* p = dynamic_cast<const ParameterLocator_t*>(param);
                assert(p != nullptr);
                m_multicastLocatorList.push_back(p->locator);
                break;
            }
            case PID_EXPECTS_INLINE_QOS:
            {
                const ParameterBool_t* p = dynamic_cast<const ParameterBool_t*>(param);
                assert(p != nullptr);
                m_expectsInlineQos = p->value;
                break;
            }
            case PID_KEY_HASH:
            {
                const ParameterKey_t* p = dynamic_cast<const ParameterKey_t*>(param);
                assert(p != nullptr);
                m_key = p->key;
                iHandle2GUID(m_guid, m_key);
                break;
            }
            case PID_DATA_REPRESENTATION:
            {
                const DataRepresentationQosPolicy* p = dynamic_cast<const DataRepresentationQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_dataRepresentation = *p;
                break;
            }
            case PID_TYPE_CONSISTENCY_ENFORCEMENT:
            {
                const TypeConsistencyEnforcementQosPolicy* p = 
                    dynamic_cast<const TypeConsistencyEnforcementQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_typeConsistency = *p;
                break;
            }
            case PID_TYPE_IDV1:
            {
                const TypeIdV1* p = dynamic_cast<const TypeIdV1*>(param);
                assert(p != nullptr);
                m_type_id = *p;
                m_topicDiscoveryKind = MINIMAL;
                if (m_type_id.m_type_identifier._d() == EK_COMPLETE)
                {
                    m_topicDiscoveryKind = COMPLETE;
                }
                break;
            }
            case PID_TYPE_OBJECTV1:
            {
                const TypeObjectV1* p = dynamic_cast<const TypeObjectV1*>(param);
                assert(p != nullptr);
                m_type = *p;
                m_topicDiscoveryKind = MINIMAL;
                if (m_type.m_type_object._d() == EK_COMPLETE)
                {
                    m_topicDiscoveryKind = COMPLETE;
                }
                break;
            }
#if HAVE_SECURITY
            case PID_ENDPOINT_SECURITY_INFO:
            {
                const ParameterEndpointSecurityInfo_t* p = 
                    dynamic_cast<const ParameterEndpointSecurityInfo_t*>(param);
                assert(p != nullptr);
                security_attributes_ = p->security_attributes;
                plugin_security_attributes_ = p->plugin_security_attributes;
            }
#endif
            default:
            {
                //logInfo(RTPS_PROXY_DATA,"Parameter with ID: "  <<(uint16_t)(param)->Pid << " NOT CONSIDERED");
                break;
            }
        }
        return true;
    };

    uint32_t qos_size;
    if (ParameterList::readParameterListfromCDRMsg(*msg, param_process, true, qos_size))
    {
        if (m_guid.entityId.value[3] == 0x04)
            m_topicKind = NO_KEY;
        else if (m_guid.entityId.value[3] == 0x07)
            m_topicKind = WITH_KEY;

        return true;
    }

    return false;
}

void ReaderProxyData::clear()
{
    m_expectsInlineQos = false;
    m_guid = c_Guid_Unknown;
    m_unicastLocatorList.clear();
    m_multicastLocatorList.clear();
    m_key = InstanceHandle_t();
    m_RTPSParticipantKey = InstanceHandle_t();
    m_typeName = "";
    m_topicName = "";
    m_userDefinedId = 0;
    m_qos = ReaderQos();
    m_isAlive = true;
    m_topicKind = NO_KEY;
}

void ReaderProxyData::update(ReaderProxyData* rdata)
{
    m_unicastLocatorList = rdata->m_unicastLocatorList;
    m_multicastLocatorList = rdata->m_multicastLocatorList;
    m_qos.setQos(rdata->m_qos,false);
    m_isAlive = rdata->m_isAlive;
    m_expectsInlineQos = rdata->m_expectsInlineQos;
}

void ReaderProxyData::copy(ReaderProxyData* rdata)
{
    m_guid = rdata->m_guid;
    m_unicastLocatorList = rdata->m_unicastLocatorList;
    m_multicastLocatorList = rdata->m_multicastLocatorList;
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

RemoteReaderAttributes::RemoteReaderAttributes(const ReaderProxyData& data)
{
    guid = data.guid();
    expectsInlineQos = data.m_expectsInlineQos;
    endpoint.durabilityKind = data.m_qos.m_durability.durabilityKind();
    endpoint.endpointKind = READER;
    endpoint.topicKind = data.topicKind();
    endpoint.reliabilityKind = data.m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    endpoint.unicastLocatorList = data.unicastLocatorList();
    endpoint.multicastLocatorList = data.multicastLocatorList();
}

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */


