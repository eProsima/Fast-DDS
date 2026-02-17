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
 * @file EDPStatic.cpp
 *
 */

#include "xmlparser/XMLParserCommon.h"
#include <rtps/builtin/discovery/endpoint/EDPStatic.h>

#include <algorithm>
#include <mutex>
#include <sstream>
#include <string>

#include <tinyxml2.h>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/builtin/discovery/participant/PDPSimple.h>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <xmlparser/XMLEndpointParser.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

const char* exchange_format_property_name = "dds.discovery.static_edp.exchange_format";
const char* exchange_format_property_value_v1 = "v1";
const char* exchange_format_property_value_v1_reduced = "v1_Reduced";
const char* exchange_format_property_value_v2 = "v2";

/**
 * Class Property, used to read and write the strings from the properties used to transmit the EntityId_t.
 */
class EDPStaticProperty
{
public:

    EDPStaticProperty()
        : m_userId(0)
    {
    }

    ~EDPStaticProperty()
    {
    }

    //!Endpoint type
    std::string m_endpointType;
    //!Status
    std::string m_status;
    //!User ID as string
    std::string m_userIdStr;
    //!User ID
    uint16_t m_userId;
    //!Entity ID
    EntityId_t m_entityId;

    /**
     * Convert information to a property
     * @param type Type of endpoint
     * @param status Status of the endpoint
     * @param id User Id
     * @param ent EntityId
     * @return Pair of two strings.
     */
    static std::pair<std::string, std::string> toProperty(
            const EDPStatic::ExchangeFormat exchange_format,
            std::string type,
            std::string status,
            uint16_t id,
            const EntityId_t& ent);
    /**
     * @param in_property Input property-
     * @return True if correctly read
     */
    bool fromProperty(
            std::pair<std::string, std::string> in_property);
};


EDPStatic::EDPStatic(
        PDP* p,
        RTPSParticipantImpl* part)
    : EDP(p, part)
    , mp_edpXML(nullptr)
{
}

EDPStatic::~EDPStatic()
{
    if (mp_edpXML != nullptr)
    {
        delete(mp_edpXML);
    }
}

bool EDPStatic::initEDP(
        BuiltinAttributes& attributes)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, "Beginning STATIC EndpointDiscoveryProtocol");

    bool returned_value = false;
    m_attributes = attributes;
    mp_edpXML = new xmlparser::XMLEndpointParser();
    std::string content(m_attributes.discovery_config.static_edp_xml_config());
    if (0 == content.rfind("data://", 0))
    {
        tinyxml2::XMLDocument xml_document;
        if (tinyxml2::XMLError::XML_SUCCESS == xml_document.Parse(content.c_str() + 7, content.size() - 7))
        {
            returned_value = (this->mp_edpXML->loadXMLNode(xml_document) == xmlparser::XMLP_ret::XML_OK);
        }
    }
    else if (0 == content.rfind("file://", 0))
    {
        std::string file_name = content.substr(7);
        returned_value =  (this->mp_edpXML->loadXMLFile(file_name) == xmlparser::XMLP_ret::XML_OK);
    }

    // Check there is a Participant's property changing the exchange format.
    auto& properties = mp_RTPSParticipant->get_attributes().properties.properties();
    for (auto& property : properties)
    {
        if (0 == property.name().compare(exchange_format_property_name))
        {
            if (0 == property.value().compare(exchange_format_property_value_v2))
            {
                exchange_format_ = ExchangeFormat::v2;
            }
            else if (0 == property.value().compare(exchange_format_property_value_v1_reduced))
            {
                exchange_format_ = ExchangeFormat::v1_Reduced;
            }
            else if (0 == property.value().compare(exchange_format_property_value_v1))
            {
                exchange_format_ = ExchangeFormat::v1;
            }
            else
            {
                returned_value = false;
            }

            break;
        }
    }

    return returned_value;
}

std::pair<std::string, std::string> EDPStaticProperty::toProperty(
        const EDPStatic::ExchangeFormat exchange_format,
        std::string type,
        std::string status,
        uint16_t id,
        const EntityId_t& ent)
{
    std::pair<std::string, std::string> prop;
    std::stringstream ss;
    switch (exchange_format)
    {
        case EDPStatic::ExchangeFormat::v1_Reduced:
            ss << "EDS_";
            if (0 == type.compare("Reader"))
            {
                ss << "R";
            }
            else
            {
                ss << "W";
            }
            if (0 == status.compare("ALIVE"))
            {
                ss << "A_";
            }
            else
            {
                ss << "E_";
            }
            ss << id;
            break;
        case EDPStatic::ExchangeFormat::v1:
        default:
            ss << "eProsimaEDPStatic_" << type << "_" << status << "_ID_" << id;
            break;
    }
    prop.first = ss.str();
    ss.clear();
    ss.str(std::string());
    bool add_dot = false;
    switch (exchange_format)
    {
        case EDPStatic::ExchangeFormat::v1_Reduced:
            if (0 != ent.value[0])
            {
                ss << (int)ent.value[0];
                add_dot = true;
            }
            if (add_dot || 0 != ent.value[1])
            {
                if (add_dot)
                {
                    ss << ".";
                }
                else
                {
                    add_dot = true;
                }
                ss << (int)ent.value[1];
            }
            if (add_dot || 0 != ent.value[2])
            {
                if (add_dot)
                {
                    ss << ".";
                }
                else
                {
                    add_dot = true;
                }
                ss << (int)ent.value[2];
            }
            if (add_dot || 0 != ent.value[3])
            {
                if (add_dot)
                {
                    ss << ".";
                }
                ss << (int)ent.value[3];
            }
            break;
        case EDPStatic::ExchangeFormat::v1:
        default:
            ss << (int)ent.value[0] << ".";
            ss << (int)ent.value[1] << ".";
            ss << (int)ent.value[2] << ".";
            ss << (int)ent.value[3];
            break;
    }
    prop.second = ss.str();
    return prop;
}

typedef unsigned char uchar;
static const std::string base64_str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//=

/**
 * Convert a vector of bits to a Base64 string.
 * @param[in] bits Vector of bits to convert.
 * @return Base64 string.
 */
std::string vector_to_string(
        const std::vector<uint8_t>& bits)
{
    assert(0 == bits.size() % 8);
    std::string bits_str;

    uint8_t byte {0};
    size_t base64_pos {0};
    for (size_t i {0}; i < bits.size(); i += 8)
    {
        for (size_t j {0}; j < 8; ++j)
        {
            byte |= bits[i + j] << (5 - base64_pos++);
            if (base64_pos == 6)
            {
                // We have a complete byte to encode
                bits_str.push_back(base64_str[byte]);
                base64_pos = 0;
                byte = 0;
            }
        }
    }

    if (base64_pos > 0)
    {
        // We have a complete byte to encode
        bits_str.push_back(base64_str[byte]);
    }

    return bits_str;
}

/**
 * Convert a Base64 string to a vector of bits.
 * @param[in] bits Base64 string to convert.
 * @return Vector of bits.
 */
std::vector<uint8_t> string_to_vector(
        const std::string& bits)
{
    std::vector<uint8_t> bits_vector;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
    {
        T[base64_str[i]] = i;
    }

    int val = 0, valb = -8;
    for (uchar c : bits)
    {
        if (T[c] == -1)
        {
            break;
        }
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0)
        {
            char decode_c {char((val >> valb) & 0xFF)};
            for (size_t i {0}; i < 8; ++i)
            {
                bits_vector.push_back((decode_c >> (7 - i)) & 0x01);
            }
            valb -= 8;
        }
    }

    return bits_vector;
}

/**
 * Enable or disable a reader on the v2 property.
 * @param[in] local_participant_name Name of the local participant.
 * @param[in,out] pdp_properties Properties of the local participant.
 * @param[in] id User defined ID of the reader.
 * @param[in] disable True to disable, false to enable.
 * @return True if the operation was successful, false otherwise.
 */
bool EDPStatic::enable_reader_on_v2_property(
        const std::string& local_participant_name,
        fastdds::dds::ParameterPropertyList_t& pdp_properties,
        uint16_t id,
        bool disable)
{
    bool ret_value {false};
    uint32_t position {0};

    if (xmlparser::XMLP_ret::XML_OK == mp_edpXML->lookforReader(local_participant_name.c_str(), id, nullptr, position))
    {
        auto property_it = std::find_if(pdp_properties.begin(), pdp_properties.end(),
                        [](const fastdds::dds::ParameterProperty_t& property)
                        {
                            if (0 == property.first().compare("ESR"))
                            {
                                return true;
                            }

                            return false;
                        });

        std::vector<uint8_t> bits;
        std::pair<std::string, std::string> prop;

        if (pdp_properties.end() == property_it)
        {
            // Create empty property.
            prop.first = "ESR";
            size_t number_of_readers = mp_edpXML->get_number_of_readers(local_participant_name);
            // Make multiple of 8 bits
            number_of_readers = (number_of_readers + 7) & ~7;
            bits = std::vector<uint8_t>(number_of_readers, 0);
            prop.second = vector_to_string(bits);
            pdp_properties.push_back(prop);
            property_it = pdp_properties.begin();
            auto next_it = pdp_properties.begin();
            while (++next_it != pdp_properties.end())
            {
                property_it = next_it;
            }
        }
        else
        {
            bits = string_to_vector(property_it->second());
        }

        bits.at(position) = disable ? 0 : 1;
        prop.first = property_it->first();
        prop.second = vector_to_string(bits);
        property_it->modify(prop);
        ret_value = true;
    }

    return ret_value;
}

/**
 * Enable or disable a writer on the v2 property.
 * @param[in] local_participant_name Name of the local participant.
 * @param[in,out] pdp_properties Properties of the local participant.
 * @param[in] id User defined ID of the writer.
 * @param[in] disable True to disable, false to enable.
 * @return True if the operation was successful, false otherwise.
 */
bool EDPStatic::enable_writer_on_v2_property(
        const std::string& local_participant_name,
        fastdds::dds::ParameterPropertyList_t& pdp_properties,
        uint16_t id,
        bool disable)
{
    bool ret_value {false};
    uint32_t position {0};

    if (xmlparser::XMLP_ret::XML_OK == mp_edpXML->lookforWriter(local_participant_name.c_str(), id, nullptr, position))
    {
        auto property_it = std::find_if(pdp_properties.begin(), pdp_properties.end(),
                        [](const fastdds::dds::ParameterProperty_t& property)
                        {
                            if (0 == property.first().compare("ESW"))
                            {
                                return true;
                            }

                            return false;
                        });

        std::vector<uint8_t> bits;
        std::pair<std::string, std::string> prop;

        if (pdp_properties.end() == property_it)
        {
            // Create empty property.
            prop.first = "ESW";
            size_t number_of_writers = mp_edpXML->get_number_of_writers(local_participant_name);
            // Make multiple of 8 bits
            number_of_writers = (number_of_writers + 7) & ~7;
            bits = std::vector<uint8_t>(number_of_writers, 0);
            prop.second = vector_to_string(bits);
            pdp_properties.push_back(prop);
            property_it = pdp_properties.begin();
            auto next_it = pdp_properties.begin();
            while (++next_it != pdp_properties.end())
            {
                property_it = next_it;
            }
        }
        else
        {
            bits = string_to_vector(property_it->second());
        }

        bits.at(position) = disable ? 0 : 1;
        prop.first = property_it->first();
        prop.second = vector_to_string(bits);
        property_it->modify(prop);
        ret_value = true;
    }

    return ret_value;
}

bool EDPStaticProperty::fromProperty(
        std::pair<std::string, std::string> prop)
{
    if (prop.first.substr(0, 17) == "eProsimaEDPStatic" && prop.first.substr(31, 2) == "ID")
    {
        this->m_endpointType = prop.first.substr(18, 6);
        this->m_status = prop.first.substr(25, 5);
        this->m_userIdStr = prop.first.substr(34, 100);
        std::stringstream ss;
        ss << m_userIdStr;
        ss >> m_userId;
        ss.clear();
        ss.str(std::string());
        ss << prop.second;
        int a, b, c, d;
        char ch;
        ss >> a >> ch >> b >> ch >> c >> ch >> d;
        m_entityId.value[0] = (octet)a; m_entityId.value[1] = (octet)b;
        m_entityId.value[2] = (octet)c; m_entityId.value[3] = (octet)d;
        return true;
    }
    else if (0 == prop.first.compare(0, 4, "EDS_"))
    {
        if (0 == prop.first.compare(4, 1, "R"))
        {
            this->m_endpointType = "Reader";
        }
        else if (0 == prop.first.compare(4, 1, "W"))
        {
            this->m_endpointType = "Writer";
        }
        else
        {
            return false;
        }
        if (0 == prop.first.compare(5, 1, "A"))
        {
            this->m_status = "ALIVE";
        }
        else if (0 == prop.first.compare(5, 1, "E"))
        {
            this->m_status = "ENDED";
        }
        else
        {
            return false;
        }
        this->m_userIdStr = prop.first.substr(7, 100);
        std::stringstream ss;
        ss << m_userIdStr;
        ss >> m_userId;
        ss.clear();
        ss.str(std::string());
        ss << prop.second;
        size_t count = std::count(prop.second.begin(), prop.second.end(), '.');
        int value = 0;
        char ch = 0;
        for (size_t it = 0; it <= count; ++it)
        {
            ss >> value;
            m_entityId.value[3 - (count - it)] = (octet)value;
            if (it != count)
            {
                ss >> ch;
            }
        }
        return true;
    }
    return false;
}

bool EDPStatic::process_reader_proxy_data(
        RTPSReader* reader,
        ReaderProxyData* rdata)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, rdata->guid.entityId << " in topic: " << rdata->topic_name);
    mp_PDP->getMutex()->lock();
    //Add the property list entry to our local pdp
    ParticipantProxyData* remote_pdata = mp_PDP->get_participant_proxy_data(reader->getGuid().guidPrefix);
    if (ExchangeFormat::v2 == exchange_format_)
    {
        enable_reader_on_v2_property(remote_pdata->participant_name.to_string(), remote_pdata->properties,
                rdata->user_defined_id(), false);
    }
    else
    {
        remote_pdata->properties.push_back(EDPStaticProperty::toProperty(exchange_format_, "Reader", "ALIVE",
                rdata->user_defined_id(), rdata->guid.entityId));
    }
    mp_PDP->getMutex()->unlock();
    this->mp_PDP->announceParticipantState(true);
    return true;
}

bool EDPStatic::process_writer_proxy_data(
        RTPSWriter* writer,
        WriterProxyData* wdata)
{
    EPROSIMA_LOG_INFO(RTPS_EDP, wdata->guid.entityId << " in topic: " << wdata->topic_name);
    mp_PDP->getMutex()->lock();
    //Add the property list entry to our local pdp
    ParticipantProxyData* remote_pdata = mp_PDP->get_participant_proxy_data(writer->getGuid().guidPrefix);
    if (ExchangeFormat::v2 == exchange_format_)
    {
        enable_writer_on_v2_property(remote_pdata->participant_name.to_string(), remote_pdata->properties,
                wdata->user_defined_id(), false);
    }
    else
    {
        remote_pdata->properties.push_back(EDPStaticProperty::toProperty(exchange_format_, "Writer", "ALIVE",
                wdata->user_defined_id(), wdata->guid.entityId));
    }
    mp_PDP->getMutex()->unlock();
    this->mp_PDP->announceParticipantState(true);
    return true;
}

bool EDPStatic::remove_reader(
        RTPSReader* rtps_reader)
{
    bool ret_value {false};
    std::lock_guard<std::recursive_mutex> guard(*mp_PDP->getMutex());
    ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
    if (ExchangeFormat::v2 == exchange_format_)
    {
        ret_value = enable_reader_on_v2_property(
            localpdata->participant_name.to_string(), localpdata->properties,
            rtps_reader->getAttributes().getUserDefinedID(), true);
    }
    else
    {
        for (ParameterPropertyList_t::iterator pit = localpdata->properties.begin();
                pit != localpdata->properties.end(); ++pit)
        {
            EDPStaticProperty staticproperty;
            if (staticproperty.fromProperty((*pit).pair()))
            {
                if (staticproperty.m_entityId == rtps_reader->getGuid().entityId)
                {
                    auto new_property = EDPStaticProperty::toProperty(exchange_format_, "Reader", "ENDED",
                                    rtps_reader->getAttributes().getUserDefinedID(), rtps_reader->getGuid().entityId);
                    if (!pit->modify(new_property))
                    {
                        EPROSIMA_LOG_ERROR(RTPS_EDP, "Failed to change property <"
                                << pit->first() << " | " << pit->second() << "> to <"
                                << new_property.first << " | " << new_property.second << ">");
                    }
                    else
                    {
                        ret_value = true;
                    }
                }
            }
        }
    }
    return ret_value;
}

bool EDPStatic::remove_writer(
        RTPSWriter* rtps_writer)
{
    bool ret_value {false};
    std::lock_guard<std::recursive_mutex> guard(*mp_PDP->getMutex());
    ParticipantProxyData* localpdata = this->mp_PDP->getLocalParticipantProxyData();
    if (ExchangeFormat::v2 == exchange_format_)
    {
        ret_value = enable_writer_on_v2_property(
            localpdata->participant_name.to_string(), localpdata->properties,
            rtps_writer->getAttributes().getUserDefinedID(), true);
    }
    else
    {
        for (ParameterPropertyList_t::iterator pit = localpdata->properties.begin();
                pit != localpdata->properties.end(); ++pit)
        {
            EDPStaticProperty staticproperty;
            if (staticproperty.fromProperty((*pit).pair()))
            {
                if (staticproperty.m_entityId == rtps_writer->getGuid().entityId)
                {
                    auto new_property = EDPStaticProperty::toProperty(exchange_format_, "Writer", "ENDED",
                                    rtps_writer->getAttributes().getUserDefinedID(), rtps_writer->getGuid().entityId);
                    if (!pit->modify(new_property))
                    {
                        EPROSIMA_LOG_ERROR(RTPS_EDP, "Failed to change property <"
                                << pit->first() << " | " << pit->second() << "> to <"
                                << new_property.first << " | " << new_property.second << ">");
                    }
                    else
                    {
                        ret_value = true;
                    }
                }
            }
        }
    }
    return ret_value;
}

void EDPStatic::assignRemoteEndpoints(
        const ParticipantProxyData& pdata,
        bool /*assign_secure_endpoints*/)
{
    GUID_t persistence_guid;

    // Fill persistence GUID if present in UserData.
    bool is_persistent_guid = (18 <= pdata.user_data.size()) &&
            ('V' == pdata.user_data.at(0)) && ('G' == pdata.user_data.at(1)) && ('W' == pdata.user_data.at(2));
    if (is_persistent_guid)
    {
        persistence_guid.guidPrefix.value[0] = pdata.user_data.at(3);
        persistence_guid.guidPrefix.value[1] = pdata.user_data.at(4);
        persistence_guid.guidPrefix.value[2] = pdata.user_data.at(5);
        persistence_guid.guidPrefix.value[3] = pdata.user_data.at(6);
        persistence_guid.guidPrefix.value[4] = pdata.user_data.at(7);
        persistence_guid.guidPrefix.value[5] = pdata.user_data.at(8);
        persistence_guid.guidPrefix.value[6] = pdata.user_data.at(9);
        persistence_guid.guidPrefix.value[7] = pdata.user_data.at(10);
        persistence_guid.guidPrefix.value[8] = pdata.user_data.at(11);
        persistence_guid.guidPrefix.value[9] = pdata.user_data.at(12);
        persistence_guid.guidPrefix.value[10] = pdata.user_data.at(13);
        persistence_guid.guidPrefix.value[11] = pdata.user_data.at(14);
        persistence_guid.entityId.value[0] = pdata.user_data.at(15);
        persistence_guid.entityId.value[1] = pdata.user_data.at(16);
        persistence_guid.entityId.value[2] = pdata.user_data.at(17);
    }

    std::string participant_name {pdata.participant_name.to_string()};
    for (ParameterPropertyList_t::const_iterator pit = pdata.properties.begin();
            pit != pdata.properties.end(); ++pit)
    {
        EDPStaticProperty staticproperty;
        persistence_guid.entityId.value[3] = 0;

        // Format v2
        if (0 == pit->first().compare("ESR"))
        {
            std::vector<uint8_t> bits = string_to_vector(pit->second());
            for (size_t i {0}; i < bits.size(); ++i)
            {
                ReaderProxyData* rdataptr {nullptr};
                if (xmlparser::XMLP_ret::XML_OK ==
                        mp_edpXML->get_reader_from_position(participant_name, i, &rdataptr))
                {
                    if (bits[i] && !this->mp_PDP->has_reader_proxy_data(rdataptr->guid))    //IF NOT FOUND, we CREATE AND PAIR IT
                    {
                        newRemoteReader(pdata.guid, pdata.participant_name, rdataptr->user_defined_id());
                    }
                    else if (!bits[i] && this->mp_PDP->has_reader_proxy_data(rdataptr->guid))
                    {
                        this->mp_PDP->removeReaderProxyData(rdataptr->guid);
                    }
                }
            }
        }
        else if (0 == pit->first().compare("ESW"))
        {
            std::vector<uint8_t> bits = string_to_vector(pit->second());
            for (size_t i {0}; i < bits.size(); ++i)
            {
                WriterProxyData* wdataptr {nullptr};
                if (xmlparser::XMLP_ret::XML_OK ==
                        mp_edpXML->get_writer_from_position(participant_name, i, &wdataptr))
                {
                    if (bits[i] && !this->mp_PDP->has_writer_proxy_data(wdataptr->guid))    //IF NOT FOUND, we CREATE AND PAIR IT
                    {
                        if (is_persistent_guid)
                        {
                            persistence_guid.entityId.value[3] = static_cast<uint8_t>(wdataptr->user_defined_id());
                        }
                        newRemoteWriter(pdata.guid, pdata.participant_name,
                                wdataptr->user_defined_id(), wdataptr->guid.entityId, persistence_guid);
                    }
                    else if (!bits[i] && this->mp_PDP->has_writer_proxy_data(wdataptr->guid))
                    {
                        this->mp_PDP->removeWriterProxyData(wdataptr->guid);
                    }
                }
            }
        }
        // Format v1 and v1_Reduced
        else if (staticproperty.fromProperty((*pit).pair()))
        {
            if (staticproperty.m_endpointType == "Reader" && staticproperty.m_status == "ALIVE")
            {
                GUID_t guid(pdata.guid.guidPrefix, staticproperty.m_entityId);
                if (!this->mp_PDP->has_reader_proxy_data(guid))    //IF NOT FOUND, we CREATE AND PAIR IT
                {
                    newRemoteReader(pdata.guid, pdata.participant_name,
                            staticproperty.m_userId, staticproperty.m_entityId);
                }
            }
            else if (staticproperty.m_endpointType == "Writer" && staticproperty.m_status == "ALIVE")
            {
                GUID_t guid(pdata.guid.guidPrefix, staticproperty.m_entityId);
                if (!this->mp_PDP->has_writer_proxy_data(guid))    //IF NOT FOUND, we CREATE AND PAIR IT
                {
                    if (is_persistent_guid)
                    {
                        persistence_guid.entityId.value[3] = static_cast<uint8_t>(staticproperty.m_userId);
                    }
                    newRemoteWriter(pdata.guid, pdata.participant_name,
                            staticproperty.m_userId, staticproperty.m_entityId, persistence_guid);
                }
            }
            else if (staticproperty.m_endpointType == "Reader" && staticproperty.m_status == "ENDED")
            {
                GUID_t guid(pdata.guid.guidPrefix, staticproperty.m_entityId);
                this->mp_PDP->removeReaderProxyData(guid);
            }
            else if (staticproperty.m_endpointType == "Writer" && staticproperty.m_status == "ENDED")
            {
                GUID_t guid(pdata.guid.guidPrefix, staticproperty.m_entityId);
                this->mp_PDP->removeWriterProxyData(guid);
            }
            else
            {
                EPROSIMA_LOG_WARNING(RTPS_EDP, "EDPStaticProperty with type: " << staticproperty.m_endpointType
                                                                               << " and status " << staticproperty.m_status <<
                        " not recognized");
            }
        }
    }
}

bool EDPStatic::newRemoteReader(
        const GUID_t& participant_guid,
        const fastcdr::string_255& participant_name,
        uint16_t user_id,
        EntityId_t ent_id)
{
    ReaderProxyData* rpd = NULL;
    uint32_t position {0};

    if (xmlparser::XMLP_ret::XML_OK == mp_edpXML->lookforReader(participant_name, user_id, &rpd, position))
    {
        EPROSIMA_LOG_INFO(RTPS_EDP, "Activating: " << rpd->guid.entityId << " in topic " << rpd->topic_name);
        GUID_t reader_guid(participant_guid.guidPrefix, ent_id != c_EntityId_Unknown ? ent_id : rpd->guid.entityId);

        auto init_fun = [this, participant_guid, reader_guid, rpd](
            ReaderProxyData* newRPD,
            bool updating,
            const ParticipantProxyData& participant_data)
                {
                    // Should be a new reader
                    (void)updating;
                    assert(!updating);

                    *newRPD = *rpd;
                    newRPD->guid = reader_guid;
                    if (!checkEntityId(newRPD))
                    {
                        EPROSIMA_LOG_ERROR(RTPS_EDP, "The provided entityId for Reader with ID: "
                                << newRPD->user_defined_id() << " does not match the topic Kind");
                        return false;
                    }
                    newRPD->key() = newRPD->guid;
                    newRPD->rtps_participant_key() = participant_guid;
                    if (!newRPD->has_locators())
                    {
                        const NetworkFactory& network = mp_RTPSParticipant->network_factory();
                        newRPD->set_remote_locators(participant_data.default_locators, network, true,
                                participant_data.is_from_this_host());
                    }

                    return true;
                };

        GUID_t temp_participant_guid;
        ReaderProxyData* reader_data = this->mp_PDP->addReaderProxyData(reader_guid, temp_participant_guid, init_fun);
        if (reader_data != nullptr)
        {
            this->pairing_reader_proxy_with_any_local_writer(participant_guid, reader_data);
            return true;
        }
    }
    return false;
}

bool EDPStatic::newRemoteWriter(
        const GUID_t& participant_guid,
        const fastcdr::string_255& participant_name,
        uint16_t user_id,
        EntityId_t ent_id,
        const GUID_t& persistence_guid)
{
    WriterProxyData* wpd = NULL;
    uint32_t position {0};

    if (xmlparser::XMLP_ret::XML_OK == mp_edpXML->lookforWriter(participant_name, user_id, &wpd, position))
    {
        EPROSIMA_LOG_INFO(RTPS_EDP, "Activating: " << wpd->guid.entityId << " in topic " << wpd->topic_name);
        GUID_t writer_guid(participant_guid.guidPrefix, ent_id != c_EntityId_Unknown ? ent_id : wpd->guid.entityId);

        auto init_fun = [this, participant_guid, writer_guid, wpd, persistence_guid](
            WriterProxyData* newWPD,
            bool updating,
            const ParticipantProxyData& participant_data)
                {
                    // Should be a new reader
                    (void)updating;
                    assert(!updating);

                    *newWPD = *wpd;
                    newWPD->guid = writer_guid;
                    if (!checkEntityId(newWPD))
                    {
                        EPROSIMA_LOG_ERROR(RTPS_EDP, "The provided entityId for Writer with User ID: "
                                << newWPD->user_defined_id() << " does not match the topic Kind");
                        return false;
                    }
                    newWPD->key() = newWPD->guid;
                    newWPD->rtps_participant_key() = participant_guid;
                    if (!newWPD->has_locators())
                    {
                        const NetworkFactory& network = mp_RTPSParticipant->network_factory();
                        newWPD->set_remote_locators(participant_data.default_locators, network, true,
                                participant_data.is_from_this_host());
                    }
                    newWPD->persistence_guid = persistence_guid;

                    return true;
                };
        GUID_t temp_participant_guid;
        WriterProxyData* writer_data = this->mp_PDP->addWriterProxyData(writer_guid, temp_participant_guid, init_fun);
        if (writer_data != nullptr)
        {
            this->pairing_writer_proxy_with_any_local_reader(participant_guid, writer_data);
            return true;
        }
    }
    return false;
}

bool EDPStatic::checkEntityId(
        ReaderProxyData* rdata)
{
    if (rdata->topic_kind == WITH_KEY && rdata->guid.entityId.value[3] == 0x07)
    {
        return true;
    }
    if (rdata->topic_kind == NO_KEY && rdata->guid.entityId.value[3] == 0x04)
    {
        return true;
    }
    return false;
}

bool EDPStatic::checkEntityId(
        WriterProxyData* wdata)
{
    if (wdata->topic_kind == WITH_KEY && wdata->guid.entityId.value[3] == 0x02)
    {
        return true;
    }
    if (wdata->topic_kind == NO_KEY && wdata->guid.entityId.value[3] == 0x03)
    {
        return true;
    }
    return false;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
