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
#include <xmlparser/XMLParser.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include <tinyxml2.h>

#include <fastdds/dds/log/FileConsumer.hpp>
#include <fastdds/dds/log/StdoutConsumer.hpp>
#include <fastdds/dds/log/StdoutErrConsumer.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/transport/network/NetmaskFilterKind.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>

#include <rtps/network/utils/netmask_filter.hpp>
#include <xmlparser/XMLParserUtils.hpp>
#include <xmlparser/XMLParserCommon.h>
#include <xmlparser/XMLParserUtils.hpp>
#include <xmlparser/XMLProfileManager.h>
#include <xmlparser/XMLTree.h>

namespace eprosima {
namespace fastdds {
namespace xmlparser {

using namespace eprosima::fastdds::xml::detail;

XMLP_ret XMLParser::loadDefaultXMLFile(
        up_base_node_t& root)
{
    // Use absolute path to ensure that the file is loaded only once
#ifdef _WIN32
    char current_directory[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, current_directory) == 0)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "GetCurrentDirectory failed " << GetLastError());
    }
    else
    {
        strcat_s(current_directory, MAX_PATH, DEFAULT_FASTDDS_PROFILES);
        return loadXML(current_directory, root, true);
    }
#else
    char current_directory[PATH_MAX];
    if (getcwd(current_directory, PATH_MAX) == NULL)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "getcwd failed " << std::strerror(errno));
    }
    else
    {
        strcat(current_directory, "/");
        strcat(current_directory, DEFAULT_FASTDDS_PROFILES);
        return loadXML(current_directory, root, true);
    }
#endif // _WIN32
    return XMLP_ret::XML_ERROR;
}

XMLP_ret XMLParser::parseXML(
        tinyxml2::XMLDocument& xmlDoc,
        up_base_node_t& root)
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
                    // Just library_settings config in the XML.
                    if (nullptr == (p_root = xmlDoc.FirstChildElement(LIBRARY_SETTINGS)))
                    {
                        EPROSIMA_LOG_ERROR(XMLPARSER, "Not found root tag");
                        ret = XMLP_ret::XML_ERROR;
                    }
                    else
                    {
                        root.reset(new BaseNode{NodeType::LIBRARY_SETTINGS});
                        ret  = parseXMLLibrarySettings(p_root);
                    }
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
        while ((nullptr != node) && (ret == XMLP_ret::XML_OK))
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
                else if (strcmp(tag, LIBRARY_SETTINGS) == 0)
                {
                    // TODO Workaround to propagate the return code upstream. A refactor is needed to propagate the
                    // return code in some other more sensible way or populate the object and change code upstream to
                    // read this new object.
                    up_base_node_t library_node = up_base_node_t{ new BaseNode{NodeType::LIBRARY_SETTINGS} };
                    if (XMLP_ret::XML_OK == (ret = parseXMLLibrarySettings(node)))
                    {
                        root->addChild(std::move(library_node));
                    }
                }
                else if (strcmp(tag, DOMAINPARTICIPANT_FACTORY) == 0)
                {
                    ret = parseXMLDomainParticipantFactoryProf(node, *root);
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
                else if (strcmp(tag, REQUESTER) == 0)
                {
                    ret = parseXMLRequesterProf(node, *root);
                }
                else if (strcmp(tag, REPLIER) == 0)
                {
                    ret = parseXMLReplierProf(node, *root);
                }
                else if (strcmp(tag, TYPES) == 0)
                {
                    // TODO Workaround to propagate the return code upstream. A refactor is needed to propagate the
                    // return code in some other more sensible way or populate the object and change code upstream to
                    // read this new object.
                    up_base_node_t types_node = up_base_node_t{ new BaseNode{NodeType::TYPES} };
                    if (XMLP_ret::XML_OK == (ret = parseXMLTypes(node)))
                    {
                        root->addChild(std::move(types_node));
                    }
                }
                else if (strcmp(tag, LOG) == 0)
                {
                    // TODO Workaround to propagate the return code upstream. A refactor is needed to propagate the
                    // return code in some other more sensible way or populate the object and change code upstream to
                    // read this new object.
                    up_base_node_t log_node = up_base_node_t{ new BaseNode{NodeType::LOG} };
                    if (XMLP_ret::XML_OK == (ret = parseLogConfig(node)))
                    {
                        root->addChild(std::move(log_node));
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Not expected tag: '" << tag << "'");
                    ret = XMLP_ret::XML_ERROR;
                }
            }

            node = node->NextSiblingElement();
        }
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLProfiles(
        tinyxml2::XMLElement& profiles,
        up_base_node_t& root)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    root.reset(new BaseNode{NodeType::PROFILES});
    ret  = parseProfiles(&profiles, *root);
    return ret;
}

XMLP_ret XMLParser::parseXMLTransportsProf(
        tinyxml2::XMLElement* p_root)
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
    while (p_element != nullptr)
    {
        ret = parseXMLTransportData(p_element);
        if (ret != XMLP_ret::XML_OK)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing transports");
            return ret;
        }
        p_element = p_element->NextSiblingElement(TRANSPORT_DESCRIPTOR);
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLTransportData(
        tinyxml2::XMLElement* p_root)
{
    /*
        <xs:complexType name="rtpsTransportDescriptorType">
            <xs:all minOccurs="0">
                <xs:element name="transport_id" type="stringType"/>
                <xs:element name="type" type="stringType"/>
                <xs:element name="sendBufferSize" type="int32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="receiveBufferSize" type="int32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="TTL" type="uint8Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="non_blocking_send" type="boolType" minOccurs="0" maxOccurs="1"/>
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
                <xs:element name="tcp_negotiation_timeout" type="uint32_t" minOccurs="0" maxOccurs="1"/>
                <xs:element name="tls" type="tlsConfigType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */

    if (XMLP_ret::XML_OK != validateXMLTransportElements(*p_root))
    {
        return XMLP_ret::XML_ERROR;
    }

    tinyxml2::XMLElement* p_aux0 = nullptr;
    p_aux0 = p_root->FirstChildElement(TRANSPORT_ID);
    if (nullptr == p_aux0)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Not found '" << TRANSPORT_ID << "' attribute");
        return XMLP_ret::XML_ERROR;
    }

    XMLP_ret ret = XMLP_ret::XML_OK;
    sp_transport_t pDescriptor = nullptr;

    std::string sId = get_element_text(p_aux0);
    if (sId.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "'" << TRANSPORT_ID << "' attribute cannot be empty");
        return XMLP_ret::XML_ERROR;
    }

    p_aux0 = p_root->FirstChildElement(TYPE);
    if (nullptr == p_aux0)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Not found '" << TYPE << "' attribute");
        return XMLP_ret::XML_ERROR;
    }

    std::string sType = get_element_text(p_aux0);
    if (sType.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "'" << TYPE << "' attribute cannot be empty");
        return XMLP_ret::XML_ERROR;
    }

    if (sType == UDPv4 || sType == UDPv6)
    {
        std::shared_ptr<fastdds::rtps::UDPTransportDescriptor> pUDPDesc;
        if (sType == UDPv4)
        {
            pDescriptor = pUDPDesc = std::make_shared<fastdds::rtps::UDPv4TransportDescriptor>();
        }
        else
        {
            pDescriptor = pUDPDesc = std::make_shared<fastdds::rtps::UDPv6TransportDescriptor>();
        }

        // Output UDP Socket
        if (nullptr != (p_aux0 = p_root->FirstChildElement(UDP_OUTPUT_PORT)))
        {
            int iSocket = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iSocket, 0) || iSocket < 0 || iSocket > 65535)
            {
                return XMLP_ret::XML_ERROR;
            }
            pUDPDesc->m_output_udp_socket = static_cast<uint16_t>(iSocket);
        }
        // Non-blocking send
        if (nullptr != (p_aux0 = p_root->FirstChildElement(NON_BLOCKING_SEND)))
        {
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &pUDPDesc->non_blocking_send, 0))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
    }
    else if (sType == TCPv4)
    {
        pDescriptor = std::make_shared<fastdds::rtps::TCPv4TransportDescriptor>();
        ret = parseXMLCommonTCPTransportData(p_root, pDescriptor);
        if (ret != XMLP_ret::XML_OK)
        {
            return ret;
        }
        else
        {
            std::shared_ptr<fastdds::rtps::TCPv4TransportDescriptor> pTCPv4Desc =
                    std::dynamic_pointer_cast<fastdds::rtps::TCPv4TransportDescriptor>(pDescriptor);

            // Wan Address
            if (nullptr != (p_aux0 = p_root->FirstChildElement(TCP_WAN_ADDR)))
            {
                std::string s;
                if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, 0))
                {
                    return XMLP_ret::XML_ERROR;
                }
                pTCPv4Desc->set_WAN_address(s);
            }
        }
    }
    else if (sType == TCPv6)
    {
        pDescriptor = std::make_shared<fastdds::rtps::TCPv6TransportDescriptor>();
        ret = parseXMLCommonTCPTransportData(p_root, pDescriptor);
        if (ret != XMLP_ret::XML_OK)
        {
            return ret;
        }
    }
    else if (sType == SHM)
    {
        pDescriptor = std::make_shared<fastdds::rtps::SharedMemTransportDescriptor>();
        ret = parseXMLCommonSharedMemTransportData(p_root, pDescriptor);
        if (ret != XMLP_ret::XML_OK)
        {
            return ret;
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid transport type: '" << sType << "'");
        return XMLP_ret::XML_ERROR;
    }

    ret = parseXMLCommonTransportData(p_root, pDescriptor);
    if (ret != XMLP_ret::XML_OK)
    {
        return ret;
    }

    std::shared_ptr<fastdds::rtps::PortBasedTransportDescriptor> temp_1 =
            std::dynamic_pointer_cast<fastdds::rtps::PortBasedTransportDescriptor>(pDescriptor);
    ret = parseXMLPortBasedTransportData(p_root, temp_1);
    if (ret != XMLP_ret::XML_OK)
    {
        return ret;
    }

    if (sType != SHM)
    {
        std::shared_ptr<fastdds::rtps::SocketTransportDescriptor> temp_2 =
                std::dynamic_pointer_cast<fastdds::rtps::SocketTransportDescriptor>(pDescriptor);
        ret = parseXMLSocketTransportData(p_root, temp_2);
        if (ret != XMLP_ret::XML_OK)
        {
            return ret;
        }
    }

    XMLProfileManager::insertTransportById(sId, pDescriptor);
    return ret;
}

XMLP_ret XMLParser::validateXMLTransportElements(
        tinyxml2::XMLElement& p_root)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    for (tinyxml2::XMLElement* p_aux0 = p_root.FirstChildElement(); p_aux0 != nullptr;
            p_aux0 = p_aux0->NextSiblingElement())
    {
        const char* name = p_aux0->Name();
        if (!(strcmp(name, TRANSPORT_ID) == 0 ||
                strcmp(name, TYPE) == 0 ||
                strcmp(name, SEND_BUFFER_SIZE) == 0 ||
                strcmp(name, RECEIVE_BUFFER_SIZE) == 0 ||
                strcmp(name, MAX_MESSAGE_SIZE) == 0 ||
                strcmp(name, MAX_INITIAL_PEERS_RANGE) == 0 ||
                strcmp(name, WHITE_LIST) == 0 ||
                strcmp(name, NETMASK_FILTER) == 0 ||
                strcmp(name, NETWORK_INTERFACES) == 0 ||
                strcmp(name, TTL) == 0 ||
                strcmp(name, NON_BLOCKING_SEND) == 0 ||
                strcmp(name, UDP_OUTPUT_PORT) == 0 ||
                strcmp(name, TCP_WAN_ADDR) == 0 ||
                strcmp(name, KEEP_ALIVE_FREQUENCY) == 0 ||
                strcmp(name, KEEP_ALIVE_TIMEOUT) == 0 ||
                strcmp(name, MAX_LOGICAL_PORT) == 0 ||
                strcmp(name, LOGICAL_PORT_RANGE) == 0 ||
                strcmp(name, LOGICAL_PORT_INCREMENT) == 0 ||
                strcmp(name, LISTENING_PORTS) == 0 ||
                strcmp(name, CALCULATE_CRC) == 0 ||
                strcmp(name, CHECK_CRC) == 0 ||
                strcmp(name, KEEP_ALIVE_THREAD) == 0 ||
                strcmp(name, ACCEPT_THREAD) == 0 ||
                strcmp(name, ENABLE_TCP_NODELAY) == 0 ||
                strcmp(name, TCP_NEGOTIATION_TIMEOUT) == 0 ||
                strcmp(name, TLS) == 0 ||
                strcmp(name, SEGMENT_SIZE) == 0 ||
                strcmp(name, PORT_QUEUE_CAPACITY) == 0 ||
                strcmp(name, HEALTHY_CHECK_TIMEOUT_MS) == 0 ||
                strcmp(name, RTPS_DUMP_FILE) == 0 ||
                strcmp(name, DEFAULT_RECEPTION_THREADS) == 0 ||
                strcmp(name, RECEPTION_THREADS) == 0 ||
                strcmp(name, DUMP_THREAD) == 0 ||
                strcmp(name, PORT_OVERFLOW_POLICY) == 0 ||
                strcmp(name, SEGMENT_OVERFLOW_POLICY) == 0))
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'transportDescriptorType'. Name: " << name);
            ret = XMLP_ret::XML_ERROR;
        }
    }

    return ret;
}

XMLP_ret XMLParser::parseXMLCommonTransportData(
        tinyxml2::XMLElement* p_root,
        sp_transport_t p_transport)
{
    /*
        <xs:complexType name="rtpsTransportDescriptorType">
            <xs:all minOccurs="0">
                <xs:element name="transport_id" type="stringType"/>
                <xs:element name="type" type="stringType"/>
                <xs:element name="maxMessageSize" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="maxInitialPeersRange" type="uint32Type" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, MAX_MESSAGE_SIZE) == 0)
        {
            // maxMessageSize - uint32Type
            uint32_t uSize = 0;
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &uSize, 0))
            {
                return XMLP_ret::XML_ERROR;
            }
            p_transport->maxMessageSize = uSize;
        }
        else if (strcmp(name, MAX_INITIAL_PEERS_RANGE) == 0)
        {
            // maxInitialPeersRange - uint32Type
            uint32_t uRange = 0;
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &uRange, 0))
            {
                return XMLP_ret::XML_ERROR;
            }
            p_transport->maxInitialPeersRange = uRange;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::parseXMLPortBasedTransportData(
        tinyxml2::XMLElement* p_root,
        std::shared_ptr<fastdds::rtps::PortBasedTransportDescriptor> p_transport)
{
    /*
        <xs:complexType name="rtpsTransportDescriptorType">
            <xs:all minOccurs="0">
                <xs:element name="default_reception_threads" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="reception_threads" type="receptionThreadsListType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, DEFAULT_RECEPTION_THREADS) == 0)
        {
            fastdds::rtps::ThreadSettings thread_settings;
            if (getXMLThreadSettings(*p_aux0, thread_settings) == XMLP_ret::XML_OK)
            {
                p_transport->default_reception_threads(thread_settings);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Incorrect thread settings");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, RECEPTION_THREADS) == 0)
        {
            fastdds::rtps::PortBasedTransportDescriptor::ReceptionThreadsConfigMap reception_threads;
            if (parseXMLReceptionThreads(*p_aux0, reception_threads) == XMLP_ret::XML_OK)
            {
                p_transport->reception_threads(reception_threads);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Incorrect thread settings");
                return XMLP_ret::XML_ERROR;
            }
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::parseXMLSocketTransportData(
        tinyxml2::XMLElement* p_root,
        std::shared_ptr<fastdds::rtps::SocketTransportDescriptor> p_transport)
{
    /*
        <xs:complexType name="rtpsTransportDescriptorType">
            <xs:all minOccurs="0">
                <xs:element name="sendBufferSize" type="int32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="receiveBufferSize" type="int32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="TTL" type="uint8Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="interfaceWhiteList" type="addressListType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="netmask_filter" type="netmaskFilterType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="interfaces" type="interfacesType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, SEND_BUFFER_SIZE) == 0)
        {
            // sendBufferSize - int32Type
            uint32_t iSize = 0;
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &iSize, 0))
            {
                return XMLP_ret::XML_ERROR;
            }
            p_transport->sendBufferSize = iSize;
        }
        else if (strcmp(name, RECEIVE_BUFFER_SIZE) == 0)
        {
            // receiveBufferSize - int32Type
            uint32_t iSize = 0;
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &iSize, 0))
            {
                return XMLP_ret::XML_ERROR;
            }
            p_transport->receiveBufferSize = iSize;
        }
        else if (strcmp(name, TTL) == 0)
        {
            // TTL - int8Type
            int iTTL = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iTTL, 0) || iTTL < 0 || iTTL > 255)
            {
                return XMLP_ret::XML_ERROR;
            }
            p_transport->TTL = static_cast<uint8_t>(iTTL);
        }
        else if (strcmp(name, WHITE_LIST) == 0)
        {
            // InterfaceWhiteList addressListType
            const char* address = nullptr;
            for (tinyxml2::XMLElement* p_aux1 = p_aux0->FirstChildElement();
                    p_aux1 != nullptr; p_aux1 = p_aux1->NextSiblingElement())
            {
                address = p_aux1->Name();
                if (strcmp(address, ADDRESS) == 0 || strcmp(address, NETWORK_INTERFACE) == 0)
                {
                    std::string text = get_element_text(p_aux1);
                    if (!text.empty())
                    {
                        p_transport->interfaceWhiteList.emplace_back(text);
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'interfaceWhiteList'. Name: " << address);
                    return XMLP_ret::XML_ERROR;
                }
            }
        }
        else if (strcmp(name, NETMASK_FILTER) == 0)
        {
            std::string netmask_filter_str;
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &netmask_filter_str, 0))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'netmask_filter'.");
                return XMLP_ret::XML_ERROR;
            }

            try
            {
                p_transport->netmask_filter = fastdds::rtps::network::netmask_filter::string_to_netmask_filter_kind(
                    netmask_filter_str);
            }
            catch (const std::invalid_argument& e)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'netmask_filter' : " << e.what());
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, NETWORK_INTERFACES) == 0)
        {
            if (XMLP_ret::XML_OK != parseXMLInterfaces(p_aux0, p_transport))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Failed to parse 'interfaces' element.");
                return XMLP_ret::XML_ERROR;
            }
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::parseXMLInterfaces(
        tinyxml2::XMLElement* p_root,
        std::shared_ptr<fastdds::rtps::SocketTransportDescriptor> p_transport)
{
    /*
        <xs:complexType name="interfacesType">
            <xs:all>
                <xs:element name="allowlist" type="allowlistType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="blocklist" type="blocklistType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, ALLOWLIST) == 0)
        {
            if (XMLP_ret::XML_OK != parseXMLAllowlist(p_aux0, p_transport))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Failed to parse 'allowlist'.");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, BLOCKLIST) == 0)
        {
            if (XMLP_ret::XML_OK != parseXMLBlocklist(p_aux0, p_transport))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Failed to parse 'blocklist'.");
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found in 'interfaces'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::parseXMLAllowlist(
        tinyxml2::XMLElement* p_root,
        std::shared_ptr<fastdds::rtps::SocketTransportDescriptor> p_transport)
{
    /*
        <xs:complexType name="allowlistType">
            <xs:sequence minOccurs="0" maxOccurs="unbounded">
                <xs:element name="interface" minOccurs="0" maxOccurs="unbounded">
                    <xs:complexType>
                        <xs:attribute name="name" type="string" use="required"/>
                        <xs:attribute name="netmask_filter" type="netmaskFilterType" use="optional"/>
                    </xs:complexType>
                </xs:element>
            </xs:sequence>
        </xs:complexType>
     */
    static const char* INTERFACE_NAME = "interface";
    static const char* NAME_ATTR_NAME = "name";
    static const char* NETMASK_FILTER_ATTR_NAME = "netmask_filter";

    const tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, INTERFACE_NAME) == 0)
        {
            // Parse interface name (device/ip)
            std::string iface_name;
            auto iface_name_attr = p_aux0->FindAttribute(NAME_ATTR_NAME);
            if (nullptr != iface_name_attr)
            {
                iface_name = iface_name_attr->Value();
                if (iface_name.empty())
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER,
                            "Failed to parse 'allowlist' element. Attribute 'name' cannot be empty.");
                    return XMLP_ret::XML_ERROR;
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Failed to parse 'allowlist' element. Required attribute 'name' not found.");
                return XMLP_ret::XML_ERROR;
            }

            // Parse netmask filter
            fastdds::rtps::NetmaskFilterKind netmask_filter{fastdds::rtps::NetmaskFilterKind::AUTO};
            auto netmask_filter_attr = p_aux0->FindAttribute(NETMASK_FILTER_ATTR_NAME);
            if (nullptr != netmask_filter_attr)
            {
                try
                {
                    netmask_filter = fastdds::rtps::network::netmask_filter::string_to_netmask_filter_kind(
                        netmask_filter_attr->Value());
                }
                catch (const std::invalid_argument& e)
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER,
                            "Failed to parse 'allowlist' element. Invalid value found in 'netmask_filter' : " <<
                            e.what());
                    return XMLP_ret::XML_ERROR;
                }
            }
            // Add valid item to allowlist
            p_transport->interface_allowlist.emplace_back(iface_name, netmask_filter);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found in 'allowlist'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::parseXMLBlocklist(
        tinyxml2::XMLElement* p_root,
        std::shared_ptr<fastdds::rtps::SocketTransportDescriptor> p_transport)
{
    /*
        <xs:complexType name="blocklistType">
            <xs:sequence minOccurs="0" maxOccurs="unbounded">
                <xs:element name="interface" minOccurs="0" maxOccurs="unbounded">
                    <xs:complexType>
                        <xs:attribute name="name" type="string" use="required"/>
                    </xs:complexType>
                </xs:element>
            </xs:sequence>
       </xs:complexType>
     */
    static const char* INTERFACE_NAME = "interface";
    static const char* NAME_ATTR_NAME = "name";

    const tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, INTERFACE_NAME) == 0)
        {
            // Parse interface name (device/ip)
            auto iface = p_aux0->FindAttribute(NAME_ATTR_NAME);
            if (nullptr != iface)
            {
                std::string iface_name = iface->Value();
                if (iface_name.empty())
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER,
                            "Failed to parse 'blocklist' element. Attribute 'name' cannot be empty.");
                    return XMLP_ret::XML_ERROR;
                }
                // Add valid item to blocklist
                p_transport->interface_blocklist.emplace_back(iface_name);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Failed to parse 'blocklist' element. Required attribute 'name' not found.");
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found in 'blocklist'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::parseXMLCommonTCPTransportData(
        tinyxml2::XMLElement* p_root,
        sp_transport_t p_transport)
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
                <xs:element name="keep_alive_thread" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="accept_thread" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */

    XMLP_ret ret = XMLP_ret::XML_OK;
    std::shared_ptr<fastdds::rtps::TCPTransportDescriptor> pTCPDesc =
            std::dynamic_pointer_cast<fastdds::rtps::TCPTransportDescriptor>(p_transport);
    if (pTCPDesc != nullptr)
    {
        tinyxml2::XMLElement* p_aux0 = nullptr;
        const char* name = nullptr;
        for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
        {
            name = p_aux0->Name();
            if (strcmp(name, KEEP_ALIVE_FREQUENCY) == 0)
            {
                // keep_alive_frequency_ms - uint32Type
                int iFrequency(0);
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iFrequency, 0))
                {
                    return XMLP_ret::XML_ERROR;
                }
                pTCPDesc->keep_alive_frequency_ms = static_cast<uint32_t>(iFrequency);
            }
            else if (strcmp(name, KEEP_ALIVE_TIMEOUT) == 0)
            {
                // keep_alive_timeout_ms - uint32Type
                int iTimeout(0);
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iTimeout, 0))
                {
                    return XMLP_ret::XML_ERROR;
                }
                pTCPDesc->keep_alive_timeout_ms = static_cast<uint32_t>(iTimeout);
            }
            else if (strcmp(name, MAX_LOGICAL_PORT) == 0)
            {
                // max_logical_port - uint16Type
                int iPort(0);
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iPort, 0) || iPort < 0 || iPort > 65535)
                {
                    return XMLP_ret::XML_ERROR;
                }
                pTCPDesc->max_logical_port = static_cast<uint16_t>(iPort);
            }
            else if (strcmp(name, LOGICAL_PORT_RANGE) == 0)
            {
                // logical_port_range - uint16Type
                int iPort(0);
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iPort, 0) || iPort < 0 || iPort > 65535)
                {
                    return XMLP_ret::XML_ERROR;
                }
                pTCPDesc->logical_port_range = static_cast<uint16_t>(iPort);
            }
            else if (strcmp(name, LOGICAL_PORT_INCREMENT) == 0)
            {
                // logical_port_increment - uint16Type
                int iPort(0);
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iPort, 0) || iPort < 0 || iPort > 65535)
                {
                    return XMLP_ret::XML_ERROR;
                }
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
            // non_blocking_send - boolType
            else if (strcmp(name, NON_BLOCKING_SEND) == 0)
            {
                if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &pTCPDesc->non_blocking_send, 0))
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
                    {
                        return XMLP_ret::XML_ERROR;
                    }

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
            else if (strcmp(name, KEEP_ALIVE_THREAD) == 0)
            {
                if (getXMLThreadSettings(*p_aux0, pTCPDesc->keep_alive_thread) != XMLP_ret::XML_OK)
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Incorrect thread settings");
                    return XMLP_ret::XML_ERROR;
                }
            }
            else if (strcmp(name, ACCEPT_THREAD) == 0)
            {
                if (getXMLThreadSettings(*p_aux0, pTCPDesc->accept_thread) != XMLP_ret::XML_OK)
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Incorrect thread settings");
                    return XMLP_ret::XML_ERROR;
                }
            }
            else if (strcmp(name, TCP_NEGOTIATION_TIMEOUT) == 0)
            {
                // tcp_negotiation_timeout - uint32Type
                int iTimeout(0);
                if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &iTimeout, 0))
                {
                    return XMLP_ret::XML_ERROR;
                }
                pTCPDesc->tcp_negotiation_timeout = static_cast<uint32_t>(iTimeout);
            }
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing TCP Transport data");
        ret = XMLP_ret::XML_ERROR;
    }

    return ret;
}

XMLP_ret XMLParser::parseXMLCommonSharedMemTransportData(
        tinyxml2::XMLElement* p_root,
        sp_transport_t p_transport)
{
    /*
        <xs:complexType name="rtpsTransportDescriptorType">
            <xs:all minOccurs="0">
                <xs:element name="maxMessageSize" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="maxInitialPeersRange" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="segment_size" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="port_queue_capacity" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="healthy_check_timeout_ms" type="uint32Type" minOccurs="0" maxOccurs="1"/>
                <xs:element name="rtps_dump_file" type="stringType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="dump_thread" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */

    XMLP_ret ret = XMLP_ret::XML_OK;
    std::shared_ptr<fastdds::rtps::SharedMemTransportDescriptor> transport_descriptor =
            std::dynamic_pointer_cast<fastdds::rtps::SharedMemTransportDescriptor>(p_transport);
    if (transport_descriptor != nullptr)
    {
        tinyxml2::XMLElement* p_aux0 = nullptr;
        const char* name = nullptr;
        for (p_aux0 = p_root->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
        {
            uint32_t aux;
            name = p_aux0->Name();
            if (strcmp(name, SEGMENT_SIZE) == 0)
            {
                if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &aux, 0))
                {
                    return XMLP_ret::XML_ERROR;
                }
                transport_descriptor->segment_size(static_cast<uint32_t>(aux));
            }
            else if (strcmp(name, PORT_QUEUE_CAPACITY) == 0)
            {
                if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &aux, 0))
                {
                    return XMLP_ret::XML_ERROR;
                }
                transport_descriptor->port_queue_capacity(static_cast<uint32_t>(aux));
            }
            else if (strcmp(name, HEALTHY_CHECK_TIMEOUT_MS) == 0)
            {
                if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &aux, 0))
                {
                    return XMLP_ret::XML_ERROR;
                }
                transport_descriptor->healthy_check_timeout_ms(static_cast<uint32_t>(aux));
            }
            else if (strcmp(name, RTPS_DUMP_FILE) == 0)
            {
                std::string str;
                if (XMLP_ret::XML_OK != getXMLString(p_aux0, &str, 0))
                {
                    return XMLP_ret::XML_ERROR;
                }
                transport_descriptor->rtps_dump_file(str);
            }
            else if (strcmp(name, DUMP_THREAD) == 0)
            {
                fastdds::rtps::ThreadSettings thread_settings;
                if (getXMLThreadSettings(*p_aux0, thread_settings) != XMLP_ret::XML_OK)
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Incorrect thread settings");
                    return XMLP_ret::XML_ERROR;
                }
                transport_descriptor->dump_thread(thread_settings);
            }
            // Do not parse nor fail on unkown tags; these may be parsed elsewhere
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing SharedMem Transport data");
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
    using namespace rtps;
    using TCPDescriptor = std::shared_ptr<fastdds::rtps::TCPTransportDescriptor>;
    using TLSVerifyMode = fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSVerifyMode;
    using TLSOption = fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSOptions;
    using TLSHandShakeMode = fastdds::rtps::TCPTransportDescriptor::TLSConfig::TLSHandShakeRole;

    XMLP_ret ret = XMLP_ret::XML_OK;

    TCPDescriptor pTCPDesc = std::dynamic_pointer_cast<fastdds::rtps::TCPTransportDescriptor>(tcp_transport);
    pTCPDesc->apply_security = true;

    tinyxml2::XMLElement* p_aux0 = nullptr;

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
            tinyxml2::XMLElement* p_path = p_aux0->FirstChildElement();

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
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Unrecognized verify paths label: " << p_path->Value());
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
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing TLS configuration handshake_mode unrecognized "
                            << handshake_mode << ".");
                    ret = XMLP_ret::XML_ERROR;
                }
            }
        }
        else if (config.compare(TLS_SERVER_NAME) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &pTCPDesc->tls_config.server_name, 0))
            {
                ret = XMLP_ret::XML_ERROR;
            }
        }
        else if (config.compare(TLS_VERIFY_MODE) == 0)
        {
            tinyxml2::XMLElement* p_verify = p_aux0->FirstChildElement();
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
                            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing TLS configuration verify_mode unrecognized "
                                    << verify_mode << ".");
                            ret = XMLP_ret::XML_ERROR;
                        }
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing TLS configuration found unrecognized node "
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
            tinyxml2::XMLElement* p_option = p_aux0->FirstChildElement();
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
                            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing TLS configuration option unrecognized "
                                    << option << ".");
                            ret = XMLP_ret::XML_ERROR;
                        }
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing TLS options found unrecognized node "
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
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing TLS configuration: Field " << config << " not recognized.");
            ret = XMLP_ret::XML_ERROR;
        }

        // Stop parsing on error
        if (ret == XMLP_ret::XML_ERROR)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing TLS configuration's field '" << config << "'.");
            break;
        }
    }

    return ret;
}

XMLP_ret XMLParser::parseXMLReceptionThreads(
        tinyxml2::XMLElement& p_root,
        fastdds::rtps::PortBasedTransportDescriptor::ReceptionThreadsConfigMap& reception_threads)
{
    /*
        <xs:complexType name="receptionThreadsListType">
            <xs:sequence minOccurs="0" maxOccurs="unbounded">
                <xs:element name="reception_thread" type="threadSettingsType" minOccurs="0" maxOccurs="unbounded"/>
            </xs:sequence>
        </xs:complexType>
     */

    /*
     * The only allowed element is <reception_thread>
     */
    XMLP_ret ret = XMLP_ret::XML_OK;
    for (tinyxml2::XMLElement* p_element = p_root.FirstChildElement(); p_element != nullptr;
            p_element = p_element->NextSiblingElement())
    {
        if (strcmp(p_element->Name(), RECEPTION_THREAD) == 0)
        {
            uint32_t port = 0;
            fastdds::rtps::ThreadSettings thread_settings;
            ret = getXMLThreadSettingsWithPort(*p_element, thread_settings, port);
            if (XMLP_ret::XML_OK != ret || reception_threads.count(port) != 0)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing reception_threads thread settings. Port: " << port);
                ret = XMLP_ret::XML_ERROR;
                break;
            }
            reception_threads[port] = thread_settings;
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing reception_threads. Wrong tag: " << p_element->Name());
            ret = XMLP_ret::XML_ERROR;
            break;
        }
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLLibrarySettings(
        tinyxml2::XMLElement* p_root)
{
    /*
        <xs:complexType name="LibrarySettingsType">
            <xs:all minOccurs="0">
                <xs:element name="intraprocess_delivery" type="IntraprocessDeliveryType"/>
            </xs:all>
        </xs:complexType>
     */

    XMLP_ret ret = XMLP_ret::XML_OK;
    std::string sId = "";

    uint8_t ident = 1;
    tinyxml2::XMLElement* p_aux0 = nullptr;
    p_aux0 = p_root->FirstChildElement(INTRAPROCESS_DELIVERY);
    if (nullptr == p_aux0)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Not found '" << INTRAPROCESS_DELIVERY << "' attribute");
        return XMLP_ret::XML_ERROR;
    }
    else
    {
        fastdds::LibrarySettings library_settings;
        if (XMLP_ret::XML_OK != getXMLEnum(p_aux0, &library_settings.intraprocess_delivery, ident))
        {
            return XMLP_ret::XML_ERROR;
        }

        XMLProfileManager::library_settings(library_settings);
    }

    return ret;
}

XMLP_ret XMLParser::parseXMLDomainParticipantFactoryProf(
        tinyxml2::XMLElement* p_root,
        BaseNode& rootNode)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    up_participantfactory_t factory_qos{new fastdds::dds::DomainParticipantFactoryQos};
    up_node_participantfactory_t factory_node{new node_participantfactory_t{NodeType::DOMAINPARTICIPANT_FACTORY,
                                                                            std::move(factory_qos)}};
    if (XMLP_ret::XML_OK == fillDataNode(p_root, *factory_node))
    {
        rootNode.addChild(std::move(factory_node));
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing participant profile");
        ret = XMLP_ret::XML_ERROR;
    }

    return ret;
}

XMLP_ret XMLParser::parseXMLParticipantProf(
        tinyxml2::XMLElement* p_root,
        BaseNode& rootNode)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    up_participant_t participant_atts{new fastdds::xmlparser::ParticipantAttributes};
    up_node_participant_t participant_node{new node_participant_t{NodeType::PARTICIPANT, std::move(participant_atts)}};
    if (XMLP_ret::XML_OK == fillDataNode(p_root, *participant_node))
    {
        rootNode.addChild(std::move(participant_node));
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing participant profile");
        ret = XMLP_ret::XML_ERROR;
    }

    return ret;
}

XMLP_ret XMLParser::parseXMLPublisherProf(
        tinyxml2::XMLElement* p_root,
        BaseNode& rootNode)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    up_publisher_t publisher_atts{new fastdds::xmlparser::PublisherAttributes};
    up_node_publisher_t publisher_node{new node_publisher_t{NodeType::PUBLISHER, std::move(publisher_atts)}};
    if (XMLP_ret::XML_OK == fillDataNode(p_root, *publisher_node))
    {
        rootNode.addChild(std::move(publisher_node));
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing publisher profile");
        ret = XMLP_ret::XML_ERROR;
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLSubscriberProf(
        tinyxml2::XMLElement* p_root,
        BaseNode& rootNode)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    up_subscriber_t subscriber_atts{new fastdds::xmlparser::SubscriberAttributes};
    up_node_subscriber_t subscriber_node{new node_subscriber_t{NodeType::SUBSCRIBER, std::move(subscriber_atts)}};
    if (XMLP_ret::XML_OK == fillDataNode(p_root, *subscriber_node))
    {
        rootNode.addChild(std::move(subscriber_node));
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing subscriber profile");
        ret = XMLP_ret::XML_ERROR;
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLTopicData(
        tinyxml2::XMLElement* p_root,
        BaseNode& rootNode)
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
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing topic data node");
        ret = XMLP_ret::XML_ERROR;
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLRequesterProf(
        tinyxml2::XMLElement* p_root,
        BaseNode& rootNode)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    up_requester_t requester_atts{new fastdds::xmlparser::RequesterAttributes};
    up_node_requester_t requester_node{new node_requester_t{NodeType::REQUESTER, std::move(requester_atts)}};
    if (XMLP_ret::XML_OK == fillDataNode(p_root, *requester_node))
    {
        rootNode.addChild(std::move(requester_node));
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing requester profile");
        ret = XMLP_ret::XML_ERROR;
    }
    return ret;
}

XMLP_ret XMLParser::parseXMLReplierProf(
        tinyxml2::XMLElement* p_root,
        BaseNode& rootNode)
{
    XMLP_ret ret = XMLP_ret::XML_OK;
    up_replier_t replier_atts{new fastdds::xmlparser::ReplierAttributes};
    up_node_replier_t replier_node{new node_replier_t{NodeType::REPLIER, std::move(replier_atts)}};
    if (XMLP_ret::XML_OK == fillDataNode(p_root, *replier_node))
    {
        rootNode.addChild(std::move(replier_node));
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing replier profile");
        ret = XMLP_ret::XML_ERROR;
    }
    return ret;
}

XMLP_ret XMLParser::parseProfiles(
        tinyxml2::XMLElement* p_root,
        BaseNode& profilesNode)
{
    /*
        <xs:element name="profiles">
            <xs:complexType>
                <xs:sequence>
                    <xs:element name="library_settings" type="LibrarySettingsType" minOccurs="0" maxOccurs="unbounded"/>
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
    XMLP_ret ret = XMLP_ret::XML_OK;
    while (nullptr != p_profile)
    {
        if (nullptr != (tag = p_profile->Value()))
        {
            // If profile parsing functions fails, log and continue.
            if (strcmp(tag, TRANSPORT_DESCRIPTORS) == 0)
            {
                parseOk &= parseXMLTransportsProf(p_profile) == XMLP_ret::XML_OK;
            }
            else if (strcmp(tag, LIBRARY_SETTINGS) == 0)
            {
                parseOk &= parseXMLLibrarySettings(p_profile) == XMLP_ret::XML_OK;
            }
            else if (strcmp(tag, DOMAINPARTICIPANT_FACTORY) == 0)
            {
                parseOk &= parseXMLDomainParticipantFactoryProf(p_profile, profilesNode) == XMLP_ret::XML_OK;
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
            else if (strcmp(tag, REQUESTER) == 0)
            {
                parseOk &= parseXMLRequesterProf(p_profile, profilesNode) == XMLP_ret::XML_OK;
            }
            else if (strcmp(tag, REPLIER) == 0)
            {
                parseOk &= parseXMLReplierProf(p_profile, profilesNode) == XMLP_ret::XML_OK;
            }
            else if (strcmp(tag, QOS_PROFILE) == 0)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Field 'QOS_PROFILE' do not supported for now");
            }
            else if (strcmp(tag, APPLICATION) == 0)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Field 'APPLICATION' do not supported for now");
            }
            else if (strcmp(tag, TYPE) == 0)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Field 'TYPE' do not supported for now");
            }
            else
            {
                parseOk = false;
                EPROSIMA_LOG_ERROR(XMLPARSER, "Not expected tag: '" << tag << "'");
            }
        }

        if (!parseOk)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing profile's tag " << tag);
            ret = XMLP_ret::XML_ERROR;
        }
        p_profile = p_profile->NextSiblingElement();
    }
    return ret;
}

XMLP_ret XMLParser::parseLogConfig(
        tinyxml2::XMLElement* p_root)
{
    /*
        <xs:element name="log" type="logType"/>
        <xs:complexType name="logType">
            <xs:sequence minOccurs="1" maxOccurs="unbounded">
                <xs:choice minOccurs="1">
                    <xs:element name="use_default" type="booleanCaps" minOccurs="0" maxOccurs="1"/>
                    <xs:element name="consumer" type="logConsumerType" minOccurs="0" maxOccurs="unbounded"/>
                    <xs:element name="thread_settings" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
                </xs:choice>
            </xs:sequence>
        </xs:complexType>
     */

    /*
     * TODO(eduponz): Uphold XSD validation in parsing
     *   Even though the XSD above enforces the log tag to have at least one consumer,
     *   the parsing allows for an empty log tag (e.g. `<log></log>`).
     *   This inconsistency is kept to keep a backwards compatible behaviour.
     *   In fact, test XMLParserTests.parseXMLNoRoot even checks that an empty log tag
     *   is valid.
     */

    XMLP_ret ret = XMLP_ret::XML_OK;

    tinyxml2::XMLElement* p_aux0 = p_root->FirstChildElement(LOG);
    if (p_aux0 == nullptr)
    {
        p_aux0 = p_root;
    }

    std::set<std::string> tags_present;
    tinyxml2::XMLElement* p_element = p_aux0->FirstChildElement();
    fastdds::rtps::ThreadSettings thread_settings;
    bool set_thread_settings = false;

    while (ret == XMLP_ret::XML_OK && nullptr != p_element)
    {
        const char* name = p_element->Name();
        const char* tag = p_element->Value();

        // Fail on duplicated not allowed elements
        if (strcmp(tag, CONSUMER) != 0 && tags_present.count(name) != 0)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'log'. Tag: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present.emplace(name);

        if (nullptr != tag)
        {
            if (strcmp(tag, USE_DEFAULT) == 0)
            {
                std::string auxBool = get_element_text(p_element);
                if (auxBool.empty())
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Cannot get text from tag: '" << tag << "'");
                    ret = XMLP_ret::XML_ERROR;
                }

                if (ret == XMLP_ret::XML_OK)
                {
                    bool use_default = true;
                    if (std::strcmp(auxBool.c_str(), "FALSE") == 0)
                    {
                        use_default = false;
                    }
                    if (!use_default)
                    {
                        eprosima::fastdds::dds::Log::ClearConsumers();
                    }
                }
            }
            else if (strcmp(tag, CONSUMER) == 0)
            {
                ret = parseXMLConsumer(*p_element);
            }
            else if (strcmp(tag, THREAD_SETTINGS) == 0)
            {
                ret = getXMLThreadSettings(*p_element, thread_settings);
                if (ret == XMLP_ret::XML_OK)
                {
                    set_thread_settings = true;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Incorrect thread settings");
                }
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Not expected tag: '" << tag << "'");
                ret = XMLP_ret::XML_ERROR;
            }
        }

        if (ret == XMLP_ret::XML_OK)
        {
            p_element = p_element->NextSiblingElement();
        }
    }

    if (ret == XMLP_ret::XML_OK && set_thread_settings)
    {
        fastdds::dds::Log::SetThreadConfig(thread_settings);
    }

    return ret;
}

XMLP_ret XMLParser::parseXMLConsumer(
        tinyxml2::XMLElement& consumer)
{
    using namespace eprosima::fastdds::dds;

    XMLP_ret ret = XMLP_ret::XML_OK;
    tinyxml2::XMLElement* p_element = consumer.FirstChildElement(CLASS);

    if (p_element != nullptr)
    {
        std::string classStr = get_element_text(p_element);

        if (std::strcmp(classStr.c_str(), "StdoutConsumer") == 0)
        {
            Log::RegisterConsumer(std::unique_ptr<LogConsumer>(new StdoutConsumer));
        }
        else if (std::strcmp(classStr.c_str(), "StdoutErrConsumer") == 0)
        {
            /* Register a StdoutErrConsumer */

            // Get first property
            tinyxml2::XMLElement* property = consumer.FirstChildElement(PROPERTY);
            if (nullptr == property)
            {
                // If no properties are specified, create the consumer with default values
                Log::RegisterConsumer(std::unique_ptr<LogConsumer>(new StdoutErrConsumer));
            }
            else
            {
                // Only one property is supported. Its name is `stderr_threshold`, and its value is a log kind specified
                // as a string in the form `Log::Kind::<Kind>`.
                tinyxml2::XMLElement* p_auxName = nullptr;    // Property name
                tinyxml2::XMLElement* p_auxValue = nullptr;   // Property value
                uint8_t stderr_threshold_property_count = 0;  // Occurrences count. Only one is allowed

                // Get default threshold
                Log::Kind threshold = StdoutErrConsumer::STDERR_THRESHOLD_DEFAULT;

                // Iterate over the properties
                while (nullptr != property)
                {
                    if (nullptr != (p_auxName = property->FirstChildElement(NAME)))
                    {
                        // Get property name
                        std::string s = get_element_text(p_auxName);

                        if (std::strcmp(s.c_str(), "stderr_threshold") == 0)
                        {
                            /* Property is a `stderr_threshold` */

                            // Update occurrence count and check how many encountered. Only the first one applies, the
                            // rest are ignored.
                            stderr_threshold_property_count++;
                            if (stderr_threshold_property_count > 1)
                            {
                                // Continue with the next property if `stderr_threshold` had been already specified.
                                EPROSIMA_LOG_ERROR(XMLParser, classStr << " only supports one occurrence of 'stderr_threshold'."
                                                                       << " Only the first one is applied.");
                                property = property->NextSiblingElement(PROPERTY);
                                ret = XMLP_ret::XML_NOK;
                                continue;
                            }

                            // Get the property value. It should be a Log::Kind.
                            if (nullptr != (p_auxValue = property->FirstChildElement(VALUE)))
                            {
                                // Get property value and use it to set the threshold.
                                std::string threshold_str = get_element_text(p_auxValue);
                                if (std::strcmp(threshold_str.c_str(), "Log::Kind::Error") == 0)
                                {
                                    threshold = Log::Kind::Error;
                                }
                                else if (std::strcmp(threshold_str.c_str(), "Log::Kind::Warning") == 0)
                                {
                                    threshold = Log::Kind::Warning;
                                }
                                else if (std::strcmp(threshold_str.c_str(), "Log::Kind::Info") == 0)
                                {
                                    threshold = Log::Kind::Info;
                                }
                                else
                                {
                                    EPROSIMA_LOG_ERROR(XMLParser, "Unkown Log::Kind '" << threshold_str
                                                                                       << "'. Using default threshold.");
                                    ret = XMLP_ret::XML_NOK;
                                }
                            }
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(XMLParser, "Unkown property value '" << s << "' in " << classStr
                                                                                    << " log consumer");
                            ret = XMLP_ret::XML_NOK;
                        }
                    }
                    // Continue with the next property
                    property = property->NextSiblingElement(PROPERTY);
                }

                // Create consumer with the specified `stderr_threshold` and register it.
                StdoutErrConsumer* log_consumer = new StdoutErrConsumer;
                log_consumer->stderr_threshold(threshold);
                Log::RegisterConsumer(std::unique_ptr<LogConsumer>(log_consumer));
            }
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
                        std::string s = get_element_text(p_auxName);

                        if (std::strcmp(s.c_str(), "filename") == 0)
                        {
                            if (nullptr == (p_auxValue = property->FirstChildElement(VALUE)) ||
                                    !get_element_text(p_auxValue, outputFile))
                            {
                                EPROSIMA_LOG_ERROR(XMLParser, "Filename value cannot be found for " << classStr
                                                                                                    << " log consumer.");
                                ret = XMLP_ret::XML_NOK;
                            }
                        }
                        else if (std::strcmp(s.c_str(), "append") == 0)
                        {
                            std::string auxBool;
                            if (nullptr != (p_auxValue = property->FirstChildElement(VALUE)) &&
                                    get_element_text(p_auxValue, auxBool))
                            {
                                if (std::strcmp(auxBool.c_str(), "TRUE") == 0)
                                {
                                    append = true;
                                }
                            }
                            else
                            {
                                EPROSIMA_LOG_ERROR(XMLParser, "Append value cannot be found for " << classStr
                                                                                                  << " log consumer.");
                                ret = XMLP_ret::XML_NOK;
                            }
                        }
                        else
                        {
                            EPROSIMA_LOG_ERROR(XMLParser, "Unknown property " << s << " in " << classStr
                                                                              << " log consumer.");
                            ret = XMLP_ret::XML_NOK;
                        }
                    }
                    property = property->NextSiblingElement(PROPERTY);
                }

                Log::RegisterConsumer(std::unique_ptr<LogConsumer>(new FileConsumer(outputFile, append)));
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLParser, "Unknown log consumer class: " << classStr);
            ret = XMLP_ret::XML_ERROR;
        }
    }

    return ret;
}

XMLP_ret XMLParser::loadXML(
        const std::string& filename,
        up_base_node_t& root)
{
    return loadXML(filename, root, false);
}

XMLP_ret XMLParser::loadXML(
        const std::string& filename,
        up_base_node_t& root,
        bool is_default)
{
    if (filename.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error loading XML file, filename empty");
        return XMLP_ret::XML_ERROR;
    }

    tinyxml2::XMLDocument xmlDoc;
    if (tinyxml2::XMLError::XML_SUCCESS != xmlDoc.LoadFile(filename.c_str()))
    {
        if (!is_default)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error opening '" << filename << "'");
        }
        return XMLP_ret::XML_ERROR;
    }

    EPROSIMA_LOG_INFO(XMLPARSER, "File '" << filename << "' opened successfully");
    return parseXML(xmlDoc, root);
}

XMLP_ret XMLParser::loadXMLProfiles(
        tinyxml2::XMLElement& xmlDoc,
        up_base_node_t& root)
{
    return parseXMLProfiles(xmlDoc, root);
}

XMLP_ret XMLParser::loadXML(
        tinyxml2::XMLDocument& xmlDoc,
        up_base_node_t& root)
{
    return parseXML(xmlDoc, root);
}

XMLP_ret XMLParser::loadXML(
        const char* data,
        size_t length,
        up_base_node_t& root)
{
    tinyxml2::XMLDocument xmlDoc;
    if (tinyxml2::XMLError::XML_SUCCESS != xmlDoc.Parse(data, length))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing XML buffer");
        return XMLP_ret::XML_ERROR;
    }
    return parseXML(xmlDoc, root);
}

template <typename T>
void XMLParser::addAllAttributes(
        tinyxml2::XMLElement* p_profile,
        DataNode<T>& node)
{
    const tinyxml2::XMLAttribute* attrib;
    for (attrib = p_profile->FirstAttribute(); attrib != nullptr; attrib = attrib->Next())
    {
        node.addAttribute(attrib->Name(), attrib->Value());
    }
}

XMLP_ret XMLParser::fillDataNode(
        tinyxml2::XMLElement* node,
        DataNode<TopicAttributes>& topic_node)
{
    if (nullptr == node)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Bad parameters!");
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

XMLP_ret XMLParser::fillDataNode(
        tinyxml2::XMLElement* p_profile,
        DataNode<fastdds::dds::DomainParticipantFactoryQos>& factory_node)
{
    /*
       <xs:complexType name="domainParticipantFactoryProfileType">
        <xs:all>
            <xs:element name="qos" type="domainParticipantFactoryQosPoliciesType" minOccurs="0" maxOccurs="1"/>
        </xs:all>
        <xs:attribute name="profile_name" type="string" use="required"/>
        <xs:attribute name="is_default_profile" type="boolean" use="optional"/>
       </xs:complexType>
     */

    if (nullptr == p_profile)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, factory_node);

    /*
     * The only allowed element <qos>, and its max is 1; look for it.
     */
    std::set<std::string> tags_present;
    for (tinyxml2::XMLElement* p_element = p_profile->FirstChildElement(); p_element != nullptr;
            p_element = p_element->NextSiblingElement())
    {
        const char* name = p_element->Name();
        if (tags_present.count(name) != 0)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'participant'. Tag: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present.emplace(name);

        if (strcmp(p_element->Name(), QOS) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLDomainParticipantFactoryQos(*p_element, *factory_node.get()))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::fillDataNode(
        tinyxml2::XMLElement* p_profile,
        DataNode<fastdds::xmlparser::ParticipantAttributes>& participant_node)
{
    /*
        <xs:complexType name="participantProfileType">
            <xs:all>
                <xs:element name="domainId" type="domainIDType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="rtps" minOccurs="0" maxOccurs="1">
                    <xs:complexType>
                        <xs:all>
                            <xs:element name="name" type="string" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="defaultUnicastLocatorList" type="locatorListType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="defaultMulticastLocatorList" type="locatorListType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="default_external_unicast_locators" type="externalLocatorListType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="ignore_non_matching_locators" type="boolean" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="sendSocketBufferSize" type="uint32" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="listenSocketBufferSize" type="uint32" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="netmask_filter" minOccurs="0" maxOccurs="1">
                                <xs:simpleType>
                                    <xs:restriction base="xs:string">
                                        <xs:enumeration value="OFF"/>
                                        <xs:enumeration value="AUTO"/>
                                        <xs:enumeration value="ON"/>
                                    </xs:restriction>
                                </xs:simpleType>
                            </xs:element>
                            <xs:element name="builtin" type="builtinAttributesType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="port" type="portType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="participantID" type="int32" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="easy_mode_ip" type="string" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="userTransports" minOccurs="0" maxOccurs="1">
                                <xs:complexType>
                                    <xs:sequence>
                                        <xs:element name="transport_id" type="string" minOccurs="1" maxOccurs="unbounded"/>
                                    </xs:sequence>
                                </xs:complexType>
                            </xs:element>
                            <xs:element name="useBuiltinTransports" type="boolean" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="builtinTransports" type="builtinTransportsType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="propertiesPolicy" type="propertyPolicyType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="allocation" type="rtpsParticipantAllocationAttributesType"  minOccurs="0" maxOccurs="1"/>
                            <xs:element name="userData" type="octectVectorQosPolicyType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="prefix" type="prefixType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="flow_controller_descriptor_list" type="flowControllerDescriptorListType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="builtin_controllers_sender_thread" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="timed_events_thread" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="discovery_server_thread" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="typelookup_service_thread" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="builtin_transports_reception_threads" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
                            <xs:element name="security_log_thread" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
                        </xs:all>
                    </xs:complexType>
                </xs:element>
            </xs:all>
        </xs:complexType>
     */

    if (nullptr == p_profile)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, participant_node);

    uint8_t ident = 1;
    tinyxml2::XMLElement* p_element;
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    std::set<std::string> tags_present;

    /*
     * The only allowed elements are <domainId> and <rtps>
     *   - The min occurrences of <domainId> are 0, and its max is 1; look for it.
     *   - The min occurrences of <rtps> are 0, and its max is 1; look for it.
     */
    for (p_element = p_profile->FirstChildElement(); p_element != nullptr; p_element = p_element->NextSiblingElement())
    {
        name = p_element->Name();
        if (tags_present.count(name) != 0)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'participant'. Tag: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present.emplace(name);

        if (strcmp(p_element->Name(), DOMAIN_ID) == 0)
        {
            // domainId - uint32
            if (XMLP_ret::XML_OK != getXMLUint(p_element, &participant_node.get()->domainId, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(p_element->Name(), RTPS) == 0)
        {
            p_aux0 = p_element;
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Found incorrect tag '" << p_element->Name() << "'");
            return XMLP_ret::XML_ERROR;
        }
    }
    tags_present.clear();

    // <rtps> is not present, but that's OK
    if (nullptr == p_aux0)
    {
        return XMLP_ret::XML_OK;
    }

    // Check contents of <rtps>
    for (p_aux0 = p_aux0->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();

        if (tags_present.count(name) != 0)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Duplicated element found in 'rtpsParticipantAttributesType'. Tag: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present.emplace(name);

        if (strcmp(name, ALLOCATION) == 0)
        {
            // allocation
            if (XMLP_ret::XML_OK !=
                    getXMLParticipantAllocationAttributes(p_aux0, participant_node.get()->rtps.allocation, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PREFIX) == 0)
        {
            // prefix
            if (XMLP_ret::XML_OK !=
                    getXMLguidPrefix(p_aux0, participant_node.get()->rtps.prefix, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, IGN_NON_MATCHING_LOCS) == 0)
        {
            // ignore_non_matching_locators - boolean
            if (XMLP_ret::XML_OK !=
                    getXMLBool(p_aux0, &participant_node.get()->rtps.ignore_non_matching_locators, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DEF_EXT_UNI_LOC_LIST) == 0)
        {
            // default_external_unicast_locators - externalLocatorListType
            if (XMLP_ret::XML_OK !=
                    getXMLExternalLocatorList(p_aux0, participant_node.get()->rtps.default_external_unicast_locators,
                    ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DEF_UNI_LOC_LIST) == 0)
        {
            // defaultUnicastLocatorList
            if (XMLP_ret::XML_OK !=
                    getXMLLocatorList(p_aux0, participant_node.get()->rtps.defaultUnicastLocatorList, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DEF_MULTI_LOC_LIST) == 0)
        {
            // defaultMulticastLocatorList
            if (XMLP_ret::XML_OK !=
                    getXMLLocatorList(p_aux0, participant_node.get()->rtps.defaultMulticastLocatorList, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, SEND_SOCK_BUF_SIZE) == 0)
        {
            // sendSocketBufferSize - uint32
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &participant_node.get()->rtps.sendSocketBufferSize, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, LIST_SOCK_BUF_SIZE) == 0)
        {
            // listenSocketBufferSize - uint32
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &participant_node.get()->rtps.listenSocketBufferSize, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, NETMASK_FILTER) == 0)
        {
            std::string netmask_filter_str;
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &netmask_filter_str, 0))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'netmask_filter'.");
                return XMLP_ret::XML_ERROR;
            }

            try
            {
                participant_node.get()->rtps.netmaskFilter =
                        fastdds::rtps::network::netmask_filter::string_to_netmask_filter_kind(netmask_filter_str);
            }
            catch (const std::invalid_argument& e)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'netmask_filter' : " << e.what());
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, BUILTIN) == 0)
        {
            // builtin
            if (XMLP_ret::XML_OK != getXMLBuiltinAttributes(p_aux0, participant_node.get()->rtps.builtin, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PORT) == 0)
        {
            // port
            if (XMLP_ret::XML_OK != getXMLPortParameters(p_aux0, participant_node.get()->rtps.port, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (0 == strcmp(name, USER_DATA))
        {
            // userData
            if (XMLP_ret::XML_OK != getXMLOctetVector(p_aux0, participant_node.get()->rtps.userData, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PART_ID) == 0)
        {
            // participantID - int32
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &participant_node.get()->rtps.participantID, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, EASY_MODE_IP) == 0)
        {
            // easy_mode_ip - string
            std::string str_aux;
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &str_aux, ident))
            {
                return XMLP_ret::XML_ERROR;
            }

            // Check that the string is a valid IPv4 address
            if (!fastdds::rtps::IPLocator::isIPv4(str_aux))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "'easy_mode_ip' is not a valid IPv4 address.");
                return XMLP_ret::XML_ERROR;
            }

            participant_node.get()->rtps.easy_mode_ip = str_aux;
        }
        else if (strcmp(name, FLOW_CONTROLLER_DESCRIPTOR_LIST) == 0)
        {
            // flow_controller_descriptors
            if (XMLP_ret::XML_OK !=
                    getXMLFlowControllerDescriptorList(p_aux0, participant_node.get()->rtps.flow_controllers, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, USER_TRANS) == 0)
        {
            // userTransports
            if (XMLP_ret::XML_OK != getXMLTransports(p_aux0, participant_node.get()->rtps.userTransports, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, USE_BUILTIN_TRANS) == 0)
        {
            // useBuiltinTransports - boolean
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &participant_node.get()->rtps.useBuiltinTransports, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, BUILTIN_TRANS) == 0)
        {
            // builtinTransports
            eprosima::fastdds::rtps::BuiltinTransports bt;
            eprosima::fastdds::rtps::BuiltinTransportsOptions bt_opts;
            if (XMLP_ret::XML_OK != getXMLBuiltinTransports(p_aux0, &bt, &bt_opts, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            participant_node.get()->rtps.setup_transports(bt, bt_opts);
        }
        else if (strcmp(name, PROPERTIES_POLICY) == 0)
        {
            // propertiesPolicy
            if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux0, participant_node.get()->rtps.properties, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, NAME) == 0)
        {
            // name - string
            std::string s;
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            participant_node.get()->rtps.setName(s.c_str());
        }
        else if (strcmp(name, BUILTIN_CONTROLLERS_SENDER_THREAD) == 0)
        {
            if (XMLP_ret::XML_OK !=
                    getXMLThreadSettings(*p_aux0,
                    participant_node.get()->rtps.builtin_controllers_sender_thread))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, TIMED_EVENTS_THREAD) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLThreadSettings(*p_aux0, participant_node.get()->rtps.timed_events_thread))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DISCOVERY_SERVER_THREAD) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLThreadSettings(*p_aux0, participant_node.get()->rtps.discovery_server_thread))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, TYPELOOKUP_SERVICE_THREAD) == 0)
        {
            if (XMLP_ret::XML_OK !=
                    getXMLThreadSettings(*p_aux0, participant_node.get()->rtps.typelookup_service_thread))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, BUILTIN_TRANSPORTS_RECEPTION_THREADS) == 0)
        {
            if (XMLP_ret::XML_OK !=
                    getXMLThreadSettings(*p_aux0,
                    participant_node.get()->rtps.builtin_transports_reception_threads))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, SECURITY_LOG_THREAD) == 0)
        {
#if HAVE_SECURITY
            if (XMLP_ret::XML_OK != getXMLThreadSettings(*p_aux0, participant_node.get()->rtps.security_log_thread))
            {
                return XMLP_ret::XML_ERROR;
            }
#else
            EPROSIMA_LOG_WARNING(XMLPARSER, "Ignoring '" << SECURITY_LOG_THREAD << "' since security is disabled");
#endif // if HAVE_SECURITY
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'rtpsParticipantAttributesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::fillDataNode(
        tinyxml2::XMLElement* p_profile,
        DataNode<fastdds::xmlparser::PublisherAttributes>& publisher_node)
{
    if (nullptr == p_profile)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, publisher_node);

    uint8_t ident = 1;
    if (XMLP_ret::XML_OK != getXMLPublisherAttributes(p_profile, *publisher_node.get(), ident))
    {
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::fillDataNode(
        tinyxml2::XMLElement* p_profile,
        DataNode<fastdds::xmlparser::SubscriberAttributes>& subscriber_node)
{
    if (nullptr == p_profile)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, subscriber_node);

    uint8_t ident = 1;
    if (XMLP_ret::XML_OK != getXMLSubscriberAttributes(p_profile, *subscriber_node.get(), ident))
    {
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::fillDataNode(
        tinyxml2::XMLElement* p_profile,
        DataNode<fastdds::xmlparser::RequesterAttributes>& requester_node)
{
    /*
        <xs:complexType name="replierRequesterProfileType">
            <xs:all minOccurs="0">
                <xs:element name="request_topic_name" type="string" minOccurs="0"/>
                <xs:element name="reply_topic_name" type="string" minOccurs="0"/>
                <xs:element name="data_writer" type="publisherProfileNoAttributesType" minOccurs="0"/>
                <xs:element name="data_reader" type="subscriberProfileNoAttributesType" minOccurs="0"/>
            </xs:all>
            <xs:attribute name="profile_name" type="string" use="required"/>
            <xs:attribute name="service_name" type="string" use="required"/>
            <xs:attribute name="request_type" type="string" use="required"/>
            <xs:attribute name="reply_type" type="string" use="required"/>
        </xs:complexType>
     */

    if (nullptr == p_profile)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, requester_node);
    auto found_attributes = requester_node.getAttributes();

    auto it_attributes = found_attributes.find(SERVICE_NAME);
    if (found_attributes.end() != it_attributes)
    {
        requester_node.get()->service_name = it_attributes->second;
        requester_node.get()->request_topic_name = it_attributes->second + "_Request";
        requester_node.get()->reply_topic_name = it_attributes->second + "_Reply";
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Not found required attribute " << SERVICE_NAME);
        return XMLP_ret::XML_ERROR;
    }

    it_attributes = found_attributes.find(REQUEST_TYPE);
    if (found_attributes.end() != it_attributes)
    {
        requester_node.get()->request_type = it_attributes->second;
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Not found required attribute " << REQUEST_TYPE);
        return XMLP_ret::XML_ERROR;
    }

    it_attributes = found_attributes.find(REPLY_TYPE);
    if (found_attributes.end() != it_attributes)
    {
        requester_node.get()->reply_type = it_attributes->second;
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Not found required attribute " << REPLY_TYPE);
        return XMLP_ret::XML_ERROR;
    }

    uint8_t ident = 1;
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;

    for (p_aux0 = p_profile->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, REQUEST_TOPIC_NAME) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &requester_node.get()->request_topic_name, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, REPLY_TOPIC_NAME) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &requester_node.get()->reply_topic_name, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PUBLISHER) == 0 || strcmp(name, DATA_WRITER) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLPublisherAttributes(p_aux0, requester_node.get()->publisher, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, SUBSCRIBER) == 0 || strcmp(name, DATA_READER) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLSubscriberAttributes(p_aux0, requester_node.get()->subscriber, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Not expected tag: '" << name << "'");
            return XMLP_ret::XML_ERROR;
        }

    }

    requester_node.get()->publisher.topic.topicDataType = requester_node.get()->request_type;
    requester_node.get()->publisher.topic.topicName = requester_node.get()->request_topic_name;

    requester_node.get()->subscriber.topic.topicDataType = requester_node.get()->reply_type;
    requester_node.get()->subscriber.topic.topicName = requester_node.get()->reply_topic_name;

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::fillDataNode(
        tinyxml2::XMLElement* p_profile,
        DataNode<fastdds::xmlparser::ReplierAttributes>& replier_node)
{
    /*
        <xs:complexType name="replierRequesterProfileType">
            <xs:all minOccurs="0">
                <xs:element name="request_topic_name" type="string" minOccurs="0"/>
                <xs:element name="reply_topic_name" type="string" minOccurs="0"/>
                <xs:element name="data_writer" type="publisherProfileNoAttributesType" minOccurs="0"/>
                <xs:element name="data_reader" type="subscriberProfileNoAttributesType" minOccurs="0"/>
            </xs:all>
            <xs:attribute name="profile_name" type="string" use="required"/>
            <xs:attribute name="service_name" type="string" use="required"/>
            <xs:attribute name="request_type" type="string" use="required"/>
            <xs:attribute name="reply_type" type="string" use="required"/>
        </xs:complexType>
     */

    if (nullptr == p_profile)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, replier_node);
    auto found_attributes = replier_node.getAttributes();

    auto it_attributes = found_attributes.find(SERVICE_NAME);
    if (found_attributes.end() != it_attributes)
    {
        replier_node.get()->service_name = it_attributes->second;
        replier_node.get()->request_topic_name = it_attributes->second + "_Request";
        replier_node.get()->reply_topic_name = it_attributes->second + "_Reply";
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Not found required attribute " << SERVICE_NAME);
        return XMLP_ret::XML_ERROR;
    }

    it_attributes = found_attributes.find(REQUEST_TYPE);
    if (found_attributes.end() != it_attributes)
    {
        replier_node.get()->request_type = it_attributes->second;
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Not found required attribute " << REQUEST_TYPE);
        return XMLP_ret::XML_ERROR;
    }

    it_attributes = found_attributes.find(REPLY_TYPE);
    if (found_attributes.end() != it_attributes)
    {
        replier_node.get()->reply_type = it_attributes->second;
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Not found required attribute " << REPLY_TYPE);
        return XMLP_ret::XML_ERROR;
    }

    uint8_t ident = 1;
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;

    for (p_aux0 = p_profile->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, REQUEST_TOPIC_NAME) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &replier_node.get()->request_topic_name, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, REPLY_TOPIC_NAME) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &replier_node.get()->reply_topic_name, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PUBLISHER) == 0 || strcmp(name, DATA_WRITER) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLPublisherAttributes(p_aux0, replier_node.get()->publisher, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, SUBSCRIBER) == 0 || strcmp(name, DATA_READER) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLSubscriberAttributes(p_aux0, replier_node.get()->subscriber, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Not expected tag: '" << name << "'");
            return XMLP_ret::XML_ERROR;
        }
    }

    replier_node.get()->subscriber.topic.topicDataType = replier_node.get()->request_type;
    replier_node.get()->subscriber.topic.topicName = replier_node.get()->request_topic_name;

    replier_node.get()->publisher.topic.topicDataType = replier_node.get()->reply_type;
    replier_node.get()->publisher.topic.topicName = replier_node.get()->reply_topic_name;

    return XMLP_ret::XML_OK;
}

} // namespace xmlparser
} // namespace fastdds
} // namespace eprosima
