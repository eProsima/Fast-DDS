// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file XMLParserExtras.cpp
 */

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <tinyxml2.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/transport/udp_tsn/UDPPriorityMappings.hpp>
#include <fastdds/rtps/transport/udp_tsn/TSN_UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/udp_tsn/TSN_UDPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/ethernet/EthernetLocator.hpp>
#include <fastdds/rtps/transport/ethernet/EthernetTransportDescriptor.hpp>
#include <fastdds/rtps/transport/low-bandwidth/HeaderReductionTransportDescriptor.hpp>
#include <fastdds/rtps/transport/low-bandwidth/PayloadCompressionTransportDescriptor.hpp>
#include <fastdds/rtps/transport/low-bandwidth/SourceTimestampTransportDescriptor.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>

#include <xmlparser/XMLParser.h>
#include <xmlparser/XMLParserCommon.h>
#include <xmlparser/XMLParserUtils.hpp>
#include <xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace fastdds {
namespace xmlparser {

using namespace eprosima::fastdds::xml::detail;

template<typename T>
static XMLP_ret process_unsigned_attribute(
        const tinyxml2::XMLElement* elem,
        const char* name,
        T& value,
        const bool required,
        const unsigned int min_value = std::numeric_limits<T>::min(),
        const unsigned int max_value = std::numeric_limits<T>::max())
{
    auto attribute = elem->FindAttribute(name);
    if (nullptr != attribute)
    {
        unsigned int v = 0;
        if (tinyxml2::XMLError::XML_SUCCESS == attribute->QueryUnsignedValue(&v))
        {
            if (min_value <= v && v <= max_value)
            {
                value = static_cast<T>(v);
                return XMLP_ret::XML_OK;
            }
        }

        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Value '" << attribute->Value() << "' for attribute '" << name << "' on '" << elem->Name()
                          << "' not in range [" << min_value << ", " << max_value << "]");
        return XMLP_ret::XML_ERROR;
    }

    if (required)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Missing mandatory attribute '" << name << "' on element '" << elem->Name() << "'");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

template<typename T>
static XMLP_ret process_signed_attribute(
        const tinyxml2::XMLElement* elem,
        const char* name,
        T& value,
        const bool required,
        const int min_value = std::numeric_limits<T>::min(),
        const int max_value = std::numeric_limits<T>::max())
{
    auto attribute = elem->FindAttribute(name);
    if (nullptr != attribute)
    {
        int v = 0;
        if (tinyxml2::XMLError::XML_SUCCESS == attribute->QueryIntValue(&v))
        {
            if (min_value <= v && v <= max_value)
            {
                value = static_cast<T>(v);
                return XMLP_ret::XML_OK;
            }
        }

        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Value '" << attribute->Value() << "' for attribute '" << name << "' on '" << elem->Name()
                          << "' not in range [" << min_value << ", " << max_value << "]");
        return XMLP_ret::XML_ERROR;
    }

    if (required)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Missing mandatory attribute '" << name << "' on element '" << elem->Name() << "'");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

class XMLEthernetParser : public XMLParser
{
public:

    static XMLP_ret parse_xml_ethernet_transport(
            tinyxml2::XMLElement* xml_root,
            rtps::EthernetTransportDescriptor& descriptor)
    {
        /*
            <xs:complexType name="transportDescriptorType">
                <xs:all minOccurs="0">
                    <xs:element name="eth_interface_name" type="string" minOccurs="0" maxOccurs="1"/>
                    <xs:element name="eth_output_port" type="uint16" minOccurs="0" maxOccurs="1"/>
                    <xs:element name="eth_priority_mappings" type="ethernetPriorityMappingsType" minOccurs="0" maxOccurs="1"/>
                </xs:all>
            </xs:complexType>
         */

        tinyxml2::XMLElement* elem = nullptr;
        XMLP_ret ret = XMLP_ret::XML_OK;

        // Process mandatory eth_interface_name
        elem = xml_root->FirstChildElement(ETH_INTERFACE_NAME);
        if (elem == nullptr)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Missing mandatory 'eth_interface_name' element in ETH transport descriptor");
            return XMLP_ret::XML_ERROR;
        }
        descriptor.interface_name = get_element_text(elem);

        // Process optional eth_output_port
        elem = xml_root->FirstChildElement(ETH_OUTPUT_PORT);
        if (elem != nullptr)
        {
            int xml_port = 0;
            if (XMLP_ret::XML_OK != XMLParser::getXMLInt(elem, &xml_port, 0) || xml_port < 0 || xml_port > 65535)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Element 'eth_output_port' out of range [0, 65535]");
                return XMLP_ret::XML_ERROR;
            }
            descriptor.default_source_port = static_cast<uint16_t>(xml_port);
        }

        // Process optional eth_priority_mappings
        elem = xml_root->FirstChildElement(ETH_PRIORITY_MAPPINGS);
        if (elem != nullptr)
        {
            ret = parse_xml_ethernet_priority_mappings(elem, descriptor.priority_mapping);
        }

        return ret;
    }

private:

    static XMLP_ret parse_xml_ethernet_priority_mappings(
            tinyxml2::XMLElement* p_root,
            std::map<int32_t, rtps::EthernetTransportDescriptor::PriorityMapping>& mappings)
    {
        /*
            <xs:complexType name="ethernetPriorityMappingsType">
                <xs:sequence minOccurs="0" maxOccurs="unbounded">
                    <xs:element name="priority" minOccurs="0" maxOccurs="unbounded">
                        <xs:complexType>
                            <xs:attribute name="value" type="int32" use="required"/>
                            <xs:attribute name="source_port" type="uint16" default="0"/>
                            <xs:attribute name="pcp" type="uint8" default="0"/>
                            <xs:attribute name="vlan_id" type="uint16" default="0"/>
                        </xs:complexType>
                    </xs:element>
                </xs:sequence>
            </xs:complexType>
         */

        tinyxml2::XMLElement* elem = p_root->FirstChildElement("priority");
        while ( elem != nullptr)
        {
            // Read priority value
            int32_t priority = 0;
            if (XMLP_ret::XML_OK != process_signed_attribute<int32_t>(elem, "value", priority, true))
            {
                return XMLP_ret::XML_ERROR;
            }

            // Check if priority already exists
            if (mappings.find(priority) != mappings.end())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated 'priority' element with value '" << priority << "'");
                return XMLP_ret::XML_ERROR;
            }

            // Read source port
            uint16_t source_port = 0;
            if (XMLP_ret::XML_OK != process_unsigned_attribute<uint16_t>(elem, "source_port", source_port, false))
            {
                return XMLP_ret::XML_ERROR;
            }

            // Read pcp
            uint8_t pcp = 0;
            if (XMLP_ret::XML_OK != process_unsigned_attribute<uint8_t>(elem, "pcp", pcp, false, 0, 7))
            {
                return XMLP_ret::XML_ERROR;
            }

            // Read vlan_id
            uint16_t vlan_id = 0;
            if (XMLP_ret::XML_OK != process_unsigned_attribute<uint16_t>(elem, "vlan_id", vlan_id, false, 0, 4094))
            {
                return XMLP_ret::XML_ERROR;
            }

            // Add mapping
            rtps::EthernetTransportDescriptor::PriorityMapping entry;
            entry.source_port = source_port;
            entry.pcp = pcp;
            entry.vlan_id = vlan_id;
            mappings.emplace(priority, entry);

            // Next priority
            elem = elem->NextSiblingElement("priority");
        }

        return XMLP_ret::XML_OK;
    }

};

static XMLP_ret parse_xml_udp_priorities(
        tinyxml2::XMLElement* xml_root,
        rtps::UDPPriorityMappings& mappings)
{
    /*
        <xs:complexType name="udpPriorityMappingsType">
            <xs:sequence minOccurs="0" maxOccurs="unbounded">
                <xs:element name="priority" minOccurs="0" maxOccurs="unbounded">
                    <xs:complexType>
                        <xs:attribute name="value" type="int32" use="required"/>
                        <xs:attribute name="dscp" type="uint8" default="0"/>
                        <xs:attribute name="source_port" type="uint16" default="0"/>
                        <xs:attribute name="interface" type="string"/>
                    </xs:complexType>
                </xs:element>
            </xs:sequence>
        </xs:complexType>
     */

    tinyxml2::XMLElement* elem = xml_root->FirstChildElement("priority");
    while (elem != nullptr)
    {
        // Read priority value
        int32_t priority = 0;
        if (XMLP_ret::XML_OK != process_signed_attribute<int32_t>(elem, "value", priority, true))
        {
            return XMLP_ret::XML_ERROR;
        }

        // Check if priority already exists
        if (mappings.find(priority) != mappings.end())
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated 'priority' element with value '" << priority << "'");
            return XMLP_ret::XML_ERROR;
        }

        // Read dscp
        uint8_t dscp = 0;
        if (XMLP_ret::XML_OK != process_unsigned_attribute<uint8_t>(elem, "dscp", dscp, false, 0, 63))
        {
            return XMLP_ret::XML_ERROR;
        }

        // Read source port
        uint16_t source_port = 0;
        if (XMLP_ret::XML_OK != process_unsigned_attribute<uint16_t>(elem, "source_port", source_port, false))
        {
            return XMLP_ret::XML_ERROR;
        }

        // Read interface
        std::string interface = "";
        const char* interface_value = elem->Attribute("interface");
        if (nullptr != interface_value)
        {
            interface = interface_value;
        }

        // Add mapping
        rtps::UDPPriorityMapping entry;
        entry.dscp = dscp;
        entry.source_port = source_port;
        entry.interface = interface;
        mappings.emplace(priority, entry);

        // Next priority
        elem = elem->NextSiblingElement("priority");
    }

    return XMLP_ret::XML_OK;
}

class XMLChainingTransportsParser : public XMLParser
{
public:

    static sp_transport_t parse_and_get_chaining_transport_low_level_transport(
            tinyxml2::XMLElement* xml_root)
    {
        /*
            <xs:complexType name="transportDescriptorType">
                <xs:all minOccurs="0">
                    <xs:element name="low_level_transport" type="string" minOccurs="0" maxOccurs="1"/>
                </xs:all>
            </xs:complexType>
         */

        sp_transport_t transport {nullptr};
        std::string low_level_transport;

        // Process mandatory eth_interface_name
        tinyxml2::XMLElement* elem {xml_root->FirstChildElement(LOW_LEVEL_TRANSPORT)};
        if (nullptr != elem)
        {
            if (XMLP_ret::XML_OK == getXMLString(elem, &low_level_transport, 0))
            {
                transport = XMLProfileManager::getTransportById(low_level_transport);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Element 'low_level_transport' is empty");
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Missing mandatory 'low_level_transport' element in transport descriptor");
        }

        return transport;
    }

};

XMLP_ret XMLParser::get_xml_locator_ethernet(
        tinyxml2::XMLElement* elem,
        rtps::Locator_t& locator,
        uint8_t ident)
{
    /*
        <xs:complexType name="ethernetLocatorType">
            <xs:all>
                <xs:element name="port" type="uint16" minOccurs="0" maxOccurs="1"/>
                <xs:element name="pcp" type="uint8" minOccurs="0" maxOccurs="1"/>
                <xs:element name="vlan_id" type="uint16" minOccurs="0" maxOccurs="1"/>
                <xs:element name="address" type="octectVector" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */

    uint16_t logical_port = 0;
    uint16_t pcp = 0;
    uint16_t vlan_id = 0;
    std::vector<uint8_t> address;

    std::unordered_map<std::string, bool> tags_present;

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (tags_present[name])
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element '" << name << "' found in 'ethernetLocatorType'");
            return XMLP_ret::XML_ERROR;
        }
        tags_present[name] = true;

        if (strcmp(name, PORT) == 0)
        {
            // port - uint16Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &logical_port, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, ADDRESS) == 0)
        {
            // address - octectVector
            if (XMLP_ret::XML_OK != parseXMLOctetVector(p_aux0, address, false))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Failed to parse ethernet locator's " << ADDRESS << " tag");
                return XMLP_ret::XML_ERROR;
            }

            if (address.size() != 6)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Failed to parse ethernet locator's " << ADDRESS
                                                              << " tag. MAC address must have exactly 6 octets");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, "pcp") == 0)
        {
            // pcp - uint8 (0 to 7)
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &pcp, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            if (pcp > 7)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Element 'pcp' out of range [0, 7]");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, "vlan_id") == 0)
        {
            // vlan_id - uint16 (0 to 4094)
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &vlan_id, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            if (vlan_id > 4094)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Element 'vlan_id' out of range [0, 4094]");
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'ethernetLocatorType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    rtps::EthernetLocator& eth_locator = reinterpret_cast<rtps::EthernetLocator&>(locator);
    eth_locator.kind = LOCATOR_KIND_ETHERNET;
    eth_locator.port = (static_cast<uint32_t>(vlan_id) << 20) | (static_cast<uint32_t>(pcp)
            << 16) | (static_cast<uint32_t>(logical_port));
    std::copy(address.begin(), address.end(), eth_locator.address.begin());

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::create_transport_descriptor_from_xml_type(
        tinyxml2::XMLElement* p_root,
        const std::string& transport_type,
        sp_transport_t& p_transport)
{
    static_cast<void>(p_root);

    if (transport_type == UDPv4)
    {
        tinyxml2::XMLElement* elem = p_root->FirstChildElement(UDP_PRIORITY_MAPPINGS);
        if (elem == nullptr)
        {
            // No priority mappings, return default UDPv4 transport
            p_transport = std::make_shared<fastdds::rtps::UDPv4TransportDescriptor>();
            return XMLP_ret::XML_OK;
        }

        // Create a TSN_UDPv4TransportDescriptor to hold the priorities
        auto udp_transport = std::make_shared<fastdds::rtps::TSN_UDPv4TransportDescriptor>();
        XMLP_ret ret = parse_xml_udp_priorities(elem, udp_transport->priority_mapping);
        if (ret == XMLP_ret::XML_OK)
        {
            p_transport = udp_transport;
        }
        return ret;
    }

    if (transport_type == UDPv6)
    {
        tinyxml2::XMLElement* elem = p_root->FirstChildElement(UDP_PRIORITY_MAPPINGS);
        if (elem == nullptr)
        {
            // No priority mappings, return default UDPv6 transport
            p_transport = std::make_shared<fastdds::rtps::UDPv6TransportDescriptor>();
            return XMLP_ret::XML_OK;
        }

        // Create a TSN_UDPv6TransportDescriptor to hold the priorities
        auto udp_transport = std::make_shared<fastdds::rtps::TSN_UDPv6TransportDescriptor>();
        XMLP_ret ret = parse_xml_udp_priorities(elem, udp_transport->priority_mapping);
        if (ret == XMLP_ret::XML_OK)
        {
            p_transport = udp_transport;
        }
        return ret;
    }

    if (transport_type == TCPv4)
    {
        p_transport = std::make_shared<fastdds::rtps::TCPv4TransportDescriptor>();
        return XMLP_ret::XML_OK;
    }

    if (transport_type == TCPv6)
    {
        p_transport = std::make_shared<fastdds::rtps::TCPv6TransportDescriptor>();
        return XMLP_ret::XML_OK;
    }

    if (transport_type == SHM)
    {
        p_transport = std::make_shared<fastdds::rtps::SharedMemTransportDescriptor>();
        return XMLP_ret::XML_OK;
    }

    if (transport_type == "ETH")
    {
        auto eth_transport = std::make_shared<fastdds::rtps::EthernetTransportDescriptor>();
        auto ret = XMLEthernetParser::parse_xml_ethernet_transport(p_root, *eth_transport);
        if (ret == XMLP_ret::XML_OK)
        {
            p_transport = eth_transport;
        }
        return ret;
    }


#if HAVE_ZLIB || HAVE_BZIP2
    if (transport_type == "PAYLOAD_COMPRESSION")
    {
        auto ret {XMLP_ret::XML_ERROR};
        auto low_level_transport =
                XMLChainingTransportsParser::parse_and_get_chaining_transport_low_level_transport(p_root);
        if (nullptr != low_level_transport)
        {
            p_transport = std::make_shared<fastdds::rtps::PayloadCompressionTransportDescriptor>(low_level_transport);
            ret = XMLP_ret::XML_OK;
        }
        return ret;
    }
#endif // HAVE_ZLIB || HAVE_BZIP2

    if (transport_type == "HEADER_REDUCTION")
    {
        auto ret {XMLP_ret::XML_ERROR};
        auto low_level_transport =
                XMLChainingTransportsParser::parse_and_get_chaining_transport_low_level_transport(p_root);
        if (nullptr != low_level_transport)
        {
            p_transport = std::make_shared<fastdds::rtps::HeaderReductionTransportDescriptor>(low_level_transport);
            ret = XMLP_ret::XML_OK;
        }
        return ret;
    }

    if (transport_type == "SOURCE_TIMESTAMP")
    {
        auto ret {XMLP_ret::XML_ERROR};
        auto low_level_transport =
                XMLChainingTransportsParser::parse_and_get_chaining_transport_low_level_transport(p_root);
        if (nullptr != low_level_transport)
        {
            p_transport = std::make_shared<fastdds::rtps::SourceTimestampTransportDescriptor>(low_level_transport);
            ret = XMLP_ret::XML_OK;
        }
        return ret;
    }

    EPROSIMA_LOG_ERROR(XMLPARSER, "Unknown transport type: '" << transport_type << "'");
    return XMLP_ret::XML_ERROR;
}

}  // namespace xmlparser
}  // namespace fastdds
}  // namespace eprosima
