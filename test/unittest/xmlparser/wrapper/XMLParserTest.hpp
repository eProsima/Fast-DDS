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

#include <fastrtps/xmlparser/XMLParser.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

using eprosima::fastrtps::xmlparser::XMLP_ret;
using eprosima::fastrtps::xmlparser::XMLParser;
using eprosima::fastrtps::xmlparser::DataNode;

// Class to test protected methods
class XMLParserTest : public XMLParser{
    public:
    static XMLP_ret getXMLWriterQosPolicies_wrapper(
        tinyxml2::XMLElement* elem,
        WriterQos& qos,
        uint8_t ident)
    {
        return getXMLWriterQosPolicies(elem, qos, ident);
    }
    static XMLP_ret getXMLReaderQosPolicies_wrapper(
        tinyxml2::XMLElement* elem,
        ReaderQos& qos,
        uint8_t ident)
    {
        return getXMLReaderQosPolicies(elem, qos, ident);
    }
    static XMLP_ret getXMLLocatorList_wrapper(
        tinyxml2::XMLElement* elem,
        LocatorList_t& locatorList,
        uint8_t ident)
    {
        return getXMLLocatorList(elem, locatorList, ident);
    }

    static XMLP_ret fillDataNode_wrapper(
        tinyxml2::XMLElement* p_profile,
        DataNode<ParticipantAttributes>& participant_node)
    {
        return fillDataNode(p_profile, participant_node);
    }

    static XMLP_ret getXMLDiscoverySettings_wrapper(
        tinyxml2::XMLElement* elem,
        rtps::DiscoverySettings& settings,
        uint8_t ident)
    {
        return getXMLDiscoverySettings(elem,settings,ident);
    }

    // INIT FUNCTIONS NACHO SECTION


    // FINISH FUNCTIONS NACHO SECTION


    // INIT FUNCTIONS RAUL SECTION


    // FINISH FUNCTIONS RAUL SECTION


    // INIT FUNCTIONS PARIS SECTION


    // FINISH FUNCTIONS PARIS SECTION

};
