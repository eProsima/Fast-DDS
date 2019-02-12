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
#include <fastrtps/xmlparser/XMLParser.h>
#include <fastrtps/xmlparser/XMLParserCommon.h>
#include <fastrtps/xmlparser/XMLTree.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastrtps/transport/UDPv4TransportDescriptor.h>
#include <fastrtps/transport/UDPv6TransportDescriptor.h>
#include <fastrtps/transport/TCPv4TransportDescriptor.h>
#include <fastrtps/transport/TCPv6TransportDescriptor.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeMember.h>

#include <fastrtps/log/StdoutConsumer.h>
#include <fastrtps/log/FileConsumer.h>

#include <tinyxml2.h>
#include <iostream>
#include <cstdlib>

namespace eprosima {
namespace fastrtps {
namespace xmlparser {

XMLP_ret XMLParser::loadDefaultXMLFile(up_base_node_t& root)
{
    return loadXML(DEFAULT_FASTRTPS_PROFILES, root);
}

XMLP_ret XMLParser::parseXML(tinyxml2::XMLDocument& xmlDoc, up_base_node_t& root)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    tinyxml2::XMLElement* p_root = xmlDoc.FirstChildElement(ROOT);
    if (nullptr == p_root)
    {
        // Just profiles in the XML.
        if (nullptr == (p_root = xmlDoc.FirstChildElement(PROFILES)))
        {
            // Just types in the XML.
            if (nullptr == (p_root = xmlDoc.FirstChildElement(TYPES)))
            {
                // Just log config in the XML.
                if (nullptr == (p_root = xmlDoc.FirstChildElement(LOG)))
                {
                    logError(XMLPARSER, "Not found root tag");
                    ret = XMLP_ret::XML_ERROR;
                }
                else
                {
                    root.reset(new BaseNode{NodeType::LOG});
                    ret  = parseLogConfig(p_root);
                }
            }
            else
            {
                root.reset(new BaseNode{ NodeType::TYPES });
                ret = parseDynamicTypes(p_root);
            }
        }
        else
        {
            root.reset(new BaseNode{ NodeType::PROFILES });
            ret = parseProfiles(p_root, *root);
        }
    }
    else
    {
        root.reset(new BaseNode{ NodeType::ROOT });
        tinyxml2::XMLElement* node = p_root->FirstChildElement();
        const char* tag = nullptr;
        while ( (nullptr != node) && (ret == XMLP_ret::XML_OK))
        {
            if (nullptr != (tag = node->Value()))
            {
                if (strcmp(tag, PROFILES) == 0)
                {
                    up_base_node_t profiles_node = up_base_node_t{ new BaseNode{NodeType::PROFILES} };
                    if (XMLP_ret::XML_OK == (ret = parseProfiles(node, *profiles_node)))
                    {
                        root->addChild(std::move(profiles_node));
                    }
                }
                else if (strcmp(tag, PARTICIPANT) == 0)
                {
                    ret = parseXMLParticipantProf(node, *root);
                }
                else if (strcmp(tag, PUBLISHER) == 0 || strcmp(tag, DATA_WRITER) == 0)
                {
                    ret = parseXMLPublisherProf(node, *root);
                }
                else if (strcmp(tag, SUBSCRIBER) == 0 || strcmp(tag, DATA_READER) == 0)
                {
                    ret = parseXMLSubscriberProf(node, *root);
                }
                else if (strcmp(tag, TOPIC) == 0)
                {
                    ret = parseXMLTopicData(node, *root);
                }
                else if (strcmp(tag, TYPES) == 0)
                {
                    ret = parseXMLTypes(node);
                }
                else if (strcmp(tag, LOG) == 0)
                {
                    ret = parseLogConfig(node);
                }
                else
                {
                    logError(XMLPARSER, "Not expected tag: '" << tag << "'");
                    ret = XMLP_ret::XML_ERROR;
                }
            }

            node = node->NextSiblingElement();
        }
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLProfiles(tinyxml2::XMLElement& profiles, up_base_node_t& root)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    root.reset(new BaseNode{NodeType::PROFILES});
    ret  = parseProfiles(&profiles, *root);
    return ret;
}

XMLP_ret XMLParser::parseXMLDynamicTypes(tinyxml2::XMLElement& types)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    ret  = parseDynamicTypes(&types);
    return ret;
}

XMLP_ret XMLParser::parseRoot(tinyxml2::XMLElement* p_root, BaseNode& rootNode)
{
    XMLP_ret ret           = XMLP_ret::XML_OK;
    tinyxml2::XMLElement* root_child = nullptr;
    if (nullptr != (root_child = p_root->FirstChildElement(PROFILES)))
    {
        up_base_node_t profiles_node = up_base_node_t(new BaseNode{NodeType::PROFILES});
        if (XMLP_ret::XML_OK == (ret = parseProfiles(root_child, *profiles_node)))
        {
            rootNode.addChild(std::move(profiles_node));
        }
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLTransportsProf(tinyxml2::XMLElement* p_root)
{
    /*
        <xs:complexType name="TransportDescriptorListType">
            <xs:sequence>
                <xs:element name="transport_descriptor" type="rtpsTransportDescriptorType"/>
            </xs:sequence>
        </xs:complexType>
    */

    XMLP_ret ret = XMLP_ret::XML_OK;
    tinyxml2::XMLElement* p_element = p_root->FirstChildElement(TRANSPORT_DESCRIPTOR);
    while(p_element != nullptr)
    {
        ret = parseXMLTransportData(p_element);
        if (ret != XMLP_ret::XML_OK)
        {
            logError(XMLPARSER, "Error parsing transports");
            return ret;
        }
        p_element = p_element->NextSiblingElement(TRANSPORT_DESCRIPTOR);
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLTypes(tinyxml2::XMLElement* p_root)
{
    /*
        <xs:element name="types">
            <xs:complexType>
                <xs:group ref="moduleElems"/>
            </xs:complexType>
        </xs:element>
    */

    XMLP_ret ret = XMLP_ret::XML_OK;
    tinyxml2::XMLElement *p_aux0 = nullptr, *p_aux1 = nullptr;
    p_aux0 = p_root->FirstChildElement(TYPES);
    if (p_aux0 != nullptr)
    {
        const char* name = nullptr;
        for (p_aux1 = p_aux0->FirstChildElement(); p_aux1 != nullptr; p_aux1 = p_aux1->NextSiblingElement())
        {
            name = p_aux1->Name();
            if (strcmp(name, TYPE) == 0)
            {
                if (XMLP_ret::XML_OK != parseXMLDynamicType(p_aux1))
                    return XMLP_ret::XML_ERROR;
            }
            else
            {
                logError(XMLPARSER, "Invalid element found into 'types'. Name: " << name);
                return XMLP_ret::XML_ERROR;
            }
        }
    }
    else // Directly root is TYPES?
    {
        const char* name = nullptr;
        for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
        {
            name = p_aux0->Name();
            if (strcmp(name, TYPE) == 0)
            {
                if (XMLP_ret::XML_OK != parseXMLDynamicType(p_aux0))
                    return XMLP_ret::XML_ERROR;
            }
        }
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLTransportData(tinyxml2::XMLElement* p_root)
{
    /*
        <xs:complexType name="rtpsTransportDescriptorType">
            <xs:all minOccurs="0">
                <xs:element name="transport_id" type="stringType"/>
                <xs:element name="type" type="stringType"/>
                <xs:element name="sendBufferSize" type="int32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="receiveBufferSize" type="int32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="TTL" type="uint8Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="maxMessageSize" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="maxInitialPeersRange" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="interfaceWhiteList" type="stringListType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="wan_addr" type="stringType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="output_port" type="uint16Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="keep_alive_frequency_ms" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="keep_alive_timeout_ms" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="max_logical_port" type="uint16Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="logical_port_range" type="uint16Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="logical_port_increment" type="uint16Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="metadata_logical_port" type="uint16Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="listening_ports" type="portListType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="calculate_crc" type="boolType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="check_crc" type="boolType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="enable_tcp_nodelay" type="boolType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="tls" type="tlsConfigType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
    */

    XMLP_ret ret = XMLP_ret::XML_OK;
    std::string sId = "";
    sp_transport_t pDescriptor = nullptr;

    tinyxml2::XMLElement *p_aux0 = nullptr;
    p_aux0 = p_root->FirstChildElement(TRANSPORT_ID);
    if (nullptr == p_aux0)
    {
        logError(XMLPARSER, "Not found '" << TRANSPORT_ID << "' attribute");
        return XMLP_ret::XML_ERROR;
    }
    else
    {
        sId = p_aux0->GetText();
    }

    p_aux0 = p_root->FirstChildElement(TYPE);
    if (nullptr == p_aux0)
    {
        logError(XMLPARSER, "Not found '" << TYPE << "' attribute");
        return XMLP_ret::XML_ERROR;
    }
    else
    {
        std::string sType = p_aux0->GetText();
        if (sType == UDPv4)
        {
            pDescriptor = std::make_shared<rtps::UDPv4TransportDescriptor>();
            std::shared_ptr<rtps::UDPv4TransportDescriptor> pUDPv4Desc =
                std::dynamic_pointer_cast<rtps::UDPv4TransportDescriptor>(pDescriptor);
            // Output UDP Socket
            if (nullptr != (p_aux0 = p_root->FirstChildElement(UDP_OUTPUT_PORT)))
            {
                int iSocket = 0;
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iSocket, 0) || iSocket < 0 || iSocket > 65535)
                    return XMLP_ret::XML_ERROR;
                pUDPv4Desc->m_output_udp_socket = static_cast<uint16_t>(iSocket);
            }
        }
        else if (sType == UDPv6)
        {
            pDescriptor = std::make_shared<rtps::UDPv6TransportDescriptor>();

            std::shared_ptr<rtps::UDPv6TransportDescriptor> pUDPv6Desc =
                std::dynamic_pointer_cast<rtps::UDPv6TransportDescriptor>(pDescriptor);
            // Output UDP Socket
            if (nullptr != (p_aux0 = p_root->FirstChildElement(UDP_OUTPUT_PORT)))
            {
                int iSocket = 0;
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iSocket, 0) || iSocket < 0 || iSocket > 65535)
                    return XMLP_ret::XML_ERROR;
                pUDPv6Desc->m_output_udp_socket = static_cast<uint16_t>(iSocket);
            }
        }
        else if (sType == TCPv4)
        {
            pDescriptor = std::make_shared<rtps::TCPv4TransportDescriptor>();
            ret = parseXMLCommonTCPTransportData(p_root, pDescriptor);
            if (ret != XMLP_ret::XML_OK)
            {
                return ret;
            }
            else
            {
                std::shared_ptr<rtps::TCPv4TransportDescriptor> pTCPv4Desc =
                    std::dynamic_pointer_cast<rtps::TCPv4TransportDescriptor>(pDescriptor);

                // Wan Address
                if (nullptr != (p_aux0 = p_root->FirstChildElement(TCP_WAN_ADDR)))
                {
                    std::string s;
                    if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, 0))
                        return XMLP_ret::XML_ERROR;
                    pTCPv4Desc->set_WAN_address(s);
                }
            }
        }
        else if (sType == TCPv6)
        {
            pDescriptor = std::make_shared<rtps::TCPv6TransportDescriptor>();
            ret = parseXMLCommonTCPTransportData(p_root, pDescriptor);
            if (ret != XMLP_ret::XML_OK)
            {
                return ret;
            }
        }
        else
        {
            logError(XMLPARSER, "Invalid transport type: '" << sType << "'");
        }

        ret = parseXMLCommonTransportData(p_root, pDescriptor);
        if (ret != XMLP_ret::XML_OK)
        {
            return ret;
        }

        XMLProfileManager::insertTransportById(sId, pDescriptor);
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLCommonTransportData(tinyxml2::XMLElement* p_root, sp_transport_t p_transport)
{
    /*
        <xs:complexType name="rtpsTransportDescriptorType">
            <xs:all minOccurs="0">
                <xs:element name="transport_id" type="stringType"/>
                <xs:element name="type" type="stringType"/>
                <xs:element name="sendBufferSize" type="int32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="receiveBufferSize" type="int32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="TTL" type="uint8Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="maxMessageSize" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="maxInitialPeersRange" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="interfaceWhiteList" type="stringListType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="wan_addr" type="stringType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="output_port" type="uint16Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="keep_alive_frequency_ms" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="keep_alive_timeout_ms" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="max_logical_port" type="uint16Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="logical_port_range" type="uint16Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="logical_port_increment" type="uint16Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="metadata_logical_port" type="uint16Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="listening_ports" type="portListType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
    */

    std::shared_ptr<rtps::SocketTransportDescriptor> pDesc = std::dynamic_pointer_cast<rtps::SocketTransportDescriptor>(p_transport);

    tinyxml2::XMLElement *p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = p_root->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, SEND_BUFFER_SIZE) == 0)
        {
            // sendBufferSize - int32Type
            int iSize = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iSize, 0) || iSize < 0)
                return XMLP_ret::XML_ERROR;
            pDesc->sendBufferSize = iSize;
        }
        else if (strcmp(name, RECEIVE_BUFFER_SIZE) == 0)
        {
            // receiveBufferSize - int32Type
            int iSize = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iSize, 0) || iSize < 0)
                return XMLP_ret::XML_ERROR;
            pDesc->receiveBufferSize = iSize;
        }
        else if (strcmp(name, TTL) == 0)
        {
            // TTL - int8Type
            int iTTL = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iTTL, 0) || iTTL < 0 || iTTL > 255)
                return XMLP_ret::XML_ERROR;
            pDesc->TTL = static_cast<uint8_t>(iTTL);
        }
        else if (strcmp(name, MAX_MESSAGE_SIZE) == 0)
        {
            // maxMessageSize - uint32Type
            uint32_t uSize = 0;
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &uSize, 0))
                return XMLP_ret::XML_ERROR;
            pDesc->maxMessageSize = uSize;
        }
        else if (strcmp(name, MAX_INITIAL_PEERS_RANGE) == 0)
        {
            // maxInitialPeersRange - uint32Type
            uint32_t uRange = 0;
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &uRange, 0))
                return XMLP_ret::XML_ERROR;
            pDesc->maxInitialPeersRange = uRange;
        }
        else if (strcmp(name, WHITE_LIST) == 0)
        {
            // InterfaceWhiteList stringListType
            tinyxml2::XMLElement* p_aux1 = p_aux0->FirstChildElement(ADDRESS);
            while (nullptr != p_aux1)
            {
                const char* text = p_aux1->GetText();
                if (nullptr != text)
                {
                    pDesc->interfaceWhiteList.emplace_back(text);
                }
                p_aux1 = p_aux1->NextSiblingElement(ADDRESS);
            }
        }
        else if (strcmp(name, TCP_WAN_ADDR) == 0 || strcmp(name, UDP_OUTPUT_PORT) == 0 ||
            strcmp(name, TRANSPORT_ID) == 0 || strcmp(name, TYPE) == 0 ||
            strcmp(name, KEEP_ALIVE_FREQUENCY) == 0 || strcmp(name, KEEP_ALIVE_TIMEOUT) == 0 ||
            strcmp(name, MAX_LOGICAL_PORT) == 0 || strcmp(name, LOGICAL_PORT_RANGE) == 0 ||
            strcmp(name, LOGICAL_PORT_INCREMENT) == 0 || strcmp(name, LISTENING_PORTS) == 0 ||
            strcmp(name, CALCULATE_CRC) == 0 || strcmp(name, CHECK_CRC) == 0 ||
            strcmp(name, ENABLE_TCP_NODELAY) == 0 || strcmp(name, TLS) == 0)
        {
            // Parsed outside of this method
        }
        else
        {
            logError(XMLPARSER, "Invalid element found into 'rtpsTransportDescriptorType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::parseXMLCommonTCPTransportData(tinyxml2::XMLElement* p_root, sp_transport_t p_transport)
{
    /*
        <xs:complexType name="rtpsTransportDescriptorType">
            <xs:all minOccurs="0">
                <xs:element name="keep_alive_frequency_ms" type="uint32Type"/>
                <xs:element name="keep_alive_timeout_ms" type="uint32Type"/>
                <xs:element name="max_logical_port" type="uint16Type"/>
                <xs:element name="logical_port_range" type="uint16Type"/>
                <xs:element name="logical_port_increment" type="uint16Type"/>
                <xs:element name="metadata_logical_port" type="uint16Type"/>
                <xs:element name="listening_ports" type="uint16ListType"/>
                <xs:sequence>
                    <xs:element name="port" type="uint16Type"/>
                </xs:sequence>
                <xs:element name="calculate_crc" type="boolType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="check_crc" type="boolType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="enable_tcp_nodelay" type="boolType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="tls" type="tlsConfigType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
    */

    XMLP_ret ret = XMLP_ret::XML_OK;
    std::shared_ptr<rtps::TCPTransportDescriptor> pTCPDesc =
        std::dynamic_pointer_cast<rtps::TCPTransportDescriptor>(p_transport);
    if (pTCPDesc != nullptr)
    {
        tinyxml2::XMLElement *p_aux0 = nullptr;
        const char* name = nullptr;
        for (p_aux0 = p_root->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
        {
            name = p_aux0->Name();
            if (strcmp(name, KEEP_ALIVE_FREQUENCY) == 0)
            {
                // keep_alive_frequency_ms - uint32Type
                int iFrequency(0);
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iFrequency, 0))
                    return XMLP_ret::XML_ERROR;
                pTCPDesc->keep_alive_frequency_ms = static_cast<uint32_t>(iFrequency);
            }
            else if (strcmp(name, KEEP_ALIVE_TIMEOUT) == 0)
            {
                // keep_alive_timeout_ms - uint32Type
                int iTimeout(0);
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iTimeout, 0))
                    return XMLP_ret::XML_ERROR;
                pTCPDesc->keep_alive_timeout_ms = static_cast<uint32_t>(iTimeout);
            }
            else if (strcmp(name, MAX_LOGICAL_PORT) == 0)
            {
                // max_logical_port - uint16Type
                int iPort(0);
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iPort, 0) || iPort < 0 || iPort > 65535)
                    return XMLP_ret::XML_ERROR;
                pTCPDesc->max_logical_port = static_cast<uint16_t>(iPort);
            }
            else if (strcmp(name, LOGICAL_PORT_RANGE) == 0)
            {
                // logical_port_range - uint16Type
                int iPort(0);
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iPort, 0) || iPort < 0 || iPort > 65535)
                    return XMLP_ret::XML_ERROR;
                pTCPDesc->logical_port_range = static_cast<uint16_t>(iPort);
            }
            else if (strcmp(name, LOGICAL_PORT_INCREMENT) == 0)
            {
                // logical_port_increment - uint16Type
                int iPort(0);
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iPort, 0) || iPort < 0 || iPort > 65535)
                    return XMLP_ret::XML_ERROR;
                pTCPDesc->logical_port_increment = static_cast<uint16_t>(iPort);
            }
            // enable_tcp_nodelay - boolType
            else if (strcmp(name, ENABLE_TCP_NODELAY) == 0)
            {
                if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &pTCPDesc->enable_tcp_nodelay, 0))
                {
                    return XMLP_ret::XML_ERROR;
                }
            }
            else if (strcmp(name, LISTENING_PORTS) == 0)
            {
                // listening_ports uint16ListType
                tinyxml2::XMLElement* p_aux1 = p_aux0->FirstChildElement(PORT);
                while (nullptr != p_aux1)
                {
                    int iPort = 0;
                    if (XMLP_ret::XML_OK != getXMLInt(p_aux1, &iPort, 0) || iPort < 0 || iPort > 65535)
                        return XMLP_ret::XML_ERROR;

                    pTCPDesc->add_listener_port(static_cast<uint16_t>(iPort));
                    p_aux1 = p_aux1->NextSiblingElement(PORT);
                }
            }
            else if (strcmp(name, CALCULATE_CRC) == 0)
            {
                if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &pTCPDesc->calculate_crc, 0))
                {
                    return XMLP_ret::XML_ERROR;
                }
            }
            else if (strcmp(name, CHECK_CRC) == 0)
            {
                if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &pTCPDesc->check_crc, 0))
                {
                    return XMLP_ret::XML_ERROR;
                }
            }
            else if (strcmp(name, TLS) == 0)
            {
                if (XMLP_ret::XML_OK != parse_tls_config(p_aux0, p_transport))
                {
                    return XMLP_ret::XML_ERROR;
                }
            }
            else if (strcmp(name, TCP_WAN_ADDR) == 0 || strcmp(name, TRANSPORT_ID) == 0 ||
                strcmp(name, TYPE) == 0 || strcmp(name, SEND_BUFFER_SIZE) == 0 ||
                strcmp(name, RECEIVE_BUFFER_SIZE) == 0 || strcmp(name, TTL) == 0 ||
                strcmp(name, MAX_MESSAGE_SIZE) == 0 || strcmp(name, MAX_INITIAL_PEERS_RANGE) == 0 ||
                strcmp(name, WHITE_LIST) == 0)
            {
                // Parsed Outside of this method
            }
            else
            {
                logError(XMLPARSER, "Invalid element found into 'rtpsTransportDescriptorType'. Name: " << name);
                return XMLP_ret::XML_ERROR;
            }
        }
    }
    else
    {
        logError(XMLPARSER, "Error parsing TCP Transport data");
        ret = XMLP_ret::XML_ERROR;
    }

    return ret;
}

XMLP_ret XMLParser::parse_tls_config(
    tinyxml2::XMLElement* p_root,
    sp_transport_t tcp_transport)
{
    /*
        XSD:
        <xs:simpleType name="tlsOptionsType">
            <xs:restriction base="xs:string">
                <xs:enumeration value="DEFAULT_WORKAROUNDS"/>
                <xs:enumeration value="NO_COMPRESSION"/>
                <xs:enumeration value="NO_SSLV2"/>
                <xs:enumeration value="NO_SSLV3"/>
                <xs:enumeration value="NO_TLSV1"/>
                <xs:enumeration value="NO_TLSV1_1"/>
                <xs:enumeration value="NO_TLSV1_2"/>
                <xs:enumeration value="NO_TLSV1_3"/>
                <xs:enumeration value="SINGLE_DH_USE"/>
            </xs:restriction>
        </xs:simpleType>

        <xs:complexType name="tlsOptionsVectorType">
            <xs:sequence>
                <xs:element name="option" type="tlsOptionsType" minOccurs="0" maxOccurs="unbounded"/>
            </xs:sequence>
        </xs:complexType>

        <xs:simpleType name="tlsVerifyModeType">
            <xs:restriction base="xs:string">
                <xs:enumeration value="VERIFY_NONE"/>
                <xs:enumeration value="VERIFY_PEER"/>
                <xs:enumeration value="VERIFY_FAIL_IF_NO_PEER_CERT"/>
                <xs:enumeration value="VERIFY_CLIENT_ONCE"/>
            </xs:restriction>
        </xs:simpleType>

        <xs:complexType name="tlsVerifyModeVectorType">
            <xs:sequence>
                <xs:element name="verify" type="tlsVerifyModeType" minOccurs="0" maxOccurs="unbounded"/>
            </xs:sequence>
        </xs:complexType>

        <xs:complexType name="tlsConfigType">
            <xs:all minOccurs="0">
                <xs:element name="password" type="stringType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="options" type="tlsOptionsVectorType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="cert_chain_file" type="stringType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="private_key_file" type="stringType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="tmp_dh_file" type="stringType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="verify_file" type="stringType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="verify_mode" type="tlsVerifyModeVectorType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="verify_paths" type="tlsVerifyPathVectorType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="default_verify_path" type="xs:boolean" minOccurs="0" maxOccurs="1"/>
                <xs:element name="verify_depth" type="xs:int" minOccurs="0" maxOccurs="1"/>
                <xs:element name="rsa_private_key_file" type="stringType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>

        XML Example:
        <tls>
            <password>Contraseña</password>
            <private_key_file>Key_file.pem</private_key_file>
            <cert_chain_file>Chain.pem</cert_chain_file>
            <tmp_dh_file>DH.pem</tmp_dh_file>
            <verify_file>verify.pem</verify_file>
            <verify_mode>
                <verify>VERIFY_PEER</verify>
            </verify_mode>
            <options>
                <option>NO_TLSV1</option>
                <option>NO_TLSV1_1</option>
            </options>
        </tls>
   */
    using TCPDescriptor = std::shared_ptr<rtps::TCPTransportDescriptor>;
    using TLSVerifyMode = TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
    using TLSOption = TCPTransportDescriptor::TLSConfig::TLSOptions;
    using TLSHandShakeMode = TCPTransportDescriptor::TLSConfig::TLSHandShakeRole;

    XMLP_ret ret = XMLP_ret::XML_OK;

    TCPDescriptor pTCPDesc = std::dynamic_pointer_cast<rtps::TCPTransportDescriptor>(tcp_transport);
    pTCPDesc->apply_security = true;

    tinyxml2::XMLElement *p_aux0 = nullptr;

    for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        const std::string config = p_aux0->Value();
        if (config.compare(TLS_PASSWORD) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &pTCPDesc->tls_config.password, 0))
            {
                ret = XMLP_ret::XML_ERROR;
            }
        }
        else if (config.compare(TLS_PRIVATE_KEY_FILE) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &pTCPDesc->tls_config.private_key_file, 0))
            {
                ret = XMLP_ret::XML_ERROR;
            }
        }
        else if (config.compare(TLS_CERT_CHAIN_FILE) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &pTCPDesc->tls_config.cert_chain_file, 0))
            {
                ret = XMLP_ret::XML_ERROR;
            }
        }
        else if (config.compare(TLS_TMP_DH_FILE) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &pTCPDesc->tls_config.tmp_dh_file, 0))
            {
                ret = XMLP_ret::XML_ERROR;
            }
        }
        else if (config.compare(TLS_VERIFY_FILE) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &pTCPDesc->tls_config.verify_file, 0))
            {
                ret = XMLP_ret::XML_ERROR;
            }
        }
        else if (config.compare(TLS_VERIFY_PATHS) == 0)
        {
            tinyxml2::XMLElement *p_path = p_aux0->FirstChildElement();

            while (p_path != nullptr)
            {
                std::string type = p_path->Value();
                if (type.compare(TLS_VERIFY_PATH) == 0)
                {
                    std::string path;

                    if (XMLP_ret::XML_OK != getXMLString(p_path, &path, 0))
                    {
                        ret = XMLP_ret::XML_ERROR;
                    }
                    else
                    {
                        pTCPDesc->tls_config.verify_paths.push_back(path);
                    }

                    if (ret == XMLP_ret::XML_ERROR)
                    {
                        // Break while loop
                        break;
                    }
                    p_path = p_path->NextSiblingElement();
                }
                else
                {
                    logError(XMLPARSER, "Unrecognized verify paths label: " << p_path->Value());
                    ret = XMLP_ret::XML_ERROR;
                    break;
                }
            }
        }
        else if (config.compare(TLS_VERIFY_DEPTH) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &pTCPDesc->tls_config.verify_depth, 0))
            {
                ret = XMLP_ret::XML_ERROR;
            }
        }
        else if (config.compare(TLS_DEFAULT_VERIFY_PATH) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &pTCPDesc->tls_config.default_verify_path, 0))
            {
                ret = XMLP_ret::XML_ERROR;
            }
        }
        else if (config.compare(TLS_RSA_PRIVATE_KEY_FILE) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &pTCPDesc->tls_config.rsa_private_key_file, 0))
            {
                ret = XMLP_ret::XML_ERROR;
            }
        }
        else if (config.compare(TLS_HANDSHAKE_ROLE) == 0)
        {
            std::string handshake_mode;
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &handshake_mode, 0))
            {
                ret = XMLP_ret::XML_ERROR;
            }
            else
            {
                if (handshake_mode.compare(TLS_HANDSHAKE_ROLE_DEFAULT) == 0)
                {
                    pTCPDesc->tls_config.handshake_role = TLSHandShakeMode::DEFAULT;
                }
                else if (handshake_mode.compare(TLS_HANDSHAKE_ROLE_SERVER) == 0)
                {
                    pTCPDesc->tls_config.handshake_role = TLSHandShakeMode::SERVER;
                }
                else if (handshake_mode.compare(TLS_HANDSHAKE_ROLE_CLIENT) == 0)
                {
                    pTCPDesc->tls_config.handshake_role = TLSHandShakeMode::CLIENT;
                }
                else
                {
                    logError(XMLPARSER, "Error parsing TLS configuration handshake_mode unrecognized "
                        << handshake_mode << ".");
                    ret = XMLP_ret::XML_ERROR;
                }
            }
        }
        else if (config.compare(TLS_VERIFY_MODE) == 0)
        {
            tinyxml2::XMLElement *p_verify = p_aux0->FirstChildElement();
            while (p_verify != nullptr)
            {
                std::string type = p_verify->Value();
                if (type.compare(TLS_VERIFY) == 0)
                {
                    std::string verify_mode;

                    if (XMLP_ret::XML_OK != getXMLString(p_verify, &verify_mode, 0))
                    {
                        ret = XMLP_ret::XML_ERROR;
                    }
                    else
                    {
                        if (verify_mode.compare(TLS_VERIFY_NONE) == 0)
                        {
                            pTCPDesc->tls_config.add_verify_mode(TLSVerifyMode::VERIFY_NONE);
                        }
                        else if (verify_mode.compare(TLS_VERIFY_PEER) == 0)
                        {
                            pTCPDesc->tls_config.add_verify_mode(TLSVerifyMode::VERIFY_PEER);
                        }
                        else if (verify_mode.compare(TLS_VERIFY_FAIL_IF_NO_PEER_CERT) == 0)
                        {
                            pTCPDesc->tls_config.add_verify_mode(TLSVerifyMode::VERIFY_FAIL_IF_NO_PEER_CERT);
                        }
                        else if (verify_mode.compare(TLS_VERIFY_CLIENT_ONCE) == 0)
                        {
                            pTCPDesc->tls_config.add_verify_mode(TLSVerifyMode::VERIFY_CLIENT_ONCE);
                        }
                        else
                        {
                            logError(XMLPARSER, "Error parsing TLS configuration verify_mode unrecognized "
                                << verify_mode << ".");
                            ret = XMLP_ret::XML_ERROR;
                        }
                    }
                }
                else
                {
                    logError(XMLPARSER, "Error parsing TLS configuration found unrecognized node "
                        << type << ".");
                    ret = XMLP_ret::XML_ERROR;
                }

                if (ret == XMLP_ret::XML_ERROR)
                {
                    // Break while loop
                    break;
                }

                p_verify = p_verify->NextSiblingElement();
            }
        }
        else if (config.compare(TLS_OPTIONS) == 0)
        {
            tinyxml2::XMLElement *p_option = p_aux0->FirstChildElement();
            while (p_option != nullptr)
            {
                std::string type = p_option->Value();
                if (type.compare(TLS_OPTION) == 0)
                {
                    std::string option;

                    if (XMLP_ret::XML_OK != getXMLString(p_option, &option, 0))
                    {
                        ret = XMLP_ret::XML_ERROR;
                    }
                    else
                    {
                        if (option.compare(TLS_DEFAULT_WORKAROUNDS) == 0)
                        {
                            pTCPDesc->tls_config.add_option(TLSOption::DEFAULT_WORKAROUNDS);
                        }
                        else if (option.compare(TLS_NO_COMPRESSION) == 0)
                        {
                            pTCPDesc->tls_config.add_option(TLSOption::NO_COMPRESSION);
                        }
                        else if (option.compare(TLS_NO_SSLV2) == 0)
                        {
                            pTCPDesc->tls_config.add_option(TLSOption::NO_SSLV2);
                        }
                        else if (option.compare(TLS_NO_SSLV3) == 0)
                        {
                            pTCPDesc->tls_config.add_option(TLSOption::NO_SSLV3);
                        }
                        else if (option.compare(TLS_NO_TLSV1) == 0)
                        {
                            pTCPDesc->tls_config.add_option(TLSOption::NO_TLSV1);
                        }
                        else if (option.compare(TLS_NO_TLSV1_1) == 0)
                        {
                            pTCPDesc->tls_config.add_option(TLSOption::NO_TLSV1_1);
                        }
                        else if (option.compare(TLS_NO_TLSV1_2) == 0)
                        {
                            pTCPDesc->tls_config.add_option(TLSOption::NO_TLSV1_2);
                        }
                        else if (option.compare(TLS_NO_TLSV1_3) == 0)
                        {
                            pTCPDesc->tls_config.add_option(TLSOption::NO_TLSV1_3);
                        }
                        else if (option.compare(TLS_SINGLE_DH_USE) == 0)
                        {
                            pTCPDesc->tls_config.add_option(TLSOption::SINGLE_DH_USE);
                        }
                        else
                        {
                            logError(XMLPARSER, "Error parsing TLS configuration option unrecognized "
                                << option << ".");
                            ret = XMLP_ret::XML_ERROR;
                        }
                    }
                }
                else
                {
                    logError(XMLPARSER, "Error parsing TLS options found unrecognized node "
                        << type << ".");
                    ret = XMLP_ret::XML_ERROR;
                }


                if (ret == XMLP_ret::XML_ERROR)
                {
                    // Break while loop
                    break;
                }

                p_option = p_option->NextSiblingElement();
            }
        }
        else
        {
            logError(XMLPARSER, "Error parsing TLS configuration: Field " << config << " not recognized.");
            ret = XMLP_ret::XML_ERROR;
        }

        // Stop parsing on error
        if (ret == XMLP_ret::XML_ERROR)
        {
            logError(XMLPARSER, "Error parsing TLS configuration's field '" << config << "'.");
            break;
        }
    }

    return ret;
}


XMLP_ret XMLParser::parseXMLDynamicType(tinyxml2::XMLElement* p_root)
{
    /*
        <xs:group name="moduleElems">
            <xs:sequence>
                <xs:choice maxOccurs="unbounded">
                    <xs:element name="struct" type="structDcl" minOccurs="0"/>
                    <xs:element name="union" type="unionDcl" minOccurs="0"/>
                    <xs:element name="enum" type="enumDcl" minOccurs="0"/>
                    <xs:element name="typedef" type="typedefDcl" minOccurs="0"/>
                </xs:choice>
            </xs:sequence>
        </xs:group>
    */
    XMLP_ret ret = XMLP_ret::XML_OK;
    tinyxml2::XMLElement *p_aux0 = nullptr;
    for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        const std::string type = p_aux0->Value();
        if (type.compare(STRUCT) == 0)
        {
            ret = parseXMLStructDynamicType(p_aux0);
        }
        else if (type.compare(UNION) == 0)
        {
            ret = parseXMLUnionDynamicType(p_aux0);
        }
        else if (type.compare(ENUM) == 0)
        {
            ret = parseXMLEnumDynamicType(p_aux0);
        }
        else if (type.compare(TYPEDEF) == 0)
        {
            ret = parseXMLAliasDynamicType(p_aux0);
        }
        else
        {
            logError(XMLPARSER, "Error parsing type: Type " << type << " not recognized.");
            ret = XMLP_ret::XML_ERROR;
        }

        if (ret != XMLP_ret::XML_OK)
        {
            logError(XMLPARSER, "Error parsing type " << type << ".");
            break;
        }
    }
    return ret;
}

static p_dynamictypebuilder_t getDiscriminatorTypeBuilder(const std::string &disc, uint32_t bound = 0);

static p_dynamictypebuilder_t getDiscriminatorTypeBuilder(const std::string &disc, uint32_t bound)
{
    /*
    mKind == TK_BOOLEAN || mKind == TK_BYTE || mKind == TK_INT16 || mKind == TK_INT32 ||
        mKind == TK_INT64 || mKind == TK_UINT16 || mKind == TK_UINT32 || mKind == TK_UINT64 ||
        mKind == TK_FLOAT32 || mKind == TK_FLOAT64 || mKind == TK_FLOAT128 || mKind == TK_CHAR8 ||
        mKind == TK_CHAR16 || mKind == TK_STRING8 || mKind == TK_STRING16 || mKind == TK_ENUM || mKind == TK_BITMASK
    */
    types::DynamicTypeBuilderFactory* factory = types::DynamicTypeBuilderFactory::GetInstance();
    if (disc.compare(BOOLEAN) == 0)
    {
        return factory->CreateBoolBuilder();
    }
    else if (disc.compare(TBYTE) == 0)
    {
        return factory->CreateByteBuilder();
    }
    else if (disc.compare(SHORT) == 0)
    {
        return factory->CreateInt16Builder();
    }
    else if (disc.compare(LONG) == 0)
    {
        return factory->CreateInt32Builder();
    }
    else if (disc.compare(LONGLONG) == 0)
    {
        return factory->CreateInt64Builder();
    }
    else if (disc.compare(USHORT) == 0)
    {
        return factory->CreateUint16Builder();
    }
    else if (disc.compare(ULONG) == 0)
    {
        return factory->CreateUint32Builder();
    }
    else if (disc.compare(ULONGLONG) == 0)
    {
        return factory->CreateUint64Builder();
    }
    else if (disc.compare(FLOAT) == 0)
    {
        return factory->CreateFloat32Builder();
    }
    else if (disc.compare(DOUBLE) == 0)
    {
        return factory->CreateFloat64Builder();
    }
    else if (disc.compare(LONGDOUBLE) == 0)
    {
        return factory->CreateFloat128Builder();
    }
    else if (disc.compare(CHAR) == 0)
    {
        return factory->CreateChar8Builder();
    }
    else if (disc.compare(WCHAR) == 0)
    {
        return factory->CreateChar16Builder();
    }
    else if (disc.compare(STRING) == 0)
    {
        return factory->CreateStringBuilder(bound);
    }
    else if (disc.compare(WSTRING) == 0)
    {
        return factory->CreateWstringBuilder(bound);
    }

    return XMLProfileManager::getDynamicTypeByName(disc);
}

XMLP_ret XMLParser::parseXMLAliasDynamicType(tinyxml2::XMLElement* p_root)
{
    /*
        <typedef name="MyAliasEnum" type="nonBasic" nonBasicTypeName="MyEnum"/>

        <typedef name="MyArray" type="int32" arrayDimensions="2,2"/>

        <xs:complexType name="typedefDcl">
            <xs:attribute name="name" type="identifierName" use="required"/>
            <xs:attribute name="type" type="string" use="required"/>
            <xs:attribute name="key_type" type="string" use="optional"/>
            <xs:attribute name="arrayDimensions" type="string" use="optional"/>
            <xs:attribute name="nonBasicTypeName" type="string" use="optional"/>
            <xs:attribute name="sequenceMaxLength" type="string" use="optional"/>
            <xs:attribute name="mapMaxLength" type="string" use="optional"/>
        </xs:complexType>
    */
    XMLP_ret ret = XMLP_ret::XML_OK;

    const char* type = p_root->Attribute(TYPE);
    if (type != nullptr)
    {
        if (strcmp(type, NON_BASIC_TYPE) == 0)
        {
            const char* typeNonBasicName = p_root->Attribute(NON_BASIC_TYPE_NAME);
            if (typeNonBasicName != nullptr)
            {
                type = typeNonBasicName;
            }
            else
            {
                logError(XMLPARSER, "Error parsing member type: Not found.");
                ret = XMLP_ret::XML_ERROR;
            }
        }

        p_dynamictypebuilder_t valueBuilder;
        if ((p_root->Attribute(ARRAY_DIMENSIONS) != nullptr) ||
            (p_root->Attribute(SEQ_MAXLENGTH) != nullptr) ||
            (p_root->Attribute(MAP_MAXLENGTH) != nullptr))
        {
            valueBuilder = parseXMLMemberDynamicType(p_root , nullptr, MEMBER_ID_INVALID);
        }
        else
        {
            uint32_t bound = 0;
            const char* boundStr = p_root->Attribute(STR_MAXLENGTH);
            if (boundStr != nullptr)
            {
                bound = std::atoi(boundStr);
            }
            valueBuilder = getDiscriminatorTypeBuilder(type, bound);
        }

        if (valueBuilder != nullptr)
        {
            const char* name = p_root->Attribute(NAME);
            p_dynamictypebuilder_t typeBuilder =
                types::DynamicTypeBuilderFactory::GetInstance()->CreateAliasBuilder(valueBuilder, name);
            XMLProfileManager::insertDynamicTypeByName(name, typeBuilder);
        }
        else
        {
            logError(XMLPARSER, "Error parsing alias type: Value not recognized.");
            ret = XMLP_ret::XML_ERROR;
        }
    }
    else
    {
        logError(XMLPARSER, "Error parsing alias type: Type not defined.");
        ret = XMLP_ret::XML_ERROR;
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLEnumDynamicType(tinyxml2::XMLElement* p_root)
{
    /*
        <xs:complexType name="enumeratorType">
            <xs:attribute name="name" type="stringType" use="required"/>
            <xs:attribute name="value" type="stringType" use="optional"/>
        </xs:complexType>

        <xs:complexType name="enum">
            <xs:attribute name="name" use="required"/>
            <xs:sequence>
                <xs:element name="enumerator" type="enumeratorType" minOccurs="0" maxOccurs="unbounded"/>
            </xs:sequence>
        </xs:complexType>

        //TODO: Enum bitbound to set the internal field
    */
    XMLP_ret ret = XMLP_ret::XML_OK;
    const char* enumName = p_root->Attribute(NAME);
    p_dynamictypebuilder_t typeBuilder = types::DynamicTypeBuilderFactory::GetInstance()->CreateEnumBuilder();
    uint32_t currValue = 0;
    for (tinyxml2::XMLElement* literal = p_root->FirstChildElement(ENUMERATOR);
            literal != nullptr; literal = literal->NextSiblingElement(ENUMERATOR))
    {
        const char* name = literal->Attribute(NAME);
        if (name == nullptr)
        {
            logError(XMLPARSER, "Error parsing enum type: Literals must have name.");
            return XMLP_ret::XML_ERROR;
        }

        const char* value = literal->Attribute(VALUE);
        if (value != nullptr)
        {
            currValue = std::atoi(value);
        }
        typeBuilder->AddEmptyMember(currValue++, name);
    }

    XMLProfileManager::insertDynamicTypeByName(enumName, typeBuilder);
    return ret;
}

XMLP_ret XMLParser::parseXMLStructDynamicType(tinyxml2::XMLElement* p_root)
{
    /*
        <xs:complexType name="structDcl">
            <xs:sequence>
                <xs:choice maxOccurs="unbounded">
                    <xs:element name="member" type="memberDcl" minOccurs="1"/>
                </xs:choice>
            </xs:sequence>
            <xs:attribute name="name" type="string" use="required"/>
        </xs:complexType>
    */
    XMLP_ret ret = XMLP_ret::XML_OK;
    const char* name = p_root->Attribute(NAME);
    p_dynamictypebuilder_t typeBuilder = types::DynamicTypeBuilderFactory::GetInstance()->CreateStructBuilder();
    typeBuilder->SetName(name);
    uint32_t mId = 0;
    const char* element_name = nullptr;
    for (tinyxml2::XMLElement *p_element = p_root->FirstChildElement();
            p_element != nullptr; p_element = p_element->NextSiblingElement())
    {
        element_name = p_element->Name();
        if (strcmp(element_name, MEMBER) == 0)
        {
            p_dynamictypebuilder_t mType = parseXMLMemberDynamicType(p_element, typeBuilder, mId++);
            if (mType == nullptr)
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            logError(XMLPARSER, "Invalid element found into 'structDcl'. Name: " << element_name);
            return XMLP_ret::XML_ERROR;
        }
    }

    XMLProfileManager::insertDynamicTypeByName(name, typeBuilder);

    return ret;
}

XMLP_ret XMLParser::parseXMLUnionDynamicType(tinyxml2::XMLElement* p_root)
{
    /*
        <xs:complexType name="caseDcl">
            <xs:sequence>
                <xs:choice minOccurs="1" maxOccurs="unbounded">
                    <xs:element name="caseValue" type="string" minOccurs="1" maxOccurs="unbounded"/>
                </xs:choice>
                <xs:element name="member" type="memberDcl" minOccurs="1" maxOccurs="1"/>
            </xs:sequence>
        </xs:complexType>

        <xs:complexType name="unionDcl">
            <xs:sequence>
                <xs:element name="discriminator" type="string" minOccurs="1"/>
                <xs:sequence maxOccurs="unbounded">
                    <xs:element name="case" type="caseDcl" minOccurs="1"/>
                </xs:sequence>
            </xs:sequence>
            <xs:attribute name="name" type="identifierName" use="required"/>
        </xs:complexType>
    */

    XMLP_ret ret = XMLP_ret::XML_OK;
    const char* name = p_root->Attribute(NAME);
    tinyxml2::XMLElement *p_element = p_root->FirstChildElement(DISCRIMINATOR);
    if (p_element != nullptr)
    {
        const char* disc = p_element->Attribute(TYPE);
        p_dynamictypebuilder_t discriminator = getDiscriminatorTypeBuilder(disc);
        if (discriminator == nullptr)
        {
            logError(XMLPARSER,
                "Error parsing union discriminator: Only primitive types allowed (found type " << disc << ").");
            ret = XMLP_ret::XML_ERROR;
        }
        else
        {
            p_dynamictypebuilder_t typeBuilder = types::DynamicTypeBuilderFactory::GetInstance()->CreateUnionBuilder(discriminator);
            typeBuilder->SetName(name);

            uint32_t mId = 0;
            for (p_element = p_root->FirstChildElement(CASE);
                    p_element != nullptr; p_element = p_element->NextSiblingElement(CASE))
            {
                std::string valuesStr = "";
                for (tinyxml2::XMLElement *caseValue = p_element->FirstChildElement(CASE_DISCRIMINATOR);
                    caseValue != nullptr; caseValue = caseValue->NextSiblingElement(CASE_DISCRIMINATOR))
                {
                    const char* values = caseValue->Attribute(VALUE);
                    if (values == nullptr)
                    {
                        logError(XMLPARSER, "Error parsing union case value: Not found.");
                        return XMLP_ret::XML_ERROR;
                    }

                    if (valuesStr.empty())
                    {
                        valuesStr = values;
                    }
                    else
                    {
                        valuesStr += std::string(",") + values;
                    }
                }

                tinyxml2::XMLElement *caseElement = p_element->FirstChildElement();
                while (caseElement != nullptr && strncmp(caseElement->Value(), CASE_DISCRIMINATOR, 10) == 0)
                {
                    caseElement = caseElement->NextSiblingElement();
                }
                if (caseElement != nullptr)
                {
                    p_dynamictypebuilder_t mType = parseXMLMemberDynamicType
                                                    (caseElement, typeBuilder, mId++, valuesStr);
                    if (mType == nullptr)
                    {
                        return XMLP_ret::XML_ERROR;
                    }
                }
                else
                {
                    logError(XMLPARSER, "Error parsing union case member: Not found.");
                    return XMLP_ret::XML_ERROR;
                }
            }

            XMLProfileManager::insertDynamicTypeByName(name, typeBuilder);
        }
    }
    else
    {
        logError(XMLPARSER, "Error parsing union discriminator: Not found.");
        ret = XMLP_ret::XML_ERROR;
    }

    return ret;
}

static void dimensionsToArrayBounds(const std::string& dimensions, std::vector<uint32_t>& bounds)
{
    std::stringstream ss(dimensions);
    std::string item;

    bounds.clear();

    while (std::getline(ss, item, ','))
    {
        bounds.push_back(std::atoi(item.c_str()));
    }
}

static bool dimensionsToLabels(const std::string& labelStr, std::vector<uint64_t>& labels)
{
    std::stringstream ss(labelStr);
    std::string item;
    bool def = false;

    labels.clear();
    while (std::getline(ss, item, ','))
    {
        if (item == DEFAULT)
            def = true;
        else
            labels.push_back(std::atoi(item.c_str()));
    }

    return def;
}

p_dynamictypebuilder_t XMLParser::parseXMLMemberDynamicType(tinyxml2::XMLElement* p_root,
        p_dynamictypebuilder_t p_dynamictype, MemberId mId)
{
    return parseXMLMemberDynamicType(p_root, p_dynamictype, mId, "");
}

p_dynamictypebuilder_t XMLParser::parseXMLMemberDynamicType(tinyxml2::XMLElement* p_root,
        p_dynamictypebuilder_t p_dynamictype, MemberId mId, const std::string& values)
{
    /*
        <xs:complexType name="memberDcl">
            <xs:attribute name="name" type="string" use="required"/>
            <xs:attribute name="type" type="string" use="required"/>
            <xs:attribute name="arrayDimensions" type="string" use="optional"/>
            <xs:attribute name="nonBasic" type="string" use="optional"/>
            <xs:attribute name="sequenceMaxLength" type="string" use="optional"/>
            <xs:attribute name="mapMaxLength" type="string" use="optional"/>
            <xs:sequence>
                <xs:element name="member" type="memberDcl" minOccurs="0"/>
            </xs:sequence>
        </xs:complexType>
    */
    if (p_root == nullptr)
    {
        logError(XMLPARSER, "Error parsing member: Node not found.");
        return nullptr;
    }

    const char* memberType = p_root->Attribute(TYPE);
    const char* memberName = p_root->Attribute(NAME);
    bool isArray = false;

    if (memberName == nullptr && p_dynamictype != nullptr)
    {
        logError(XMLPARSER, "Error parsing member name: Not found.");
        return nullptr;
    }

    if (memberType == nullptr)
    {
        logError(XMLPARSER, "Error parsing member type: Not found.");
        return nullptr;
    }

    const char* memberArray = p_root->Attribute(ARRAY_DIMENSIONS);
    if (memberArray != nullptr)
    {
        isArray = true;
    }

    if (strcmp(memberType, NON_BASIC_TYPE) == 0)
    {
        const char* memberNonBasicTypeName = p_root->Attribute(NON_BASIC_TYPE_NAME);
        if (memberNonBasicTypeName != nullptr)
        {
            memberType = memberNonBasicTypeName;
        }
        else
        {
            logError(XMLPARSER, "Error parsing member type: Not found.");
            return nullptr;
        }
    }

    types::DynamicTypeBuilder* memberBuilder = nullptr;
    types::DynamicTypeBuilderFactory* factory = types::DynamicTypeBuilderFactory::GetInstance();

    const char* memberSequence = p_root->Attribute(SEQ_MAXLENGTH);
    if (memberSequence != nullptr)
    {
        /*
            In sequences allowed formats are (complex format includes the basic):
            sequence<sequence<long,2>,2>
            <sequence name="my_sequence" length="2">
                <sequence name="inner_sequence" type="long" length="2"/>
            </sequence>
            In this example, inner sequence's name is ignored and can be omited.
         */
        p_dynamictypebuilder_t contentType = getDiscriminatorTypeBuilder(memberType);
        if (contentType == nullptr)
        {
            logError(XMLPARSER, "Error parsing sequence element type: Cannot be recognized: " << memberType);
            return nullptr;
        }

        const char* lengthStr = p_root->Attribute(SEQ_MAXLENGTH);
        uint32_t length = MAX_ELEMENTS_COUNT;
        if (lengthStr != nullptr)
        {
            length = std::stoi(lengthStr);
        }

        if (!isArray)
        {
            memberBuilder = factory->CreateSequenceBuilder(contentType, length);
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateSequenceBuilder(contentType, length);
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (p_root->Attribute(MAP_MAXLENGTH) != nullptr)
    {
        /*
            In maps allowed formats are (complex format includes the basic):
            map<map<long, long, 6>, map<long, map<long, short>,2>
            <map name="my_map" length="2">
                <key_type>
                    <map name="inner_key_map" key_type="long" value_type="long" length="6"/>
                </key_type>
                </value_type>
                    <map name="inner_value_map" key_type="long" length="2">
                        </value_type>
                            <map name="inner_value_value_map" key_type="long" value_type="short"/>
                        </value_type>
                    </map>
                </value_type>
            </map>
            In this example, inner maps names are ignored and can be omited.
         */
        // Parse key

        //const char* keyType = p_root->Attribute(KEY);
        p_dynamictypebuilder_t keyTypeBuilder = nullptr;
        const char* memberMapKeyType = p_root->Attribute(MAP_KEY_TYPE);
        if (memberMapKeyType != nullptr)
        {
            keyTypeBuilder = getDiscriminatorTypeBuilder(memberMapKeyType);
            if (keyTypeBuilder == nullptr)
            {
                logError(XMLPARSER, "Error parsing map's key element type: Cannot be recognized.");
                return nullptr;
            }
        }
        else
        {
            logError(XMLPARSER, "Error parsing key_type element: Not found.");
            return nullptr;
        }

        // Parse value
        p_dynamictypebuilder_t valueTypeBuilder;
        if (memberType != nullptr)
        {
            valueTypeBuilder = getDiscriminatorTypeBuilder(memberType);
            if (valueTypeBuilder == nullptr)
            {
                logError(XMLPARSER, "Error parsing map's value element type: Cannot be recognized.");
                return nullptr;
            }
        }
        else
        {
            logError(XMLPARSER, "Error parsing value_value element: Not found.");
            return nullptr;
        }

        const char* lengthStr = p_root->Attribute(MAP_MAXLENGTH);
        uint32_t length = MAX_ELEMENTS_COUNT;
        if (lengthStr != nullptr)
        {
            length = std::stoi(lengthStr);
        }

        if (!isArray)
        {
            memberBuilder = factory->CreateMapBuilder(keyTypeBuilder, valueTypeBuilder, length);
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder =
                    factory->CreateMapBuilder(keyTypeBuilder, valueTypeBuilder, length);
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, BOOLEAN, 8) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateBoolBuilder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateBoolBuilder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, CHAR, 5) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateChar8Builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateChar8Builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, WCHAR, 6) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateChar16Builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateChar16Builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, TBYTE, 6) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateByteBuilder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateByteBuilder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, SHORT, 6) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateInt16Builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateInt16Builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, LONG, 5) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateInt32Builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateInt32Builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, ULONG, 13) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateUint32Builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateUint32Builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, USHORT, 14) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateUint16Builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateUint16Builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, LONGLONG, 9) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateInt64Builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateInt64Builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, ULONGLONG, 17) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateUint64Builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateUint64Builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, FLOAT, 6) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateFloat32Builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateFloat32Builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, DOUBLE, 7) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateFloat64Builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateFloat64Builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, LONGDOUBLE, 11) == 0)
    {
        if (!isArray)
        {
            memberBuilder = factory->CreateFloat128Builder();
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateFloat128Builder();
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, STRING, 7) == 0)
    {
        uint32_t bound = 0;
        const char* boundStr = p_root->Attribute(STR_MAXLENGTH);
        if (boundStr != nullptr)
        {
            bound = std::atoi(boundStr);
        }
        if (!isArray)
        {
            memberBuilder = factory->CreateStringBuilder(bound);
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateStringBuilder(bound);
            std::vector<uint32_t> boundsArray;
            dimensionsToArrayBounds(memberArray, boundsArray);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, boundsArray);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else if (strncmp(memberType, WSTRING, 8) == 0)
    {
        uint32_t bound = 0;
        const char* boundStr = p_root->Attribute(STR_MAXLENGTH);
        if (boundStr != nullptr)
        {
            bound = std::atoi(boundStr);
        }
        if (!isArray)
        {
            memberBuilder = factory->CreateWstringBuilder(bound);
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = factory->CreateWstringBuilder(bound);
            std::vector<uint32_t> boundsArray;
            dimensionsToArrayBounds(memberArray, boundsArray);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, boundsArray);
            //factory->DeleteBuilder(innerBuilder);
        }
    }
    else // Complex type?
    {
        p_dynamictypebuilder_t typePtr = XMLProfileManager::getDynamicTypeByName(memberType);
        if (!isArray)
        {
            memberBuilder = typePtr;
        }
        else
        {
            types::DynamicTypeBuilder* innerBuilder = typePtr;
            std::vector<uint32_t> bounds;
            dimensionsToArrayBounds(memberArray, bounds);
            memberBuilder = factory->CreateArrayBuilder(innerBuilder, bounds);
            // Don't delete innerBuilder, it will be freed on destructors.
        }
    }


    if (memberBuilder == nullptr)
    {
        if (!isArray)
        {
            logError(XMLPARSER, "Failed creating " << memberType << ": " << memberName);
        }
        else
        {
            logError(XMLPARSER, "Failed creating " << memberType << " array: " << memberName);
        }
    }

    const char* memberTopicKey = p_root->Attribute(KEY);
    if (memberTopicKey != nullptr)
    {
        if (strncmp(memberTopicKey, "true", 5) == 0)
        {
            memberBuilder->ApplyAnnotation("@Key", "true");
            if (p_dynamictype != nullptr)
            {
                p_dynamictype->ApplyAnnotation("@Key", "true");
            }
        }
    }

    if (p_dynamictype != nullptr)
    {
        if (!values.empty())
        {
            std::vector<uint64_t> labels;
            bool defaultLabel = dimensionsToLabels(values, labels);
            p_dynamictype->AddMember(mId, memberName, memberBuilder,
                "", labels, defaultLabel);
        }
        else
        {
            p_dynamictype->AddMember(mId, memberName, memberBuilder);
        }
    }
    //factory->DeleteBuilder(memberBuilder);

    return memberBuilder;
}

XMLP_ret XMLParser::parseXMLParticipantProf(tinyxml2::XMLElement* p_root, BaseNode& rootNode)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    up_participant_t participant_atts{new ParticipantAttributes};
    up_node_participant_t participant_node{new node_participant_t{NodeType::PARTICIPANT, std::move(participant_atts)}};
    if (XMLP_ret::XML_OK == fillDataNode(p_root, *participant_node))
    {
        rootNode.addChild(std::move(participant_node));
    }
    else
    {
        logError(XMLPARSER, "Error parsing participant profile");
        ret = XMLP_ret::XML_ERROR;
    }

    return ret;
}

XMLP_ret XMLParser::parseXMLPublisherProf(tinyxml2::XMLElement* p_root, BaseNode& rootNode)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    up_publisher_t publisher_atts{new PublisherAttributes};
    up_node_publisher_t publisher_node{new node_publisher_t{NodeType::PUBLISHER, std::move(publisher_atts)}};
    if (XMLP_ret::XML_OK == fillDataNode(p_root, *publisher_node))
    {
        rootNode.addChild(std::move(publisher_node));
    }
    else
    {
        logError(XMLPARSER, "Error parsing publisher profile");
        ret = XMLP_ret::XML_ERROR;
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLSubscriberProf(tinyxml2::XMLElement* p_root, BaseNode& rootNode)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    up_subscriber_t subscriber_atts{new SubscriberAttributes};
    up_node_subscriber_t subscriber_node{new node_subscriber_t{NodeType::SUBSCRIBER, std::move(subscriber_atts)}};
    if (XMLP_ret::XML_OK == fillDataNode(p_root, *subscriber_node))
    {
        rootNode.addChild(std::move(subscriber_node));
    }
    else
    {
        logError(XMLPARSER, "Error parsing subscriber profile");
        ret = XMLP_ret::XML_ERROR;
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLTopicData(tinyxml2::XMLElement* p_root, BaseNode& rootNode)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    up_topic_t topic_atts{new TopicAttributes};
    up_node_topic_t topic_node{new node_topic_t{NodeType::TOPIC, std::move(topic_atts)}};
    if (XMLP_ret::XML_OK == fillDataNode(p_root, *topic_node))
    {
        rootNode.addChild(std::move(topic_node));
    }
    else
    {
        logError(XMLPARSER, "Error parsing topic data node");
        ret = XMLP_ret::XML_ERROR;
    }
    return ret;
}

XMLP_ret XMLParser::parseProfiles(tinyxml2::XMLElement* p_root, BaseNode& profilesNode)
{
    /*
        <xs:element name="profiles">
            <xs:complexType>
                <xs:sequence>
                    <xs:element name="transport_descriptors" type="TransportDescriptorListType" minOccurs="0" maxOccurs="unbounded"/>
                    <xs:element name="participant" type="participantProfileType" minOccurs="0" maxOccurs="unbounded"/>
                    <xs:element name="publisher" type="publisherProfileType" minOccurs="0" maxOccurs="unbounded"/>
                    <xs:element name="subscriber" type="subscriberProfileType" minOccurs="0" maxOccurs="unbounded"/>
                    <xs:element name="topic" type="topicAttributesType" minOccurs="0" maxOccurs="unbounded"/>
                </xs:sequence>
            </xs:complexType>
        </xs:element>
    */

    tinyxml2::XMLElement* p_profile = p_root->FirstChildElement();
    const char* tag = nullptr;
    bool parseOk = true;
    while (nullptr != p_profile)
    {
        if (nullptr != (tag = p_profile->Value()))
        {
            // If profile parsing functions fails, log and continue.
            if (strcmp(tag, TRANSPORT_DESCRIPTORS) == 0)
            {
                parseOk &= parseXMLTransportsProf(p_profile) == XMLP_ret::XML_OK;
            }
            else if (strcmp(tag, PARTICIPANT) == 0)
            {
                parseOk &= parseXMLParticipantProf(p_profile, profilesNode) == XMLP_ret::XML_OK;
            }
            else if (strcmp(tag, PUBLISHER) == 0 || strcmp(tag, DATA_WRITER) == 0)
            {
                parseOk &= parseXMLPublisherProf(p_profile, profilesNode) == XMLP_ret::XML_OK;
            }
            else if (strcmp(tag, SUBSCRIBER) == 0 || strcmp(tag, DATA_READER) == 0)
            {
                parseOk &= parseXMLSubscriberProf(p_profile, profilesNode) == XMLP_ret::XML_OK;
            }
            else if (strcmp(tag, TOPIC) == 0)
            {
                parseOk &= parseXMLTopicData(p_profile, profilesNode) == XMLP_ret::XML_OK;
            }
            else if (strcmp(tag, TYPES) == 0)
            {
                parseOk &= parseXMLTypes(p_profile) == XMLP_ret::XML_OK;
            }
            else if (strcmp(tag, QOS_PROFILE) == 0)
            {
                logError(XMLPARSER, "Field 'QOS_PROFILE' do not supported for now");
            }
            else if (strcmp(tag, APPLICATION) == 0)
            {
                logError(XMLPARSER, "Field 'APPLICATION' do not supported for now");
            }
            else if (strcmp(tag, TYPE) == 0)
            {
                logError(XMLPARSER, "Field 'TYPE' do not supported for now");
            }
            else
            {
                logError(XMLPARSER, "Not expected tag: '" << tag << "'");
            }
        }

        if (!parseOk)
        {
            logError(XMLPARSER, "Error parsing profile's tag " << tag);
            return XMLP_ret::XML_ERROR;
        }
        p_profile = p_profile->NextSiblingElement();
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::parseDynamicTypes(tinyxml2::XMLElement* p_root)
{
    return parseXMLTypes(p_root);
}

XMLP_ret XMLParser::parseLogConfig(tinyxml2::XMLElement* p_root)
{
    /*
    <xs:element name="log">
      <xs:complexType>
        <xs:boolean name="use_default"/>
        <xs:sequence>
          <xs:element maxOccurs="consumer">
            <xs:complexType>
              <xs:element name="class" type="string" minOccurs="1" maxOccurs="1"/>
              <xs:sequence>
                <xs:element name="propertyType"/>
              </xs:sequence>
            </xs:complexType>
        </xs:sequence>
      </xs:complexType>
    </xs:element>
    */

    XMLP_ret ret = XMLP_ret::XML_OK;
    tinyxml2::XMLElement *p_aux0 = p_root->FirstChildElement(LOG);
    if (p_aux0 == nullptr)
    {
        p_aux0 = p_root;
    }

    tinyxml2::XMLElement* p_element = p_aux0->FirstChildElement();
    const char* tag = nullptr;
    while (nullptr != p_element)
    {
        if (nullptr != (tag = p_element->Value()))
        {
            if (strcmp(tag, USE_DEFAULT) == 0)
            {
                bool use_default = true;
                std::string auxBool = p_element->GetText();
                if (std::strcmp(auxBool.c_str(), "FALSE") == 0)
                {
                    use_default = false;
                }
                if (!use_default)
                {
                    Log::ClearConsumers();
                }
            }
            else if (strcmp(tag, CONSUMER) == 0)
            {
                ret = parseXMLConsumer(*p_element);
                if (ret != XMLP_ret::XML_OK)
                {
                    return ret;
                }
            }
            else
            {
                logError(XMLPARSER, "Not expected tag: '" << tag << "'");
                ret = XMLP_ret::XML_ERROR;
            }
        }
        p_element = p_element->NextSiblingElement(CONSUMER);
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLConsumer(tinyxml2::XMLElement& consumer)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    tinyxml2::XMLElement* p_element = consumer.FirstChildElement(CLASS);

    if (p_element != nullptr)
    {
        std::string classStr = p_element->GetText();

        if (std::strcmp(classStr.c_str(), "StdoutConsumer") == 0)
        {
            Log::RegisterConsumer(std::unique_ptr<LogConsumer>(new StdoutConsumer));
        }
        else if (std::strcmp(classStr.c_str(), "FileConsumer") == 0)
        {
            std::string outputFile = "output.log";
            bool append = false;

            tinyxml2::XMLElement* property = consumer.FirstChildElement(PROPERTY);
            if (nullptr == property)
            {
                Log::RegisterConsumer(std::unique_ptr<LogConsumer>(new FileConsumer));
            }
            else
            {
                tinyxml2::XMLElement* p_auxName = nullptr;
                tinyxml2::XMLElement* p_auxValue = nullptr;
                while (nullptr != property)
                {
                    // name - stringType
                    if (nullptr != (p_auxName = property->FirstChildElement(NAME)))
                    {
                        std::string s = p_auxName->GetText();

                        if (std::strcmp(s.c_str(), "filename") == 0)
                        {
                            if (nullptr != (p_auxValue = property->FirstChildElement(VALUE)))
                            {
                                outputFile = p_auxValue->GetText();
                            }
                            else
                            {
                                logError(XMLParser, "Filename value cannot be found for " << classStr
                                    << " log consumer.");
                            }
                        }
                        else if (std::strcmp(s.c_str(), "append") == 0)
                        {
                            if (nullptr != (p_auxValue = property->FirstChildElement(VALUE)))
                            {
                                std::string auxBool = p_auxValue->GetText();
                                if (std::strcmp(auxBool.c_str(), "TRUE") == 0)
                                {
                                    append = true;
                                }
                            }
                            else
                            {
                                logError(XMLParser, "Append value cannot be found for " << classStr
                                    << " log consumer.");
                            }
                        }
                        else
                        {
                            logError(XMLParser, "Unknown property " << s << " in " << classStr
                                << " log consumer.");
                        }
                    }
                    property = property->NextSiblingElement(PROPERTY);
                }

                Log::RegisterConsumer(std::unique_ptr<LogConsumer>(new FileConsumer(outputFile, append)));
            }
        }
        else
        {
            logError(XMLParser, "Unknown log consumer class: " << classStr);
            ret = XMLP_ret::XML_ERROR;
        }
    }

    return ret;
}

XMLP_ret XMLParser::loadXML(const std::string& filename, up_base_node_t& root)
{
    if (filename.empty())
    {
        logError(XMLPARSER, "Error loading XML file, filename empty");
        return XMLP_ret::XML_ERROR;
    }

    tinyxml2::XMLDocument xmlDoc;
    if (tinyxml2::XMLError::XML_SUCCESS != xmlDoc.LoadFile(filename.c_str()))
    {
        if (filename != std::string(DEFAULT_FASTRTPS_PROFILES))
            logError(XMLPARSER, "Error opening '" << filename << "'");

        return XMLP_ret::XML_ERROR;
    }

    logInfo(XMLPARSER, "File '" << filename << "' opened successfully");
    return parseXML(xmlDoc, root);
}

XMLP_ret XMLParser::loadXMLProfiles(tinyxml2::XMLElement &xmlDoc, up_base_node_t& root)
{
    return parseXMLProfiles(xmlDoc, root);
}

XMLP_ret XMLParser::loadXMLDynamicTypes(tinyxml2::XMLElement &xmlDoc)
{
    return parseXMLDynamicTypes(xmlDoc);
}

XMLP_ret XMLParser::loadXML(tinyxml2::XMLDocument &xmlDoc, up_base_node_t& root)
{
    return parseXML(xmlDoc, root);
}

XMLP_ret XMLParser::loadXML(const char* data, size_t length, up_base_node_t& root)
{
    tinyxml2::XMLDocument xmlDoc;
    if (tinyxml2::XMLError::XML_SUCCESS != xmlDoc.Parse(data, length))
    {
        logError(XMLPARSER, "Error parsing XML buffer");
        return XMLP_ret::XML_ERROR;
    }
    return parseXML(xmlDoc, root);
}

template <typename T>
void XMLParser::addAllAttributes(tinyxml2::XMLElement* p_profile, DataNode<T>& node)
{
    const tinyxml2::XMLAttribute* attrib;
    for (attrib = p_profile->FirstAttribute(); attrib != NULL; attrib = attrib->Next())
    {
        node.addAttribute(attrib->Name(), attrib->Value());
    }
}

XMLP_ret XMLParser::fillDataNode(tinyxml2::XMLElement* node, DataNode<TopicAttributes>& topic_node)
{
    if (nullptr == node)
    {
        logError(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(node, topic_node);

    uint8_t ident = 1;
    if (XMLP_ret::XML_OK != getXMLTopicAttributes(node, *topic_node.get(), ident))
    {
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::fillDataNode(tinyxml2::XMLElement* p_profile, DataNode<ParticipantAttributes>& participant_node)
{
    /*
        <xs:complexType name="rtpsParticipantAttributesType">
            <xs:all minOccurs="0">
                <xs:element name="defaultUnicastLocatorList" type="locatorListType" minOccurs="0"/>
                <xs:element name="defaultMulticastLocatorList" type="locatorListType" minOccurs="0"/>
                <xs:element name="sendSocketBufferSize" type="uint32Type" minOccurs="0"/>
                <xs:element name="listenSocketBufferSize" type="uint32Type" minOccurs="0"/>
                <xs:element name="builtin" type="builtinAttributesType" minOccurs="0"/>
                <xs:element name="port" type="portType" minOccurs="0"/>
                <xs:element name="userData" type="octetVectorType" minOccurs="0"/>
                <xs:element name="participantID" type="int32Type" minOccurs="0"/>
                <xs:element name="throughputController" type="throughputControllerType" minOccurs="0"/>
                <xs:element name="userTransports" type="stringListType" minOccurs="0"/>
                <xs:element name="useBuiltinTransports" type="boolType" minOccurs="0"/>
                <xs:element name="propertiesPolicy" type="propertyPolicyType" minOccurs="0"/>
                <xs:element name="name" type="stringType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
    */

    if (nullptr == p_profile)
    {
        logError(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, participant_node);

    tinyxml2::XMLElement* p_element = p_profile->FirstChildElement(RTPS);
    if (nullptr == p_element)
    {
        logError(XMLPARSER, "Not found '" << RTPS << "' tag");
        return XMLP_ret::XML_ERROR;
    }

    uint8_t ident = 1;
    tinyxml2::XMLElement *p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = p_element->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, DEF_UNI_LOC_LIST) == 0)
        {
            // defaultUnicastLocatorList
            if (XMLP_ret::XML_OK !=
                getXMLLocatorList(p_aux0, participant_node.get()->rtps.defaultUnicastLocatorList, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, DEF_MULTI_LOC_LIST) == 0)
        {
            // defaultMulticastLocatorList
            if (XMLP_ret::XML_OK !=
                getXMLLocatorList(p_aux0, participant_node.get()->rtps.defaultMulticastLocatorList, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, SEND_SOCK_BUF_SIZE) == 0)
        {
            // sendSocketBufferSize - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &participant_node.get()->rtps.sendSocketBufferSize, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, LIST_SOCK_BUF_SIZE) == 0)
        {
            // listenSocketBufferSize - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &participant_node.get()->rtps.listenSocketBufferSize, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, BUILTIN) == 0)
        {
            // builtin
            if (XMLP_ret::XML_OK != getXMLBuiltinAttributes(p_aux0, participant_node.get()->rtps.builtin, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, PORT) == 0)
        {
            // port
            if (XMLP_ret::XML_OK != getXMLPortParameters(p_aux0, participant_node.get()->rtps.port, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, USER_DATA) == 0)
        {
            // TODO: userData
            if (XMLP_ret::XML_OK != getXMLOctetVector(p_aux0, participant_node.get()->rtps.userData, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, PART_ID) == 0)
        {
            // participantID - int32Type
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &participant_node.get()->rtps.participantID, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, THROUGHPUT_CONT) == 0)
        {
            // throughputController
            if (XMLP_ret::XML_OK !=
                getXMLThroughputController(p_aux0, participant_node.get()->rtps.throughputController, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, USER_TRANS) == 0)
        {
            // userTransports
            if (XMLP_ret::XML_OK != getXMLTransports(p_aux0, participant_node.get()->rtps.userTransports, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, USE_BUILTIN_TRANS) == 0)
        {
            // useBuiltinTransports - boolType
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &participant_node.get()->rtps.useBuiltinTransports, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, PROPERTIES_POLICY) == 0)
        {
            // propertiesPolicy
            if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux0, participant_node.get()->rtps.properties, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, NAME) == 0)
        {
            // name - stringType
            std::string s;
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident))
                return XMLP_ret::XML_ERROR;
            participant_node.get()->rtps.setName(s.c_str());
        }
        else
        {
            logError(XMLPARSER, "Invalid element found into 'rtpsParticipantAttributesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::fillDataNode(tinyxml2::XMLElement* p_profile, DataNode<PublisherAttributes>& publisher_node)
{
    /*
        <xs:complexType name="publisherProfileType">
            <xs:all minOccurs="0">
                <xs:element name="topic" type="topicAttributesType" minOccurs="0"/>
                <xs:element name="qos" type="writerQosPoliciesType" minOccurs="0"/>
                <xs:element name="times" type="writerTimesType" minOccurs="0"/>
                <xs:element name="unicastLocatorList" type="locatorListType" minOccurs="0"/>
                <xs:element name="multicastLocatorList" type="locatorListType" minOccurs="0"/>
                <xs:element name="throughputController" type="throughputControllerType" minOccurs="0"/>
                <xs:element name="historyMemoryPolicy" type="historyMemoryPolicyType" minOccurs="0"/>
                <xs:element name="propertiesPolicy" type="propertyPolicyType" minOccurs="0"/>
                <xs:element name="userDefinedID" type="int16Type" minOccurs="0"/>
                <xs:element name="entityID" type="int16Type" minOccurs="0"/>
            </xs:all>
            <xs:attribute name="profile_name" type="stringType" use="required"/>
        </xs:complexType>
    */

    if (nullptr == p_profile)
    {
        logError(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, publisher_node);

    uint8_t ident = 1;
    tinyxml2::XMLElement *p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = p_profile->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, TOPIC) == 0)
        {
            // topic
            if (XMLP_ret::XML_OK != getXMLTopicAttributes(p_aux0, publisher_node.get()->topic, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, QOS) == 0)
        {
            // qos
            if (XMLP_ret::XML_OK != getXMLWriterQosPolicies(p_aux0, publisher_node.get()->qos, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, TIMES) == 0)
        {
            // times
            if (XMLP_ret::XML_OK != getXMLWriterTimes(p_aux0, publisher_node.get()->times, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, UNI_LOC_LIST) == 0)
        {
            // unicastLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, publisher_node.get()->unicastLocatorList, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, MULTI_LOC_LIST) == 0)
        {
            // multicastLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, publisher_node.get()->multicastLocatorList, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, REM_LOC_LIST) == 0)
        {
            // remoteLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, publisher_node.get()->remoteLocatorList, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, THROUGHPUT_CONT) == 0)
        {
            // throughputController
            if (XMLP_ret::XML_OK !=
                getXMLThroughputController(p_aux0, publisher_node.get()->throughputController, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, HIST_MEM_POLICY) == 0)
        {
            // historyMemoryPolicy
            if (XMLP_ret::XML_OK != getXMLHistoryMemoryPolicy(p_aux0, publisher_node.get()->historyMemoryPolicy, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, PROPERTIES_POLICY) == 0)
        {
            // propertiesPolicy
            if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux0, publisher_node.get()->properties, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, USER_DEF_ID) == 0)
        {
            // userDefinedID - int16type
            int i = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &i, ident) || i > 255)
                return XMLP_ret::XML_ERROR;
            publisher_node.get()->setUserDefinedID(static_cast<uint8_t>(i));
        }
        else if (strcmp(name, ENTITY_ID) == 0)
        {
            // entityID - int16Type
            int i = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &i, ident) || i > 255)
                return XMLP_ret::XML_ERROR;
            publisher_node.get()->setEntityID(static_cast<uint8_t>(i));
        }
        else if (strcmp(name, MATCHED_SUBSCRIBERS_ALLOCATION) == 0)
        {
            // matchedSubscribersAllocation - containerAllocationConfigType
            if(XMLP_ret::XML_OK != getXMLContainerAllocationConfig(p_aux0, publisher_node.get()->matched_subscriber_allocation, ident))
                return XMLP_ret::XML_ERROR;
        }
        else
        {
            logError(XMLPARSER, "Invalid element found into 'publisherProfileType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::fillDataNode(tinyxml2::XMLElement* p_profile, DataNode<SubscriberAttributes>& subscriber_node)
{
    /*
        <xs:complexType name="subscriberProfileType">
            <xs:all minOccurs="0">
                <xs:element name="topic" type="topicAttributesType" minOccurs="0"/>
                <xs:element name="qos" type="readerQosPoliciesType" minOccurs="0"/>
                <xs:element name="times" type="readerTimesType" minOccurs="0"/>
                <xs:element name="unicastLocatorList" type="locatorListType" minOccurs="0"/>
                <xs:element name="multicastLocatorList" type="locatorListType" minOccurs="0"/>
                <xs:element name="expectsInlineQos" type="boolType" minOccurs="0"/>
                <xs:element name="historyMemoryPolicy" type="historyMemoryPolicyType" minOccurs="0"/>
                <xs:element name="propertiesPolicy" type="propertyPolicyType" minOccurs="0"/>
                <xs:element name="userDefinedID" type="int16Type" minOccurs="0"/>
                <xs:element name="entityID" type="int16Type" minOccurs="0"/>
            </xs:all>
            <xs:attribute name="profile_name" type="stringType" use="required"/>
        </xs:complexType>
    */

    if (nullptr == p_profile)
    {
        logError(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, subscriber_node);

    uint8_t ident = 1;
    tinyxml2::XMLElement *p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = p_profile->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, TOPIC) == 0)
        {
            // topic
            if (XMLP_ret::XML_OK != getXMLTopicAttributes(p_aux0, subscriber_node.get()->topic, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, QOS) == 0)
        {
            // qos
            if (XMLP_ret::XML_OK != getXMLReaderQosPolicies(p_aux0, subscriber_node.get()->qos, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, TIMES) == 0)
        {
            // times
            if (XMLP_ret::XML_OK != getXMLReaderTimes(p_aux0, subscriber_node.get()->times, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, UNI_LOC_LIST) == 0)
        {
            // unicastLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, subscriber_node.get()->unicastLocatorList, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, MULTI_LOC_LIST) == 0)
        {
            // multicastLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, subscriber_node.get()->multicastLocatorList, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, REM_LOC_LIST) == 0)
        {
            // remote LocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, subscriber_node.get()->remoteLocatorList, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, EXP_INLINE_QOS) == 0)
        {
            // expectsInlineQos - boolType
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &subscriber_node.get()->expectsInlineQos, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, HIST_MEM_POLICY) == 0)
        {
            // historyMemoryPolicy
            if (XMLP_ret::XML_OK != getXMLHistoryMemoryPolicy(p_aux0, subscriber_node.get()->historyMemoryPolicy, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, PROPERTIES_POLICY) == 0)
        {
            // propertiesPolicy
            if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux0, subscriber_node.get()->properties, ident))
                return XMLP_ret::XML_ERROR;
        }
        else if (strcmp(name, USER_DEF_ID) == 0)
        {
            // userDefinedID - int16Type
            int i = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &i, ident) || i > 255)
                return XMLP_ret::XML_ERROR;
            subscriber_node.get()->setUserDefinedID(static_cast<uint8_t>(i));
        }
        else if (strcmp(name, ENTITY_ID) == 0)
        {
            // entityID - int16Type
            int i = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &i, ident) || i > 255)
                return XMLP_ret::XML_ERROR;
            subscriber_node.get()->setEntityID(static_cast<uint8_t>(i));
        }
        else if (strcmp(name, MATCHED_PUBLISHERS_ALLOCATION) == 0)
        {
            // matchedPublishersAllocation - containerAllocationConfigType
            if (XMLP_ret::XML_OK != getXMLContainerAllocationConfig(p_aux0, subscriber_node.get()->matched_publisher_allocation, ident))
                return XMLP_ret::XML_ERROR;
        }
        else
        {
            logError(XMLPARSER, "Invalid element found into 'subscriberProfileType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

} // namespace xmlparser
} // namespace fastrtps
} // namespace eprosima
