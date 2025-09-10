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

#include <cstdint>
#include <memory>
#include <string>

#include <tinyxml2.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>

#include <xmlparser/XMLParser.h>
#include <xmlparser/XMLParserCommon.h>
#include <xmlparser/XMLParserUtils.hpp>
#include <xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

class Locator_t;

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

namespace eprosima {
namespace fastdds {
namespace xmlparser {

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

    static_cast<void>(elem);
    static_cast<void>(locator);
    static_cast<void>(ident);

    EPROSIMA_LOG_ERROR(XMLPARSER, "Support for ethernet locators not implemented");
    return XMLP_ret::XML_ERROR;
}

XMLP_ret XMLParser::create_transport_descriptor_from_xml_type(
        tinyxml2::XMLElement* p_root,
        const std::string& transport_type,
        sp_transport_t& p_transport)
{
    static_cast<void>(p_root);

    if (transport_type == UDPv4)
    {
        p_transport = std::make_shared<fastdds::rtps::UDPv4TransportDescriptor>();
        return XMLP_ret::XML_OK;
    }

    if (transport_type == UDPv6)
    {
        p_transport = std::make_shared<fastdds::rtps::UDPv6TransportDescriptor>();
        return XMLP_ret::XML_OK;
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

    EPROSIMA_LOG_ERROR(XMLPARSER, "Unknown transport type: '" << transport_type << "'");
    return XMLP_ret::XML_ERROR;
}

}  // namespace xmlparser
}  // namespace fastdds
}  // namespace eprosima
