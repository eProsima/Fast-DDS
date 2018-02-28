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


WriterProxyData::WriterProxyData():
    m_userDefinedId(0),
    m_typeMaxSerialized(0),
    m_isAlive(true),
    m_topicKind(NO_KEY)
    {
        // TODO Auto-generated constructor stub
    }

WriterProxyData::WriterProxyData(const WriterProxyData& writerInfo) :
    m_guid(writerInfo.m_guid),
    m_unicastLocatorList(writerInfo.m_unicastLocatorList),
    m_multicastLocatorList(writerInfo.m_multicastLocatorList),
    m_key(writerInfo.m_key),
    m_RTPSParticipantKey(writerInfo.m_RTPSParticipantKey),
    m_typeName(writerInfo.m_typeName),
    m_topicName(writerInfo.m_topicName),
    m_userDefinedId(writerInfo.m_userDefinedId),
    m_typeMaxSerialized(writerInfo.m_typeMaxSerialized),
    m_isAlive(writerInfo.m_isAlive),
    m_topicKind(writerInfo.m_topicKind),
    persistence_guid_(writerInfo.persistence_guid_)
{
    m_qos.setQos(writerInfo.m_qos, true);
}

WriterProxyData::~WriterProxyData() {
    // TODO Auto-generated destructor stub
    logInfo(RTPS_PROXY_DATA,this->m_guid);
}

WriterProxyData& WriterProxyData::operator=(const WriterProxyData& writerInfo)
{
    m_guid = writerInfo.m_guid;
    m_unicastLocatorList = writerInfo.m_unicastLocatorList;
    m_multicastLocatorList = writerInfo.m_multicastLocatorList;
    m_key = writerInfo.m_key;
    m_RTPSParticipantKey = writerInfo.m_RTPSParticipantKey;
    m_typeName = writerInfo.m_typeName;
    m_topicName = writerInfo.m_topicName;
    m_userDefinedId = writerInfo.m_userDefinedId;
    m_typeMaxSerialized = writerInfo.m_typeMaxSerialized;
    m_isAlive = writerInfo.m_isAlive;
    m_topicKind = writerInfo.m_topicKind;
    persistence_guid_ = writerInfo.persistence_guid_;
    m_qos.setQos(writerInfo.m_qos, true);

    return *this;
}

ParameterList_t WriterProxyData::toParameterList()
{
    ParameterList_t parameter_list;

    for(LocatorListIterator lit = m_unicastLocatorList.begin();
            lit!=m_unicastLocatorList.end();++lit)
    {
        ParameterLocator_t* p = new ParameterLocator_t(PID_UNICAST_LOCATOR,PARAMETER_LOCATOR_LENGTH,*lit);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    for(LocatorListIterator lit = m_multicastLocatorList.begin();
            lit!=m_multicastLocatorList.end();++lit)
    {
        ParameterLocator_t* p = new ParameterLocator_t(PID_MULTICAST_LOCATOR,PARAMETER_LOCATOR_LENGTH,*lit);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterGuid_t* p = new ParameterGuid_t(PID_PARTICIPANT_GUID,PARAMETER_GUID_LENGTH,m_RTPSParticipantKey);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterString_t * p = new ParameterString_t(PID_TOPIC_NAME,0,m_topicName);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterString_t * p = new ParameterString_t(PID_TYPE_NAME,0,m_typeName);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterKey_t * p = new ParameterKey_t(PID_KEY_HASH,16,m_key);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterGuid_t * p = new ParameterGuid_t(PID_ENDPOINT_GUID,16,m_guid);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterPort_t* p = new ParameterPort_t(PID_TYPE_MAX_SIZE_SERIALIZED,4,m_typeMaxSerialized);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterProtocolVersion_t* p = new ParameterProtocolVersion_t(PID_PROTOCOL_VERSION,4);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    {
        ParameterVendorId_t*p = new ParameterVendorId_t(PID_VENDORID,4);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(persistence_guid_ != c_Guid_Unknown)
    {
        ParameterGuid_t * p = new ParameterGuid_t(PID_PERSISTENCE_GUID, 16, persistence_guid_);
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if( m_qos.m_durability.sendAlways() || m_qos.m_durability.hasChanged)
    {
        DurabilityQosPolicy*p = new DurabilityQosPolicy();
        *p = m_qos.m_durability;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_durabilityService.sendAlways() || m_qos.m_durabilityService.hasChanged)
    {
        DurabilityServiceQosPolicy*p = new DurabilityServiceQosPolicy();
        *p = m_qos.m_durabilityService;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_deadline.sendAlways() ||  m_qos.m_deadline.hasChanged)
    {
        DeadlineQosPolicy*p = new DeadlineQosPolicy();
        *p = m_qos.m_deadline;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_latencyBudget.sendAlways() ||  m_qos.m_latencyBudget.hasChanged)
    {
        LatencyBudgetQosPolicy*p = new LatencyBudgetQosPolicy();
        *p = m_qos.m_latencyBudget;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_durability.sendAlways() ||  m_qos.m_liveliness.hasChanged)
    {
        LivelinessQosPolicy*p = new LivelinessQosPolicy();
        *p = m_qos.m_liveliness;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_reliability.sendAlways() ||  m_qos.m_reliability.hasChanged)
    {
        ReliabilityQosPolicy*p = new ReliabilityQosPolicy();
        *p = m_qos.m_reliability;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_lifespan.sendAlways() ||  m_qos.m_lifespan.hasChanged)
    {
        LifespanQosPolicy*p = new LifespanQosPolicy();
        *p = m_qos.m_lifespan;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if( m_qos.m_userData.sendAlways() || m_qos.m_userData.hasChanged)
    {
        UserDataQosPolicy*p = new UserDataQosPolicy();
        *p = m_qos.m_userData;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_timeBasedFilter.sendAlways() ||  m_qos.m_timeBasedFilter.hasChanged)
    {
        TimeBasedFilterQosPolicy*p = new TimeBasedFilterQosPolicy();
        *p = m_qos.m_timeBasedFilter;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_ownership.sendAlways() ||  m_qos.m_ownership.hasChanged)
    {
        OwnershipQosPolicy*p = new OwnershipQosPolicy();
        *p = m_qos.m_ownership;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_durability.sendAlways() ||  m_qos.m_ownershipStrength.hasChanged)
    {
        OwnershipStrengthQosPolicy*p = new OwnershipStrengthQosPolicy();
        *p = m_qos.m_ownershipStrength;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_destinationOrder.sendAlways() ||  m_qos.m_destinationOrder.hasChanged)
    {
        DestinationOrderQosPolicy*p = new DestinationOrderQosPolicy();
        *p = m_qos.m_destinationOrder;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_presentation.sendAlways() ||  m_qos.m_presentation.hasChanged)
    {
        PresentationQosPolicy*p = new PresentationQosPolicy();
        *p = m_qos.m_presentation;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_partition.sendAlways() ||  m_qos.m_partition.hasChanged)
    {
        PartitionQosPolicy*p = new PartitionQosPolicy();
        *p = m_qos.m_partition;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_topicData.sendAlways() || m_qos.m_topicData.hasChanged)
    {
        TopicDataQosPolicy*p = new TopicDataQosPolicy();
        *p = m_qos.m_topicData;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    if(m_qos.m_groupData.sendAlways() ||  m_qos.m_groupData.hasChanged)
    {
        GroupDataQosPolicy*p = new GroupDataQosPolicy();
        *p = m_qos.m_groupData;
        parameter_list.m_parameters.push_back((Parameter_t*)p);
    }
    logInfo(RTPS_PROXY_DATA," with " << parameter_list.m_parameters.size()<< " parameters");
    return parameter_list;
}

bool WriterProxyData::readFromCDRMessage(CDRMessage_t* msg)
{
    ParameterList_t parameter_list;
    if(ParameterList::readParameterListfromCDRMsg(msg, &parameter_list, NULL, true) > 0)
    {
        for(std::vector<Parameter_t*>::iterator it = parameter_list.m_parameters.begin();
                it!=parameter_list.m_parameters.end();++it)
        {
            switch((*it)->Pid)
            {
                case PID_DURABILITY:
                    {
                        DurabilityQosPolicy * p = (DurabilityQosPolicy*)(*it);
                        m_qos.m_durability = *p;
                        break;
                    }
                case PID_DURABILITY_SERVICE:
                    {
                        DurabilityServiceQosPolicy * p = (DurabilityServiceQosPolicy*)(*it);
                        m_qos.m_durabilityService = *p;
                        break;
                    }
                case PID_DEADLINE:
                    {
                        DeadlineQosPolicy * p = (DeadlineQosPolicy*)(*it);
                        m_qos.m_deadline = *p;
                        break;
                    }
                case PID_LATENCY_BUDGET:
                    {
                        LatencyBudgetQosPolicy * p = (LatencyBudgetQosPolicy*)(*it);
                        m_qos.m_latencyBudget = *p;
                        break;
                    }
                case PID_LIVELINESS:
                    {
                        LivelinessQosPolicy * p = (LivelinessQosPolicy*)(*it);
                        m_qos.m_liveliness = *p;
                        break;
                    }
                case PID_RELIABILITY:
                    {
                        ReliabilityQosPolicy * p = (ReliabilityQosPolicy*)(*it);
                        m_qos.m_reliability = *p;
                        break;
                    }
                case PID_LIFESPAN:
                    {

                        LifespanQosPolicy * p = (LifespanQosPolicy*)(*it);
                        m_qos.m_lifespan = *p;
                        break;
                    }
                case PID_USER_DATA:
                    {
                        UserDataQosPolicy * p = (UserDataQosPolicy*)(*it);
                        m_qos.m_userData = *p;
                        break;
                    }
                case PID_TIME_BASED_FILTER:
                    {
                        TimeBasedFilterQosPolicy * p = (TimeBasedFilterQosPolicy*)(*it);
                        m_qos.m_timeBasedFilter = *p;
                        break;
                    }
                case PID_OWNERSHIP:
                    {
                        OwnershipQosPolicy * p = (OwnershipQosPolicy*)(*it);
                        m_qos.m_ownership = *p;
                        break;
                    }
                case PID_OWNERSHIP_STRENGTH:
                    {
                        OwnershipStrengthQosPolicy * p = (OwnershipStrengthQosPolicy*)(*it);
                        m_qos.m_ownershipStrength = *p;
                        break;
                    }
                case PID_DESTINATION_ORDER:
                    {
                        DestinationOrderQosPolicy * p = (DestinationOrderQosPolicy*)(*it);
                        m_qos.m_destinationOrder = *p;
                        break;
                    }

                case PID_PRESENTATION:
                    {
                        PresentationQosPolicy * p = (PresentationQosPolicy*)(*it);
                        m_qos.m_presentation = *p;
                        break;
                    }
                case PID_PARTITION:
                    {
                        PartitionQosPolicy * p = (PartitionQosPolicy*)(*it);
                        m_qos.m_partition = *p;
                        break;
                    }
                case PID_TOPIC_DATA:
                    {
                        TopicDataQosPolicy * p = (TopicDataQosPolicy*)(*it);
                        m_qos.m_topicData = *p;
                        break;
                    }
                case PID_GROUP_DATA:
                    {

                        GroupDataQosPolicy * p = (GroupDataQosPolicy*)(*it);
                        m_qos.m_groupData = *p;
                        break;
                    }
                case PID_TOPIC_NAME:
                    {
                        ParameterString_t*p = (ParameterString_t*)(*it);
                        m_topicName = std::string(p->getName());
                        break;
                    }
                case PID_TYPE_NAME:
                    {
                        ParameterString_t*p = (ParameterString_t*)(*it);
                        m_typeName = std::string(p->getName());
                        break;
                    }
                case PID_PARTICIPANT_GUID:
                    {
                        ParameterGuid_t * p = (ParameterGuid_t*)(*it);
                        for(uint8_t i = 0; i < 16; ++i)
                        {
                            if(i < 12)
                                m_RTPSParticipantKey.value[i] = p->guid.guidPrefix.value[i];
                            else
                                m_RTPSParticipantKey.value[i] = p->guid.entityId.value[i - 12];
                        }
                        break;
                    }
                case PID_ENDPOINT_GUID:
                    {
                        ParameterGuid_t * p = (ParameterGuid_t*)(*it);
                        m_guid = p->guid;
                        for(uint8_t i=0;i<16;++i)
                        {
                            if(i<12)
                                m_key.value[i] = p->guid.guidPrefix.value[i];
                            else
                                m_key.value[i] = p->guid.entityId.value[i - 12];
                        }
                        break;
                    }
                case PID_PERSISTENCE_GUID:
                    {
                        ParameterGuid_t * p = (ParameterGuid_t*)(*it);
                        persistence_guid_ = p->guid;
                    }
                    break;
                case PID_UNICAST_LOCATOR:
                    {
                        ParameterLocator_t* p = (ParameterLocator_t*)(*it);
                        m_unicastLocatorList.push_back(p->locator);
                        break;
                    }
                case PID_MULTICAST_LOCATOR:
                    {
                        ParameterLocator_t* p = (ParameterLocator_t*)(*it);
                        m_multicastLocatorList.push_back(p->locator);
                        break;
                    }
                case PID_KEY_HASH:
                    {
                        ParameterKey_t*p=(ParameterKey_t*)(*it);
                        m_key = p->key;
                        iHandle2GUID(m_guid,m_key);
                        break;
                    }
                default:
                    {
                        //logInfo(RTPS_PROXY_DATA,"Parameter with ID: " << (uint16_t)(*it)->Pid <<" NOT CONSIDERED");
                        break;
                    }
            }
            if(m_guid.entityId.value[3] == 0x03)
                m_topicKind = NO_KEY;
            else if(m_guid.entityId.value[3] == 0x02)
                m_topicKind = WITH_KEY;
        }
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
    m_isAlive = true;
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
    m_isAlive = wdata->m_isAlive;
    m_topicKind = wdata->m_topicKind;
    persistence_guid_ = wdata->persistence_guid_;
}


void WriterProxyData::update(WriterProxyData* wdata)
{
    m_unicastLocatorList = wdata->m_unicastLocatorList;
    m_multicastLocatorList = wdata->m_multicastLocatorList;
    m_qos.setQos(wdata->m_qos,false);
    m_isAlive = wdata->m_isAlive;

}

RemoteWriterAttributes WriterProxyData::toRemoteWriterAttributes() const
{
    RemoteWriterAttributes remoteAtt;

    remoteAtt.guid = m_guid;
    remoteAtt.livelinessLeaseDuration = m_qos.m_liveliness.lease_duration;
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


