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

namespace eprosima {
namespace fastrtps{
namespace rtps {


WriterProxyData::WriterProxyData()
#if HAVE_SECURITY
    : security_attributes_(0)
    , plugin_security_attributes_(0)
    , m_userDefinedId(0)
#else
    : m_userDefinedId(0)
#endif
    , m_typeMaxSerialized(0)
    , m_topicKind(NO_KEY)
    , m_topicDiscoveryKind(NO_CHECK)
    {
        // TODO Auto-generated constructor stub
    }

WriterProxyData::WriterProxyData(const WriterProxyData& writerInfo)
#if HAVE_SECURITY
    : security_attributes_(writerInfo.security_attributes_)
    , plugin_security_attributes_(writerInfo.plugin_security_attributes_)
    , m_guid(writerInfo.m_guid)
#else
    : m_guid(writerInfo.m_guid)
#endif
    , m_unicastLocatorList(writerInfo.m_unicastLocatorList)
    , m_multicastLocatorList(writerInfo.m_multicastLocatorList)
    , m_key(writerInfo.m_key)
    , m_RTPSParticipantKey(writerInfo.m_RTPSParticipantKey)
    , m_typeName(writerInfo.m_typeName)
    , m_topicName(writerInfo.m_topicName)
    , m_userDefinedId(writerInfo.m_userDefinedId)
    , m_typeMaxSerialized(writerInfo.m_typeMaxSerialized)
    , m_topicKind(writerInfo.m_topicKind)
    , persistence_guid_(writerInfo.persistence_guid_)
    , m_topicDiscoveryKind(writerInfo.m_topicDiscoveryKind)
    , m_type_id(writerInfo.m_type_id)
    , m_type(writerInfo.m_type)
{
    m_qos.setQos(writerInfo.m_qos, true);
}

WriterProxyData::~WriterProxyData() {
    // TODO Auto-generated destructor stub
    logInfo(RTPS_PROXY_DATA,this->m_guid);
}

WriterProxyData& WriterProxyData::operator=(const WriterProxyData& writerInfo)
{
#if HAVE_SECURITY
    security_attributes_ = writerInfo.security_attributes_;
    plugin_security_attributes_ = writerInfo.plugin_security_attributes_;
#endif
    m_guid = writerInfo.m_guid;
    m_unicastLocatorList = writerInfo.m_unicastLocatorList;
    m_multicastLocatorList = writerInfo.m_multicastLocatorList;
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
    m_type_id = writerInfo.m_type_id;
    m_type = writerInfo.m_type;

    return *this;
}

bool WriterProxyData::writeToCDRMessage(CDRMessage_t* msg, bool write_encapsulation)
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
        ParameterPort_t p(PID_TYPE_MAX_SIZE_SERIALIZED,4,m_typeMaxSerialized);
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
    if(persistence_guid_ != c_Guid_Unknown)
    {
        ParameterGuid_t p(PID_PERSISTENCE_GUID, 16, persistence_guid_);
        if (!p.addToCDRMessage(msg)) return false;
    }
    if( m_qos.m_durability.sendAlways() || m_qos.m_durability.hasChanged)
    {
        if (!m_qos.m_durability.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_durabilityService.sendAlways() || m_qos.m_durabilityService.hasChanged)
    {
        if (!m_qos.m_durabilityService.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_deadline.sendAlways() ||  m_qos.m_deadline.hasChanged)
    {
        if (!m_qos.m_deadline.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_latencyBudget.sendAlways() ||  m_qos.m_latencyBudget.hasChanged)
    {
        if (!m_qos.m_latencyBudget.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_durability.sendAlways() ||  m_qos.m_liveliness.hasChanged)
    {
        if (!m_qos.m_liveliness.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_reliability.sendAlways() ||  m_qos.m_reliability.hasChanged)
    {
        if (!m_qos.m_reliability.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_lifespan.sendAlways() ||  m_qos.m_lifespan.hasChanged)
    {
        if (!m_qos.m_lifespan.addToCDRMessage(msg)) return false;
    }
    if( m_qos.m_userData.sendAlways() || m_qos.m_userData.hasChanged)
    {
        if (!m_qos.m_userData.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_timeBasedFilter.sendAlways() ||  m_qos.m_timeBasedFilter.hasChanged)
    {
        if (!m_qos.m_timeBasedFilter.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_ownership.sendAlways() ||  m_qos.m_ownership.hasChanged)
    {
        if (!m_qos.m_ownership.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_durability.sendAlways() ||  m_qos.m_ownershipStrength.hasChanged)
    {
        if (!m_qos.m_ownershipStrength.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_destinationOrder.sendAlways() ||  m_qos.m_destinationOrder.hasChanged)
    {
        if (!m_qos.m_destinationOrder.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_presentation.sendAlways() ||  m_qos.m_presentation.hasChanged)
    {
        if (!m_qos.m_presentation.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_partition.sendAlways() ||  m_qos.m_partition.hasChanged)
    {
        if (!m_qos.m_partition.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_topicData.sendAlways() || m_qos.m_topicData.hasChanged)
    {
        if (!m_qos.m_topicData.addToCDRMessage(msg)) return false;
    }
    if(m_qos.m_disablePositiveACKs.sendAlways() || m_qos.m_topicData.hasChanged)
    {
        if (!m_qos.m_disablePositiveACKs.addToCDRMessage(msg))
        {
            return false;
        }
    }
    if(m_qos.m_groupData.sendAlways() ||  m_qos.m_groupData.hasChanged)
    {
        GroupDataQosPolicy*p = new GroupDataQosPolicy();
        *p = m_qos.m_groupData;
        if (!m_qos.m_groupData.addToCDRMessage(msg)) return false;
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

bool WriterProxyData::readFromCDRMessage(CDRMessage_t* msg)
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
            case PID_OWNERSHIP_STRENGTH:
            {
                const OwnershipStrengthQosPolicy* p = dynamic_cast<const OwnershipStrengthQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_ownershipStrength = *p;
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
            case PID_PERSISTENCE_GUID:
            {
                const ParameterGuid_t* p = dynamic_cast<const ParameterGuid_t*>(param);
                assert(p != nullptr);
                persistence_guid_ = p->guid;
            }
            break;
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
            case PID_KEY_HASH:
            {
                const ParameterKey_t* p = dynamic_cast<const ParameterKey_t*>(param);
                assert(p != nullptr);
                m_key = p->key;
                iHandle2GUID(m_guid, m_key);
                break;
            }
            case PID_TYPE_IDV1:
            {
                const TypeIdV1* p = dynamic_cast<const TypeIdV1*>(param);
                assert(p != nullptr);
                m_type_id = *p;
                m_topicDiscoveryKind = MINIMAL;
                if (m_type_id.m_type_identifier._d() == types::EK_COMPLETE)
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
                if (m_type.m_type_object._d() == types::EK_COMPLETE)
                {
                    m_topicDiscoveryKind = COMPLETE;
                }
                break;
            }
            case PID_DISABLE_POSITIVE_ACKS:
            {
                const DisablePositiveACKsQosPolicy* p = dynamic_cast<const DisablePositiveACKsQosPolicy*>(param);
                assert(p != nullptr);
                m_qos.m_disablePositiveACKs = *p;
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
                //logInfo(RTPS_PROXY_DATA,"Parameter with ID: " << (uint16_t)(param)->Pid <<" NOT CONSIDERED");
                break;
            }
        }
        return true;
    };

    uint32_t qos_size;
    if (ParameterList::readParameterListfromCDRMsg(*msg, param_process, true, qos_size))
    {
        if (m_guid.entityId.value[3] == 0x03)
            m_topicKind = NO_KEY;
        else if (m_guid.entityId.value[3] == 0x02)
            m_topicKind = WITH_KEY;

        return true;
    }

    return false;
}

void WriterProxyData::clear()
{
    m_guid = c_Guid_Unknown;
    m_unicastLocatorList.clear();
    m_multicastLocatorList.clear();
    m_key = InstanceHandle_t();
    m_RTPSParticipantKey = InstanceHandle_t();
    m_typeName = "";
    m_topicName = "";
    m_userDefinedId = 0;
    m_qos = WriterQos();
    m_typeMaxSerialized = 0;
    m_topicKind = NO_KEY;
    persistence_guid_ = c_Guid_Unknown;
}

void WriterProxyData::copy(WriterProxyData* wdata)
{
    m_guid = wdata->m_guid;
    m_unicastLocatorList = wdata->m_unicastLocatorList;
    m_multicastLocatorList = wdata->m_multicastLocatorList;
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


void WriterProxyData::update(WriterProxyData* wdata)
{
    m_unicastLocatorList = wdata->m_unicastLocatorList;
    m_multicastLocatorList = wdata->m_multicastLocatorList;
    m_qos.setQos(wdata->m_qos,false);
}

RemoteWriterAttributes WriterProxyData::toRemoteWriterAttributes() const
{
    RemoteWriterAttributes remoteAtt;

    remoteAtt.guid = m_guid;
    remoteAtt.liveliness_kind = m_qos.m_liveliness.kind;
    remoteAtt.liveliness_lease_duration = m_qos.m_liveliness.lease_duration;
    remoteAtt.ownershipStrength = (uint16_t)m_qos.m_ownershipStrength.value;
    remoteAtt.endpoint.durabilityKind = m_qos.m_durability.durabilityKind();
    remoteAtt.endpoint.endpointKind = WRITER;
    remoteAtt.endpoint.topicKind = m_topicKind;
    remoteAtt.endpoint.reliabilityKind = m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    remoteAtt.endpoint.unicastLocatorList = this->m_unicastLocatorList;
    remoteAtt.endpoint.multicastLocatorList = this->m_multicastLocatorList;
    remoteAtt.endpoint.persistence_guid = (persistence_guid_ == c_Guid_Unknown) ? m_guid : persistence_guid_;

    return remoteAtt;
}

}
} /* namespace rtps */
} /* namespace eprosima */
