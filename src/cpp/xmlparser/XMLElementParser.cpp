// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
//
#include <cstring>
#include <sstream>
#include <tinyxml2.h>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/xmlparser/XMLParserCommon.h>
#include <fastrtps/xmlparser/XMLParser.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastrtps::xmlparser;

XMLP_ret XMLParser::getXMLBuiltinAttributes(tinyxml2::XMLElement *elem, BuiltinAttributes &builtin, uint8_t ident)
{
    /*<xs:complexType name="builtinAttributesType">
      <xs:all minOccurs="0">
        <xs:element name="use_SIMPLE_RTPS_PDP" type="boolType"/>
        <xs:element name="use_WriterLivelinessProtocol" type="boolType"/>
        <xs:element name="EDP" type="EDPType"/>
        <xs:element name="domainId" type="uint32Type"/>
        <xs:element name="leaseDuration" type="durationType"/>
        <xs:element name="leaseAnnouncement" type="durationType"/>
        <xs:element name="simpleEDP" type="simpleEDPType"/>
        <xs:element name="metatrafficUnicastLocatorList" type="locatorListType"/>
        <xs:element name="metatrafficMulticastLocatorList" type="locatorListType"/>
        <xs:element name="initialPeersList" type="locatorListType"/>
        <xs:element name="staticEndpointXMLFilename" type="stringType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr, *p_aux1 = nullptr;
    // use_SIMPLE_RTPS_PDP - boolType
    if (nullptr != (p_aux0 = elem->FirstChildElement(SIMPLE_RTPS_PDP)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol, ident))
            return XMLP_ret::XML_ERROR;
    }
    // use_WriterLivelinessProtocol - boolType
    if (nullptr != (p_aux0 = elem->FirstChildElement(WRITER_LVESS_PROTOCOL)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &builtin.use_WriterLivelinessProtocol, ident))
            return XMLP_ret::XML_ERROR;
    }
    // EDP
    if (nullptr != (p_aux0 = elem->FirstChildElement(_EDP)))
    {
        /*<xs:simpleType name="EDPType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="SIMPLE"/>
            <xs:enumeration value="STATIC"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << _EDP << "' without content");
            return XMLP_ret::XML_ERROR;
        }
        if (strcmp(text, SIMPLE) == 0)
        {
            builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
            builtin.use_STATIC_EndpointDiscoveryProtocol = false;
        }
        else if (strcmp(text, STATIC) == 0)
        {
            builtin.use_SIMPLE_EndpointDiscoveryProtocol = false;
            builtin.use_STATIC_EndpointDiscoveryProtocol = true;
        }
        else
        {
            logError(XMLPARSER, "Node '" << _EDP << "' with bad content");
            return XMLP_ret::XML_ERROR;
        }
    }
    // domainId - uint32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(DOMAIN_ID)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &builtin.domainId, ident))
            return XMLP_ret::XML_ERROR;
    }
    // leaseDuration - durationType
    if (nullptr != (p_aux0 = elem->FirstChildElement(LEASEDURATION)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, builtin.leaseDuration, ident))
            return XMLP_ret::XML_ERROR;
    }
    // leaseAnnouncement - durationType
    if (nullptr != (p_aux0 = elem->FirstChildElement(LEASE_ANNOUNCE)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, builtin.leaseDuration_announcementperiod, ident))
            return XMLP_ret::XML_ERROR;
    }
    // simpleEDP
    if (nullptr != (p_aux0 = elem->FirstChildElement(SIMPLE_EDP)))
    {
        // PUBWRITER_SUBREADER - boolType
        if (nullptr != (p_aux1 = p_aux0->FirstChildElement(PUBWRITER_SUBREADER)))
        {
            if (XMLP_ret::XML_OK != getXMLBool(p_aux1, &builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader, ident + 1))
                return XMLP_ret::XML_ERROR;
        }
        // PUBREADER_SUBWRITER - boolType
        if (nullptr != (p_aux1 = p_aux0->FirstChildElement(PUBREADER_SUBWRITER)))
        {
            if (XMLP_ret::XML_OK != getXMLBool(p_aux1, &builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter, ident + 1))
                return XMLP_ret::XML_ERROR;
        }
    }
    // metatrafficUnicastLocatorList
    if (nullptr != (p_aux0 = elem->FirstChildElement(META_UNI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, builtin.metatrafficUnicastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // metatrafficMulticastLocatorList
    if (nullptr != (p_aux0 = elem->FirstChildElement(META_MULTI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, builtin.metatrafficMulticastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // initialPeersList
    if (nullptr != (p_aux0 = elem->FirstChildElement(INIT_PEERS_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, builtin.initialPeersList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // staticEndpointXMLFilename - stringType
    if (nullptr != (p_aux0 = elem->FirstChildElement(STATIC_ENDPOINT_XML)))
    {
        std::string s = "";
        if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident)) return XMLP_ret::XML_ERROR;
        builtin.setStaticEndpointXMLFilename(s.c_str());
    }

    return XMLP_ret::XML_OK;

}

XMLP_ret XMLParser::getXMLPortParameters(tinyxml2::XMLElement *elem, PortParameters &port, uint8_t ident)
{
    /*<xs:complexType name="portType">
      <xs:all minOccurs="0">
        <xs:element name="portBase" type="uint16Type"/>
        <xs:element name="domainIDGain" type="uint16Type"/>
        <xs:element name="participantIDGain" type="uint16Type"/>
        <xs:element name="offsetd0" type="uint16Type"/>
        <xs:element name="offsetd1" type="uint16Type"/>
        <xs:element name="offsetd2" type="uint16Type"/>
        <xs:element name="offsetd3" type="uint16Type"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;
    // portBase - uint16Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(PORT_BASE)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.portBase, ident)) return XMLP_ret::XML_ERROR;
    }
    // domainIDGain - uint16Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(DOMAIN_ID_GAIN)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.domainIDGain, ident)) return XMLP_ret::XML_ERROR;
    }
    // participantIDGain - uint16Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(PARTICIPANT_ID_GAIN)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.participantIDGain, ident)) return XMLP_ret::XML_ERROR;
    }
    // offsetd0 - uint16Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(OFFSETD0)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.offsetd0, ident)) return XMLP_ret::XML_ERROR;
    }
    // offsetd1 - uint16Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(OFFSETD1)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.offsetd1, ident)) return XMLP_ret::XML_ERROR;
    }
    // offsetd2 - uint16Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(OFFSETD2)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.offsetd2, ident)) return XMLP_ret::XML_ERROR;
    }
    // offsetd3 - uint16Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(OFFSETD3)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.offsetd3, ident)) return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLThroughputController(tinyxml2::XMLElement *elem,
                                                      ThroughputControllerDescriptor &throughputController,
                                                      uint8_t ident)
{
    /*<xs:complexType name="throughputControllerType">
      <xs:all minOccurs="0">
        <xs:element name="bytesPerPeriod" type="uint32Type"/>
        <xs:element name="periodMillisecs" type="uint32Type"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    // bytesPerPeriod - uint32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(BYTES_PER_SECOND)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &throughputController.bytesPerPeriod, ident)) return XMLP_ret::XML_ERROR;
    }
    // periodMillisecs - uint32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(PERIOD_MILLISECS)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &throughputController.periodMillisecs, ident)) return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLTopicAttributes(tinyxml2::XMLElement *elem, TopicAttributes &topic, uint8_t ident)
{
    /*<xs:complexType name="topicAttributesType">
      <xs:all minOccurs="0">
        <xs:element name="kind" type="topicKindType"/>
        <xs:element name="name" type="stringType"/>
        <xs:element name="dataType" type="stringType"/>
        <xs:element name="historyQos" type="historyQosPolicyType"/>
        <xs:element name="resourceLimitsQos" type="resourceLimitsQosPolicyType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    // kind
    if (nullptr != (p_aux0 = elem->FirstChildElement(KIND)))
    {
        /*<xs:simpleType name="topicKindType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="NO_KEY"/>
            <xs:enumeration value="WITH_KEY"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << KIND << "' without content");
            return XMLP_ret::XML_ERROR;
        }
             if (strcmp(text,   _NO_KEY) == 0) topic.topicKind = TopicKind_t::NO_KEY;
        else if (strcmp(text, _WITH_KEY) == 0) topic.topicKind = TopicKind_t::WITH_KEY;
        else
        {
            logError(XMLPARSER, "Node '" << KIND << "' with bad content");
            return XMLP_ret::XML_ERROR;
        }
    }
    // name - stringType
    if (nullptr != (p_aux0 = elem->FirstChildElement(NAME)))
    {
        if (XMLP_ret::XML_OK != getXMLString(p_aux0, &topic.topicName, ident)) return XMLP_ret::XML_ERROR;
    }
    // dataType - stringType
    if (nullptr != (p_aux0 = elem->FirstChildElement(DATA_TYPE)))
    {
        if (XMLP_ret::XML_OK != getXMLString(p_aux0, &topic.topicDataType, ident)) return XMLP_ret::XML_ERROR;
    }
    // historyQos
    if (nullptr != (p_aux0 = elem->FirstChildElement(HISTORY_QOS)))
    {
        if (XMLP_ret::XML_OK != getXMLHistoryQosPolicy(p_aux0, topic.historyQos, ident)) return XMLP_ret::XML_ERROR;
    }
    // resourceLimitsQos
    if (nullptr != (p_aux0 = elem->FirstChildElement(RES_LIMITS_QOS)))
    {
        if (XMLP_ret::XML_OK != getXMLResourceLimitsQos(p_aux0, topic.resourceLimitsQos, ident)) return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLResourceLimitsQos(tinyxml2::XMLElement *elem,
                                                   ResourceLimitsQosPolicy &resourceLimitsQos,
                                                   uint8_t ident)
{
    /*<xs:complexType name="resourceLimitsQosPolicyType">
      <xs:all minOccurs="0">
        <xs:element name="max_samples" type="int32Type"/>
        <xs:element name="max_instances" type="int32Type"/>
        <xs:element name="max_samples_per_instance" type="int32Type"/>
        <xs:element name="allocated_samples" type="int32Type"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    // max_samples - int32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(MAX_SAMPLES)))
    {
        if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &resourceLimitsQos.max_samples, ident))
            return XMLP_ret::XML_ERROR;
    }
    // max_instances - int32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(MAX_INSTANCES)))
    {
        if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &resourceLimitsQos.max_instances, ident))
            return XMLP_ret::XML_ERROR;
    }
    // max_samples_per_instance - int32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(MAX_SAMPLES_INSTANCE)))
    {
        if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &resourceLimitsQos.max_samples_per_instance, ident))
            return XMLP_ret::XML_ERROR;
    }
    // allocated_samples - int32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(ALLOCATED_SAMPLES)))
    {
        if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &resourceLimitsQos.allocated_samples, ident))
            return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLHistoryQosPolicy(tinyxml2::XMLElement *elem, HistoryQosPolicy &historyQos, uint8_t ident)
{
    /*<xs:complexType name="historyQosPolicyType">
      <xs:all minOccurs="0">
        <xs:element name="kind" type="historyQosKindType"/>
        <xs:element name="depth" type="int32Type"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    // kind
    if (nullptr != (p_aux0 = elem->FirstChildElement(KIND)))
    {
        /*<xs:simpleType name="historyQosKindType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="KEEP_LAST"/>
            <xs:enumeration value="KEEP_ALL"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << KIND << "' without content");
            return XMLP_ret::XML_ERROR;
        }
             if (strcmp(text, KEEP_LAST) == 0) historyQos.kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
        else if (strcmp(text,  KEEP_ALL) == 0) historyQos.kind = HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
        else
        {
            logError(XMLPARSER, "Node '" << KIND << "' with bad content");
            return XMLP_ret::XML_ERROR;
        }
    }
    // depth - uint32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(DEPTH)))
    {
        if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &historyQos.depth, ident)) return XMLP_ret::XML_ERROR;
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLWriterQosPolicies(tinyxml2::XMLElement *elem, WriterQos &qos, uint8_t ident)
{
    /*<xs:complexType name="writerQosPoliciesType">
        <xs:all minOccurs="0">
            <xs:element name="durability" type="durabilityQosPolicyType"/>
            <xs:element name="durabilityService" type="durabilityServiceQosPolicyType"/>
            <xs:element name="deadline" type="deadlineQosPolicyType"/>
            <xs:element name="latencyBudget" type="latencyBudgetQosPolicyType"/>
            <xs:element name="liveliness" type="livelinessQosPolicyType"/>
            <xs:element name="reliability" type="reliabilityQosPolicyType"/>
            <xs:element name="lifespan" type="lifespanQosPolicyType"/>
            <xs:element name="userData" type="userDataQosPolicyType"/>
            <xs:element name="timeBasedFilter" type="timeBasedFilterQosPolicyType"/>
            <xs:element name="ownership" type="ownershipQosPolicyType"/>
            <xs:element name="ownershipStrength" type="ownershipStrengthQosPolicyType"/>
            <xs:element name="destinationOrder" type="destinationOrderQosPolicyType"/>
            <xs:element name="presentation" type="presentationQosPolicyType"/>
            <xs:element name="partition" type="partitionQosPolicyType"/>
            <xs:element name="topicData" type="topicDataQosPolicyType"/>
            <xs:element name="groupData" type="groupDataQosPolicyType"/>
            <xs:element name="publishMode" type="publishModeQosPolicyType"/>
        </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux = nullptr;

    // durability
    if (nullptr != (p_aux = elem->FirstChildElement(        DURABILITY)))
    {
        if (XMLP_ret::XML_OK != getXMLDurabilityQos(p_aux, qos.m_durability, ident)) return XMLP_ret::XML_ERROR;
    }
    // liveliness
    if (nullptr != (p_aux = elem->FirstChildElement(        LIVELINESS)))
    {
        if (XMLP_ret::XML_OK != getXMLLivelinessQos(p_aux, qos.m_liveliness, ident)) return XMLP_ret::XML_ERROR;
    }
    // reliability
    if (nullptr != (p_aux = elem->FirstChildElement(       RELIABILITY)))
    {
        if (XMLP_ret::XML_OK != getXMLReliabilityQos(p_aux, qos.m_reliability, ident)) return XMLP_ret::XML_ERROR;
    }
    // partition
    if (nullptr != (p_aux = elem->FirstChildElement(         PARTITION)))
    {
        if (XMLP_ret::XML_OK != getXMLPartitionQos(p_aux, qos.m_partition, ident)) return XMLP_ret::XML_ERROR;
    }
    // publishMode
    if (nullptr != (p_aux = elem->FirstChildElement(          PUB_MODE)))
    {
        if (XMLP_ret::XML_OK != getXMLPublishModeQos(p_aux, qos.m_publishMode, ident)) return XMLP_ret::XML_ERROR;
    }

    if (nullptr != (p_aux = elem->FirstChildElement(    DURABILITY_SRV)) ||
        nullptr != (p_aux = elem->FirstChildElement(          DEADLINE)) ||
        nullptr != (p_aux = elem->FirstChildElement(    LATENCY_BUDGET)) ||
        nullptr != (p_aux = elem->FirstChildElement(          LIFESPAN)) ||
        nullptr != (p_aux = elem->FirstChildElement(         USER_DATA)) ||
        nullptr != (p_aux = elem->FirstChildElement(       TIME_FILTER)) ||
        nullptr != (p_aux = elem->FirstChildElement(         OWNERSHIP)) ||
        nullptr != (p_aux = elem->FirstChildElement(OWNERSHIP_STRENGTH)) ||
        nullptr != (p_aux = elem->FirstChildElement(        DEST_ORDER)) ||
        nullptr != (p_aux = elem->FirstChildElement(      PRESENTATION)) ||
        nullptr != (p_aux = elem->FirstChildElement(        TOPIC_DATA)) ||
        nullptr != (p_aux = elem->FirstChildElement(        GROUP_DATA)))

        logError(XMLPARSER, "Quality os Service '" << p_aux->Value() << "' do not supported for now");

    // TODO: Do not supported for now
    //if (nullptr != (p_aux = elem->FirstChildElement(    DURABILITY_SRV))) getXMLDurabilityServiceQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(          DEADLINE))) getXMLDeadlineQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(    LATENCY_BUDGET))) getXMLLatencyBudgetQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(          LIFESPAN))) getXMLLifespanQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(         USER_DATA))) getXMLUserDataQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(       TIME_FILTER))) getXMLTimeBasedFilterQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(         OWNERSHIP))) getXMLOwnershipQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(OWNERSHIP_STRENGTH))) getXMLOwnershipStrengthQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(        DEST_ORDER))) getXMLDestinationOrderQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(      PRESENTATION))) getXMLPresentationQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(        TOPIC_DATA))) getXMLTopicDataQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(        GROUP_DATA))) getXMLGroupDataQos(p_aux, ident);

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLReaderQosPolicies(tinyxml2::XMLElement *elem, ReaderQos &qos, uint8_t ident)
{
    /*<xs:complexType name="readerQosPoliciesType">
        <xs:all minOccurs="0">
            <xs:element name="durability" type="durabilityQosPolicyType"/>
            <xs:element name="durabilityService" type="durabilityServiceQosPolicyType"/>
            <xs:element name="deadline" type="deadlineQosPolicyType"/>
            <xs:element name="latencyBudget" type="latencyBudgetQosPolicyType"/>
            <xs:element name="liveliness" type="livelinessQosPolicyType"/>
            <xs:element name="reliability" type="reliabilityQosPolicyType"/>
            <xs:element name="lifespan" type="lifespanQosPolicyType"/>
            <xs:element name="userData" type="userDataQosPolicyType"/>
            <xs:element name="timeBasedFilter" type="timeBasedFilterQosPolicyType"/>
            <xs:element name="ownership" type="ownershipQosPolicyType"/>
            <xs:element name="destinationOrder" type="destinationOrderQosPolicyType"/>
            <xs:element name="presentation" type="presentationQosPolicyType"/>
            <xs:element name="partition" type="partitionQosPolicyType"/>
            <xs:element name="topicData" type="topicDataQosPolicyType"/>
            <xs:element name="groupData" type="groupDataQosPolicyType"/>
        </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux = nullptr;

    // durability
    if (nullptr != (p_aux = elem->FirstChildElement(        DURABILITY)))
    {
        if (XMLP_ret::XML_OK != getXMLDurabilityQos(p_aux, qos.m_durability, ident)) return XMLP_ret::XML_ERROR;
    }
    // liveliness
    if (nullptr != (p_aux = elem->FirstChildElement(        LIVELINESS)))
    {
        if (XMLP_ret::XML_OK != getXMLLivelinessQos(p_aux, qos.m_liveliness, ident)) return XMLP_ret::XML_ERROR;
    }
    // reliability
    if (nullptr != (p_aux = elem->FirstChildElement(       RELIABILITY)))
    {
        if (XMLP_ret::XML_OK != getXMLReliabilityQos(p_aux, qos.m_reliability, ident)) return XMLP_ret::XML_ERROR;
    }
    // partition
    if (nullptr != (p_aux = elem->FirstChildElement(         PARTITION)))
    {
        if (XMLP_ret::XML_OK != getXMLPartitionQos(p_aux, qos.m_partition, ident)) return XMLP_ret::XML_ERROR;
    }

    if (nullptr != (p_aux = elem->FirstChildElement(    DURABILITY_SRV)) ||
        nullptr != (p_aux = elem->FirstChildElement(          DEADLINE)) ||
        nullptr != (p_aux = elem->FirstChildElement(    LATENCY_BUDGET)) ||
        nullptr != (p_aux = elem->FirstChildElement(          LIFESPAN)) ||
        nullptr != (p_aux = elem->FirstChildElement(         USER_DATA)) ||
        nullptr != (p_aux = elem->FirstChildElement(       TIME_FILTER)) ||
        nullptr != (p_aux = elem->FirstChildElement(         OWNERSHIP)) ||
        nullptr != (p_aux = elem->FirstChildElement(OWNERSHIP_STRENGTH)) ||
        nullptr != (p_aux = elem->FirstChildElement(        DEST_ORDER)) ||
        nullptr != (p_aux = elem->FirstChildElement(      PRESENTATION)) ||
        nullptr != (p_aux = elem->FirstChildElement(        TOPIC_DATA)) ||
        nullptr != (p_aux = elem->FirstChildElement(        GROUP_DATA)))

        logError(XMLPARSER, "Quality os Service '" << p_aux->Value() << "' do not supported for now");

    // TODO: Do not supported for now
    //if (nullptr != (p_aux = elem->FirstChildElement(    DURABILITY_SRV))) getXMLDurabilityServiceQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(          DEADLINE))) getXMLDeadlineQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(    LATENCY_BUDGET))) getXMLLatencyBudgetQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(          LIFESPAN))) getXMLLifespanQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(         USER_DATA))) getXMLUserDataQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(       TIME_FILTER))) getXMLTimeBasedFilterQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(         OWNERSHIP))) getXMLOwnershipQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(        DEST_ORDER))) getXMLDestinationOrderQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(      PRESENTATION))) getXMLPresentationQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(        TOPIC_DATA))) getXMLTopicDataQos(p_aux, ident);
    //if (nullptr != (p_aux = elem->FirstChildElement(        GROUP_DATA))) getXMLGroupDataQos(p_aux, ident);

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLDurabilityQos(tinyxml2::XMLElement *elem, DurabilityQosPolicy &durability, uint8_t /*ident*/)
{
    /*<xs:complexType name="durabilityQosPolicyType">
          <xs:all minOccurs="0">
            <xs:element name="kind" type="durabilityQosKindType"/>
          </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    // kind
    if (nullptr != (p_aux0 = elem->FirstChildElement(KIND)))
    {
        /*<xs:simpleType name="durabilityQosKindType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="VOLATILE"/>
            <xs:enumeration value="TRANSIENT_LOCAL"/>
            <xs:enumeration value="TRANSIENT"/>
            <xs:enumeration value="PERSISTENT"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << KIND << "' without content");
            return XMLP_ret::XML_ERROR;
        }
             if (strcmp(text,         _VOLATILE) == 0)
            durability.kind = DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS;
        else if (strcmp(text,  _TRANSIENT_LOCAL) == 0)
            durability.kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
        else if (strcmp(text,        _TRANSIENT) == 0)
            durability.kind = DurabilityQosPolicyKind::TRANSIENT_DURABILITY_QOS;
        else if (strcmp(text,       _PERSISTENT) == 0)
            durability.kind = DurabilityQosPolicyKind::PERSISTENT_DURABILITY_QOS;
        else
        {
            logError(XMLPARSER, "Node '" << KIND << "' with bad content");
            return XMLP_ret::XML_ERROR;
        }
    }
    else
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLDurabilityServiceQos(tinyxml2::XMLElement *elem,
                                                      DurabilityServiceQosPolicy &durabilityService,
                                                      uint8_t ident)
{
    /*<xs:complexType name="durabilityServiceQosPolicyType">
          <xs:all minOccurs="0">
            <xs:element name="service_cleanup_delay" type="durationType"/>
            <xs:element name="history_kind" type="historyQosKindType"/>
            <xs:element name="history_depth" type="uint32Type"/>
            <xs:element name="max_samples" type="uint32Type"/>
            <xs:element name="max_instances" type="uint32Type"/>
            <xs:element name="max_samples_per_instance" type="uint32Type"/>
          </xs:all>
        </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    // service_cleanup_delay - durationType
    if (nullptr != (p_aux0 = elem->FirstChildElement(SRV_CLEAN_DELAY)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, durabilityService.service_cleanup_delay, ident))
            return XMLP_ret::XML_ERROR;
    }
    // history_kind
    if (nullptr != (p_aux0 = elem->FirstChildElement(HISTORY_KIND)))
    {
        /*<xs:simpleType name="historyQosKindType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="KEEP_LAST"/>
            <xs:enumeration value="KEEP_ALL"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << HISTORY_KIND << "' without content");
            return XMLP_ret::XML_ERROR;
        }
             if (strcmp(text, KEEP_LAST) == 0)
            durabilityService.history_kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
        else if (strcmp(text,  KEEP_ALL) == 0)
            durabilityService.history_kind = HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
        else
        {
            logError(XMLPARSER, "Node '" << HISTORY_KIND << "' with bad content");
            return XMLP_ret::XML_ERROR;
        }
    }
    // history_depth - uint32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(HISTORY_DEPTH)))
    {
        if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &durabilityService.history_depth, ident))
            return XMLP_ret::XML_ERROR;
    }
    // max_samples - uint32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(MAX_SAMPLES)))
    {
        if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &durabilityService.max_samples, ident))
            return XMLP_ret::XML_ERROR;
    }
    // max_instances - uint32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(MAX_INSTANCES)))
    {
        if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &durabilityService.max_instances, ident))
            return XMLP_ret::XML_ERROR;
    }
    // max_samples_per_instance - uint32Type
    if (nullptr != (p_aux0 = elem->FirstChildElement(MAX_SAMPLES_INSTANCE)))
    {
        if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &durabilityService.max_samples_per_instance, ident))
            return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLDeadlineQos(tinyxml2::XMLElement *elem, DeadlineQosPolicy &deadline, uint8_t ident)
{
    /*<xs:complexType name="deadlineQosPolicyType">
      <xs:all>
        <xs:element name="period" type="durationType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    if (nullptr != (p_aux0 = elem->FirstChildElement(PERIOD)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, deadline.period, ident)) return XMLP_ret::XML_ERROR;
    }
    else
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLatencyBudgetQos(tinyxml2::XMLElement *elem, LatencyBudgetQosPolicy &latencyBudget, uint8_t ident)
{
    /*<xs:complexType name="latencyBudgetQosPolicyType">
      <xs:all>
        <xs:element name="duration" type="durationType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    if (nullptr != (p_aux0 = elem->FirstChildElement(DURATION)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, latencyBudget.duration, ident)) return XMLP_ret::XML_ERROR;
    }
    else
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLivelinessQos(tinyxml2::XMLElement *elem, LivelinessQosPolicy &liveliness, uint8_t ident)
{
    /*<xs:complexType name="livelinessQosPolicyType">
      <xs:all minOccurs="0">
        <xs:element name="kind" type="livelinessQosKindType"/>
        <xs:element name="lease_duration" type="durationType"/>
        <xs:element name="announcement_period" type="durationType"/>
      </xs:all>
    </xs:complexType>*/

    bool haveSomeField = false;
    tinyxml2::XMLElement *p_aux0 = nullptr;
    // kind
    if (nullptr != (p_aux0 = elem->FirstChildElement(KIND)))
    {
        /*<xs:simpleType name="livelinessQosKindType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="AUTOMATIC"/>
            <xs:enumeration value="MANUAL_BY_PARTICIPANT"/>
            <xs:enumeration value="MANUAL_BY_TOPIC"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << KIND << "' without content");
            return XMLP_ret::XML_ERROR;
        }
             if (strcmp(text,             AUTOMATIC) == 0)
            liveliness.kind = LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS;
        else if (strcmp(text, MANUAL_BY_PARTICIPANT) == 0)
            liveliness.kind = LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
        else if (strcmp(text,       MANUAL_BY_TOPIC) == 0)
            liveliness.kind = LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS;
        else
        {
            logError(XMLPARSER, "Node '" << KIND << "' with bad content");
            return XMLP_ret::XML_ERROR;
        }
        haveSomeField = true;
    }
    // lease_duration
    if (nullptr != (p_aux0 = elem->FirstChildElement(LEASE_DURATION)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, liveliness.lease_duration, ident)) return XMLP_ret::XML_ERROR;
        haveSomeField = true;
    }
    // announcement_period
    if (nullptr != (p_aux0 = elem->FirstChildElement(ANNOUNCE_PERIOD)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, liveliness.announcement_period, ident)) return XMLP_ret::XML_ERROR;
        haveSomeField = true;
    }
    if (!haveSomeField)
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLReliabilityQos(tinyxml2::XMLElement *elem, ReliabilityQosPolicy &reliability, uint8_t ident)
{
    /*<xs:complexType name="reliabilityQosPolicyType">
      <xs:all>
        <xs:element name="kind" type="reliabilityQosKindType"/>
        <xs:element name="max_blocking_time" type="durationType"/>
      </xs:all>
    </xs:complexType>*/

    bool haveSomeField = false;
    tinyxml2::XMLElement *p_aux0 = nullptr;
    // kind
    if (nullptr != (p_aux0 = elem->FirstChildElement(KIND)))
    {
        /*<xs:simpleType name="reliabilityQosKindType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="BEST_EFFORT"/>
            <xs:enumeration value="RELIABLE"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << KIND << "' without content");
            return XMLP_ret::XML_ERROR;
        }
             if (strcmp(text, _BEST_EFFORT) == 0)
            reliability.kind = ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
        else if (strcmp(text,    _RELIABLE) == 0)
            reliability.kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
        else
        {
            logError(XMLPARSER, "Node '" << KIND << "' with bad content");
            return XMLP_ret::XML_ERROR;
        }
        haveSomeField = true;
    }
    // max_blocking_time
    if (nullptr != (p_aux0 = elem->FirstChildElement(MAX_BLOCK_TIME)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, reliability.max_blocking_time, ident)) return XMLP_ret::XML_ERROR;
        haveSomeField = true;
    }
    if (!haveSomeField)
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLifespanQos(tinyxml2::XMLElement *elem, LifespanQosPolicy &lifespan, uint8_t ident)
{
    /*<xs:complexType name="lifespanQosPolicyType">
      <xs:all>
        <xs:element name="duration" type="durationType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    if (nullptr != (p_aux0 = elem->FirstChildElement(DURATION)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, lifespan.duration, ident)) return XMLP_ret::XML_ERROR;
    }
    else
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLTimeBasedFilterQos(tinyxml2::XMLElement *elem,
                                                    TimeBasedFilterQosPolicy &timeBasedFilter,
                                                    uint8_t ident)
{
    /*<xs:complexType name="timeBasedFilterQosPolicyType">
          <xs:all>
            <xs:element name="minimum_separation" type="durationType"/>
          </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    if (nullptr != (p_aux0 = elem->FirstChildElement(MIN_SEPARATION)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, timeBasedFilter.minimum_separation, ident)) return XMLP_ret::XML_ERROR;
    }
    else
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLOwnershipQos(tinyxml2::XMLElement *elem, OwnershipQosPolicy &ownership, uint8_t /*ident*/)
{
    /*<xs:complexType name="ownershipQosPolicyType">
      <xs:all>
        <xs:element name="kind" type="ownershipQosKindType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    if (nullptr != (p_aux0 = elem->FirstChildElement(KIND)))
    {
        /*<xs:simpleType name="ownershipQosKindType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="SHARED"/>
            <xs:enumeration value="EXCLUSIVE"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << KIND << "' without content");
            return XMLP_ret::XML_ERROR;
        }
             if (strcmp(text,    SHARED) == 0)
            ownership.kind = OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS;
        else if (strcmp(text, EXCLUSIVE) == 0)
            ownership.kind = OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS;
        else
        {
            logError(XMLPARSER, "Node '" << KIND << "' with bad content");
            return XMLP_ret::XML_ERROR;
        }
    }
    else
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLOwnershipStrengthQos(tinyxml2::XMLElement *elem,
                                                      OwnershipStrengthQosPolicy &ownershipStrength,
                                                      uint8_t ident)
{
    /*<xs:complexType name="ownershipStrengthQosPolicyType">
      <xs:all>
        <xs:element name="value" type="uint32Type"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    if (nullptr != (p_aux0 = elem->FirstChildElement(VALUE)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &ownershipStrength.value, ident)) return XMLP_ret::XML_ERROR;
    }
    else
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLDestinationOrderQos(tinyxml2::XMLElement *elem,
                                                     DestinationOrderQosPolicy &destinationOrder,
                                                     uint8_t /*ident*/)
{
    /*<xs:complexType name="destinationOrderQosPolicyType">
      <xs:all>
        <xs:element name="kind" type="destinationOrderQosKindType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    if (nullptr != (p_aux0 = elem->FirstChildElement(KIND)))
    {
        /*<xs:simpleType name="destinationOrderQosKindType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="BY_RECEPTION_TIMESTAMP"/>
            <xs:enumeration value="BY_SOURCE_TIMESTAMP"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << KIND << "' without content");
            return XMLP_ret::XML_ERROR;
        }
             if (strcmp(text, BY_RECEPTION_TIMESTAMP) == 0)
            destinationOrder.kind = DestinationOrderQosPolicyKind::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
        else if (strcmp(text,    BY_SOURCE_TIMESTAMP) == 0)
            destinationOrder.kind = DestinationOrderQosPolicyKind::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
        else
        {
            logError(XMLPARSER, "Node '" << KIND << "' bad content");
            return XMLP_ret::XML_ERROR;
        }
    }
    else
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLPresentationQos(tinyxml2::XMLElement *elem, PresentationQosPolicy &presentation, uint8_t ident)
{
    /*<xs:complexType name="presentationQosPolicyType">
      <xs:all minOccurs="0">
        <xs:element name="access_scope" type="presentationQosKindType"/>
        <xs:element name="coherent_access" type="boolType"/>
        <xs:element name="ordered_access" type="boolType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;
    // access_scope
    if (nullptr != (p_aux0 = elem->FirstChildElement(ACCESS_SCOPE)))
    {
        /*<xs:simpleType name="presentationQosKindType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="INSTANCE"/>
            <xs:enumeration value="TOPIC"/>
            <xs:enumeration value="GROUP"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << ACCESS_SCOPE << "' without content");
            return XMLP_ret::XML_ERROR;
        }
             if (strcmp(text, INSTANCE) == 0)
            presentation.access_scope = PresentationQosPolicyAccessScopeKind::INSTANCE_PRESENTATION_QOS;
        else if (strcmp(text,    TOPIC) == 0)
            presentation.access_scope = PresentationQosPolicyAccessScopeKind::TOPIC_PRESENTATION_QOS;
        else if (strcmp(text,    GROUP) == 0)
            presentation.access_scope = PresentationQosPolicyAccessScopeKind::GROUP_PRESENTATION_QOS;
        else
        {
            logError(XMLPARSER, "Node '" << ACCESS_SCOPE << "' bad content");
            return XMLP_ret::XML_ERROR;
        }
    }
    // coherent_access - boolType
    if (nullptr != (p_aux0 = elem->FirstChildElement(COHERENT_ACCESS)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &presentation.coherent_access, ident)) return XMLP_ret::XML_ERROR;
    }
    // ordered_access - boolType
    if (nullptr != (p_aux0 = elem->FirstChildElement(ORDERED_ACCESS)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &presentation.ordered_access, ident)) return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLPartitionQos(tinyxml2::XMLElement *elem, PartitionQosPolicy &partition, uint8_t ident)
{
    /*<xs:complexType name="partitionQosPolicyType">
      <xs:all>
        <xs:element name="names" type="stringVectorType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr, *p_aux1 = nullptr;

    if (nullptr != (p_aux0 = elem->FirstChildElement(NAMES)))
    {
        p_aux1 = p_aux0->FirstChildElement(NAME);
        if (nullptr == p_aux1)
        {
            // Not even one
            logError(XMLPARSER, "Node '" << NAMES << "' without content");
            return XMLP_ret::XML_ERROR;
        }
        std::vector<std::string> names;
        while (nullptr != p_aux1)
        {
            std::string name = "";
            if (XMLP_ret::XML_OK != getXMLString(p_aux1, &name, ident)) return XMLP_ret::XML_ERROR;
            names.push_back(name);
            p_aux1 = p_aux1->NextSiblingElement(NAME);
        }
        partition.setNames(names);
    }
    else
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLPublishModeQos(tinyxml2::XMLElement *elem, PublishModeQosPolicy &publishMode, uint8_t /*ident*/)
{
    /*<xs:complexType name="publishModeQosPolicyType">
      <xs:all>
        <xs:element name="kind" type="publishModeQosKindType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;

    if (nullptr != (p_aux0 = elem->FirstChildElement(KIND)))
    {
        /*<xs:simpleType name="publishModeQosKindType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="SYNCHRONOUS"/>
            <xs:enumeration value="ASYNCHRONOUS"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << KIND << "' without content");
            return XMLP_ret::XML_ERROR;
        }
             if (strcmp(text,  SYNCHRONOUS) == 0)
            publishMode.kind = PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE;
        else if (strcmp(text, ASYNCHRONOUS) == 0)
            publishMode.kind = PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE;
        else
        {
            logError(XMLPARSER, "Node '" << KIND << "' bad content");
            return XMLP_ret::XML_ERROR;
        }
    }
    else
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLDuration(tinyxml2::XMLElement *elem, Duration_t &duration, uint8_t ident)
{
    /*<xs:complexType name="durationType">
        <xs:choice>
            <xs:element name="durationbyname" type="durationTypeEnum"/>
            <xs:element name="durationbyval" type="durationTypeValue"/>
        </xs:choice>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr, *p_aux1 = nullptr;

    if (nullptr != (p_aux0 = elem->FirstChildElement(BY_NAME)))
    {
        /*<xs:simpleType name="durationTypeEnum">
          <xs:restriction base="xs:string">
            <xs:enumeration value="INFINITE"/>
            <xs:enumeration value="ZERO"/>
            <xs:enumeration value="INVALID"/>
          </xs:restriction>
        </xs:simpleType>*/
        const char* text = p_aux0->GetText();
        if (nullptr == text)
        {
            logError(XMLPARSER, "Node '" << BY_NAME << "' without content");
            return XMLP_ret::XML_ERROR;
        }
             if (strcmp(text, _INFINITE) == 0) duration = c_TimeInfinite;
        else if (strcmp(text,      ZERO) == 0) duration = c_TimeZero;
        else if (strcmp(text,   INVALID) == 0) duration = c_TimeInvalid;
        else
        {
            logError(XMLPARSER, "Node '" << BY_NAME << "' bad content");
            return XMLP_ret::XML_ERROR;
        }

        // Both ways forbidden
        if (nullptr != (p_aux0 = elem->FirstChildElement(BY_VAL)))
        {
            logError(XMLPARSER, "Node '" << BY_NAME << "' with several definitions");
            return XMLP_ret::XML_ERROR;
        }
    }

    if (nullptr != (p_aux0 = elem->FirstChildElement(BY_VAL)))
    {
        /*<xs:complexType name="durationTypeValue">
          <xs:sequence>
            <xs:element name="seconds" type="int32Type"/>
            <xs:element name="fraction" type="uint32Type"/>
          </xs:sequence>
        </xs:complexType>*/

        // seconds - uint32Type
        if (nullptr != (p_aux1 = p_aux0->FirstChildElement(SECONDS)))
        {
            if (XMLP_ret::XML_OK != getXMLInt(p_aux1, &duration.seconds, ident)) return XMLP_ret::XML_ERROR; // TODO: getXMLUint
        }
        // fraction - uint32Type
        if (nullptr != (p_aux1 = p_aux0->FirstChildElement(FRACTION)))
        {
            if (XMLP_ret::XML_OK != getXMLUint(p_aux1, &duration.fraction, ident)) return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLWriterTimes(tinyxml2::XMLElement *elem, WriterTimes &times, uint8_t ident)
{
    /*<xs:complexType name="writerTimesType">
      <xs:all minOccurs="0">
        <xs:element name="initialHeartbeatDelay" type="durationType"/>
        <xs:element name="heartbeatPeriod" type="durationType"/>
        <xs:element name="nackResponseDelay" type="durationType"/>
        <xs:element name="nackSupressionDuration" type="durationType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;
    // initialHeartbeatDelay
    if (nullptr != (p_aux0 = elem->FirstChildElement(INIT_HEARTB_DELAY)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.initialHeartbeatDelay, ident)) return XMLP_ret::XML_ERROR;
    }
    // heartbeatPeriod
    if (nullptr != (p_aux0 = elem->FirstChildElement(HEARTB_PERIOD)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.heartbeatPeriod, ident)) return XMLP_ret::XML_ERROR;
    }
    // nackResponseDelay
    if (nullptr != (p_aux0 = elem->FirstChildElement(NACK_RESP_DELAY)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.nackResponseDelay, ident)) return XMLP_ret::XML_ERROR;
    }
    // nackSupressionDuration
    if (nullptr != (p_aux0 = elem->FirstChildElement(NACK_SUPRESSION)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.nackSupressionDuration, ident)) return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLReaderTimes(tinyxml2::XMLElement *elem, ReaderTimes &times, uint8_t ident)
{
    /*<xs:complexType name="readerTimesType">
      <xs:all minOccurs="0">
        <xs:element name="initialAcknackDelay" type="durationType"/>
        <xs:element name="heartbeatResponseDelay" type="durationType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr;
    // initialAcknackDelay
    if (nullptr != (p_aux0 = elem->FirstChildElement(INIT_ACKNACK_DELAY)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.initialAcknackDelay, ident)) return XMLP_ret::XML_ERROR;
    }
    // heartbeatResponseDelay
    if (nullptr != (p_aux0 = elem->FirstChildElement(HEARTB_RESP_DELAY)))
    {
        if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.heartbeatResponseDelay, ident)) return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLocatorList(tinyxml2::XMLElement *elem, LocatorList_t &locatorList, uint8_t ident)
{
    /*<xs:complexType name="locatorListType">
      <xs:sequence>
        <xs:element name="locator" type="locatorType" minOccurs="0" maxOccurs="unbounded"/>
      </xs:sequence>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr, *p_aux1 = nullptr;

    p_aux0 = elem->FirstChildElement(LOCATOR);
    if (nullptr == p_aux0)
    {
        logError(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    while (nullptr != p_aux0)
    {
        /*<xs:complexType name="locatorType">
          <xs:all minOccurs="0">
            <xs:element name="kind" type="locatorKindType"/>
            <xs:element name="port" type="uint32Type"/>
            <xs:element name="address" type="stringType"/> <!-- octet address[16] -->
          </xs:all>
        </xs:complexType>*/
        Locator_t loc;
        // kind
        if (nullptr != (p_aux1 = p_aux0->FirstChildElement(KIND)))
        {
            /*<xs:simpleType name="locatorKindType">
              <xs:restriction base="xs:string">
                <xs:enumeration value="RESERVED"/>
                <xs:enumeration value="UDPv4"/>
                <xs:enumeration value="UDPv6"/>
              </xs:restriction>
            </xs:simpleType>*/
            const char* text = p_aux1->GetText();
            if (nullptr == text)
            {
                logError(XMLPARSER, "Node '" << KIND << "' without content");
                return XMLP_ret::XML_ERROR;
            }
                 if (strcmp(text, RESERVED) == 0)
                loc.kind = LOCATOR_KIND_RESERVED;
            else if (strcmp(text,    UDPv4) == 0)
                loc.kind = LOCATOR_KIND_UDPv4;
            else if (strcmp(text,    UDPv6) == 0)
                loc.kind = LOCATOR_KIND_UDPv6;
            else
            {
                logError(XMLPARSER, "Node '" << KIND << "' bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        // port - uint32Type
        if (nullptr != (p_aux1 = p_aux0->FirstChildElement(PORT)))
        {
            if (XMLP_ret::XML_OK != getXMLUint(p_aux1, &loc.port, ident + 1)) return XMLP_ret::XML_ERROR;
        }
        /// address - stringType
        if (nullptr != (p_aux1 = p_aux0->FirstChildElement(ADDRESS)))
        {
            std::string s = "";
            if (XMLP_ret::XML_OK != getXMLString(p_aux1, &s, ident + 1)) return XMLP_ret::XML_ERROR;
            loc.set_IP4_address(s);
        }

        locatorList.push_back(loc);
        p_aux0 = p_aux0->NextSiblingElement(LOCATOR);
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLHistoryMemoryPolicy(tinyxml2::XMLElement *elem,
                                                     MemoryManagementPolicy_t &historyMemoryPolicy,
                                                     uint8_t /*ident*/)
{
    /*<xs:simpleType name="historyMemoryPolicyType">
      <xs:restriction base="xs:string">
        <xs:enumeration value="PREALLOCATED"/>
        <xs:enumeration value="PREALLOCATED_WITH_REALLOC"/>
        <xs:enumeration value="DYNAMIC"/>
      </xs:restriction>
    </xs:simpleType>*/
    const char* text = elem->GetText();
    if (nullptr == text)
    {
        logError(XMLPARSER, "Node '" << KIND << "' without content");
        return XMLP_ret::XML_ERROR;
    }
         if (strcmp(text,              PREALLOCATED) == 0)
        historyMemoryPolicy = MemoryManagementPolicy::PREALLOCATED_MEMORY_MODE;
    else if (strcmp(text, PREALLOCATED_WITH_REALLOC) == 0)
        historyMemoryPolicy = MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    else if (strcmp(text,                   DYNAMIC) == 0)
        historyMemoryPolicy = MemoryManagementPolicy::DYNAMIC_RESERVE_MEMORY_MODE;
    else
    {
        logError(XMLPARSER, "Node '" << KIND << "' bad content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLPropertiesPolicy(tinyxml2::XMLElement *elem, PropertyPolicy &propertiesPolicy, uint8_t ident)
{
    /*<xs:complexType name="propertyPolicyType">
      <xs:all minOccurs="0">
        <xs:element name="properties" type="propertyVectorType"/>
        <xs:element name="binary_properties" type="binaryPropertyVectorType"/>
      </xs:all>
    </xs:complexType>*/

    tinyxml2::XMLElement *p_aux0 = nullptr, *p_aux1 = nullptr, *p_aux2 = nullptr;

    if (nullptr != (p_aux0 = elem->FirstChildElement(PROPERTIES)))
    {
        p_aux1 = p_aux0->FirstChildElement(PROPERTY);
        if (nullptr == p_aux1)
        {
            logError(XMLPARSER, "Node '" << PROPERTIES << "' without content");
            return XMLP_ret::XML_ERROR;
        }

        while (nullptr != p_aux1)
        {
            /*<xs:complexType name="propertyType">
              <xs:all>
                <xs:element name="name" type="stringType"/>
                <xs:element name="value" type="stringType"/>
                <xs:element name="propagate" type="boolType"/>
              </xs:all>
            </xs:complexType>*/
            Property prop;
            // name - stringType
            if (nullptr != (p_aux2 = p_aux1->FirstChildElement(NAME)))
            {
                std::string s = "";
                if (XMLP_ret::XML_OK != getXMLString(p_aux2, &s, ident + 2)) return XMLP_ret::XML_ERROR;
                prop.name(s);
            }
            // value - stringType
            if (nullptr != (p_aux2 = p_aux1->FirstChildElement(VALUE)))
            {
                std::string s = "";
                if (XMLP_ret::XML_OK != getXMLString(p_aux2, &s, ident + 2)) return XMLP_ret::XML_ERROR;
                prop.value(s);
            }
            // propagate - boolType
            if (nullptr != (p_aux2 = p_aux1->FirstChildElement(PROPAGATE)))
            {
                bool b = false;
                if (XMLP_ret::XML_OK != getXMLBool(p_aux2, &b, ident + 2)) return XMLP_ret::XML_ERROR;
                prop.propagate(b);
            }
            propertiesPolicy.properties().push_back(prop);
            p_aux1 = p_aux1->NextSiblingElement(PROPERTY);
        }
    }

    // TODO: The value will be std::vector<uint8_t>
    if (nullptr != (p_aux0 = elem->FirstChildElement(BIN_PROPERTIES)))
    {
        p_aux1 = p_aux0->FirstChildElement(PROPERTY);
        if (nullptr == p_aux1)
        {
            logError(XMLPARSER, "Node '" << BIN_PROPERTIES << "' without content");
            return XMLP_ret::XML_ERROR;
        }

        while (nullptr != p_aux1)
        {
            /*<xs:complexType name="binaryPropertyType">
              <xs:all>
                  <xs:element name="name" type="stringType"/>
                  <xs:element name="value" type="stringType"/> <!-- std::vector<uint8_t> -->
                  <xs:element name="propagate" type="boolType"/>
              </xs:all>
            </xs:complexType>*/
            BinaryProperty bin_prop;
            // name - stringType
            if (nullptr != (p_aux2 = p_aux1->FirstChildElement(NAME)))
            {
                std::string s = "";
                if (XMLP_ret::XML_OK != getXMLString(p_aux2, &s, ident + 2)) return XMLP_ret::XML_ERROR;
                bin_prop.name(s);
            }
            // TODO:
            // value - stringType
            if (nullptr != (p_aux2 = p_aux1->FirstChildElement(VALUE)))
            {
                logError(XMLPARSER, "Tag '" << p_aux2->Value() << "' do not supported for now");
                /*std::string s = "";
                if (XMLP_ret::XML_OK != getXMLString(p_aux2, &s, ident + 2)) return XMLP_ret::XML_ERROR;
                bin_prop.value(s);*/
            }
            // propagate - boolType
            if (nullptr != (p_aux2 = p_aux1->FirstChildElement(PROPAGATE)))
            {
                bool b = false;
                if (XMLP_ret::XML_OK != getXMLBool(p_aux2, &b, ident + 2)) return XMLP_ret::XML_ERROR;
                bin_prop.propagate(b);
            }
            propertiesPolicy.binary_properties().push_back(bin_prop);
            p_aux1 = p_aux1->NextSiblingElement(PROPERTY);
        }
    }
    return XMLP_ret::XML_OK;
}

// TODO
XMLP_ret XMLParser::getXMLOctetVector(tinyxml2::XMLElement *elem, std::vector<octet> &/*octetVector*/, uint8_t /*ident*/)
{
    logError(XMLPARSER, "Tag '" << elem->Value() << "' octetVector do not supported for now");
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLInt(tinyxml2::XMLElement *elem, int *in, uint8_t /*ident*/)
{
    if (nullptr == elem || nullptr == in)
    {
        logError(XMLPARSER, "nullptr when getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    else if (tinyxml2::XMLError::XML_SUCCESS != elem->QueryIntText(in))
    {
        logError(XMLPARSER, "<" << elem->Value() << "> getXMLInt XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLUint(tinyxml2::XMLElement *elem, unsigned int *ui, uint8_t /*ident*/)
{
    if (nullptr == elem || nullptr == ui)
    {
        logError(XMLPARSER, "nullptr when getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    else if (tinyxml2::XMLError::XML_SUCCESS != elem->QueryUnsignedText(ui))
    {
        logError(XMLPARSER, "<" << elem->Value() << "> getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLUint(tinyxml2::XMLElement *elem, uint16_t *ui16, uint8_t /*ident*/)
{
    unsigned int ui = 0u;
    if (nullptr == elem || nullptr == ui16)
    {
        logError(XMLPARSER, "nullptr when getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    else if (tinyxml2::XMLError::XML_SUCCESS != elem->QueryUnsignedText(&ui) ||
        ui >= 65536)
    {
        logError(XMLPARSER, "<" << elem->Value() << "> getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    *ui16 = static_cast<uint16_t>(ui);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLBool(tinyxml2::XMLElement *elem, bool *b, uint8_t /*ident*/)
{
    if (nullptr == elem || nullptr == b)
    {
        logError(XMLPARSER, "nullptr when getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    else if (tinyxml2::XMLError::XML_SUCCESS != elem->QueryBoolText(b))
    {
        logError(XMLPARSER, "<" << elem->Value() << "> getXMLBool XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLString(tinyxml2::XMLElement *elem, std::string *s, uint8_t /*ident*/)
{
    const char* text = nullptr;

    if (nullptr == elem || nullptr == s)
    {
        logError(XMLPARSER, "nullptr when getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    else if (nullptr == (text = elem->GetText()))
    {
        logError(XMLPARSER, "<" << elem->Value() << "> getXMLString XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    *s = text;
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLTransportList(
        tinyxml2::XMLElement *elem,
        std::vector<std::shared_ptr<TransportDescriptorInterface> > &transportList,
        uint8_t ident)
{
  /*<xs:complexType name="transportListType">
      <xs:sequence>
        <xs:element name="transport" type="transportType" minOccurs="0" maxOccurs="unbounded"/>
      </xs:sequence>
    </xs:complexType>*/
  tinyxml2::XMLElement *p_aux0 = nullptr, *p_aux1 = nullptr;
  p_aux0 = elem->FirstChildElement(TRANSPORT);
  if (nullptr == p_aux0)
  {
    logError(XMLPROFILEPARSER, "Node '" << elem->Value() << "' without content");
    return XMLP_ret::XML_ERROR;
  }

  while (nullptr != p_aux0)
  {
    /*<xs:complexType name="transportType">
        <xs:all minOccurs="0">
          <xs:element name="kind" type="transportKindType"/>
          <xs:element name="interfaceWhiteList" type="stringType"/>
          <xs:element name="whiteListOutput" type="boolType"/>
          <xs:element name="whiteListInput" type="boolType"/>
          <xs:element name="whiteListLocators" type="boolType"/>
        </xs:all>
      </xs:complexType>*/

    // kind
    if (nullptr != (p_aux1 = p_aux0->FirstChildElement(KIND)))
    {
      /*<xs:simpleType name="transportKindType">
          <xs:restriction base="xs:string">
            <xs:enumeration value="UDPv4"/>
            <xs:enumeration value="UDPv6"/>
          </xs:restriction>
        </xs:simpleType>*/
      const char* text = p_aux1->GetText();
      if (nullptr == text)
      {
        logError(XMLPROFILEPARSER, "Node '" << KIND << "' without content");
        return XMLP_ret::XML_ERROR;
      }
      bool whiteListOutput = false;
      bool whiteListInput = false;
      bool whiteListLocators = false;
      std::vector<std::string> interfaceWhiteList;

      // whiteListOutput - boolType
      if (nullptr != (p_aux1 = p_aux0->FirstChildElement(WHITE_LIST_OUTPUT)))
      {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux1, &(whiteListOutput), ident + 1))
        {
          return XMLP_ret::XML_ERROR;
        }
      }

      // whiteListInput - boolType
      if (nullptr != (p_aux1 = p_aux0->FirstChildElement(WHITE_LIST_INPUT)))
      {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux1, &(whiteListInput), ident + 1))
        {
          return XMLP_ret::XML_ERROR;
        }
      }

      // whiteListLocators - boolType
      if (nullptr != (p_aux1 = p_aux0->FirstChildElement(WHITE_LIST_LOCATORS)))
      {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux1, &(whiteListLocators), ident + 1))
        {
          return XMLP_ret::XML_ERROR;
        }
      }

      // interfaceWhiteList - stringType
      if (nullptr != (p_aux1 = p_aux0->FirstChildElement(INTERFACE_WHITE_LIST)))
      {
        std::string s = "";
        if (XMLP_ret::XML_OK != getXMLString(p_aux1, &s, ident + 1))
        {
          return XMLP_ret::XML_ERROR;
        }
        std::stringstream ss(s);
        std::string interface;
        while (getline(ss, interface, ','))
        {
          interfaceWhiteList.push_back(interface);
        }
      }

      if (strcmp(text, UDPv4) == 0)
      {
        auto transport = std::make_shared<UDPv4TransportDescriptor>();
        transport->whiteListOutput = whiteListOutput;
        transport->whiteListInput = whiteListInput;
        transport->whiteListLocators = whiteListLocators;
        transport->interfaceWhiteList = interfaceWhiteList;
        transportList.push_back(transport);
      }
      else if (strcmp(text, UDPv6) == 0)
      {
        auto transport = std::make_shared<UDPv6TransportDescriptor>();
        transport->whiteListOutput = whiteListOutput;
        transport->whiteListInput = whiteListInput;
        transport->whiteListLocators = whiteListLocators;
        transport->interfaceWhiteList = interfaceWhiteList;
        transportList.push_back(transport);
      }
      else
      {
        logError(XMLPROFILEPARSER, "Node '" << KIND << "' bad content");
        return XMLP_ret::XML_ERROR;
      }
    }

    p_aux0 = p_aux0->NextSiblingElement(TRANSPORT);
  }

  return XMLP_ret::XML_OK;
}
