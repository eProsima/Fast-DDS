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

#include <cassert>
#include <cstring>
#include <regex>
#include <string>
#include <unordered_map>

#include <tinyxml2.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/utils/IPLocator.h>
#include <fastrtps/xmlparser/XMLParser.h>
#include <fastrtps/xmlparser/XMLParserCommon.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <rtps/xmlparser/XMLParserUtils.hpp>
#include <utils/SystemInfo.hpp>

template<typename _Enum>
static bool get_element_enum_value(
        const char* input,
        _Enum& output,
        const char* name,
        _Enum value)
{
    if (0 == strcmp(input, name))
    {
        output = value;
        return true;
    }

    return false;
}

template<typename _Enum, typename ... Targs>
static bool get_element_enum_value(
        const char* input,
        _Enum& output,
        const char* name,
        _Enum value,
        Targs... args)
{
    if (0 == strcmp(input, name))
    {
        output = value;
        return true;
    }

    return get_element_enum_value(input, output, args ...);
}

namespace eprosima {
namespace fastdds {
namespace xml {
namespace detail {

static std::string process_environment(
        const std::string& input)
{
    std::string ret_val = input;
    /* From [IEEE Std 1003.1]:(https://pubs.opengroup.org/onlinepubs/000095399/basedefs/xbd_chap08.html)
     * Environment variable names used ... consist solely of uppercase letters, digits, and the '_' (underscore)
     * from the characters defined in Portable Character Set and do not begin with a digit.
     */
    std::regex expression("\\$\\{([A-Z_][A-Z0-9_]*)\\}");
    std::smatch match;

    do
    {
        std::regex_search(ret_val, match, expression);
        if (!match.empty())
        {
            std::string var_name = match[1];
            std::string value;
            if (ReturnCode_t::RETCODE_OK == SystemInfo::get_env(var_name, value))
            {
                ret_val = match.prefix().str() + value + match.suffix().str();
            }
            else
            {
                ret_val = match.prefix().str() + match.suffix().str();
                EPROSIMA_LOG_ERROR(XMLPARSER, "Could not find a value for environment variable " << var_name);
            }
        }
    } while (!match.empty());

    return ret_val;
}

std::string get_element_text(
        tinyxml2::XMLElement* element)
{
    std::string ret_val{};

    assert(nullptr != element);
    const char* text = element->GetText();
    if (nullptr != text)
    {
        ret_val = process_environment(text);
    }

    return ret_val;
}

}  // namespace detail
}  // namespace xml
}  // namespace fastdds
}  // namespace eprosima


namespace eprosima {
namespace fastrtps {
namespace xmlparser {

using namespace eprosima::fastrtps::rtps;
using namespace eprosima::fastdds::xml::detail;

static XMLP_ret parseXMLOctetVector(
        tinyxml2::XMLElement* elem,
        std::vector<octet>& octet_vector,
        bool allow_empty)
{
    std::string text = get_element_text(elem);
    if (text.empty() && allow_empty)
    {
        return XMLP_ret::XML_OK;
    }

    std::istringstream ss(text);
    ss >> std::hex;

    while (!ss.eof())
    {
        uint16_t o = 0;
        ss >> o;

        if (!ss || std::numeric_limits<octet>::max() < o)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Expected an octet value on line " << elem->GetLineNum());
            return XMLP_ret::XML_ERROR;
        }

        // Add octet in vector.
        octet_vector.push_back(static_cast<octet>(o));

        if (!ss.eof())
        {
            char c = 0;
            ss >> c;

            if (!ss || '.' != c)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Expected a '.' separator on line " << elem->GetLineNum());
                return XMLP_ret::XML_ERROR;
            }
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLParticipantAllocationAttributes(
        tinyxml2::XMLElement* elem,
        rtps::RTPSParticipantAllocationAttributes& allocation,
        uint8_t ident)
{
    /*
        <xs:complexType name="rtpsParticipantAllocationAttributesType">
            <xs:all minOccurs="0">
                <xs:element name="remote_locators" type="remoteLocatorsAllocationConfigType" minOccurs="0"/>
                <xs:element name="total_participants" type="containerAllocationConfigType" minOccurs="0"/>
                <xs:element name="total_readers" type="containerAllocationConfigType" minOccurs="0"/>
                <xs:element name="total_writers" type="containerAllocationConfigType" minOccurs="0"/>
                <xs:element name="send_buffers" type="sendBuffersAllocationConfigType" minOccurs="0"/>
                <xs:element name="max_properties" type="uint32Type" minOccurs="0"/>
                <xs:element name="max_user_data" type="uint32Type" minOccurs="0"/>
                <xs:element name="max_partitions" type="uint32Type" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    uint32_t tmp;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, REMOTE_LOCATORS) == 0)
        {
            // leaseDuration - durationType
            if (XMLP_ret::XML_OK != getXMLRemoteLocatorsAllocationAttributes(p_aux0, allocation.locators, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, TOTAL_PARTICIPANTS) == 0)
        {
            // total_participants - containerAllocationConfigType
            if (XMLP_ret::XML_OK != getXMLContainerAllocationConfig(p_aux0, allocation.participants, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, TOTAL_READERS) == 0)
        {
            // total_readers - containerAllocationConfigType
            if (XMLP_ret::XML_OK != getXMLContainerAllocationConfig(p_aux0, allocation.readers, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, TOTAL_WRITERS) == 0)
        {
            // total_writers - containerAllocationConfigType
            if (XMLP_ret::XML_OK != getXMLContainerAllocationConfig(p_aux0, allocation.writers, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, SEND_BUFFERS) == 0)
        {
            // send_buffers - sendBuffersAllocationConfigType
            if (XMLP_ret::XML_OK != getXMLSendBuffersAllocationAttributes(p_aux0, allocation.send_buffers, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, MAX_PROPERTIES) == 0)
        {
            // max number of properties in incoming message - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &tmp, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            allocation.data_limits.max_properties = tmp;
        }
        else if (strcmp(name, MAX_USER_DATA) == 0)
        {
            // max number of user data in incoming message - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &tmp, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            allocation.data_limits.max_user_data = tmp;
        }
        else if (strcmp(name, MAX_PARTITIONS) == 0)
        {
            // max number of user data in incoming message - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &tmp, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            allocation.data_limits.max_partitions = tmp;
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Invalid element found into 'rtpsParticipantAllocationAttributesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLRemoteLocatorsAllocationAttributes(
        tinyxml2::XMLElement* elem,
        rtps::RemoteLocatorsAllocationAttributes& allocation,
        uint8_t ident)
{
    /*
        <xs:complexType name="remoteLocatorsAllocationConfigType">
            <xs:all minOccurs="0">
                <xs:element name="max_unicast_locators" type="uint32Type" minOccurs="0"/>
                <xs:element name="max_multicast_locators" type="uint32Type" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    uint32_t tmp;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, MAX_UNICAST_LOCATORS) == 0)
        {
            // max_unicast_locators - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &tmp, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            allocation.max_unicast_locators = tmp;
        }
        else if (strcmp(name, MAX_MULTICAST_LOCATORS) == 0)
        {
            // max_multicast_locators - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &tmp, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            allocation.max_multicast_locators = tmp;
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Invalid element found into 'remoteLocatorsAllocationConfigType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLSendBuffersAllocationAttributes(
        tinyxml2::XMLElement* elem,
        rtps::SendBuffersAllocationAttributes& allocation,
        uint8_t ident)
{
    /*
        <xs:complexType name="sendBuffersAllocationConfigType">
            <xs:all minOccurs="0">
                <xs:element name="preallocated_number" type="uint32Type" minOccurs="0"/>
                <xs:element name="dynamic" type="boolType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    uint32_t tmp;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, PREALLOCATED_NUMBER) == 0)
        {
            // preallocated_number - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &tmp, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            allocation.preallocated_number = tmp;
        }
        else if (strcmp(name, DYNAMIC_LC) == 0)
        {
            // dynamic - boolType
            bool tmp_bool = false;
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &tmp_bool, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            allocation.dynamic = tmp_bool;
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Invalid element found into 'sendBuffersAllocationConfigType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLDiscoverySettings(
        tinyxml2::XMLElement* elem,
        rtps::DiscoverySettings& settings,
        uint8_t ident)
{
    /*
       <xs:complexType name="discoverySettingsType">
        <xs:all minOccurs="0">
            <xs:element name="discoveryProtocol" type="DiscoveryProtocol" minOccurs="0"/>
            <xs:element name="ignoreParticipantFlags" type="ParticipantFlags" minOccurs="0"/>
            <xs:element name="EDP" type="EDPType" minOccurs="0"/>
            <xs:element name="leaseDuration" type="durationType" minOccurs="0"/>
            <xs:element name="leaseAnnouncement" type="durationType" minOccurs="0"/>
            <xs:element name="simpleEDP" type="simpleEDPType" minOccurs="0"/>
            <xs:element name="clientAnnouncementPeriod" type="durationType" minOccurs="0"/>
            <xs:element name="discoveryServersList" type="DiscoveryServerList" minOccurs="0"/>
            <xs:element name="staticEndpointXMLFilename" type="stringType" minOccurs="0"/>
            <xs:element name="static_edp_xml_config" type="stringType" minOccurs="0"/>
        </xs:all>
       </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr, * p_aux1 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, RTPS_PDP_TYPE) == 0)
        {
            // discoveryProtocol - DiscoveryProtocol
            if (XMLP_ret::XML_OK != getXMLEnum(p_aux0, &settings.discoveryProtocol, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, IGNORE_PARTICIPANT_FLAGS) == 0)
        {
            // ignoreParticipantFlags - ParticipantFlags
            if (XMLP_ret::XML_OK != getXMLEnum(p_aux0, &settings.ignoreParticipantFlags, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, _EDP) == 0)
        {
            /*
                <xs:simpleType name="EDPType">
                    <xs:restriction base="xs:string">
                        <xs:enumeration value="SIMPLE"/>
                        <xs:enumeration value="STATIC"/>
                    </xs:restriction>
                </xs:simpleType>
             */
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << _EDP << "' without content");
                return XMLP_ret::XML_ERROR;
            }
            else if (strcmp(text.c_str(), SIMPLE) == 0)
            {
                settings.use_SIMPLE_EndpointDiscoveryProtocol = true;
                settings.use_STATIC_EndpointDiscoveryProtocol = false;
            }
            else if (strcmp(text.c_str(), STATIC) == 0)
            {
                settings.use_SIMPLE_EndpointDiscoveryProtocol = false;
                settings.use_STATIC_EndpointDiscoveryProtocol = true;
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << _EDP << "' with bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, LEASEDURATION) == 0)
        {
            // leaseDuration - durationType
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, settings.leaseDuration, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, LEASE_ANNOUNCE) == 0)
        {
            // leaseAnnouncement - durationType
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, settings.leaseDuration_announcementperiod, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, INITIAL_ANNOUNCEMENTS) == 0)
        {
            // initialAnnouncements - initialAnnouncementsType
            if (XMLP_ret::XML_OK !=
                    getXMLInitialAnnouncementsConfig(p_aux0, settings.initial_announcements, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, SIMPLE_EDP) == 0)
        {
            // simpleEDP
            for (p_aux1 = p_aux0->FirstChildElement(); p_aux1 != nullptr; p_aux1 = p_aux1->NextSiblingElement())
            {
                name = p_aux1->Name();
                if (strcmp(name, PUBWRITER_SUBREADER) == 0)
                {
                    // PUBWRITER_SUBREADER - boolType
                    if (XMLP_ret::XML_OK !=
                            getXMLBool(p_aux1, &settings.m_simpleEDP.use_PublicationWriterANDSubscriptionReader,
                            ident + 1))
                    {
                        return XMLP_ret::XML_ERROR;
                    }
                }
                else if (strcmp(name, PUBREADER_SUBWRITER) == 0)
                {
                    // PUBREADER_SUBWRITER - boolType
                    if (XMLP_ret::XML_OK !=
                            getXMLBool(p_aux1, &settings.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter,
                            ident + 1))
                    {
                        return XMLP_ret::XML_ERROR;
                    }
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'simpleEDP'. Name: " << name);
                    return XMLP_ret::XML_ERROR;
                }
            }
        }
        else if (strcmp(name, CLIENTANNOUNCEMENTPERIOD) == 0)
        {
            // clientAnnouncementPeriod - durationType
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, settings.discoveryServer_client_syncperiod, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, SERVER_LIST) == 0)
        {
            // discoverServersList - DiscoveryServerList
            if (XMLP_ret::XML_OK != getXMLList(p_aux0, settings.m_DiscoveryServers, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, STATIC_ENDPOINT_XML) == 0)
        {
            // staticEndpointXMLFilename - stringType
            std::string s = "";
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            std::string file_name = "file://" + s;
            settings.static_edp_xml_config(file_name.c_str());
        }
        else if (strcmp(name, STATIC_ENDPOINT_XML_URI) == 0)
        {
            // staticEndpointXMLFilename - stringType
            std::string s = "";
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            settings.static_edp_xml_config(s.c_str());
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'discoverySettingsType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;

}

XMLP_ret XMLParser::getXMLTypeLookupSettings(
        tinyxml2::XMLElement* elem,
        rtps::TypeLookupSettings& settings,
        uint8_t ident)
{
    /*
       <xs:complexType name="typelookupSettingsType">
        <xs:all minOccurs="0">
          <xs:element name="use_client" type="boolType" minOccurs="0"/>
          <xs:element name="use_server" type="boolType" minOccurs="0"/>
        </xs:all>
       </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, TYPELOOKUP_USE_SERVER) == 0)
        {
            //
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &settings.use_server, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, TYPELOOKUP_USE_CLIENT) == 0)
        {
            //
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &settings.use_client, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'typelookupSettingsType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLBuiltinAttributes(
        tinyxml2::XMLElement* elem,
        BuiltinAttributes& builtin,
        uint8_t ident)
{
    /*
       <xs:complexType name="builtinAttributesType">
        <xs:all minOccurs="0">
            <xs:element name="discovery_config" type="discoverySettingsType" minOccurs="0"/>
            <xs:element name="use_WriterLivelinessProtocol" type="boolType" minOccurs="0"/>
            <xs:element name="metatraffic_external_unicast_locators" type="externalLocatorListType" minOccurs="0"/>
            <xs:element name="metatrafficUnicastLocatorList" type="locatorListType" minOccurs="0"/>
            <xs:element name="metatrafficMulticastLocatorList" type="locatorListType" minOccurs="0"/>
            <xs:element name="initialPeersList" type="locatorListType" minOccurs="0"/>
            <xs:element name="readerHistoryMemoryPolicy" type="historyMemoryPolicyType" minOccurs="0"/>
            <xs:element name="writerHistoryMemoryPolicy" type="historyMemoryPolicyType" minOccurs="0"/>
            <xs:element name="mutation_tries" type="uint32Type" minOccurs="0"/>
        </xs:all>
       </xs:complexType>
     */

    std::unordered_map<std::string, bool> tags_present;

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();

        if (tags_present[name])
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'builtinAttributesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present[name] = true;

        if (strcmp(name, DISCOVERY_SETTINGS) == 0)
        {
            // discovery_config - DiscoverySettings
            if (XMLP_ret::XML_OK != getXMLDiscoverySettings(p_aux0, builtin.discovery_config, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, TYPELOOKUP_CONFIG) == 0)
        {
            // typelookup_config - boolType
            if (XMLP_ret::XML_OK != getXMLTypeLookupSettings(p_aux0, builtin.typelookup_config, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, WRITER_LVESS_PROTOCOL) == 0)
        {
            // use_WriterLivelinessProtocol - boolType
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &builtin.use_WriterLivelinessProtocol, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, META_EXT_UNI_LOC_LIST) == 0)
        {
            // metatraffic_external_unicast_locators - externalLocatorListType
            if (XMLP_ret::XML_OK !=
                    getXMLExternalLocatorList(p_aux0, builtin.metatraffic_external_unicast_locators, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, META_UNI_LOC_LIST) == 0)
        {
            // metatrafficUnicastLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, builtin.metatrafficUnicastLocatorList, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, META_MULTI_LOC_LIST) == 0)
        {
            // metatrafficMulticastLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, builtin.metatrafficMulticastLocatorList, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, INIT_PEERS_LIST) == 0)
        {
            // initialPeersList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, builtin.initialPeersList, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, READER_HIST_MEM_POLICY) == 0)
        {
            // readerhistoryMemoryPolicy
            if (XMLP_ret::XML_OK != getXMLHistoryMemoryPolicy(p_aux0, builtin.readerHistoryMemoryPolicy, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, WRITER_HIST_MEM_POLICY) == 0)
        {
            // writerhistoryMemoryPolicy
            if (XMLP_ret::XML_OK != getXMLHistoryMemoryPolicy(p_aux0, builtin.writerHistoryMemoryPolicy, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, READER_PAYLOAD_SIZE) == 0)
        {
            // readerPayloadSize
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &builtin.readerPayloadSize, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, WRITER_PAYLOAD_SIZE) == 0)
        {
            // readerPayloadSize
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &builtin.writerPayloadSize, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, MUTATION_TRIES) == 0)
        {
            // mutation_tries - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &builtin.mutation_tries, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, AVOID_BUILTIN_MULTICAST) == 0)
        {
            // avoid_builtin_multicast - boolType
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &builtin.avoid_builtin_multicast, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'builtinAttributesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLInitialAnnouncementsConfig(
        tinyxml2::XMLElement* elem,
        InitialAnnouncementConfig& config,
        uint8_t ident)
{
    /*
        <xs:complexType name="initialAnnouncementsType">
            <xs:all minOccurs="0">
                <xs:element name="count" type="uint32Type" minOccurs="0"/>
                <xs:element name="period" type="durationType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, COUNT) == 0)
        {
            // portBase - uint16Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &config.count, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PERIOD) == 0)
        {
            // domainIDGain - uint16Type
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, config.period, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'portType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLPortParameters(
        tinyxml2::XMLElement* elem,
        PortParameters& port,
        uint8_t ident)
{
    /*
        <xs:complexType name="portType">
            <xs:all minOccurs="0">
                <xs:element name="portBase" type="uint16Type" minOccurs="0"/>
                <xs:element name="domainIDGain" type="uint16Type" minOccurs="0"/>
                <xs:element name="participantIDGain" type="uint16Type" minOccurs="0"/>
                <xs:element name="offsetd0" type="uint16Type" minOccurs="0"/>
                <xs:element name="offsetd1" type="uint16Type" minOccurs="0"/>
                <xs:element name="offsetd2" type="uint16Type" minOccurs="0"/>
                <xs:element name="offsetd3" type="uint16Type" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, PORT_BASE) == 0)
        {
            // portBase - uint16Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.portBase, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DOMAIN_ID_GAIN) == 0)
        {
            // domainIDGain - uint16Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.domainIDGain, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PARTICIPANT_ID_GAIN) == 0)
        {
            // participantIDGain - uint16Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.participantIDGain, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, OFFSETD0) == 0)
        {
            // offsetd0 - uint16Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.offsetd0, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, OFFSETD1) == 0)
        {
            // offsetd1 - uint16Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.offsetd1, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, OFFSETD2) == 0)
        {
            // offsetd2 - uint16Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.offsetd2, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, OFFSETD3) == 0)
        {
            // offsetd3 - uint16Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.offsetd3, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'portType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLTransports(
        tinyxml2::XMLElement* elem,
        std::vector<std::shared_ptr<TransportDescriptorInterface>>& transports,
        uint8_t /*ident*/)
{
    /*
        <xs:complexType name="stringListType">
            <xs:sequence>
                <xs:element name="id" type="stringType" minOccurs="0" maxOccurs="unbounded"/>
            </xs:sequence>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    p_aux0 = elem->FirstChildElement(TRANSPORT_ID);
    if (nullptr == p_aux0)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    while (nullptr != p_aux0)
    {
        std::string text = get_element_text(p_aux0);
        if (text.empty())
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << TRANSPORT_ID << "' without content");
            return XMLP_ret::XML_ERROR;
        }
        else
        {
            std::shared_ptr<TransportDescriptorInterface> pDescriptor = XMLProfileManager::getTransportById(text);
            if (pDescriptor != nullptr)
            {
                transports.emplace_back(pDescriptor);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Transport Node not found. Given ID: " << text);
                return XMLP_ret::XML_ERROR;
            }
        }
        p_aux0 = p_aux0->NextSiblingElement(TRANSPORT_ID);
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLThroughputController(
        tinyxml2::XMLElement* elem,
        ThroughputControllerDescriptor& throughputController,
        uint8_t ident)
{
    /*
        <xs:complexType name="throughputControllerType">
            <xs:all minOccurs="0">
                <xs:element name="bytesPerPeriod" type="uint32Type" minOccurs="0"/>
                <xs:element name="periodMillisecs" type="uint32Type" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, BYTES_PER_SECOND) == 0)
        {
            // bytesPerPeriod - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &throughputController.bytesPerPeriod, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PERIOD_MILLISECS) == 0)
        {
            // periodMillisecs - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &throughputController.periodMillisecs, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'portType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLTopicAttributes(
        tinyxml2::XMLElement* elem,
        TopicAttributes& topic,
        uint8_t ident)
{
    /*
        <xs:complexType name="topicAttributesType">
            <xs:all minOccurs="0">
                <xs:element name="kind" type="topicKindType" minOccurs="0"/>
                <xs:element name="name" type="stringType" minOccurs="0"/>
                <xs:element name="dataType" type="stringType" minOccurs="0"/>
                <xs:element name="historyQos" type="historyQosPolicyType" minOccurs="0"/>
                <xs:element name="resourceLimitsQos" type="resourceLimitsQosPolicyType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, KIND) == 0)
        {
            // kind
            /*
                <xs:simpleType name="topicKindType">
                    <xs:restriction base="xs:string">
                        <xs:enumeration value="NO_KEY"/>
                        <xs:enumeration value="WITH_KEY"/>
                    </xs:restriction>
                </xs:simpleType>
             */
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' without content");
                return XMLP_ret::XML_ERROR;
            }

            if (!get_element_enum_value(text.c_str(), topic.topicKind,
                    _NO_KEY, TopicKind_t::NO_KEY,
                    _WITH_KEY, TopicKind_t::WITH_KEY))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' with bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, NAME) == 0)
        {
            // name - stringType
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "<" << p_aux0->Value() << "> getXMLString XML_ERROR!");
                return XMLP_ret::XML_ERROR;
            }
            topic.topicName = text;
        }
        else if (strcmp(name, DATA_TYPE) == 0)
        {
            // dataType - stringType
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "<" << p_aux0->Value() << "> getXMLString XML_ERROR!");
                return XMLP_ret::XML_ERROR;
            }
            topic.topicDataType = text;
        }
        else if (strcmp(name, HISTORY_QOS) == 0)
        {
            // historyQos
            if (XMLP_ret::XML_OK != getXMLHistoryQosPolicy(p_aux0, topic.historyQos, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, RES_LIMITS_QOS) == 0)
        {
            // resourceLimitsQos
            if (XMLP_ret::XML_OK != getXMLResourceLimitsQos(p_aux0, topic.resourceLimitsQos, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'topicAttributesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLResourceLimitsQos(
        tinyxml2::XMLElement* elem,
        ResourceLimitsQosPolicy& resourceLimitsQos,
        uint8_t ident)
{
    /*
        <xs:complexType name="resourceLimitsQosPolicyType">
            <xs:all minOccurs="0">
                <xs:element name="max_samples" type="int32Type" minOccurs="0"/>
                <xs:element name="max_instances" type="int32Type" minOccurs="0"/>
                <xs:element name="max_samples_per_instance" type="int32Type" minOccurs="0"/>
                <xs:element name="allocated_samples" type="int32Type" minOccurs="0"/>
                <xs:element name="extra_samples" type="int32Type" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, MAX_SAMPLES) == 0)
        {
            // max_samples - int32Type
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &resourceLimitsQos.max_samples, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, MAX_INSTANCES) == 0)
        {
            // max_instances - int32Type
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &resourceLimitsQos.max_instances, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, MAX_SAMPLES_INSTANCE) == 0)
        {
            // max_samples_per_instance - int32Type
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &resourceLimitsQos.max_samples_per_instance, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, ALLOCATED_SAMPLES) == 0)
        {
            // allocated_samples - int32Type
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &resourceLimitsQos.allocated_samples, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, EXTRA_SAMPLES) == 0)
        {
            // extra_samples - int32Type
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &resourceLimitsQos.extra_samples, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'resourceLimitsQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLContainerAllocationConfig(
        tinyxml2::XMLElement* elem,
        ResourceLimitedContainerConfig& allocation_config,
        uint8_t ident)
{
    /*
        <xs:complexType name="containerAllocationConfigType">
            <xs:all minOccurs="0">
                <xs:element name="initial" type="uint32Type" minOccurs="0"/>
                <xs:element name="maximum" type="uint32Type" minOccurs="0"/>
                <xs:element name="increment" type="uint32Type" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    // First set default values
    allocation_config = ResourceLimitedContainerConfig();

    // Then parse XML
    uint32_t aux_value;
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, INITIAL) == 0)
        {
            // initial - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &aux_value, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            allocation_config.initial = static_cast<size_t>(aux_value);
        }
        else if (strcmp(name, MAXIMUM) == 0)
        {
            // maximum - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &aux_value, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            allocation_config.maximum = (aux_value == 0u) ?
                    std::numeric_limits<size_t>::max() : static_cast<size_t>(aux_value);
        }
        else if (strcmp(name, INCREMENT) == 0)
        {
            // increment - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &aux_value, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            allocation_config.increment = static_cast<size_t>(aux_value);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'containerAllocationConfigType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    // Check results
    if (allocation_config.initial > allocation_config.maximum)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "Parsing 'containerAllocationConfigType': Field 'initial' cannot be greater than 'maximum'.");
        return XMLP_ret::XML_ERROR;
    }
    else if ((allocation_config.increment == 0) && (allocation_config.initial != allocation_config.maximum))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Parsing 'containerAllocationConfigType': Field 'increment' cannot be zero.");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLHistoryQosPolicy(
        tinyxml2::XMLElement* elem,
        HistoryQosPolicy& historyQos,
        uint8_t ident)
{
    /*
        <xs:complexType name="historyQosPolicyType">
            <xs:all minOccurs="0">
                <xs:element name="kind" type="historyQosKindType" minOccurs="0"/>
                <xs:element name="depth" type="int32Type" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, KIND) == 0)
        {
            // kind
            /*
                <xs:simpleType name="historyQosKindType">
                    <xs:restriction base="xs:string">
                        <xs:enumeration value="KEEP_LAST"/>
                        <xs:enumeration value="KEEP_ALL"/>
                    </xs:restriction>
                </xs:simpleType>
             */
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' without content");
                return XMLP_ret::XML_ERROR;
            }

            if (!get_element_enum_value(text.c_str(), historyQos.kind,
                    KEEP_LAST, HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS,
                    KEEP_ALL, HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' with bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DEPTH) == 0)
        {
            // depth - uint32Type
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &historyQos.depth, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'historyQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLWriterQosPolicies(
        tinyxml2::XMLElement* elem,
        WriterQos& qos,
        uint8_t ident)
{
    /*
        <xs:complexType name="writerQosPoliciesType">
            <xs:all minOccurs="0">
                <xs:element name="durability" type="durabilityQosPolicyType" minOccurs="0"/>
                <xs:element name="durabilityService" type="durabilityServiceQosPolicyType" minOccurs="0"/>
                <xs:element name="deadline" type="deadlineQosPolicyType" minOccurs="0"/>
                <xs:element name="latencyBudget" type="latencyBudgetQosPolicyType" minOccurs="0"/>
                <xs:element name="liveliness" type="livelinessQosPolicyType" minOccurs="0"/>
                <xs:element name="reliability" type="reliabilityQosPolicyType" minOccurs="0"/>
                <xs:element name="lifespan" type="lifespanQosPolicyType" minOccurs="0"/>
                <xs:element name="userData" type="userDataQosPolicyType" minOccurs="0"/>
                <xs:element name="timeBasedFilter" type="timeBasedFilterQosPolicyType" minOccurs="0"/>
                <xs:element name="ownership" type="ownershipQosPolicyType" minOccurs="0"/>
                <xs:element name="ownershipStrength" type="ownershipStrengthQosPolicyType" minOccurs="0"/>
                <xs:element name="destinationOrder" type="destinationOrderQosPolicyType" minOccurs="0"/>
                <xs:element name="presentation" type="presentationQosPolicyType" minOccurs="0"/>
                <xs:element name="partition" type="partitionQosPolicyType" minOccurs="0"/>
                <xs:element name="topicData" type="topicDataQosPolicyType" minOccurs="0"/>
                <xs:element name="groupData" type="groupDataQosPolicyType" minOccurs="0"/>
                <xs:element name="publishMode" type="publishModeQosPolicyType" minOccurs="0"/>
                <xs:element name="data_sharing" type="dataSharingQosPolicyType" minOccurs="0"/>
                <xs:element name="disablePositiveAcks" type="disablePositiveAcksQosPolicyType" minOccurs="0"/>
                <xs:element name="disable_heartbeat_piggyback" type="boolType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, DURABILITY) == 0)
        {
            // durability
            if (XMLP_ret::XML_OK != getXMLDurabilityQos(p_aux0, qos.m_durability, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, LIVELINESS) == 0)
        {
            // liveliness
            if (XMLP_ret::XML_OK != getXMLLivelinessQos(p_aux0, qos.m_liveliness, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, RELIABILITY) == 0)
        {
            // reliability
            if (XMLP_ret::XML_OK != getXMLReliabilityQos(p_aux0, qos.m_reliability, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PARTITION) == 0)
        {
            // partition
            if (XMLP_ret::XML_OK != getXMLPartitionQos(p_aux0, qos.m_partition, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PUB_MODE) == 0)
        {
            // publishMode
            if (XMLP_ret::XML_OK != getXMLPublishModeQos(p_aux0, qos.m_publishMode, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DEADLINE) == 0)
        {
            // deadline
            if (XMLP_ret::XML_OK != getXMLDeadlineQos(p_aux0, qos.m_deadline, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, LIFESPAN) == 0)
        {
            // lifespan
            if (XMLP_ret::XML_OK != getXMLLifespanQos(p_aux0, qos.m_lifespan, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DISABLE_POSITIVE_ACKS) == 0)
        {
            // Disable positive acks
            if (XMLP_ret::XML_OK != getXMLDisablePositiveAcksQos(p_aux0, qos.m_disablePositiveACKs, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, LATENCY_BUDGET) == 0)
        {
            //Latency Budget
            if (XMLP_ret::XML_OK != getXMLLatencyBudgetQos(p_aux0, qos.m_latencyBudget, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DURABILITY_SRV) == 0 ||
                strcmp(name, TIME_FILTER) == 0 || strcmp(name, DEST_ORDER) == 0 ||
                strcmp(name, PRESENTATION) == 0)
        {
            // TODO: Do not supported for now
            //if (nullptr != (p_aux = elem->FirstChildElement(    DURABILITY_SRV))) getXMLDurabilityServiceQos(p_aux, ident);
            //if (nullptr != (p_aux = elem->FirstChildElement(       TIME_FILTER))) getXMLTimeBasedFilterQos(p_aux, ident);
            //if (nullptr != (p_aux = elem->FirstChildElement(        DEST_ORDER))) getXMLDestinationOrderQos(p_aux, ident);
            //if (nullptr != (p_aux = elem->FirstChildElement(      PRESENTATION))) getXMLPresentationQos(p_aux, ident);
            EPROSIMA_LOG_ERROR(XMLPARSER, "Quality of Service '" << p_aux0->Value() << "' do not supported for now");
        }
        else if (strcmp(name, DATA_SHARING) == 0)
        {
            //data sharing
            if (XMLP_ret::XML_OK != getXMLDataSharingQos(p_aux0, qos.data_sharing, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, OWNERSHIP) == 0)
        {
            //ownership
            if (XMLP_ret::XML_OK != getXMLOwnershipQos(p_aux0, qos.m_ownership, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, OWNERSHIP_STRENGTH) == 0)
        {
            //ownership
            if (XMLP_ret::XML_OK != getXMLOwnershipStrengthQos(p_aux0, qos.m_ownershipStrength, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DISABLE_HEARTBEAT_PIGGYBACK) == 0)
        {
            // Disable heartbeat piggyback
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &qos.disable_heartbeat_piggyback, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (0 == strcmp(name, USER_DATA))
        {
            // userData
            if (XMLP_ret::XML_OK != getXMLOctetVector(p_aux0, qos.m_userData.data_vec(), ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (0 == strcmp(name, TOPIC_DATA))
        {
            // userData
            if (XMLP_ret::XML_OK != getXMLOctetVector(p_aux0, qos.m_topicData.data_vec(), ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (0 == strcmp(name, GROUP_DATA))
        {
            // userData
            if (XMLP_ret::XML_OK != getXMLOctetVector(p_aux0, qos.m_groupData.data_vec(), ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'writerQosPoliciesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLReaderQosPolicies(
        tinyxml2::XMLElement* elem,
        ReaderQos& qos,
        uint8_t ident)
{
    /*
        <xs:complexType name="readerQosPoliciesType">
            <xs:all minOccurs="0">
                <xs:element name="durability" type="durabilityQosPolicyType" minOccurs="0"/>
                <xs:element name="durabilityService" type="durabilityServiceQosPolicyType" minOccurs="0"/>
                <xs:element name="deadline" type="deadlineQosPolicyType" minOccurs="0"/>
                <xs:element name="latencyBudget" type="latencyBudgetQosPolicyType" minOccurs="0"/>
                <xs:element name="liveliness" type="livelinessQosPolicyType" minOccurs="0"/>
                <xs:element name="reliability" type="reliabilityQosPolicyType" minOccurs="0"/>
                <xs:element name="lifespan" type="lifespanQosPolicyType" minOccurs="0"/>
                <xs:element name="userData" type="userDataQosPolicyType" minOccurs="0"/>
                <xs:element name="timeBasedFilter" type="timeBasedFilterQosPolicyType" minOccurs="0"/>
                <xs:element name="ownership" type="ownershipQosPolicyType" minOccurs="0"/>
                <xs:element name="destinationOrder" type="destinationOrderQosPolicyType" minOccurs="0"/>
                <xs:element name="presentation" type="presentationQosPolicyType" minOccurs="0"/>
                <xs:element name="partition" type="partitionQosPolicyType" minOccurs="0"/>
                <xs:element name="topicData" type="topicDataQosPolicyType" minOccurs="0"/>
                <xs:element name="groupData" type="groupDataQosPolicyType" minOccurs="0"/>
                <xs:element name="data_sharing" type="dataSharingQosPolicyType" minOccurs="0"/>
                <xs:element name="disablePositiveAcks" type="disablePositiveAcksQosPolicyType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, DURABILITY) == 0)
        {
            // durability
            if (XMLP_ret::XML_OK != getXMLDurabilityQos(p_aux0, qos.m_durability, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, LIVELINESS) == 0)
        {
            // liveliness
            if (XMLP_ret::XML_OK != getXMLLivelinessQos(p_aux0, qos.m_liveliness, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, RELIABILITY) == 0)
        {
            // reliability
            if (XMLP_ret::XML_OK != getXMLReliabilityQos(p_aux0, qos.m_reliability, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PARTITION) == 0)
        {
            // partition
            if (XMLP_ret::XML_OK != getXMLPartitionQos(p_aux0, qos.m_partition, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DEADLINE) == 0)
        {
            // deadline
            if (XMLP_ret::XML_OK != getXMLDeadlineQos(p_aux0, qos.m_deadline, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, LIFESPAN) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLLifespanQos(p_aux0, qos.m_lifespan, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DISABLE_POSITIVE_ACKS) == 0)
        {
            // Disable positive acks
            if (XMLP_ret::XML_OK != getXMLDisablePositiveAcksQos(p_aux0, qos.m_disablePositiveACKs, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, LATENCY_BUDGET) == 0)
        {
            //Latency Budget
            if (XMLP_ret::XML_OK != getXMLLatencyBudgetQos(p_aux0, qos.m_latencyBudget, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DURABILITY_SRV) == 0 ||
                strcmp(name, TIME_FILTER) == 0 || strcmp(name, DEST_ORDER) == 0 ||
                strcmp(name, PRESENTATION) == 0)
        {
            // TODO: Do not supported for now
            //if (nullptr != (p_aux = elem->FirstChildElement(    DURABILITY_SRV))) getXMLDurabilityServiceQos(p_aux, ident);
            //if (nullptr != (p_aux = elem->FirstChildElement(       TIME_FILTER))) getXMLTimeBasedFilterQos(p_aux, ident);
            //if (nullptr != (p_aux = elem->FirstChildElement(        DEST_ORDER))) getXMLDestinationOrderQos(p_aux, ident);
            //if (nullptr != (p_aux = elem->FirstChildElement(      PRESENTATION))) getXMLPresentationQos(p_aux, ident);
            EPROSIMA_LOG_ERROR(XMLPARSER, "Quality of Service '" << p_aux0->Value() << "' do not supported for now");
        }
        else if (strcmp(name, DATA_SHARING) == 0)
        {
            //data sharing
            if (XMLP_ret::XML_OK != getXMLDataSharingQos(p_aux0, qos.data_sharing, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, OWNERSHIP) == 0)
        {
            //ownership
            if (XMLP_ret::XML_OK != getXMLOwnershipQos(p_aux0, qos.m_ownership, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (0 == strcmp(name, USER_DATA))
        {
            // userData
            if (XMLP_ret::XML_OK != getXMLOctetVector(p_aux0, qos.m_userData.data_vec(), ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (0 == strcmp(name, TOPIC_DATA))
        {
            // userData
            if (XMLP_ret::XML_OK != getXMLOctetVector(p_aux0, qos.m_topicData.data_vec(), ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (0 == strcmp(name, GROUP_DATA))
        {
            // userData
            if (XMLP_ret::XML_OK != getXMLOctetVector(p_aux0, qos.m_groupData.data_vec(), ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'readerQosPoliciesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLDurabilityQos(
        tinyxml2::XMLElement* elem,
        DurabilityQosPolicy& durability,
        uint8_t /*ident*/)
{
    /*
        <xs:complexType name="durabilityQosPolicyType">
            <xs:all>
                <xs:element name="kind" type="durabilityQosKindType"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    bool bKindDefined = false;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, KIND) == 0)
        {
            // kind
            /*
                <xs:simpleType name="durabilityQosKindType">
                    <xs:restriction base="xs:string">
                        <xs:enumeration value="VOLATILE"/>
                        <xs:enumeration value="TRANSIENT_LOCAL"/>
                        <xs:enumeration value="TRANSIENT"/>
                        <xs:enumeration value="PERSISTENT"/>
                    </xs:restriction>
                </xs:simpleType>
             */
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' without content");
                return XMLP_ret::XML_ERROR;
            }
            bKindDefined = true;

            if (!get_element_enum_value(text.c_str(), durability.kind,
                    _VOLATILE, DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS,
                    _TRANSIENT_LOCAL, DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS,
                    _TRANSIENT, DurabilityQosPolicyKind::TRANSIENT_DURABILITY_QOS,
                    _PERSISTENT, DurabilityQosPolicyKind::PERSISTENT_DURABILITY_QOS))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' with bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'durabilityQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    if (!bKindDefined)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'durabilityQosPolicyType' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

// TODO Implement DurabilityServiceQos
/*
   XMLP_ret XMLParser::getXMLDurabilityServiceQos(
        tinyxml2::XMLElement* elem,
        DurabilityServiceQosPolicy& durabilityService,
        uint8_t ident)
   {

    //    <xs:complexType name="durabilityServiceQosPolicyType">
    //        <xs:all minOccurs="0">
    //            <xs:element name="service_cleanup_delay" type="durationType" minOccurs="0"/>
    //            <xs:element name="history_kind" type="historyQosKindType" minOccurs="0"/>
    //            <xs:element name="history_depth" type="uint32Type" minOccurs="0"/>
    //            <xs:element name="max_samples" type="uint32Type" minOccurs="0"/>
    //            <xs:element name="max_instances" type="uint32Type" minOccurs="0"/>
    //            <xs:element name="max_samples_per_instance" type="uint32Type" minOccurs="0"/>
    //        </xs:all>
    //    </xs:complexType>


    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, SRV_CLEAN_DELAY) == 0)
        {
            // service_cleanup_delay - durationType
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, durabilityService.service_cleanup_delay, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, HISTORY_KIND) == 0)
        {
            // history_kind
            //
            //    <xs:simpleType name="historyQosKindType">
            //        <xs:restriction base="xs:string">
            //            <xs:enumeration value="KEEP_LAST"/>
            //            <xs:enumeration value="KEEP_ALL"/>
            //        </xs:restriction>
            //    </xs:simpleType>
            //
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << HISTORY_KIND << "' without content");
                return XMLP_ret::XML_ERROR;
            }
            if (strcmp(text.c_str(), KEEP_LAST) == 0)
            {
                durabilityService.history_kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
            }
            else if (strcmp(text.c_str(), KEEP_ALL) == 0)
            {
                durabilityService.history_kind = HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << HISTORY_KIND << "' with bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, HISTORY_DEPTH) == 0)
        {
            // history_depth - uint32Type
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &durabilityService.history_depth, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, MAX_SAMPLES) == 0)
        {
            // max_samples - uint32Type
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &durabilityService.max_samples, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, MAX_INSTANCES) == 0)
        {
            // max_instances - uint32Type
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &durabilityService.max_instances, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, MAX_SAMPLES_INSTANCE) == 0)
        {
            // max_samples_per_instance - uint32Type
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &durabilityService.max_samples_per_instance, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'durabilityServiceQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
   }
 */

XMLP_ret XMLParser::getXMLDeadlineQos(
        tinyxml2::XMLElement* elem,
        DeadlineQosPolicy& deadline,
        uint8_t ident)
{
    /*
        <xs:complexType name="deadlineQosPolicyType">
            <xs:all>
                <xs:element name="period" type="durationType"/>
            </xs:all>
        </xs:complexType>
     */
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    bool bPeriodDefined = false;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, PERIOD) == 0)
        {
            bPeriodDefined = true;
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, deadline.period, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'deadlineQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    if (!bPeriodDefined)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'deadlineQosPolicyType' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLatencyBudgetQos(
        tinyxml2::XMLElement* elem,
        LatencyBudgetQosPolicy& latencyBudget,
        uint8_t ident)
{
    /*
        <xs:complexType name="latencyBudgetQosPolicyType">
            <xs:all>
                <xs:element name="duration" type="durationType"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    bool bDurationDefined = false;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, DURATION) == 0)
        {
            bDurationDefined = true;
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, latencyBudget.duration, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'latencyBudgetQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    if (!bDurationDefined)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'latencyBudgetQosPolicyType' without content");
        return XMLP_ret::XML_ERROR;
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLivelinessQos(
        tinyxml2::XMLElement* elem,
        LivelinessQosPolicy& liveliness,
        uint8_t ident)
{
    /*
        <xs:complexType name="livelinessQosPolicyType">
            <xs:all minOccurs="0">
                <xs:element name="kind" type="livelinessQosKindType" minOccurs="0"/>
                <xs:element name="leaseDuration" type="durationType" minOccurs="0"/>
                <xs:element name="announcement_period" type="durationType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, KIND) == 0)
        {
            // kind
            /*
                <xs:simpleType name="livelinessQosKindType">
                    <xs:restriction base="xs:string">
                        <xs:enumeration value="AUTOMATIC"/>
                        <xs:enumeration value="MANUAL_BY_PARTICIPANT"/>
                        <xs:enumeration value="MANUAL_BY_TOPIC"/>
                    </xs:restriction>
                </xs:simpleType>
             */
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' without content");
                return XMLP_ret::XML_ERROR;
            }

            if (!get_element_enum_value(text.c_str(), liveliness.kind,
                    AUTOMATIC, LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS,
                    MANUAL_BY_PARTICIPANT, LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
                    MANUAL_BY_TOPIC, LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' with bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, LEASE_DURATION) == 0)
        {
            // lease_duration
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, liveliness.lease_duration, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, ANNOUNCE_PERIOD) == 0)
        {
            // announcement_period
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, liveliness.announcement_period, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'livelinessQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLReliabilityQos(
        tinyxml2::XMLElement* elem,
        ReliabilityQosPolicy& reliability,
        uint8_t ident)
{
    /*
        <xs:complexType name="reliabilityQosPolicyType">
            <xs:all>
                <xs:element name="kind" type="reliabilityQosKindType" minOccurs="0"/>
                <xs:element name="max_blocking_time" type="durationType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, KIND) == 0)
        {
            // kind
            /*
                <xs:simpleType name="reliabilityQosKindType">
                    <xs:restriction base="xs:string">
                        <xs:enumeration value="BEST_EFFORT"/>
                        <xs:enumeration value="RELIABLE"/>
                    </xs:restriction>
                </xs:simpleType>
             */
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' without content");
                return XMLP_ret::XML_ERROR;
            }

            if (!get_element_enum_value(text.c_str(), reliability.kind,
                    _BEST_EFFORT, ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS,
                    _RELIABLE, ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' with bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, MAX_BLOCK_TIME) == 0)
        {
            // max_blocking_time
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, reliability.max_blocking_time, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'reliabilityQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLifespanQos(
        tinyxml2::XMLElement* elem,
        LifespanQosPolicy& lifespan,
        uint8_t ident)
{
    /*
        <xs:complexType name="lifespanQosPolicyType">
            <xs:all>
                <xs:element name="duration" type="durationType"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    bool bDurationDefined = false;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, DURATION) == 0)
        {
            bDurationDefined = true;
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, lifespan.duration, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'lifespanQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    if (!bDurationDefined)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'lifespanQosPolicyType' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLDisablePositiveAcksQos(
        tinyxml2::XMLElement* elem,
        DisablePositiveACKsQosPolicy& disablePositiveAcks,
        uint8_t ident)
{
    /*
        <xs:complexType name="disablePositiveAcksQosPolicyType">
            <xs:all>
                <xs:element name="enabled" type="boolType"/>
                <xs:element name="duration" type="durationType"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, ENABLED) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &disablePositiveAcks.enabled, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, DURATION) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, disablePositiveAcks.duration, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'disablePositiveAcksQosPolicyType' with unknown content");
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLDataSharingQos(
        tinyxml2::XMLElement* elem,
        DataSharingQosPolicy& data_sharing,
        uint8_t ident)
{
    /*
        <xs:complexType name="dataSharingQosPolicyType">
            <xs:all>
                <xs:element name="kind" type="datasharingQosKindType" minOccurs="1"/>
                <xs:element name="shared_dir" type="stringType" minOccurs="0"/>
                <xs:element name="domain_ids" type="domainIdVectorType" minOccurs="0"/>
                <xs:element name="max_domains" type="uint32Type" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */
    bool kind_found = false;
    DataSharingKind kind = DataSharingKind::AUTO;
    std::string shm_directory = "";
    int32_t max_domains = 0;
    std::vector<uint16_t> domain_ids;

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, KIND) == 0)
        {
            /*
                <xs:simpleType name="datasharingQosKindType">
                    <xs:restriction base="xs:string">
                        <xs:enumeration value="ON"/>
                        <xs:enumeration value="OFF"/>
                        <xs:enumeration value="DEFAULT"/>
                    </xs:restriction>
                </xs:simpleType>
             */
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' without content");
                return XMLP_ret::XML_ERROR;
            }

            if (!get_element_enum_value(text.c_str(), kind,
                    ON, DataSharingKind::ON,
                    OFF, DataSharingKind::OFF,
                    AUTOMATIC, DataSharingKind::AUTO,
                    AUTO, DataSharingKind::AUTO))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' with bad content");
                return XMLP_ret::XML_ERROR;
            }
            kind_found = true;
        }
        else if (strcmp(name, SHARED_DIR) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &shm_directory, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, MAX_DOMAINS) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &max_domains, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
            if (max_domains < 0)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "max domains cannot be negative");
                return XMLP_ret::XML_ERROR;
            }

        }
        else if (strcmp(name, DOMAIN_IDS) == 0)
        {
            /*
                <xs:complexType name="domainIdVectorType">
                    <xs:sequence>
                        <xs:element name="domainId" type="uint16Type" maxOccurs="unbounded"/>
                    </xs:sequence>
                </xs:complexType>
             */

            tinyxml2::XMLElement* p_aux1;
            const char* name1 = nullptr;
            bool domain_id_found = false;
            for (p_aux1 = p_aux0->FirstChildElement(); p_aux1 != NULL; p_aux1 = p_aux1->NextSiblingElement())
            {
                name1 = p_aux1->Name();
                if (strcmp(name1, DOMAIN_ID) == 0)
                {
                    uint16_t id;
                    if (XMLP_ret::XML_OK != getXMLUint(p_aux1, &id, ident))
                    {
                        return XMLP_ret::XML_ERROR;
                    }
                    domain_ids.push_back(id);
                    domain_id_found = true;
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found in 'domain_ids'. Name: " << name1);
                    return XMLP_ret::XML_ERROR;
                }
            }

            if (!domain_id_found)
            {
                // Not even one
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << DOMAIN_IDS << "' without content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found in 'data_sharing'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    if (!kind_found)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'data_sharing' without kind");
        return XMLP_ret::XML_ERROR;
    }

    if (max_domains != 0 && domain_ids.size() > static_cast<uint32_t>(max_domains))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'data_sharing' defines a maximum of " << max_domains
                                                                                  << " domain IDs but also define " << domain_ids.size() <<
                " domain IDs");
        return XMLP_ret::XML_ERROR;
    }

    data_sharing.set_max_domains(static_cast<uint32_t>(max_domains));

    switch (kind)
    {
        case DataSharingKind::ON:
            data_sharing.on(shm_directory, domain_ids);
            break;

        case DataSharingKind::AUTO:
            data_sharing.automatic(shm_directory, domain_ids);
            break;

        case DataSharingKind::OFF:
            data_sharing.off();
            break;

        default:
            break;
    }

    return XMLP_ret::XML_OK;
}

// TODO Implement TimeBasedFilterQos
/*
   XMLP_ret XMLParser::getXMLTimeBasedFilterQos(
        tinyxml2::XMLElement* elem,
        TimeBasedFilterQosPolicy& timeBasedFilter,
        uint8_t ident)
   {

    //    <xs:complexType name="timeBasedFilterQosPolicyType">
    //        <xs:all>
    //          <xs:element name="minimum_separation" type="durationType"/>
    //        </xs:all>
    //    </xs:complexType>


    tinyxml2::XMLElement* p_aux0 = nullptr;
    bool bSeparationDefined = false;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, MIN_SEPARATION) == 0)
        {
            bSeparationDefined = true;
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, timeBasedFilter.minimum_separation, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'timeBasedFilterQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    if (!bSeparationDefined)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'timeBasedFilterQosPolicyType' without content");
        return XMLP_ret::XML_ERROR;
    }
    return XMLP_ret::XML_OK;
   }
 */

XMLP_ret XMLParser::getXMLOwnershipQos(
        tinyxml2::XMLElement* elem,
        OwnershipQosPolicy& ownership,
        uint8_t ident)
{
    (void)ident;

    //    <xs:complexType name="ownershipQosPolicyType">
    //        <xs:all>
    //            <xs:element name="kind" type="ownershipQosKindType"/>
    //        </xs:all>
    //    </xs:complexType>

    tinyxml2::XMLElement* p_aux0 = nullptr;
    bool bKindDefined = false;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, KIND) == 0)
        {

            //    <xs:simpleType name="ownershipQosKindType">
            //        <xs:restriction base="xs:string">
            //            <xs:enumeration value="SHARED"/>
            //            <xs:enumeration value="EXCLUSIVE"/>
            //        </xs:restriction>
            //    </xs:simpleType>

            bKindDefined = true;
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' without content");
                return XMLP_ret::XML_ERROR;
            }

            if (!get_element_enum_value(text.c_str(), ownership.kind,
                    SHARED, OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS,
                    EXCLUSIVE, OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' with bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'ownershipQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    if (!bKindDefined)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'ownershipQosPolicyType' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLOwnershipStrengthQos(
        tinyxml2::XMLElement* elem,
        OwnershipStrengthQosPolicy& ownershipStrength,
        uint8_t ident)
{

    //    <xs:complexType name="ownershipStrengthQosPolicyType">
    //        <xs:all>
    //            <xs:element name="value" type="uint32Type"/>
    //        </xs:all>
    //    </xs:complexType>

    tinyxml2::XMLElement* p_aux0 = nullptr;
    bool bValueDefined = false;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, VALUE) == 0)
        {
            bValueDefined = true;
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &ownershipStrength.value, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Invalid element found into 'ownershipStrengthQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    if (!bValueDefined)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'ownershipStrengthQosPolicyType' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

// TODO Implement DestinationOrderQos
/*
   XMLP_ret XMLParser::getXMLDestinationOrderQos(
        tinyxml2::XMLElement* elem,
        DestinationOrderQosPolicy& destinationOrder,
        uint8_t ident)
   {

    //    <xs:complexType name="destinationOrderQosPolicyType">
    //        <xs:all>
    //            <xs:element name="kind" type="destinationOrderQosKindType"/>
    //        </xs:all>
    //    </xs:complexType>


    tinyxml2::XMLElement* p_aux0 = nullptr;
    bool bKindDefined = false;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, KIND) == 0)
        {

            //    <xs:simpleType name="destinationOrderQosKindType">
            //        <xs:restriction base="xs:string">
            //            <xs:enumeration value="BY_RECEPTION_TIMESTAMP"/>
            //            <xs:enumeration value="BY_SOURCE_TIMESTAMP"/>
            //        </xs:restriction>
            //    </xs:simpleType>

            bKindDefined = true;
            const char* text = get_element_text(p_aux0);
            if (nullptr == text)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' without content");
                return XMLP_ret::XML_ERROR;
            }
            if (strcmp(text, BY_RECEPTION_TIMESTAMP) == 0)
            {
                destinationOrder.kind = DestinationOrderQosPolicyKind::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
            }
            else if (strcmp(text, BY_SOURCE_TIMESTAMP) == 0)
            {
                destinationOrder.kind = DestinationOrderQosPolicyKind::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'destinationOrderQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    if (!bKindDefined)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'destinationOrderQosPolicyType' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
   }
 */

// TODO Implement PresentationQos
/*
   XMLP_ret XMLParser::getXMLPresentationQos(
        tinyxml2::XMLElement* elem,
        PresentationQosPolicy& presentation,
        uint8_t ident)
   {

    //    <xs:complexType name="presentationQosPolicyType">
    //        <xs:all minOccurs="0">
    //            <xs:element name="access_scope" type="presentationQosKindType" minOccurs="0"/>
    //            <xs:element name="coherent_access" type="boolType" minOccurs="0"/>
    //            <xs:element name="ordered_access" type="boolType" minOccurs="0"/>
    //        </xs:all>
    //    </xs:complexType>


    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, ACCESS_SCOPE) == 0)
        {
            // access_scope

            //    <xs:simpleType name="presentationQosKindType">
            //        <xs:restriction base="xs:string">
            //            <xs:enumeration value="INSTANCE"/>
            //            <xs:enumeration value="TOPIC"/>
            //            <xs:enumeration value="GROUP"/>
            //        </xs:restriction>
            //    </xs:simpleType>

            const char* text = get_element_text(p_aux0);
            if (nullptr == text)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << ACCESS_SCOPE << "' without content");
                return XMLP_ret::XML_ERROR;
            }
            if (strcmp(text, INSTANCE) == 0)
            {
                presentation.access_scope = PresentationQosPolicyAccessScopeKind::INSTANCE_PRESENTATION_QOS;
            }
            else if (strcmp(text, TOPIC) == 0)
            {
                presentation.access_scope = PresentationQosPolicyAccessScopeKind::TOPIC_PRESENTATION_QOS;
            }
            else if (strcmp(text, GROUP) == 0)
            {
                presentation.access_scope = PresentationQosPolicyAccessScopeKind::GROUP_PRESENTATION_QOS;
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << ACCESS_SCOPE << "' bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, COHERENT_ACCESS) == 0)
        {
            // coherent_access - boolType
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &presentation.coherent_access, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, ORDERED_ACCESS) == 0)
        {
            // ordered_access - boolType
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &presentation.ordered_access, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'presentationQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
   }
 */

XMLP_ret XMLParser::getXMLPartitionQos(
        tinyxml2::XMLElement* elem,
        PartitionQosPolicy& partition,
        uint8_t ident)
{
    /*
        <xs:complexType name="partitionQosPolicyType">
            <xs:all>
                <xs:element name="names" type="nameVectorType"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr, * p_aux1 = nullptr;
    bool bNamesDefined = false;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, NAMES) == 0)
        {
            bNamesDefined = true;
            p_aux1 = p_aux0->FirstChildElement(NAME);
            if (nullptr == p_aux1)
            {
                // Not even one
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << NAMES << "' without content");
                return XMLP_ret::XML_ERROR;
            }

            std::vector<std::string> names;
            while (nullptr != p_aux1)
            {
                std::string sName = "";
                if (XMLP_ret::XML_OK != getXMLString(p_aux1, &sName, ident))
                {
                    return XMLP_ret::XML_ERROR;
                }
                names.push_back(sName);
                p_aux1 = p_aux1->NextSiblingElement(NAME);
            }
            partition.names(names);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'partitionQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    if (!bNamesDefined)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'partitionQosPolicyType' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLPublishModeQos(
        tinyxml2::XMLElement* elem,
        PublishModeQosPolicy& publishMode,
        uint8_t /*ident*/)
{
    /*
        <xs:complexType name="publishModeQosPolicyType">
            <xs:all>
                <xs:element name="kind" type="publishModeQosKindType"/>
            </xs:all>
        </xs:complexType>
     */
    tinyxml2::XMLElement* p_aux0 = nullptr;
    bool bKindDefined = false;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, KIND) == 0)
        {
            /*
                <xs:simpleType name="publishModeQosKindType">
                    <xs:restriction base="xs:string">
                        <xs:enumeration value="SYNCHRONOUS"/>
                        <xs:enumeration value="ASYNCHRONOUS"/>
                    </xs:restriction>
                </xs:simpleType>
             */
            bKindDefined = true;
            std::string text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' without content");
                return XMLP_ret::XML_ERROR;
            }

            if (!get_element_enum_value(text.c_str(), publishMode.kind,
                    SYNCHRONOUS, PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE,
                    ASYNCHRONOUS, PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'publishModeQosPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    if (!bKindDefined)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'publishModeQosPolicyType' without content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLDuration(
        tinyxml2::XMLElement* elem,
        Duration_t& duration,
        uint8_t ident)
{
    /*
       <xs:complexType name="durationType" mixed="true">
       <xs:sequence>
        <xs:choice minOccurs="0">
         <xs:sequence>
          <xs:element name="sec" type="nonNegativeInteger_Duration_SEC" default="0" minOccurs="1" maxOccurs="1"/>
          <xs:element name="nanosec" type="nonNegativeInteger_Duration_NSEC" default="0" minOccurs="0" maxOccurs="1"/>
         </xs:sequence>
         <xs:sequence>
          <xs:element name="nanosec" type="nonNegativeInteger_Duration_NSEC" default="0" minOccurs="1" maxOccurs="1"/>
          <xs:element name="sec" type="nonNegativeInteger_Duration_SEC" default="0" minOccurs="0" maxOccurs="1"/>
        </xs:sequence>
       </xs:choice>
       </xs:sequence>
       </xs:complexType>
     */

    // set default values
    duration.seconds = 0;
    duration.nanosec = 0;

    // it's mandatory to provide a sec or nanocsec child item
    bool empty = true;

    // First we check if it matches the schema pattern
    std::regex infinite(DURATION_INFINITY);
    std::regex infinite_sec(DURATION_INFINITE_SEC);
    std::regex infinite_nsec(DURATION_INFINITE_NSEC);
    std::string text = get_element_text(elem);

    if (!text.empty() && std::regex_match(text, infinite))
    {
        empty = false;
        duration = c_TimeInfinite;

        if (elem->FirstChildElement() != nullptr)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "If a Duration_t type element is defined as DURATION_INFINITY it cannot have <sec> or"
                    " <nanosec> subelements.");
            return XMLP_ret::XML_ERROR;
        }
    }

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        // there is at least a child element
        empty = false;

        name = p_aux0->Name();
        if (strcmp(name, SECONDS) == 0)
        {
            /*
               <xs:simpleType name="nonNegativeInteger_Duration_SEC">
                    <xs:union>
                        <xs:simpleType>
                            <xs:restriction base="xs:string">
                                <xs:pattern value="\s*(DURATION_INFINITY|DURATION_INFINITE_SEC)\s*"/>
                            </xs:restriction>
                        </xs:simpleType>
                        <xs:simpleType>
                            <xs:restriction base="xs:unsignedInt"/>
                        </xs:simpleType>
                    </xs:union>
                </xs:simpleType>
             */
            text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'SECONDS' without content");
                return XMLP_ret::XML_ERROR;
            }
            else if (std::regex_match(text, infinite_sec))
            {
                // if either SECONDS or NANOSECONDS is set to infinity then all of it is
                duration = c_TimeInfinite;
                return XMLP_ret::XML_OK;
            }
            else if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &duration.seconds, ident))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLInt XML_ERROR!");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, NANOSECONDS) == 0)
        {
            /*
                <xs:simpleType name="nonNegativeInteger_Duration_NSEC">
                    <xs:union>
                        <xs:simpleType>
                            <xs:restriction base="xs:string">
                                <xs:pattern value="\s*(DURATION_INFINITY|DURATION_INFINITE_NSEC)\s*"/>
                            </xs:restriction>
                        </xs:simpleType>
                        <xs:simpleType>
                            <xs:restriction base="xs:unsignedInt"/>
                        </xs:simpleType>
                    </xs:union>
                </xs:simpleType>
             */
            text = get_element_text(p_aux0);
            if (text.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node 'NANOSECONDS' without content");
                return XMLP_ret::XML_ERROR;
            }
            else if (std::regex_match(text, infinite_nsec))
            {
                // if either SECONDS or NANOSECONDS is set to infinity then all of it is
                duration = c_TimeInfinite;
                return XMLP_ret::XML_OK;
            }
            else if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &duration.nanosec, ident))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLInt XML_ERROR!");
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'durationType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    // An empty Duration_t xml is forbidden
    if (empty)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "'durationType' elements cannot be empty."
                "At least second or nanoseconds should be provided");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLWriterTimes(
        tinyxml2::XMLElement* elem,
        WriterTimes& times,
        uint8_t ident)
{
    /*
        <xs:complexType name="writerTimesType">
            <xs:all minOccurs="0">
                <xs:element name="initialHeartbeatDelay" type="durationType" minOccurs="0"/>
                <xs:element name="heartbeatPeriod" type="durationType" minOccurs="0"/>
                <xs:element name="nackResponseDelay" type="durationType" minOccurs="0"/>
                <xs:element name="nackSupressionDuration" type="durationType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, INIT_HEARTB_DELAY) == 0)
        {
            // initialHeartbeatDelay
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.initialHeartbeatDelay, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, HEARTB_PERIOD) == 0)
        {
            // heartbeatPeriod
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.heartbeatPeriod, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, NACK_RESP_DELAY) == 0)
        {
            // nackResponseDelay
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.nackResponseDelay, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, NACK_SUPRESSION) == 0)
        {
            // nackSupressionDuration
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.nackSupressionDuration, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'writerTimesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLReaderTimes(
        tinyxml2::XMLElement* elem,
        ReaderTimes& times,
        uint8_t ident)
{
    /*
        <xs:complexType name="readerTimesType">
            <xs:all minOccurs="0">
                <xs:element name="initialAcknackDelay" type="durationType" minOccurs="0"/>
                <xs:element name="heartbeatResponseDelay" type="durationType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, INIT_ACKNACK_DELAY) == 0)
        {
            // initialAcknackDelay
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.initialAcknackDelay, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, HEARTB_RESP_DELAY) == 0)
        {
            // heartbeatResponseDelay
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.heartbeatResponseDelay, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'readerTimesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLocatorUDPv4(
        tinyxml2::XMLElement* elem,
        rtps::Locator_t& locator,
        uint8_t ident)
{
    /*
        <xs:complexType name="udpv4LocatorType">
            <xs:all minOccurs="0">
                <xs:element name="port" type="uint32Type" minOccurs="0"/>
                <xs:element name="address" type="stringType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    std::unordered_map<std::string, bool> tags_present;

    locator.kind = LOCATOR_KIND_UDPv4;
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (tags_present[name])
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'udpv4LocatorType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present[name] = true;

        if (strcmp(name, PORT) == 0)
        {
            // port - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &locator.port, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, ADDRESS) == 0)
        {
            // address - stringType
            std::string s = "";
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            // Check whether the address is IPv4
            if (!IPLocator::isIPv4(s))
            {
                auto response = rtps::IPLocator::resolveNameDNS(s);

                // Add the first valid IPv4 address that we can find
                if (response.first.size() > 0)
                {
                    s = response.first.begin()->data();
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER,
                            "DNS server did not return any IPv4 address for: '" << s << "'. Name: " << name);
                    return XMLP_ret::XML_ERROR;
                }
            }
            IPLocator::setIPv4(locator, s);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'udpv4LocatorType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLocatorUDPv6(
        tinyxml2::XMLElement* elem,
        rtps::Locator_t& locator,
        uint8_t ident)
{
    /*
        <xs:complexType name="udpv6LocatorType">
            <xs:all minOccurs="0">
                <xs:element name="port" type="uint32Type" minOccurs="0"/>
                <xs:element name="address" type="stringType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    std::unordered_map<std::string, bool> tags_present;

    locator.kind = LOCATOR_KIND_UDPv6;
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (tags_present[name])
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'udpv6LocatorType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present[name] = true;

        if (strcmp(name, PORT) == 0)
        {
            // port - uint32Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &locator.port, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, ADDRESS) == 0)
        {
            // address - stringType
            std::string s = "";
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            // Check whether the address is IPv6
            if (!IPLocator::isIPv6(s))
            {
                auto response = rtps::IPLocator::resolveNameDNS(s);

                // Add the first valid IPv6 address that we can find
                if (response.second.size() > 0)
                {
                    s = response.second.begin()->data();
                }
                else
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER,
                            "DNS server did not return any IPv6 address for: '" << s << "'. Name: " << name);
                    return XMLP_ret::XML_ERROR;
                }
            }
            IPLocator::setIPv6(locator, s);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'udpv6LocatorType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLocatorTCPv4(
        tinyxml2::XMLElement* elem,
        rtps::Locator_t& locator,
        uint8_t ident)
{
    /*
        <xs:complexType name="tcpv4LocatorType">
            <xs:all minOccurs="0">
                <xs:element name="port" type="uint16Type" minOccurs="0"/>
                <xs:element name="physical_port" type="uint16Type" minOccurs="0"/>
                <xs:element name="address" type="stringType" minOccurs="0"/>
                <xs:element name="wan_address" type="stringType" minOccurs="0"/>
                <xs:element name="unique_lan_id" type="stringType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    std::unordered_map<std::string, bool> tags_present;

    locator.kind = LOCATOR_KIND_TCPv4;
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (tags_present[name])
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'tcpv4LocatorType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present[name] = true;

        if (strcmp(name, PORT) == 0)
        {
            // port - uint16Type
            uint16_t port(0);
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            IPLocator::setLogicalPort(locator, port);
        }
        else if (strcmp(name, PHYSICAL_PORT) == 0)
        {
            // physical_port - uint16Type
            uint16_t port(0);
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            IPLocator::setPhysicalPort(locator, port);
        }
        else if (strcmp(name, ADDRESS) == 0)
        {
            // address - stringType
            std::string s = "";
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            IPLocator::setIPv4(locator, s);
        }
        else if (strcmp(name, WAN_ADDRESS) == 0)
        {
            // address - stringType
            std::string s = "";
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            IPLocator::setWan(locator, s);
        }
        else if (strcmp(name, UNIQUE_LAN_ID) == 0)
        {
            // address - stringType
            std::string s = "";
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            IPLocator::setLanID(locator, s);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'tcpv4LocatorType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLocatorTCPv6(
        tinyxml2::XMLElement* elem,
        rtps::Locator_t& locator,
        uint8_t ident)
{
    /*
        <xs:complexType name="tcpv6LocatorType">
            <xs:choice>
                <xs:element name="port" type="uint16Type" minOccurs="0"/>
                <xs:element name="physical_port" type="uint16Type" minOccurs="0"/>
                <xs:element name="address" type="stringType" minOccurs="0"/>
            </xs:choice>
        </xs:complexType>
     */

    std::unordered_map<std::string, bool> tags_present;

    locator.kind = LOCATOR_KIND_TCPv6;
    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != NULL; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (tags_present[name])
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'tcpv6LocatorType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present[name] = true;

        if (strcmp(name, PORT) == 0)
        {
            // port - uint16Type
            uint16_t port(0);
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            IPLocator::setLogicalPort(locator, port);
        }
        else if (strcmp(name, PHYSICAL_PORT) == 0)
        {
            // physical_port - uint16Type
            uint16_t port(0);
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            IPLocator::setPhysicalPort(locator, port);
        }
        else if (strcmp(name, ADDRESS) == 0)
        {
            // address - stringType
            std::string s = "";
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            IPLocator::setIPv6(locator, s);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'tcpv6LocatorType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

template<typename T>
static XMLP_ret process_unsigned_attribute(
        const tinyxml2::XMLElement* elem,
        const char* name,
        T& value,
        const unsigned int min_value,
        const unsigned int max_value)
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
                "Wrong value '" << attribute->Value() << "' for attribute '" << name << "' on '" <<
                elem->Name() << "'");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

static XMLP_ret process_external_locator_attributes(
        const tinyxml2::XMLElement* elem,
        eprosima::fastdds::rtps::LocatorWithMask& locator,
        eprosima::fastdds::rtps::ExternalLocators& external_locators)
{
    static const char* EXTERNALITY_ATTR_NAME = "externality";
    static const char* COST_ATTR_NAME = "cost";
    static const char* MASK_ATTR_NAME = "mask";

    // Attributes initialized with default value
    uint8_t externality = 1;
    uint8_t cost = 0;
    uint8_t mask = 24;

    if (XMLP_ret::XML_OK != process_unsigned_attribute(elem, EXTERNALITY_ATTR_NAME, externality, 1, 255) ||
            XMLP_ret::XML_OK != process_unsigned_attribute(elem, COST_ATTR_NAME, cost, 0, 255) ||
            XMLP_ret::XML_OK != process_unsigned_attribute(elem, MASK_ATTR_NAME, mask, 1, 127))
    {
        return XMLP_ret::XML_ERROR;
    }

    locator.mask(mask);
    external_locators[externality][cost].push_back(locator);

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLExternalLocatorList(
        tinyxml2::XMLElement* elem,
        eprosima::fastdds::rtps::ExternalLocators& external_locators,
        uint8_t ident)
{
    /*
        <xs:complexType name="externalLocatorListType">
            <xs:sequence>
              <xs:choice>
                <xs:element name="udpv4" type="udpExternalLocatorType"/>
                <xs:element name="udpv6" type="udpExternalLocatorType"/>
              </xs:choice>
            </xs:sequence>
        </xs:complexType>
     */

    external_locators.clear();

    tinyxml2::XMLElement* child = nullptr;
    for (child = elem->FirstChildElement(); nullptr != child; child = child->NextSiblingElement())
    {
        fastdds::rtps::LocatorWithMask locator;
        const char* name = child->Name();
        if (strcmp(name, UDPv4_LOCATOR) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLLocatorUDPv4(child, locator, ident + 1))
            {
                external_locators.clear();
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, UDPv6_LOCATOR) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLLocatorUDPv6(child, locator, ident + 1))
            {
                external_locators.clear();
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found inside 'externalLocatorListType'. Name: " << name);
            external_locators.clear();
            return XMLP_ret::XML_ERROR;
        }

        if (IPLocator::isAny(locator) || 0 == locator.port)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Address and port are mandatory for 'udpExternalLocatorType'.");
            external_locators.clear();
            return XMLP_ret::XML_ERROR;
        }

        if (IPLocator::isMulticast(locator))
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Address should be unicast for 'udpExternalLocatorType'.");
            external_locators.clear();
            return XMLP_ret::XML_ERROR;
        }

        if (XMLP_ret::XML_OK != process_external_locator_attributes(child, locator, external_locators))
        {
            external_locators.clear();
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLLocatorList(
        tinyxml2::XMLElement* elem,
        LocatorList_t& locatorList,
        uint8_t ident)
{
    /*
        <xs:complexType name="locatorListType">
            <xs:sequence>
                <xs:element name="locator" type="locatorType" minOccurs="0" maxOccurs="unbounded"/>
            </xs:sequence>
        </xs:complexType>
     */
    tinyxml2::XMLElement* p_aux0 = nullptr, * p_aux1 = nullptr;
    p_aux0 = elem->FirstChildElement(LOCATOR);
    if (nullptr == p_aux0)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    while (nullptr != p_aux0)
    {
        /*
            <xs:complexType name="locatorType">
                <xs:choice>
                    <xs:element name="udpv4" type="udpv4LocatorType"/>
                    <xs:element name="udpv6" type="udpv6LocatorType"/>
                    <xs:element name="tcpv4" type="tcpv4LocatorType"/>
                    <xs:element name="tcpv6" type="tcpv6LocatorType"/>
                </xs:choice>
            </xs:complexType>
         */
        Locator_t loc;
        if (nullptr != (p_aux1 = p_aux0->FirstChildElement(UDPv4_LOCATOR)))
        {
            if (XMLP_ret::XML_OK != getXMLLocatorUDPv4(p_aux1, loc, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (nullptr != (p_aux1 = p_aux0->FirstChildElement(UDPv6_LOCATOR)))
        {
            if (XMLP_ret::XML_OK != getXMLLocatorUDPv6(p_aux1, loc, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (nullptr != (p_aux1 = p_aux0->FirstChildElement(TCPv4_LOCATOR)))
        {
            if (XMLP_ret::XML_OK != getXMLLocatorTCPv4(p_aux1, loc, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (nullptr != (p_aux1 = p_aux0->FirstChildElement(TCPv6_LOCATOR)))
        {
            if (XMLP_ret::XML_OK != getXMLLocatorTCPv6(p_aux1, loc, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (nullptr != (p_aux1 = p_aux0->FirstChildElement()))
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'locatorType'. Name: " << p_aux1->Name());
            return XMLP_ret::XML_ERROR;
        }

        locatorList.push_back(loc);
        p_aux0 = p_aux0->NextSiblingElement(LOCATOR);
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLHistoryMemoryPolicy(
        tinyxml2::XMLElement* elem,
        MemoryManagementPolicy_t& historyMemoryPolicy,
        uint8_t /*ident*/)
{
    /*
        <xs:simpleType name="historyMemoryPolicyType">
            <xs:restriction base="xs:string">
                <xs:enumeration value="PREALLOCATED"/>
                <xs:enumeration value="PREALLOCATED_WITH_REALLOC"/>
                <xs:enumeration value="DYNAMIC"/>
                <xs:enumeration value="DYNAMIC_REUSABLE"/>
            </xs:restriction>
        </xs:simpleType>
     */
    std::string text = get_element_text(elem);
    if (text.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    if (!get_element_enum_value(text.c_str(), historyMemoryPolicy,
            PREALLOCATED, MemoryManagementPolicy::PREALLOCATED_MEMORY_MODE,
            PREALLOCATED_WITH_REALLOC, MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE,
            DYNAMIC, MemoryManagementPolicy::DYNAMIC_RESERVE_MEMORY_MODE,
            DYNAMIC_REUSABLE, MemoryManagementPolicy::DYNAMIC_REUSABLE_MEMORY_MODE))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' bad content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLPropertiesPolicy(
        tinyxml2::XMLElement* elem,
        PropertyPolicy& propertiesPolicy,
        uint8_t ident)
{
    /*
        <xs:complexType name="propertyPolicyType">
            <xs:all minOccurs="0">
                <xs:element name="properties" type="propertyVectorType" minOccurs="0"/>
                <xs:element name="binary_properties" type="binaryPropertyVectorType" minOccurs="0"/>
            </xs:all>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr, * p_aux1 = nullptr, * p_aux2 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (strcmp(name, PROPERTIES) == 0)
        {
            p_aux1 = p_aux0->FirstChildElement(PROPERTY);
            if (nullptr == p_aux1)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << PROPERTIES << "' without content");
                return XMLP_ret::XML_ERROR;
            }

            while (nullptr != p_aux1)
            {
                /*
                    <xs:complexType name="propertyType">
                        <xs:all>
                            <xs:element name="name" type="stringType"/>
                            <xs:element name="value" type="stringType"/>
                            <xs:element name="propagate" type="boolType"/>
                        </xs:all>
                    </xs:complexType>
                 */

                const char* sub_name = nullptr;
                Property prop;
                for (p_aux2 = p_aux1->FirstChildElement(); p_aux2 != NULL; p_aux2 = p_aux2->NextSiblingElement())
                {
                    sub_name = p_aux2->Name();
                    if (strcmp(sub_name, NAME) == 0)
                    {
                        // name - stringType
                        std::string s = "";
                        if (XMLP_ret::XML_OK != getXMLString(p_aux2, &s, ident + 2))
                        {
                            return XMLP_ret::XML_ERROR;
                        }
                        prop.name(s);
                    }
                    else if (strcmp(sub_name, VALUE) == 0)
                    {
                        // value - stringType
                        std::string s = "";
                        if (XMLP_ret::XML_OK != getXMLString(p_aux2, &s, ident + 2))
                        {
                            return XMLP_ret::XML_ERROR;
                        }
                        prop.value(s);
                    }
                    else if (strcmp(sub_name, PROPAGATE) == 0)
                    {
                        // propagate - boolType
                        bool b = false;
                        if (XMLP_ret::XML_OK != getXMLBool(p_aux2, &b, ident + 2))
                        {
                            return XMLP_ret::XML_ERROR;
                        }
                        prop.propagate(b);
                    }
                }
                propertiesPolicy.properties().push_back(prop);
                p_aux1 = p_aux1->NextSiblingElement(PROPERTY);
            }
        }
        else if (strcmp(name, BIN_PROPERTIES) == 0)
        {
            // TODO: The value will be std::vector<uint8_t>
            p_aux1 = p_aux0->FirstChildElement(PROPERTY);
            if (nullptr == p_aux1)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << BIN_PROPERTIES << "' without content");
                return XMLP_ret::XML_ERROR;
            }

            while (nullptr != p_aux1)
            {
                /*
                    <xs:complexType name="binaryPropertyType">
                        <xs:all>
                            <xs:element name="name" type="stringType"/>
                            <xs:element name="value" type="stringType"/><!-- std::vector<uint8_t> -->
                            <xs:element name="propagate" type="boolType"/>
                        </xs:all>
                    </xs:complexType>
                 */
                const char* sub_name = nullptr;
                BinaryProperty bin_prop;
                for (p_aux2 = p_aux1->FirstChildElement(); p_aux2 != NULL; p_aux2 = p_aux2->NextSiblingElement())
                {
                    sub_name = p_aux2->Name();
                    if (strcmp(sub_name, NAME) == 0)
                    {
                        // name - stringType
                        std::string s = "";
                        if (XMLP_ret::XML_OK != getXMLString(p_aux2, &s, ident + 2))
                        {
                            return XMLP_ret::XML_ERROR;
                        }
                        bin_prop.name(s);
                    }
                    else if (strcmp(sub_name, VALUE) == 0)
                    {
                        if (XMLP_ret::XML_OK != parseXMLOctetVector(p_aux2, bin_prop.value(), true))
                        {
                            return XMLP_ret::XML_ERROR;
                        }
                    }
                    else if (strcmp(sub_name, PROPAGATE) == 0)
                    {
                        // propagate - boolType
                        bool b = false;
                        if (XMLP_ret::XML_OK != getXMLBool(p_aux2, &b, ident + 2))
                        {
                            return XMLP_ret::XML_ERROR;
                        }
                        bin_prop.propagate(b);
                    }
                }
                propertiesPolicy.binary_properties().push_back(bin_prop);
                p_aux1 = p_aux1->NextSiblingElement(PROPERTY);
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'propertyPolicyType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLOctetVector(
        tinyxml2::XMLElement* elem,
        std::vector<octet>& octet_vector,
        uint8_t /*ident*/)
{
    if (nullptr == elem)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "preconditions error");
        return XMLP_ret::XML_ERROR;
    }

    tinyxml2::XMLElement* p_aux0 = nullptr;
    size_t num_elems = 0;
    XMLP_ret ret_val = XMLP_ret::XML_OK;

    for (p_aux0 = elem->FirstChildElement(); nullptr != p_aux0; p_aux0 = p_aux0->NextSiblingElement())
    {
        if (1 < ++num_elems)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "More than one tag on " << p_aux0->GetLineNum());
            return XMLP_ret::XML_ERROR;
        }
        if (0 == strcmp(p_aux0->Name(), VALUE))
        {
            ret_val = parseXMLOctetVector(p_aux0, octet_vector, false);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Invalid tag with name of " << p_aux0->Name() << " on line " << p_aux0->GetLineNum());
            return XMLP_ret::XML_ERROR;
        }
    }

    return ret_val;
}

XMLP_ret XMLParser::getXMLInt(
        tinyxml2::XMLElement* elem,
        int* in,
        uint8_t /*ident*/)
{
    if (nullptr == elem || nullptr == in)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    std::string text = get_element_text(elem);
    if (text.empty() || !tinyxml2::XMLUtil::ToInt(text.c_str(), in))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLInt XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLUint(
        tinyxml2::XMLElement* elem,
        unsigned int* ui,
        uint8_t /*ident*/)
{
    if (nullptr == elem || nullptr == ui)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    std::string text = get_element_text(elem);
    if (text.empty() || !tinyxml2::XMLUtil::ToUnsigned(text.c_str(), ui))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLUint(
        tinyxml2::XMLElement* elem,
        uint16_t* ui16,
        uint8_t /*ident*/)
{
    unsigned int ui = 0u;
    if (nullptr == elem || nullptr == ui16)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    std::string text = get_element_text(elem);
    if (text.empty() || !tinyxml2::XMLUtil::ToUnsigned(text.c_str(), &ui) || ui >= 65536)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    *ui16 = static_cast<uint16_t>(ui);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLBool(
        tinyxml2::XMLElement* elem,
        bool* b,
        uint8_t /*ident*/)
{
    if (nullptr == elem || nullptr == b)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    std::string text = get_element_text(elem);
    if (text.empty() || !tinyxml2::XMLUtil::ToBool(text.c_str(), b))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLBool XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLEnum(
        tinyxml2::XMLElement* elem,
        IntraprocessDeliveryType* e,
        uint8_t /*ident*/)
{
    //<xs:simpleType name="IntraprocessDeliveryType">
    //    <xs:restriction base="xs:string">
    //        <xs:enumeration value="OFF"/>
    //        <xs:enumeration value="USER_DATA_ONLY"/>
    //        <xs:enumeration value="FULL"/>
    //    </xs:restriction>
    //</xs:simpleType>

    if (nullptr == elem || nullptr == e)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLEnum XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    std::string text = get_element_text(elem);
    if (text.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLEnum XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    if (!get_element_enum_value(text.c_str(), *e,
            OFF, IntraprocessDeliveryType::INTRAPROCESS_OFF,
            USER_DATA_ONLY, IntraprocessDeliveryType::INTRAPROCESS_USER_DATA_ONLY,
            FULL, IntraprocessDeliveryType::INTRAPROCESS_FULL))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << INTRAPROCESS_DELIVERY << "' with bad content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLEnum(
        tinyxml2::XMLElement* elem,
        DiscoveryProtocol_t* e,
        uint8_t /*ident*/)
{
    /*
        <xs:simpleType name="DiscoveryProtocol">
            <xs:restriction base="xs:string">
                <xs:enumeration value="NONE"/>
                <xs:enumeration value="SIMPLE"/>
                <xs:enumeration value="CLIENT"/>
                <xs:enumeration value="SERVER"/>
                <xs:enumeration value="BACKUP"/>
                <xs:enumeration value="SUPER_CLIENT"/>
            </xs:restriction>
        </xs:simpleType>
     */


    if (nullptr == elem || nullptr == e)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLEnum XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    std::string text = get_element_text(elem);
    if (text.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLEnum XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    if (!get_element_enum_value(text.c_str(), *e,
            NONE, DiscoveryProtocol_t::NONE,
            SIMPLE, DiscoveryProtocol_t::SIMPLE,
            CLIENT, DiscoveryProtocol_t::CLIENT,
            SERVER, DiscoveryProtocol_t::SERVER,
            BACKUP, DiscoveryProtocol_t::BACKUP,
            SUPER_CLIENT, DiscoveryProtocol_t::SUPER_CLIENT))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << RTPS_PDP_TYPE << "' with bad content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLEnum(
        tinyxml2::XMLElement* elem,
        ParticipantFilteringFlags_t* e,
        uint8_t /*ident*/)
{
    /*
        <xs:simpleType name="ParticipantFlags">
            <xs:restriction base="xs:string">
                <xs:pattern value="((FILTER_DIFFERENT_HOST|FILTER_DIFFERENT_PROCESS|FILTER_SAME_PROCESS)(\||\s)*)*" />
            </xs:restriction>
        </xs:simpleType>
     */

    if (nullptr == elem || nullptr == e)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLEnum XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    std::string text = get_element_text(elem);
    if (text.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLEnum XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    // First we check if it matches the schema pattern
    std::regex schema("((FILTER_DIFFERENT_HOST|FILTER_DIFFERENT_PROCESS|FILTER_SAME_PROCESS|NO_FILTER)*(\\||\\s)*)*");

    if (!std::regex_match(text, schema))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "provided flags doesn't match expected ParticipantFilteringFlags!");
        return XMLP_ret::XML_ERROR;
    }

    // Lets parse the flags, we assume the flags argument has been already flushed
    std::regex flags("FILTER_DIFFERENT_HOST|FILTER_DIFFERENT_PROCESS|FILTER_SAME_PROCESS");
    std::cregex_iterator it(text.c_str(), text.c_str() + strlen(text.c_str()), flags);
    uint32_t newflags = *e;

    while (it != std::cregex_iterator())
    {
        std::string flag(it++->str());

        if (flag == FILTER_DIFFERENT_HOST )
        {
            newflags |= ParticipantFilteringFlags_t::FILTER_DIFFERENT_HOST;
        }
        else if (flag == FILTER_DIFFERENT_PROCESS )
        {
            newflags |= ParticipantFilteringFlags_t::FILTER_DIFFERENT_PROCESS;
        }
        else if (flag == FILTER_SAME_PROCESS )
        {
            newflags |= ParticipantFilteringFlags_t::FILTER_SAME_PROCESS;
        }
    }

    *e = static_cast<ParticipantFilteringFlags_t>(newflags);

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLRemoteServer(
        tinyxml2::XMLElement* elem,
        eprosima::fastdds::rtps::RemoteServerAttributes& server,
        uint8_t ident)
{
    /*
        <xs:complexType name="RemoteServerAttributes">
            <xs:all minOccurs="1">
                <xs:element name="metatrafficUnicastLocatorList" type="locatorListType" minOccurs="0"/>
                <xs:element name="metatrafficMulticastLocatorList" type="locatorListType" minOccurs="0"/>
            </xs:all>
            <xs:attribute name="guidPrefix" type="guid" use="required"/>
        </xs:complexType>
     */

    const char* Prefix = nullptr;
    tinyxml2::XMLElement* pLU = nullptr, * pLM = nullptr;

    if (nullptr == elem )
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLRemoteServer XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    else if (nullptr == (Prefix = elem->Attribute(PREFIX)))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLRemoteServer try to recover server's guidPrefix XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    else if (!server.ReadguidPrefix(Prefix))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "getXMLRemoteServer found an invalid server's guidPrefix XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    pLU = elem->FirstChildElement(META_UNI_LOC_LIST);
    pLM = elem->FirstChildElement(META_MULTI_LOC_LIST);

    if ( pLU == nullptr && pLM == nullptr )
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "getXMLRemoteServer couldn't find any server's locator XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    if (pLU && XMLP_ret::XML_OK != getXMLLocatorList(pLU, server.metatrafficUnicastLocatorList, ident))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "getXMLRemoteServer was given a misformatted server's " << META_UNI_LOC_LIST << " XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    if (pLM && XMLP_ret::XML_OK != getXMLLocatorList(pLM, server.metatrafficMulticastLocatorList, ident))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER,
                "getXMLRemoteServer was given a misformatted server's " << META_MULTI_LOC_LIST << " XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLList(
        tinyxml2::XMLElement* elem,
        eprosima::fastdds::rtps::RemoteServerList_t& list,
        uint8_t ident)
{
    /*
        <xs:complexType name="DiscoveryServerList">
            <xs:sequence>
                <xs:element name="RemoteServer" type="RemoteServerAttributes" minOccurs="0" maxOccurs="unbounded"/>
            </xs:sequence>
        </xs:complexType>
     */

    tinyxml2::XMLElement* pS = nullptr;

    if (nullptr == elem)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLList XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    else if (nullptr == (pS = elem->FirstChildElement(RSERVER)))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "getXMLList couldn't find any RemoteServer XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    while (pS)
    {
        eprosima::fastdds::rtps::RemoteServerAttributes server;
        if (XMLP_ret::XML_OK != getXMLRemoteServer(pS, server, ident))
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "getXMLList was given a misformatted RemoteServer XML_ERROR!");
            return XMLP_ret::XML_ERROR;
        }
        list.push_back(std::move(server));

        pS = pS->NextSiblingElement(RSERVER);
    }

    return XMLP_ret::XML_OK;

}

XMLP_ret XMLParser::getXMLString(
        tinyxml2::XMLElement* elem,
        std::string* s,
        uint8_t /*ident*/)
{
    if (nullptr == elem || nullptr == s)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    std::string text = get_element_text(elem);
    if (text.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLString XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    *s = text;
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLguidPrefix(
        tinyxml2::XMLElement* elem,
        GuidPrefix_t& prefix,
        uint8_t /*ident*/)
{
    if (nullptr == elem )
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLguidPrefix XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    std::string text = get_element_text(elem);
    if (text.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLguidPrefix XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    std::istringstream is(text);
    return (is >> prefix ? XMLP_ret::XML_OK : XMLP_ret::XML_ERROR);

}

XMLP_ret XMLParser::getXMLPublisherAttributes(
        tinyxml2::XMLElement* elem,
        PublisherAttributes& publisher,
        uint8_t ident)
{
    /*
        <xs:complexType name="publisherProfileType">
            <xs:all minOccurs="0">
                <xs:element name="topic" type="topicAttributesType" minOccurs="0"/>
                <xs:element name="qos" type="writerQosPoliciesType" minOccurs="0"/>
                <xs:element name="times" type="writerTimesType" minOccurs="0"/>
                <xs:element name="external_unicast_locators" type="externalLocatorListType" minOccurs="0"/>
                <xs:element name="ignore_non_matching_locators" type="boolType" minOccurs="0"/>
                <xs:element name="unicastLocatorList" type="locatorListType" minOccurs="0"/>
                <xs:element name="multicastLocatorList" type="locatorListType" minOccurs="0"/>
                <xs:element name="throughputController" type="throughputControllerType" minOccurs="0"/>
                <xs:element name="historyMemoryPolicy" type="historyMemoryPolicyType" minOccurs="0"/>
                <xs:element name="propertiesPolicy" type="propertyPolicyType" minOccurs="0"/>
                <xs:element name="userDefinedID" type="int16Type" minOccurs="0"/>
                <xs:element name="entityID" type="int16Type" minOccurs="0"/>
                <xs:element name="matchedSubscribersAllocation" type="containerAllocationConfigType" minOccurs="0"/>
            </xs:all>
            <xs:attribute name="profile_name" type="stringType" use="required"/>
        </xs:complexType>
     */

    std::unordered_map<std::string, bool> tags_present;

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (tags_present[name])
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'publisherProfileType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present[name] = true;

        if (strcmp(name, TOPIC) == 0)
        {
            // topic
            if (XMLP_ret::XML_OK != getXMLTopicAttributes(p_aux0, publisher.topic, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, QOS) == 0)
        {
            // qos
            if (XMLP_ret::XML_OK != getXMLWriterQosPolicies(p_aux0, publisher.qos, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, TIMES) == 0)
        {
            // times
            if (XMLP_ret::XML_OK != getXMLWriterTimes(p_aux0, publisher.times, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, IGN_NON_MATCHING_LOCS) == 0)
        {
            // ignore_non_matching_locators - boolType
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &publisher.ignore_non_matching_locators, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, EXT_UNI_LOC_LIST) == 0)
        {
            // external_unicast_locators - externalLocatorListType
            if (XMLP_ret::XML_OK != getXMLExternalLocatorList(p_aux0, publisher.external_unicast_locators, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, UNI_LOC_LIST) == 0)
        {
            // unicastLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, publisher.unicastLocatorList, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, MULTI_LOC_LIST) == 0)
        {
            // multicastLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, publisher.multicastLocatorList, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, REM_LOC_LIST) == 0)
        {
            // remoteLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, publisher.remoteLocatorList, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, THROUGHPUT_CONT) == 0)
        {
            // throughputController
            if (XMLP_ret::XML_OK !=
                    getXMLThroughputController(p_aux0, publisher.throughputController, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, HIST_MEM_POLICY) == 0)
        {
            // historyMemoryPolicy
            if (XMLP_ret::XML_OK != getXMLHistoryMemoryPolicy(p_aux0, publisher.historyMemoryPolicy, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PROPERTIES_POLICY) == 0)
        {
            // propertiesPolicy
            if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux0, publisher.properties, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, USER_DEF_ID) == 0)
        {
            // userDefinedID - int16type
            int i = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &i, ident) || i > 255)
            {
                return XMLP_ret::XML_ERROR;
            }
            publisher.setUserDefinedID(static_cast<uint8_t>(i));
        }
        else if (strcmp(name, ENTITY_ID) == 0)
        {
            // entityID - int16Type
            int i = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &i, ident) || i > 255)
            {
                return XMLP_ret::XML_ERROR;
            }
            publisher.setEntityID(static_cast<uint8_t>(i));
        }
        else if (strcmp(name, MATCHED_SUBSCRIBERS_ALLOCATION) == 0)
        {
            // matchedSubscribersAllocation - containerAllocationConfigType
            if (XMLP_ret::XML_OK !=
                    getXMLContainerAllocationConfig(p_aux0, publisher.matched_subscriber_allocation, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'publisherProfileType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLSubscriberAttributes(
        tinyxml2::XMLElement* elem,
        SubscriberAttributes& subscriber,
        uint8_t ident)
{
    /*
        <xs:complexType name="subscriberProfileType">
            <xs:all minOccurs="0">
                <xs:element name="topic" type="topicAttributesType" minOccurs="0"/>
                <xs:element name="qos" type="readerQosPoliciesType" minOccurs="0"/>
                <xs:element name="times" type="readerTimesType" minOccurs="0"/>
                <xs:element name="external_unicast_locators" type="externalLocatorListType" minOccurs="0"/>
                <xs:element name="ignore_non_matching_locators" type="boolType" minOccurs="0"/>
                <xs:element name="unicastLocatorList" type="locatorListType" minOccurs="0"/>
                <xs:element name="multicastLocatorList" type="locatorListType" minOccurs="0"/>
                <xs:element name="expectsInlineQos" type="boolType" minOccurs="0"/>
                <xs:element name="historyMemoryPolicy" type="historyMemoryPolicyType" minOccurs="0"/>
                <xs:element name="propertiesPolicy" type="propertyPolicyType" minOccurs="0"/>
                <xs:element name="userDefinedID" type="int16Type" minOccurs="0"/>
                <xs:element name="entityID" type="int16Type" minOccurs="0"/>
                <xs:element name="matchedPublishersAllocation" type="containerAllocationConfigType" minOccurs="0"/>
            </xs:all>
            <xs:attribute name="profile_name" type="stringType" use="required"/>
        </xs:complexType>
     */

    std::unordered_map<std::string, bool> tags_present;

    tinyxml2::XMLElement* p_aux0 = nullptr;
    const char* name = nullptr;
    for (p_aux0 = elem->FirstChildElement(); p_aux0 != nullptr; p_aux0 = p_aux0->NextSiblingElement())
    {
        name = p_aux0->Name();
        if (tags_present[name])
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'subscriberProfileType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present[name] = true;

        if (strcmp(name, TOPIC) == 0)
        {
            // topic
            if (XMLP_ret::XML_OK != getXMLTopicAttributes(p_aux0, subscriber.topic, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, QOS) == 0)
        {
            // qos
            if (XMLP_ret::XML_OK != getXMLReaderQosPolicies(p_aux0, subscriber.qos, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, TIMES) == 0)
        {
            // times
            if (XMLP_ret::XML_OK != getXMLReaderTimes(p_aux0, subscriber.times, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, IGN_NON_MATCHING_LOCS) == 0)
        {
            // ignore_non_matching_locators - boolType
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &subscriber.ignore_non_matching_locators, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, EXT_UNI_LOC_LIST) == 0)
        {
            // external_unicast_locators - externalLocatorListType
            if (XMLP_ret::XML_OK != getXMLExternalLocatorList(p_aux0, subscriber.external_unicast_locators, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, UNI_LOC_LIST) == 0)
        {
            // unicastLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, subscriber.unicastLocatorList, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, MULTI_LOC_LIST) == 0)
        {
            // multicastLocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, subscriber.multicastLocatorList, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, REM_LOC_LIST) == 0)
        {
            // remote LocatorList
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, subscriber.remoteLocatorList, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, EXP_INLINE_QOS) == 0)
        {
            // expectsInlineQos - boolType
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &subscriber.expectsInlineQos, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, HIST_MEM_POLICY) == 0)
        {
            // historyMemoryPolicy
            if (XMLP_ret::XML_OK != getXMLHistoryMemoryPolicy(
                        p_aux0,
                        subscriber.historyMemoryPolicy,
                        ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, PROPERTIES_POLICY) == 0)
        {
            // propertiesPolicy
            if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux0, subscriber.properties, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, USER_DEF_ID) == 0)
        {
            // userDefinedID - int16Type
            int i = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &i, ident) || i > 255)
            {
                return XMLP_ret::XML_ERROR;
            }
            subscriber.setUserDefinedID(static_cast<uint8_t>(i));
        }
        else if (strcmp(name, ENTITY_ID) == 0)
        {
            // entityID - int16Type
            int i = 0;
            if (XMLP_ret::XML_OK != getXMLInt(p_aux0, &i, ident) || i > 255)
            {
                return XMLP_ret::XML_ERROR;
            }
            subscriber.setEntityID(static_cast<uint8_t>(i));
        }
        else if (strcmp(name, MATCHED_PUBLISHERS_ALLOCATION) == 0)
        {
            // matchedPublishersAllocation - containerAllocationConfigType
            if (XMLP_ret::XML_OK != getXMLContainerAllocationConfig(
                        p_aux0,
                        subscriber.matched_publisher_allocation,
                        ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Invalid element found into 'subscriberProfileType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }

    return XMLP_ret::XML_OK;
}

}  // namespace xmlparser
}  // namespace fastrtps
}  // namespace eprosima
