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

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <regex>
#include <set>
#include <string>
#include <unordered_map>

#include <tinyxml2.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <utils/string_utilities.hpp>
#include <utils/SystemInfo.hpp>
#include <utils/UnitsParser.hpp>
#include <xmlparser/XMLParser.h>
#include <xmlparser/XMLParserCommon.h>
#include <xmlparser/XMLParserUtils.hpp>
#include <xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace fastdds {
namespace xml {
namespace detail {

static std::string process_environment(
        const std::string& input)
{
    /* From [IEEE Std 1003.1]:(https://pubs.opengroup.org/onlinepubs/000095399/basedefs/xbd_chap08.html)
     * Environment variable names used ... consist solely of uppercase letters, digits, and the '_' (underscore)
     * from the characters defined in Portable Character Set and do not begin with a digit.
     */
    std::regex expression("\\$\\{([A-Z_][A-Z0-9_]*)\\}");

    // Algorithm inspired by https://stackoverflow.com/a/37516316/559350

    // This will hold the accumulated substitution result
    std::string ret_val{};

    // Iterators to first and last character of the input string
    auto first = input.cbegin();
    auto last = input.cend();

    // Position of last match in the input string
    std::smatch::difference_type last_match_position = 0;

    // Iterator to the character after the last match in the input string
    auto last_match_end = first;

    // Functor called to process each match
    auto match_cb = [&](const std::smatch& match)
            {
                // Compute substitution value
                std::string var_name = match[1];
                std::string value = "";
                if (var_name == "_")
                {
                    // Silently ignore ${_} since it might expose sensitive information (full path to executable).
                    EPROSIMA_LOG_WARNING(XMLPARSER, "Ignoring environment variable ${_}");
                }
                else if (dds::RETCODE_OK != SystemInfo::get_env(var_name, value))
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Could not find a value for environment variable " << var_name);
                }

                // Compute number of non-matching characters between this match and the last one
                auto this_match_position = match.position();
                auto diff = this_match_position - last_match_position;

                // Append non-matching characters to return value
                auto this_match_start = last_match_end;
                std::advance(this_match_start, diff);
                ret_val.append(last_match_end, this_match_start);

                // Append substitution value to return value
                ret_val.append(value);

                // Prepare for next iteration
                auto match_length = match.length();
                last_match_position = this_match_position + match_length;
                last_match_end = this_match_start;
                std::advance(last_match_end, match_length);
            };

    // Substitute all matches
    std::sregex_iterator begin(first, last, expression);
    std::sregex_iterator end;
    std::for_each(begin, end, match_cb);

    // Append characters after last match
    ret_val.append(last_match_end, last);

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
namespace fastdds {
namespace xmlparser {

using namespace eprosima::fastdds::xml::detail;
using namespace eprosima::fastdds::rtps;

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
                <xs:element name="network_buffers_config" type="allocationConfigType" minOccurs="0"/>
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
        else if (strcmp(name, NETWORK_BUFFERS_CONFIG) == 0)
        {
            // preallocated_network_buffers - uint32Type
            if (XMLP_ret::XML_OK != getXMLContainerAllocationConfig(p_aux0, allocation.network_buffers_config, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
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
            // discoverServersList - locatorListType
            if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux0, settings.m_DiscoveryServers, ident))
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
            <xs:element name="flow_controller_name" type="stringType" minOccurs="0"/>
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
        else if (strcmp(name, FLOW_CONTROLLER_NAME) == 0)
        {
            // flow_controller_name - stringType
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &builtin.flow_controller_name, ident))
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
                <xs:element name="offsetd4" type="uint16Type" minOccurs="0"/>
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
        else if (strcmp(name, OFFSETD4) == 0)
        {
            // offsetd4 - uint16Type
            if (XMLP_ret::XML_OK != getXMLUint(p_aux0, &port.offsetd4, ident))
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
        std::vector<std::shared_ptr<fastdds::rtps::TransportDescriptorInterface>>& transports,
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
            std::shared_ptr<fastdds::rtps::TransportDescriptorInterface> pDescriptor =
                    XMLProfileManager::getTransportById(text);
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

XMLP_ret XMLParser::getXMLFlowControllerDescriptorList(
        tinyxml2::XMLElement* elem,
        FlowControllerDescriptorList& flow_controller_descriptor_list,
        uint8_t ident)
{
    /*
        <xs:complexType name="flowControllerDescriptorListType">
            <xs:sequence>
                <xs:element name="flow_controller_descriptor" type="flowControllerDescriptorType" maxOccurs="unbounded"/>
            </xs:sequence>
        </xs:complexType>
     */

    tinyxml2::XMLElement* p_aux0 = nullptr;
    p_aux0 = elem->FirstChildElement(FLOW_CONTROLLER_DESCRIPTOR);
    if (nullptr == p_aux0)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << elem->Value() << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    while (nullptr != p_aux0)
    {
        /*
            <xs:complexType name="flowControllerDescriptorType">
                <xs:all>
                    <xs:element name="name" type="string" minOccurs="1" maxOccurs="1"/>
                    <xs:element name="scheduler" type="flowControllerSchedulerPolicy" minOccurs="0" maxOccurs="1"/>
                    <xs:element name="max_bytes_per_period" type="int32" minOccurs="0" maxOccurs="1"/>
                    <xs:element name="period_ms" type="uint64" minOccurs="0" maxOccurs="1"/>
                    <xs:element name="sender_thread" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
                </xs:all>
            </xs:complexType>

            <xs:simpleType name="flowControllerSchedulerPolicy">
                <xs:restriction base="xs:string">
                    <xs:enumeration value="FIFO" />
                    <xs:enumeration value="ROUND_ROBIN" />
                    <xs:enumeration value="HIGH_PRIORITY" />
                    <xs:enumeration value="PRIORITY_WITH_RESERVATION" />
                </xs:restriction>
            </xs:simpleType>
         */

        tinyxml2::XMLElement* p_aux1;
        bool name_defined = false;
        std::set<std::string> tags_present;

        auto flow_controller_descriptor = std::make_shared<FlowControllerDescriptor>();

        for (p_aux1 = p_aux0->FirstChildElement(); p_aux1 != NULL; p_aux1 = p_aux1->NextSiblingElement())
        {
            const char* name = p_aux1->Name();

            if (tags_present.count(name) != 0)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Duplicated element found in 'flowControllerDescriptorType'. Name: " << name);
                return XMLP_ret::XML_ERROR;
            }
            else
            {
                tags_present.emplace(name);
            }

            if (strcmp(name, NAME) == 0)
            {
                // name - stringType
                flow_controller_descriptor->name = get_element_text(p_aux1);
                if (flow_controller_descriptor->name.empty())
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "<" << p_aux1->Value() << "> getXMLString XML_ERROR!");
                    return XMLP_ret::XML_ERROR;
                }
                name_defined = true;
            }
            else if (strcmp(name, SCHEDULER) == 0)
            {
                std::string text = get_element_text(p_aux1);
                if (text.empty())
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << SCHEDULER << "' without content");
                    return XMLP_ret::XML_ERROR;
                }

                // scheduler - flowControllerSchedulerPolicy
                if (!get_element_enum_value(text.c_str(), flow_controller_descriptor->scheduler,
                        FIFO, FlowControllerSchedulerPolicy::FIFO,
                        HIGH_PRIORITY, FlowControllerSchedulerPolicy::HIGH_PRIORITY,
                        ROUND_ROBIN, FlowControllerSchedulerPolicy::ROUND_ROBIN,
                        PRIORITY_WITH_RESERVATION, FlowControllerSchedulerPolicy::PRIORITY_WITH_RESERVATION))
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << SCHEDULER << "' with bad content");
                    return XMLP_ret::XML_ERROR;
                }
            }
            else if (strcmp(name, MAX_BYTES_PER_PERIOD) == 0)
            {
                // max_bytes_per_period - int32Type
                if (XMLP_ret::XML_OK != getXMLInt(p_aux1, &flow_controller_descriptor->max_bytes_per_period, ident))
                {
                    return XMLP_ret::XML_ERROR;
                }
            }
            else if (strcmp(name, PERIOD_MILLISECS) == 0)
            {
                // period_ms - uint64Type
                if (XMLP_ret::XML_OK != getXMLUint(p_aux1, &flow_controller_descriptor->period_ms, ident))
                {
                    return XMLP_ret::XML_ERROR;
                }
            }
            else if (strcmp(name, SENDER_THREAD) == 0)
            {
                // sender_thread - threadSettingsType
                getXMLThreadSettings(*p_aux1, flow_controller_descriptor->sender_thread);
            }
            else
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Invalid element found into 'flowControllerDescriptorType'. Name: " << name);
                return XMLP_ret::XML_ERROR;
            }
        }

        if (!name_defined)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Flow Controller Descriptor requires a 'name'");
            return XMLP_ret::XML_ERROR;
        }

        flow_controller_descriptor_list.push_back(flow_controller_descriptor);
        p_aux0 = p_aux0->NextSiblingElement(FLOW_CONTROLLER_DESCRIPTOR);
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
        dds::ResourceLimitsQosPolicy& resourceLimitsQos,
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
        dds::HistoryQosPolicy& historyQos,
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
                    KEEP_LAST, dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS,
                    KEEP_ALL, dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS))
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
        fastdds::dds::WriterQos& qos,
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
        fastdds::dds::ReaderQos& qos,
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
        dds::DurabilityQosPolicy& durability,
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
                    _VOLATILE, dds::DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS,
                    _TRANSIENT_LOCAL, dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS,
                    _TRANSIENT, dds::DurabilityQosPolicyKind::TRANSIENT_DURABILITY_QOS,
                    _PERSISTENT, dds::DurabilityQosPolicyKind::PERSISTENT_DURABILITY_QOS))
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
                durabilityService.history_kind = dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
            }
            else if (strcmp(text.c_str(), KEEP_ALL) == 0)
            {
                durabilityService.history_kind = dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
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
        dds::DeadlineQosPolicy& deadline,
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
        dds::LatencyBudgetQosPolicy& latencyBudget,
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
        dds::LivelinessQosPolicy& liveliness,
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
                    AUTOMATIC, dds::LivelinessQosPolicyKind::AUTOMATIC_LIVELINESS_QOS,
                    MANUAL_BY_PARTICIPANT, dds::LivelinessQosPolicyKind::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
                    MANUAL_BY_TOPIC, dds::LivelinessQosPolicyKind::MANUAL_BY_TOPIC_LIVELINESS_QOS))
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
        dds::ReliabilityQosPolicy& reliability,
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
                    _BEST_EFFORT, dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS,
                    _RELIABLE, dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS))
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
        dds::LifespanQosPolicy& lifespan,
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
        dds::DisablePositiveACKsQosPolicy& disablePositiveAcks,
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
        dds::DataSharingQosPolicy& data_sharing,
        uint8_t ident)
{
    /*
        <xs:complexType name="dataSharingQosPolicyType">
            <xs:all>
                <xs:element name="kind" minOccurs="1" maxOccurs="1">
                    <xs:simpleType>
                        <xs:restriction base="xs:string">
                            <xs:enumeration value="AUTOMATIC"/>
                            <xs:enumeration value="ON"/>
                            <xs:enumeration value="OFF"/>
                        </xs:restriction>
                    </xs:simpleType>
                </xs:element>
                <xs:element name="shared_dir" type="string" minOccurs="0" maxOccurs="1"/>
                <xs:element name="domain_ids" minOccurs="0" maxOccurs="1">
                    <xs:complexType>
                        <xs:sequence>
                            <xs:element name="domainId" type="domainIDType" minOccurs="0" maxOccurs="unbounded"/>
                        </xs:sequence>
                    </xs:complexType>
                </xs:element>
                <xs:element name="max_domains" type="uint32" minOccurs="0" maxOccurs="1"/>
                <xs:element name="data_sharing_listener_thread" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */
    bool kind_found = false;
    dds::DataSharingKind kind = dds::DataSharingKind::AUTO;
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
                    ON, dds::DataSharingKind::ON,
                    OFF, dds::DataSharingKind::OFF,
                    AUTOMATIC, dds::DataSharingKind::AUTO,
                    AUTO, dds::DataSharingKind::AUTO))
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
        else if (strcmp(name, DATA_SHARING_LISTENER_THREAD) == 0)
        {
            // data_sharing_listener_thread
            if (XMLP_ret::XML_OK != getXMLThreadSettings(*p_aux0, data_sharing.data_sharing_listener_thread()))
            {
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
        case dds::DataSharingKind::ON:
            data_sharing.on(shm_directory, domain_ids);
            break;

        case dds::DataSharingKind::AUTO:
            data_sharing.automatic(shm_directory, domain_ids);
            break;

        case dds::DataSharingKind::OFF:
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
        dds::OwnershipQosPolicy& ownership,
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
                    SHARED, dds::OwnershipQosPolicyKind::SHARED_OWNERSHIP_QOS,
                    EXCLUSIVE, dds::OwnershipQosPolicyKind::EXCLUSIVE_OWNERSHIP_QOS))
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
        dds::OwnershipStrengthQosPolicy& ownershipStrength,
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
        dds::PartitionQosPolicy& partition,
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
        dds::PublishModeQosPolicy& publishMode,
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
                    SYNCHRONOUS, dds::PublishModeQosPolicyKind::SYNCHRONOUS_PUBLISH_MODE,
                    ASYNCHRONOUS, dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' bad content");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, FLOW_CONTROLLER_NAME) == 0)
        {

            publishMode.flow_controller_name = get_element_text(p_aux0);
            if (publishMode.flow_controller_name.empty())
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << FLOW_CONTROLLER_NAME << "' without content");
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
        dds::Duration_t& duration,
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
        duration = dds::c_TimeInfinite;

        if (elem->FirstChildElement() != nullptr)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "If a dds::Duration_t type element is defined as DURATION_INFINITY it cannot have <sec> or"
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
                duration = dds::c_TimeInfinite;
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
                duration = dds::c_TimeInfinite;
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

    // An empty dds::Duration_t xml is forbidden
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
                <xs:element name="initial_heartbeat_delay" type="durationType" minOccurs="0"/>
                <xs:element name="heartbeat_period" type="durationType" minOccurs="0"/>
                <xs:element name="nack_response_delay" type="durationType" minOccurs="0"/>
                <xs:element name="nack_supression_duration" type="durationType" minOccurs="0"/>
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
            // initial_heartbeat_delay
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.initial_heartbeat_delay, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, HEARTB_PERIOD) == 0)
        {
            // heartbeat_period
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.heartbeat_period, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, NACK_RESP_DELAY) == 0)
        {
            // nack_response_delay
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.nack_response_delay, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, NACK_SUPRESSION) == 0)
        {
            // nack_supression_duration
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.nack_supression_duration, ident))
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
                <xs:element name="initial_acknack_delay" type="durationType" minOccurs="0"/>
                <xs:element name="heartbeat_response_delay" type="durationType" minOccurs="0"/>
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
            // initial_acknack_delay
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.initial_acknack_delay, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, HEARTB_RESP_DELAY) == 0)
        {
            // heartbeat_response_delay
            if (XMLP_ret::XML_OK != getXMLDuration(p_aux0, times.heartbeat_response_delay, ident))
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
            if (!IPLocator::setIPv4(locator, s))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Failed to parse UDPv4 locator's " << ADDRESS << " tag");
                return XMLP_ret::XML_ERROR;
            }
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
            if (!IPLocator::setIPv6(locator, s))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Failed to parse UDPv6 locator's " << ADDRESS << " tag");
                return XMLP_ret::XML_ERROR;
            }
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
            if (!IPLocator::setIPv4(locator, s))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Failed to parse TCPv4 locator's " << ADDRESS << " tag");
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, WAN_ADDRESS) == 0)
        {
            // address - stringType
            std::string s = "";
            if (XMLP_ret::XML_OK != getXMLString(p_aux0, &s, ident + 1))
            {
                return XMLP_ret::XML_ERROR;
            }
            if (!IPLocator::setWan(locator, s))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Failed to parse TCPv4 locator's " << WAN_ADDRESS << " tag");
                return XMLP_ret::XML_ERROR;
            }
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
            if (!IPLocator::setIPv6(locator, s))
            {
                EPROSIMA_LOG_ERROR(XMLPARSER, "Failed to parse TCPv6 locator's " << ADDRESS << " tag");
                return XMLP_ret::XML_ERROR;
            }
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

XMLP_ret XMLParser::getXMLUint(
        tinyxml2::XMLElement* elem,
        uint64_t* ui64,
        uint8_t /*ident*/)
{
    unsigned long int ui = 0u;
    if (nullptr == elem || nullptr == ui64)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "nullptr when getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }

    auto to_uint64 = [](const char* str, unsigned long int* value) -> bool
            {
                // Look for a '-' sign
                bool ret = false;
                const char minus = '-';
                const char* minus_result = str;
                if (nullptr == std::strchr(minus_result, minus))
                {
                    // Minus not found
                    ret = true;
                }

                if (ret)
                {
                    ret = false;
#ifdef _WIN32
                    if (sscanf_s(str, "%lu", value) == 1)
#else
                    if (sscanf(str, "%lu", value) == 1)
#endif // ifdef _WIN32
                    {
                        // Number found
                        ret = true;
                    }
                }
                return ret;
            };

    std::string text = get_element_text(elem);
    if (text.empty() || !to_uint64(text.c_str(), &ui))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "<" << elem->Value() << "> getXMLUint XML_ERROR!");
        return XMLP_ret::XML_ERROR;
    }
    *ui64 = static_cast<uint64_t>(ui);
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
        fastdds::IntraprocessDeliveryType* e,
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
            OFF, fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF,
            USER_DATA_ONLY, fastdds::IntraprocessDeliveryType::INTRAPROCESS_USER_DATA_ONLY,
            FULL, fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << INTRAPROCESS_DELIVERY << "' with bad content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLEnum(
        tinyxml2::XMLElement* elem,
        DiscoveryProtocol* e,
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
            NONE, DiscoveryProtocol::NONE,
            SIMPLE, DiscoveryProtocol::SIMPLE,
            CLIENT, DiscoveryProtocol::CLIENT,
            SERVER, DiscoveryProtocol::SERVER,
            BACKUP, DiscoveryProtocol::BACKUP,
            SUPER_CLIENT, DiscoveryProtocol::SUPER_CLIENT))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << RTPS_PDP_TYPE << "' with bad content");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLEnum(
        tinyxml2::XMLElement* elem,
        ParticipantFilteringFlags* e,
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
            newflags |= ParticipantFilteringFlags::FILTER_DIFFERENT_HOST;
        }
        else if (flag == FILTER_DIFFERENT_PROCESS )
        {
            newflags |= ParticipantFilteringFlags::FILTER_DIFFERENT_PROCESS;
        }
        else if (flag == FILTER_SAME_PROCESS )
        {
            newflags |= ParticipantFilteringFlags::FILTER_SAME_PROCESS;
        }
    }

    *e = static_cast<ParticipantFilteringFlags>(newflags);

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

XMLP_ret XMLParser::getXMLDomainParticipantFactoryQos(
        tinyxml2::XMLElement& elem,
        fastdds::dds::DomainParticipantFactoryQos& qos)
{
    /*
        <xs:complexType name="domainParticipantFactoryQosPoliciesType">
            <xs:all>
                <xs:element name="entity_factory" type="entityFactoryQosPolicyType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="shm_watchdog_thread" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
                <xs:element name="file_watch_threads" type="threadSettingsType" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */

    std::set<std::string> tags_present;

    for (tinyxml2::XMLElement* element = elem.FirstChildElement(); element != nullptr;
            element = element->NextSiblingElement())
    {
        const char* name = element->Name();
        if (tags_present.count(name) != 0)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Duplicated element found in 'domainParticipantFactoryQosPoliciesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present.emplace(name);

        if (strcmp(name, ENTITY_FACTORY) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLEntityFactoryQos(*element, qos.entity_factory()))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, SHM_WATCHDOG_THREAD) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLThreadSettings(*element, qos.shm_watchdog_thread()))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else if (strcmp(name, FILE_WATCH_THREADS) == 0)
        {
            if (XMLP_ret::XML_OK != getXMLThreadSettings(*element, qos.file_watch_threads()))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER,
                    "Invalid element found into 'domainParticipantFactoryQosPoliciesType'. Name: " << name);
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLPublisherAttributes(
        tinyxml2::XMLElement* elem,
        fastdds::xmlparser::PublisherAttributes& publisher,
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
        fastdds::xmlparser::SubscriberAttributes& subscriber,
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
                <xs:element name="expects_inline_qos" type="boolType" minOccurs="0"/>
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
            // expects_inline_qos - boolType
            if (XMLP_ret::XML_OK != getXMLBool(p_aux0, &subscriber.expects_inline_qos, ident))
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

XMLP_ret XMLParser::getXMLThreadSettings(
        tinyxml2::XMLElement& elem,
        fastdds::rtps::ThreadSettings& thread_setting)
{
    /*
        <xs:complexType name="threadSettingsType">
            <xs:all>
                <xs:element name="scheduling_policy" type="int32" minOccurs="0" maxOccurs="1"/>
                <xs:element name="priority" type="int32" minOccurs="0" maxOccurs="1"/>
                <xs:element name="affinity" type="uint32" minOccurs="0" maxOccurs="1"/>
                <xs:element name="stack_size" type="int32" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */
    uint32_t port = 0;
    return getXMLThreadSettingsWithPort(elem, thread_setting,
                   port) != XMLP_ret::XML_ERROR ? XMLP_ret::XML_OK : XMLP_ret::XML_ERROR;
}

XMLP_ret XMLParser::getXMLThreadSettingsWithPort(
        tinyxml2::XMLElement& elem,
        fastdds::rtps::ThreadSettings& thread_setting,
        uint32_t& port)
{
    /*
        <xs:complexType name="threadSettingsWithPortType">
            <xs:complexContent>
                <xs:extension base="threadSettingsType">
                    <xs:attribute name="port" type="uint32" use="optional"/>
                </xs:extension>
            </xs:complexContent>
        </xs:complexType>
     */

    /*
     * The are 4 allowed elements, all their min occurrences are 0, and their max are 1.
     * In case port is not present, return NOK instead of ERROR
     */
    XMLP_ret ret = XMLP_ret::XML_OK;
    bool port_found = false;
    for (const tinyxml2::XMLAttribute* attrib = elem.FirstAttribute(); attrib != nullptr; attrib = attrib->Next())
    {
        if (strcmp(attrib->Name(), PORT) == 0)
        {
            try
            {
                std::string temp = attrib->Value();
                temp.erase(std::remove_if(temp.begin(), temp.end(), [](unsigned char c)
                        {
                            return std::isspace(c);
                        }), temp.end());
                if (attrib->Value()[0] == '-')
                {
                    throw std::invalid_argument("Negative value detected");
                }
                port = static_cast<uint32_t>(std::stoul(attrib->Value()));
                port_found = true;
            }
            catch (std::invalid_argument& except)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Found wrong value " << attrib->Value() << " for port attribute. " <<
                        except.what());
                ret = XMLP_ret::XML_ERROR;
                break;
            }
            catch (const std::out_of_range& except)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Value of the port attribute " << attrib->Value() << " out of range. " <<
                        except.what());
                ret = XMLP_ret::XML_ERROR;
                break;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Found wrong attribute " << attrib->Name() << " in 'thread_settings");
            ret = XMLP_ret::XML_ERROR;
            break;
        }
    }

    // Set ret to NOK if port attribute was not present
    if (ret == XMLP_ret::XML_OK && !port_found)
    {
        ret = XMLP_ret::XML_NOK;
    }

    const uint8_t ident = 1;
    std::set<std::string> tags_present;

    for (tinyxml2::XMLElement* current_elem = elem.FirstChildElement();
            current_elem != nullptr && ret != XMLP_ret::XML_ERROR;
            current_elem = current_elem->NextSiblingElement())
    {
        const char* name = current_elem->Name();
        if (tags_present.count(name) != 0)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'thread_settings'. Tag: " << name);
            ret = XMLP_ret::XML_ERROR;
            break;
        }
        tags_present.emplace(name);

        if (strcmp(current_elem->Name(), SCHEDULING_POLICY) == 0)
        {
            // scheduling_policy - int32Type
            if (XMLP_ret::XML_OK != getXMLInt(current_elem, &thread_setting.scheduling_policy, ident) ||
                    thread_setting.scheduling_policy < -1)
            {
                ret = XMLP_ret::XML_ERROR;
                break;
            }
        }
        else if (strcmp(current_elem->Name(), PRIORITY) == 0)
        {
            // priority - int32Type
            if (XMLP_ret::XML_OK != getXMLInt(current_elem, &thread_setting.priority, ident))
            {
                ret = XMLP_ret::XML_ERROR;
                break;
            }
        }
        else if (strcmp(current_elem->Name(), AFFINITY) == 0)
        {
            // affinity - uint64Type
            if (XMLP_ret::XML_OK != getXMLUint(current_elem, &thread_setting.affinity, ident))
            {
                ret = XMLP_ret::XML_ERROR;
                break;
            }
        }
        else if (strcmp(current_elem->Name(), STACK_SIZE) == 0)
        {
            // stack_size - int32Type
            if (XMLP_ret::XML_OK != getXMLInt(current_elem, &thread_setting.stack_size, ident) ||
                    thread_setting.stack_size < -1)
            {
                ret = XMLP_ret::XML_ERROR;
                break;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Found incorrect tag '" << current_elem->Name() << "'");
            ret = XMLP_ret::XML_ERROR;
            break;
        }
    }
    return ret;
}

XMLP_ret XMLParser::getXMLEntityFactoryQos(
        tinyxml2::XMLElement& elem,
        fastdds::dds::EntityFactoryQosPolicy& entity_factory)
{
    /*
        <xs:complexType name="entityFactoryQosPolicyType">
            <xs:all>
                <xs:element name="autoenable_created_entities" type="boolean" minOccurs="0" maxOccurs="1"/>
            </xs:all>
        </xs:complexType>
     */

    /*
     * The only allowed element is autoenable_created_entities, its min occurrences is 0, and its max is 1.
     */
    const uint8_t ident = 1;
    std::set<std::string> tags_present;

    for (tinyxml2::XMLElement* current_elem = elem.FirstChildElement(); current_elem != nullptr;
            current_elem = current_elem->NextSiblingElement())
    {
        const char* name = current_elem->Name();
        if (tags_present.count(name) != 0)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Duplicated element found in 'entityFactoryQosPolicyType'. Tag: " << name);
            return XMLP_ret::XML_ERROR;
        }
        tags_present.emplace(name);

        if (strcmp(current_elem->Name(), AUTOENABLE_CREATED_ENTITIES) == 0)
        {
            // autoenable_created_entities - boolean
            if (XMLP_ret::XML_OK != getXMLBool(current_elem, &entity_factory.autoenable_created_entities, ident))
            {
                return XMLP_ret::XML_ERROR;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Found incorrect tag '" << current_elem->Name() << "'");
            return XMLP_ret::XML_ERROR;
        }
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::getXMLBuiltinTransports(
        tinyxml2::XMLElement* elem,
        eprosima::fastdds::rtps::BuiltinTransports* bt,
        eprosima::fastdds::rtps::BuiltinTransportsOptions* bt_opts,
        uint8_t /*ident*/)
{
    /*
        <xs:simpleType name="builtinTransportKind">
            <xs:restriction base="xs:string">
                <xs:enumeration value="NONE" />
                <xs:enumeration value="DEFAULT" />
                <xs:enumeration value="DEFAULTv6" />
                <xs:enumeration value="SHM" />
                <xs:enumeration value="UDPv4" />
                <xs:enumeration value="UDPv6" />
                <xs:enumeration value="LARGE_DATA" />
                <xs:enumeration value="LARGE_DATAv6" />
                <xs:enumeration value="P2P" />
            </xs:restriction>
        </xs:simpleType>

        <xs:complexType name="builtinTransportsType">
            <xs:simpleContent>
                <xs:extension base="builtinTransportKind">
                    <xs:attribute name="max_msg_size" type="string" use="optional"/>
                    <xs:attribute name="sockets_size" type="string" use="optional"/>
                    <xs:attribute name="non_blocking" type="bool" use="optional"/>
                    <xs:attribute name="tcp_negotiation_timeout" type="uint32" use="optional"/>
                </xs:extension>
            </xs:simpleContent>
        </xs:complexType>
     */

    XMLP_ret ret = XMLP_ret::XML_OK;
    for (const tinyxml2::XMLAttribute* attrib = elem->FirstAttribute(); attrib != nullptr; attrib = attrib->Next())
    {
        if (strcmp(attrib->Name(), MAX_MSG_SIZE_LARGE_DATA) == 0)
        {
            // max_msg_size - stringType
            try
            {
                std::string temp = attrib->Value();
                temp.erase(std::remove_if(temp.begin(), temp.end(), [](unsigned char c)
                        {
                            return std::isspace(c);
                        }), temp.end());
                if (attrib->Value()[0] == '-')
                {
                    throw std::invalid_argument("Negative value detected.");
                }
                std::regex msg_size_regex(R"((\d+)(\w*))");
                std::smatch mr;
                if (std::regex_search(temp, mr, msg_size_regex, std::regex_constants::match_not_null))
                {
                    std::string value = mr[1];
                    std::string unit = mr[2].str();
                    bt_opts->maxMessageSize = eprosima::fastdds::dds::utils::parse_value_and_units(value, unit);
                }
            }
            catch (std::invalid_argument& except)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Found wrong value " << attrib->Value() << " for max_msg_size attribute. " <<
                        except.what());
                ret = XMLP_ret::XML_ERROR;
                break;
            }
        }
        else if (strcmp(attrib->Name(), SOCKETS_SIZE_LARGE_DATA) == 0)
        {
            // sockets_size - stringType
            try
            {
                std::string temp = attrib->Value();
                temp.erase(std::remove_if(temp.begin(), temp.end(), [](unsigned char c)
                        {
                            return std::isspace(c);
                        }), temp.end());
                if (attrib->Value()[0] == '-')
                {
                    throw std::invalid_argument("Negative value detected.");
                }
                std::regex sockets_size_regex(R"((\d+)(\w*))");
                std::smatch mr;
                if (std::regex_search(temp, mr, sockets_size_regex, std::regex_constants::match_not_null))
                {
                    std::string value = mr[1];
                    std::string unit = mr[2].str();
                    bt_opts->sockets_buffer_size = eprosima::fastdds::dds::utils::parse_value_and_units(value, unit);
                }
            }
            catch (std::invalid_argument& except)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Found wrong value " << attrib->Value() << " for sockets_size attribute. " <<
                        except.what());
                ret = XMLP_ret::XML_ERROR;
                break;
            }
        }
        else if (strcmp(attrib->Name(), NON_BLOCKING_LARGE_DATA) == 0)
        {
            // non_blocking - stringType
            try
            {
                std::string temp = attrib->Value();
                temp.erase(std::remove_if(temp.begin(), temp.end(), [](unsigned char c)
                        {
                            return std::isspace(c);
                        }), temp.end());
                if (temp != "true" && temp != "false")
                {
                    throw std::invalid_argument("Only \"true\" or \"false\" values allowed.");
                }
                bt_opts->non_blocking_send = temp == "true" ? true : false;
            }
            catch (std::invalid_argument& except)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Found wrong value " << attrib->Value() << " for non_blocking attribute. " <<
                        except.what());
                ret = XMLP_ret::XML_ERROR;
                break;
            }
        }
        else if (strcmp(attrib->Name(), TCP_NEGOTIATION_TIMEOUT) == 0)
        {
            // tcp_negotiation_timeout - stringType
            try
            {
                std::string temp = attrib->Value();
                temp.erase(std::remove_if(temp.begin(), temp.end(), [](unsigned char c)
                        {
                            return std::isspace(c);
                        }), temp.end());
                if (attrib->Value()[0] == '-')
                {
                    throw std::invalid_argument("Negative value detected.");
                }
                uint64_t value = std::stoull(temp);
                if (value > std::numeric_limits<std::uint32_t>::max())
                {
                    throw std::out_of_range("Value for timeout out of range. Max uint32_t.");
                }
                bt_opts->tcp_negotiation_timeout = static_cast<uint32_t>(std::stoul(temp));
            }
            catch (std::exception& except)
            {
                EPROSIMA_LOG_ERROR(XMLPARSER,
                        "Found wrong value " << attrib->Value() << " for tcp_negotiation_timeout attribute. " <<
                        except.what());
                ret = XMLP_ret::XML_ERROR;
                break;
            }
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Found wrong attribute " << attrib->Name() << " in 'builtin_transports");
            ret = XMLP_ret::XML_ERROR;
            break;
        }
    }

    std::string mode = get_element_text(elem);

    if (mode.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' without content");
        return XMLP_ret::XML_ERROR;
    }

    if (!get_element_enum_value(mode.c_str(), *bt,
            NONE, eprosima::fastdds::rtps::BuiltinTransports::NONE,
            DEFAULT_C, eprosima::fastdds::rtps::BuiltinTransports::DEFAULT,
            DEFAULTv6, eprosima::fastdds::rtps::BuiltinTransports::DEFAULTv6,
            SHM, eprosima::fastdds::rtps::BuiltinTransports::SHM,
            UDPv4, eprosima::fastdds::rtps::BuiltinTransports::UDPv4,
            UDPv6, eprosima::fastdds::rtps::BuiltinTransports::UDPv6,
            LARGE_DATA, eprosima::fastdds::rtps::BuiltinTransports::LARGE_DATA,
            LARGE_DATAv6, eprosima::fastdds::rtps::BuiltinTransports::LARGE_DATAv6,
            P2P, eprosima::fastdds::rtps::BuiltinTransports::P2P))
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Node '" << KIND << "' bad content");
        ret = XMLP_ret::XML_ERROR;
    }

    return ret;
}

}  // namespace xmlparser
}  // namespace fastdds
}  // namespace eprosima
