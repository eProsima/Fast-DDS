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
 * @file XMLEndpointParser.cpp
 *
 */

#include <xmlparser/XMLEndpointParser.h>

#include <cstdlib>
#include <string>

#include <tinyxml2.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <utils/TimeConversion.hpp>
#include <xmlparser/XMLParserUtils.hpp>

namespace eprosima {
namespace fastdds {
namespace xmlparser {

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::xml::detail;

XMLEndpointParser::XMLEndpointParser()
{
}

XMLEndpointParser::~XMLEndpointParser()
{
    for (std::vector<StaticRTPSParticipantInfo*>::iterator pit = m_RTPSParticipants.begin();
            pit != m_RTPSParticipants.end(); ++pit)
    {
        for (std::vector<ReaderProxyData*>::iterator rit = (*pit)->m_readers.begin();
                rit != (*pit)->m_readers.end(); ++rit)
        {
            delete(*rit);
        }
        for (std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
                wit != (*pit)->m_writers.end(); ++wit)
        {
            delete(*wit);
        }

        delete(*pit);
    }
}

XMLP_ret XMLEndpointParser::loadXMLFile(
        std::string& filename)
{
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError eResult;

    if (0 == filename.rfind("data://", 0))
    {
        EPROSIMA_LOG_INFO(RTPS_EDP, filename);
        eResult = doc.Parse(filename.c_str() + 7, filename.size() - 7);
    }
    else if (0 == filename.rfind("file://", 0))
    {
        EPROSIMA_LOG_INFO(RTPS_EDP, filename);
        eResult = doc.LoadFile(filename.substr(7).c_str());
    }
    else
    {
        EPROSIMA_LOG_INFO(RTPS_EDP, "FileName: " << filename);
        eResult = doc.LoadFile(filename.c_str());
    }

    if (tinyxml2::XML_SUCCESS != eResult)
    {
        EPROSIMA_LOG_ERROR(RTPS_EDP, filename << " bad file");
        return XMLP_ret::XML_ERROR;
    }

    tinyxml2::XMLNode* root = doc.FirstChildElement(STATICDISCOVERY);
    if (!root)
    {
        EPROSIMA_LOG_ERROR(RTPS_EDP, filename << " XML has errors");
        return XMLP_ret::XML_ERROR;
    }

    tinyxml2::XMLElement* xml_RTPSParticipant = root->FirstChildElement();

    while (xml_RTPSParticipant != nullptr)
    {
        std::string key(xml_RTPSParticipant->Name());
        if (key == PARTICIPANT)
        {
            StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
            loadXMLParticipantEndpoint(xml_RTPSParticipant, pdata);
            m_RTPSParticipants.push_back(pdata);
        }
        xml_RTPSParticipant = xml_RTPSParticipant->NextSiblingElement();
    }

    EPROSIMA_LOG_INFO(RTPS_EDP, "Finished parsing, " << m_RTPSParticipants.size() << " participants found.");
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLEndpointParser::loadXMLNode(
        tinyxml2::XMLDocument& doc)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, "XML node");

    tinyxml2::XMLNode* root = doc.FirstChildElement(STATICDISCOVERY);
    if (!root)
    {
        EPROSIMA_LOG_ERROR(RTPS_EDP, "XML node has errors");
        return XMLP_ret::XML_ERROR;
    }


    tinyxml2::XMLElement* xml_RTPSParticipant = root->FirstChildElement();

    while (xml_RTPSParticipant != nullptr)
    {
        std::string key(xml_RTPSParticipant->Name());
        if (key == PARTICIPANT)
        {
            StaticRTPSParticipantInfo* pdata = new StaticRTPSParticipantInfo();
            loadXMLParticipantEndpoint(xml_RTPSParticipant, pdata);
            m_RTPSParticipants.push_back(pdata);
        }
        xml_RTPSParticipant = xml_RTPSParticipant->NextSiblingElement();
    }

    EPROSIMA_LOG_INFO(RTPS_EDP, "Finished parsing, " << m_RTPSParticipants.size() << " participants found.");
    return XMLP_ret::XML_OK;
}

void XMLEndpointParser::loadXMLParticipantEndpoint(
        tinyxml2::XMLElement* xml_endpoint,
        StaticRTPSParticipantInfo* pdata)
{
    tinyxml2::XMLNode* xml_RTPSParticipant_child = xml_endpoint;
    tinyxml2::XMLElement* element = xml_RTPSParticipant_child->FirstChildElement();

    while (element != nullptr)
    {
        std::string key(element->Name());

        if (key == NAME)
        {
            pdata->m_RTPSParticipantName = get_element_text(element);
        }
        else if (key == READER)
        {
            if (loadXMLReaderEndpoint(element, pdata) != XMLP_ret::XML_OK)
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Reader Endpoint has error, ignoring");
            }
        }
        else if (key == WRITER)
        {
            if (loadXMLWriterEndpoint(element, pdata) != XMLP_ret::XML_OK)
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Writer Endpoint has error, ignoring");
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(RTPS_EDP, "Unknown XMK tag: " << key);
        }

        element = element->NextSiblingElement();
    }
}

XMLP_ret XMLEndpointParser::loadXMLReaderEndpoint(
        tinyxml2::XMLElement* xml_endpoint,
        StaticRTPSParticipantInfo* pdata)
{
    LocatorList_t unicast_locators;
    LocatorList_t multicast_locators;

    tinyxml2::XMLNode* xml_endpoint_child = xml_endpoint;
    tinyxml2::XMLElement* element = xml_endpoint_child->FirstChildElement();

    // Parse locators first
    while (element != nullptr)
    {
        std::string key(element->Name());
        if (key == UNICAST_LOCATOR)
        {
            Locator_t loc;
            loc.kind = 1;
            const char* address = element->Attribute(ADDRESS);
            std::string auxString(address ? address : "");
            IPLocator::setIPv4(loc, auxString);
            int port = 0;
            element->QueryIntAttribute(PORT, &port);
            loc.port = static_cast<uint16_t>(port);
            unicast_locators.push_back(loc);
        }
        else if (key == MULTICAST_LOCATOR)
        {
            Locator_t loc;
            loc.kind = 1;
            const char* address = element->Attribute(ADDRESS);
            std::string auxString(address ? address : "");
            IPLocator::setIPv4(loc, auxString);
            int port = 0;
            element->QueryIntAttribute(PORT, &port);
            loc.port = static_cast<uint16_t>(port);
            multicast_locators.push_back(loc);
        }

        element = element->NextSiblingElement();
    }


    ReaderProxyData* rdata = new ReaderProxyData(unicast_locators.size(), multicast_locators.size());

    xml_endpoint_child = xml_endpoint;
    element = xml_endpoint_child->FirstChildElement();

    while (element != nullptr)
    {
        std::string key(element->Name());
        if (key == USER_ID)
        {
            int16_t id = static_cast<int16_t>(std::strtol(get_element_text(element).c_str(), nullptr, 10));
            if (id <= 0 || m_endpointIds.insert(id).second == false)
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Repeated or negative ID in XML file");
                delete(rdata);
                return XMLP_ret::XML_ERROR;
            }
            rdata->user_defined_id(id);
        }
        else if (key == ENTITY_ID)
        {
            int32_t id = std::strtol(get_element_text(element).c_str(), nullptr, 10);
            if (id <= 0 || m_entityIds.insert(id).second == false)
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Repeated or negative entityId in XML file");
                delete(rdata);
                return XMLP_ret::XML_ERROR;
            }
            octet* c = (octet*)&id;
            rdata->guid.entityId.value[2] = c[0];
            rdata->guid.entityId.value[1] = c[1];
            rdata->guid.entityId.value[0] = c[2];
        }
        else if (key == EXPECT_INLINE_QOS)
        {
            std::string auxString(get_element_text(element));
            if (auxString == "true")
            {
                rdata->expects_inline_qos = true;
            }
            else if (auxString == "false")
            {
                rdata->expects_inline_qos = false;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP,
                        "Bad XML file, endpoint of expects_inline_qos: " << auxString << " is not valid");
                delete(rdata);
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (key == TOPIC) // deprecated api
        {
            const char* topicName = element->Attribute(NAME);
            const char* typeName = element->Attribute(DATA_TYPE);
            const char* kind = element->Attribute(KIND);

            rdata->topic_name = (topicName != nullptr) ? std::string(topicName) : std::string("");
            rdata->type_name = (topicName != nullptr) ? std::string(typeName) : std::string("");
            std::string auxString(kind ? kind : "");
            if (auxString == _NO_KEY)
            {
                rdata->topic_kind = NO_KEY;
                rdata->guid.entityId.value[3] = 0x04;
            }
            else if (auxString == _WITH_KEY)
            {
                rdata->topic_kind = WITH_KEY;
                rdata->guid.entityId.value[3] = 0x07;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, topic of kind: " << auxString << " is not valid");
                delete(rdata);
                return XMLP_ret::XML_ERROR;
            }
            if (rdata->topic_name == EPROSIMA_UNKNOWN_STRING || rdata->type_name == EPROSIMA_UNKNOWN_STRING)
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP,
                        "Bad XML file, topic: " << rdata->topic_name << " or typeName: " << rdata->type_name <<
                        " undefined");
                delete(rdata);
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (key == TOPIC_NAME)
        {
            rdata->topic_name = get_element_text(element);
        }
        else if (key == TOPIC_DATA_TYPE)
        {
            rdata->type_name = get_element_text(element);
        }
        else if (key == TOPIC_KIND)
        {
            std::string auxString(get_element_text(element));
            if (auxString == _NO_KEY)
            {
                rdata->topic_kind = NO_KEY;
                rdata->guid.entityId.value[3] = 0x04;
            }
            else if (auxString == _WITH_KEY)
            {
                rdata->topic_kind = WITH_KEY;
                rdata->guid.entityId.value[3] = 0x07;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, topic of kind: " << auxString << " is not valid");
                delete(rdata);
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (key == RELIABILITY_QOS)
        {
            std::string auxString(get_element_text(element));
            if (auxString == _RELIABLE_RELIABILITY_QOS)
            {
                rdata->reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
            }
            else if (auxString == _BEST_EFFORT_RELIABILITY_QOS)
            {
                rdata->reliability.kind = dds::BEST_EFFORT_RELIABILITY_QOS;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, endpoint of stateKind: " << auxString << " is not valid");
                delete(rdata);
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (key == UNICAST_LOCATOR)
        {
            // Empty but necessary to avoid warning on last else
        }
        else if (key == MULTICAST_LOCATOR)
        {
            // Empty but necessary to avoid warning on last else
        }
        else if (key == DURABILITY_QOS)
        {
            std::string auxstring(get_element_text(element));
            if (auxstring == _PERSISTENT_DURABILITY_QOS)
            {
                rdata->durability.kind = dds::PERSISTENT_DURABILITY_QOS;
            }
            else if (auxstring == _TRANSIENT_DURABILITY_QOS)
            {
                rdata->durability.kind = dds::TRANSIENT_DURABILITY_QOS;
            }
            else if (auxstring == _TRANSIENT_LOCAL_DURABILITY_QOS)
            {
                rdata->durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
            }
            else if (auxstring == _VOLATILE_DURABILITY_QOS)
            {
                rdata->durability.kind = dds::VOLATILE_DURABILITY_QOS;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, durability of kind: " << auxstring << " is not valid");
                delete(rdata);
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (key == OWNERSHIP_QOS)
        {
            const char* ownership = element->Attribute(KIND);
            std::string auxstring(ownership ? ownership : OWNERSHIP_KIND_NOT_PRESENT);
            if (auxstring == _SHARED_OWNERSHIP_QOS)
            {
                rdata->ownership.kind = dds::SHARED_OWNERSHIP_QOS;
            }
            else if (auxstring == _EXCLUSIVE_OWNERSHIP_QOS)
            {
                rdata->ownership.kind = dds::EXCLUSIVE_OWNERSHIP_QOS;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, ownership of kind: " << auxstring << " is not valid");
                delete(rdata);
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (key == PARTITION_QOS)
        {
            rdata->partition.push_back(get_element_text(element).c_str());
        }
        else if (key == LIVELINESS_QOS)
        {
            const char* kind = element->Attribute(KIND);
            std::string auxstring(kind ? kind : LIVELINESS_KIND_NOT_PRESENT);
            if (auxstring == _AUTOMATIC_LIVELINESS_QOS)
            {
                rdata->liveliness.kind = dds::AUTOMATIC_LIVELINESS_QOS;
            }
            else if (auxstring == _MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            {
                rdata->liveliness.kind = dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
            }
            else if (auxstring == _MANUAL_BY_TOPIC_LIVELINESS_QOS)
            {
                rdata->liveliness.kind = dds::MANUAL_BY_TOPIC_LIVELINESS_QOS;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, liveliness of kind: " << auxstring << " is not valid");
                delete(rdata);
                return XMLP_ret::XML_ERROR;
            }
            const char* leaseDuration_ms = element->Attribute(LEASE_DURATION_MS);
            auxstring = std::string(leaseDuration_ms ? leaseDuration_ms : _INF);
            if (auxstring == _INF)
            {
                rdata->liveliness.lease_duration = dds::c_TimeInfinite;
            }
            else
            {
                uint32_t milliseclease = std::strtoul(auxstring.c_str(), nullptr, 10);
                rdata->liveliness.lease_duration =
                        fastdds::rtps::TimeConv::MilliSeconds2Time_t((double)milliseclease).to_duration_t();
                if (milliseclease == 0)
                {
                    EPROSIMA_LOG_WARNING(RTPS_EDP, "BAD XML:livelinessQos leaseDuration is 0");
                }
            }
        }
        else if (key == DISABLE_POSITIVE_ACKS)
        {
            // Disable positive acks
            if (XMLP_ret::XML_OK !=
                    XMLParser::getXMLDisablePositiveAcksQos(element, rdata->disable_positive_acks, 0))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_EDP, "Unkown Endpoint-XML tag, ignoring " << key);
        }

        element = element->NextSiblingElement();
    }

    if (rdata->user_defined_id() == 0)
    {
        EPROSIMA_LOG_ERROR(RTPS_EDP, "Reader XML endpoint with NO ID defined");
        delete(rdata);
        return XMLP_ret::XML_ERROR;
    }

    for (const Locator_t& loc : unicast_locators)
    {
        rdata->add_unicast_locator(loc);
    }

    for (const Locator_t& loc : multicast_locators)
    {
        rdata->add_multicast_locator(loc);
    }

    pdata->m_readers.push_back(rdata);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLEndpointParser::loadXMLWriterEndpoint(
        tinyxml2::XMLElement* xml_endpoint,
        StaticRTPSParticipantInfo* pdata)
{
    LocatorList_t unicast_locators;
    LocatorList_t multicast_locators;

    tinyxml2::XMLNode* xml_endpoint_child = xml_endpoint;
    tinyxml2::XMLElement* element = xml_endpoint_child->FirstChildElement();

    while (element != nullptr)
    {
        std::string key(element->Name());
        if (key == UNICAST_LOCATOR)
        {
            Locator_t loc;
            loc.kind = 1;
            const char* address = element->Attribute(ADDRESS);
            std::string auxString(address ? address : "");
            IPLocator::setIPv4(loc, auxString);
            int port = 0;
            element->QueryIntAttribute(PORT, &port);
            loc.port = static_cast<uint16_t>(port);
            unicast_locators.push_back(loc);
        }
        else if (key == MULTICAST_LOCATOR)
        {
            Locator_t loc;
            loc.kind = 1;
            const char* address = element->Attribute(ADDRESS);
            std::string auxString(address ? address : "");
            IPLocator::setIPv4(loc, auxString);
            int port = 0;
            element->QueryIntAttribute(PORT, &port);
            loc.port = static_cast<uint16_t>(port);
            multicast_locators.push_back(loc);
        }
        element = element->NextSiblingElement();
    }

    WriterProxyData* wdata = new WriterProxyData(unicast_locators.size(), multicast_locators.size());

    xml_endpoint_child = xml_endpoint;
    element = xml_endpoint_child->FirstChildElement();
    while (element != nullptr)
    {
        std::string key(element->Name());
        if (key == USER_ID)
        {
            int16_t id = static_cast<int16_t>(std::strtol(get_element_text(element).c_str(), nullptr, 10));
            if (id <= 0 || m_endpointIds.insert(id).second == false)
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Repeated or negative ID in XML file");
                delete(wdata);
                return XMLP_ret::XML_ERROR;
            }
            wdata->user_defined_id(id);
        }
        else if (key == ENTITY_ID)
        {
            int32_t id = std::strtol(get_element_text(element).c_str(), nullptr, 10);
            if (id <= 0 || m_entityIds.insert(id).second == false)
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Repeated or negative entityId in XML file");
                delete(wdata);
                return XMLP_ret::XML_ERROR;
            }
            octet* c = (octet*)&id;
            wdata->guid.entityId.value[2] = c[0];
            wdata->guid.entityId.value[1] = c[1];
            wdata->guid.entityId.value[0] = c[2];
        }
        else if (key == EXPECT_INLINE_QOS)
        {
            EPROSIMA_LOG_WARNING(RTPS_EDP, "BAD XML tag: Writers don't use expectInlineQos tag");
        }
        else if (key == TOPIC)
        {
            const char* topicName = element->Attribute(NAME);
            wdata->topic_name = std::string(topicName ? topicName : EPROSIMA_UNKNOWN_STRING);
            const char* typeName = element->Attribute(DATA_TYPE);
            wdata->type_name = std::string(typeName ? typeName : EPROSIMA_UNKNOWN_STRING);
            const char* kind = element->Attribute(KIND);
            std::string auxString(kind ? kind : "");
            if (auxString == _NO_KEY)
            {
                wdata->topic_kind = NO_KEY;
                wdata->guid.entityId.value[3] = 0x03;
            }
            else if (auxString == _WITH_KEY)
            {
                wdata->topic_kind = WITH_KEY;
                wdata->guid.entityId.value[3] = 0x02;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, topic of kind: " << auxString << " is not valid");
                delete(wdata);
                return XMLP_ret::XML_ERROR;
            }
            if (wdata->topic_name == EPROSIMA_UNKNOWN_STRING || wdata->type_name == EPROSIMA_UNKNOWN_STRING)
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP,
                        "Bad XML file, topic: " << wdata->topic_name << " or typeName: " << wdata->type_name <<
                        " undefined");
                delete(wdata);
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (key == TOPIC_NAME)
        {
            wdata->topic_name = std::string(get_element_text(element));
        }
        else if (key == TOPIC_DATA_TYPE)
        {
            wdata->type_name = std::string(get_element_text(element));
        }
        else if (key == TOPIC_KIND)
        {
            std::string auxString = std::string(get_element_text(element));
            if (auxString == _NO_KEY)
            {
                wdata->topic_kind = NO_KEY;
                wdata->guid.entityId.value[3] = 0x03;
            }
            else if (auxString == _WITH_KEY)
            {
                wdata->topic_kind = WITH_KEY;
                wdata->guid.entityId.value[3] = 0x02;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, topic of kind: " << auxString << " is not valid");
                delete(wdata);
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (key == RELIABILITY_QOS)
        {
            std::string auxString = std::string(get_element_text(element));
            if (auxString == _RELIABLE_RELIABILITY_QOS)
            {
                wdata->reliability.kind = dds::RELIABLE_RELIABILITY_QOS;
            }
            else if (auxString == _BEST_EFFORT_RELIABILITY_QOS)
            {
                wdata->reliability.kind = dds::BEST_EFFORT_RELIABILITY_QOS;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, endpoint of stateKind: " << auxString << " is not valid");
                delete(wdata);
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (key == UNICAST_LOCATOR)
        {
            // Empty but necessary to avoid warning on last else
        }
        else if (key == MULTICAST_LOCATOR)
        {
            // Empty but necessary to avoid warning on last else
        }
        else if (key == DURABILITY_QOS)
        {
            std::string auxstring = std::string(get_element_text(element));
            if (auxstring == _PERSISTENT_DURABILITY_QOS)
            {
                wdata->durability.kind = dds::PERSISTENT_DURABILITY_QOS;
            }
            else if (auxstring == _TRANSIENT_DURABILITY_QOS)
            {
                wdata->durability.kind = dds::TRANSIENT_DURABILITY_QOS;
            }
            else if (auxstring == _TRANSIENT_LOCAL_DURABILITY_QOS)
            {
                wdata->durability.kind = dds::TRANSIENT_LOCAL_DURABILITY_QOS;
            }
            else if (auxstring == _VOLATILE_DURABILITY_QOS)
            {
                wdata->durability.kind = dds::VOLATILE_DURABILITY_QOS;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, durability of kind: " << auxstring << " is not valid");
                delete(wdata);
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (key == OWNERSHIP_QOS)
        {
            const char* kind = element->Attribute(KIND);
            std::string auxstring(kind ? kind : OWNERSHIP_KIND_NOT_PRESENT);
            if (auxstring == _SHARED_OWNERSHIP_QOS)
            {
                wdata->ownership.kind = dds::SHARED_OWNERSHIP_QOS;
            }
            else if (auxstring == _EXCLUSIVE_OWNERSHIP_QOS)
            {
                wdata->ownership.kind = dds::EXCLUSIVE_OWNERSHIP_QOS;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, ownership of kind: " << auxstring << " is not valid");
                delete(wdata);
                return XMLP_ret::XML_ERROR;
            }
            int strength = 0;
            element->QueryIntAttribute(STRENGTH, &strength);
            wdata->ownership_strength.value = strength;
        }
        else if (key == PARTITION_QOS)
        {
            wdata->partition.push_back(get_element_text(element).c_str());
        }
        else if (key == LIVELINESS_QOS)
        {
            const char* kind = element->Attribute(KIND);
            std::string auxstring(kind ? kind : LIVELINESS_KIND_NOT_PRESENT);
            if (auxstring == _AUTOMATIC_LIVELINESS_QOS)
            {
                wdata->liveliness.kind = dds::AUTOMATIC_LIVELINESS_QOS;
            }
            else if (auxstring == _MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            {
                wdata->liveliness.kind = dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
            }
            else if (auxstring == _MANUAL_BY_TOPIC_LIVELINESS_QOS)
            {
                wdata->liveliness.kind = dds::MANUAL_BY_TOPIC_LIVELINESS_QOS;
            }
            else
            {
                EPROSIMA_LOG_ERROR(RTPS_EDP, "Bad XML file, liveliness of kind: " << auxstring << " is not valid");
                delete(wdata);
                return XMLP_ret::XML_ERROR;
            }
            const char* leaseDuration_ms = element->Attribute(LEASE_DURATION_MS);
            auxstring = std::string(leaseDuration_ms ? leaseDuration_ms : _INF);
            if (auxstring == _INF)
            {
                wdata->liveliness.lease_duration = dds::c_TimeInfinite;
            }
            else
            {
                uint32_t milliseclease = std::strtoul(auxstring.c_str(), nullptr, 10);
                wdata->liveliness.lease_duration =
                        fastdds::rtps::TimeConv::MilliSeconds2Time_t((double)milliseclease).to_duration_t();
                if (milliseclease == 0)
                {
                    EPROSIMA_LOG_WARNING(RTPS_EDP, "BAD XML:livelinessQos leaseDuration is 0");
                }
            }
        }
        else if (key == DISABLE_POSITIVE_ACKS)
        {
            // Disable positive acks
            if (XMLP_ret::XML_OK !=
                    XMLParser::getXMLDisablePositiveAcksQos(element, wdata->disable_positive_acks, 0))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_WARNING(RTPS_EDP, "Unkown Endpoint-XML tag, ignoring " << key);
        }

        element = element->NextSiblingElement();
    }

    if (wdata->user_defined_id() == 0)
    {
        EPROSIMA_LOG_ERROR(RTPS_EDP, "Writer XML endpoint with NO ID defined");
        delete(wdata);
        return XMLP_ret::XML_ERROR;
    }

    for (const Locator_t& loc : unicast_locators)
    {
        wdata->add_unicast_locator(loc);
    }

    for (const Locator_t& loc : multicast_locators)
    {
        wdata->add_multicast_locator(loc);
    }

    pdata->m_writers.push_back(wdata);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLEndpointParser::lookforReader(
        const char* partname,
        uint16_t id,
        ReaderProxyData** rdataptr)
{
    for (std::vector<StaticRTPSParticipantInfo*>::iterator pit = m_RTPSParticipants.begin();
            pit != m_RTPSParticipants.end(); ++pit)
    {
        if ((*pit)->m_RTPSParticipantName == partname || true) //it doenst matter the name fo the RTPSParticipant, only for organizational purposes
        {
            for (std::vector<ReaderProxyData*>::iterator rit = (*pit)->m_readers.begin();
                    rit != (*pit)->m_readers.end(); ++rit)
            {
                if ((*rit)->user_defined_id() == id)
                {
                    *rdataptr = *rit;
                    return XMLP_ret::XML_OK;
                }
            }
        }
    }
    return XMLP_ret::XML_ERROR;
}

XMLP_ret XMLEndpointParser::lookforWriter(
        const char* partname,
        uint16_t id,
        WriterProxyData** wdataptr)
{
    for (std::vector<StaticRTPSParticipantInfo*>::iterator pit = m_RTPSParticipants.begin();
            pit != m_RTPSParticipants.end(); ++pit)
    {
        if ((*pit)->m_RTPSParticipantName == partname || true) //it doenst matter the name fo the RTPSParticipant, only for organizational purposes
        {
            for (std::vector<WriterProxyData*>::iterator wit = (*pit)->m_writers.begin();
                    wit != (*pit)->m_writers.end(); ++wit)
            {
                if ((*wit)->user_defined_id() == id)
                {
                    *wdataptr = *wit;
                    return XMLP_ret::XML_OK;
                }
            }
        }
    }
    return XMLP_ret::XML_ERROR;
}

}  // namespace xmlparser
}  // namespace fastdds
}  // namespace eprosima
