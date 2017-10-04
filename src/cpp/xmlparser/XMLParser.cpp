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

#include <tinyxml2.h>

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
            logError(XMLPARSER, "Not found root tag");
            ret = XMLP_ret::XML_ERROR;
        }
        else
        {
            root.reset(new BaseNode{NodeType::PROFILES});
            ret  = parseProfiles(p_root, *root);
        }
    }
    else
    {
        root.reset(new BaseNode{NodeType::ROOT});
        tinyxml2::XMLElement* node = p_root->FirstChildElement();
        const char* tag       = nullptr;
        while (nullptr != node)
        {
            if (nullptr != (tag = node->Value()))
            {
                if (strcmp(tag, PROFILES) == 0)
                {
                    up_base_node_t profiles_node = up_base_node_t{new BaseNode{NodeType::PROFILES}};
                    if (XMLP_ret::XML_OK == (ret = parseProfiles(node, *profiles_node)))
                    {
                        root->addChild(std::move(profiles_node));
                    }
                }
                else if (strcmp(tag, TOPIC) == 0)
                {
                    ret = parseXMLTopicData(node, *root);
                }
            }

            node = node->NextSiblingElement();
        }
    }
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
    tinyxml2::XMLElement* p_profile = p_root->FirstChildElement();
    const char* tag       = nullptr;
    while (nullptr != p_profile)
    {
        if (nullptr != (tag = p_profile->Value()))
        {
            // If profile parsing functions fails, log and continue.
            if (strcmp(tag, PARTICIPANT) == 0)
            {
                parseXMLParticipantProf(p_profile, profilesNode);
            }
            else if (strcmp(tag, PUBLISHER) == 0)
            {
                parseXMLPublisherProf(p_profile, profilesNode);
            }
            else if (strcmp(tag, SUBSCRIBER) == 0)
            {
                parseXMLSubscriberProf(p_profile, profilesNode);
            }
            else if (strcmp(tag, QOS_PROFILE))
            {
            }
            else if (strcmp(tag, APPLICATION))
            {
            }
            else if (strcmp(tag, TYPE))
            {
            }
            else if (strcmp(tag, TOPIC))
            {
            }
            else if (strcmp(tag, DATA_WRITER))
            {
            }
            else if (strcmp(tag, DATA_READER))
            {
            }
            else
            {
                logError(XMLPARSER, "Not expected tag: '" << tag << "'");
            }
        }
        p_profile = p_profile->NextSiblingElement();
    }
    return XMLP_ret::XML_OK;
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

    uint8_t ident     = 1;
    if (XMLP_ret::XML_OK != getXMLTopicAttributes(node, *topic_node.get(), ident))
    {
            return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::fillDataNode(tinyxml2::XMLElement* p_profile, DataNode<ParticipantAttributes>& participant_node)
{
    /*<xs:complexType name="rtpsParticipantAttributesType">
      <xs:all minOccurs="0">
        <xs:element name="defaultUnicastLocatorList" type="locatorListType"/>
        <xs:element name="defaultMulticastLocatorList" type="locatorListType"/>
        <xs:element name="defaultOutLocatorList" type="locatorListType"/>
        <xs:element name="defaultSendPort" type="uint32Type"/>
        <xs:element name="sendSocketBufferSize" type="uint32Type"/>
        <xs:element name="listenSocketBufferSize" type="uint32Type"/>
        <xs:element name="builtin" type="builtinAttributesType"/>
        <xs:element name="port" type="portType"/>
        <xs:element name="userData" type="octetVectorType"/>
        <xs:element name="participantID" type="int32Type"/>
        <xs:element name="use_IP4_to_send" type="boolType"/>
        <xs:element name="use_IP6_to_send" type="boolType"/>
        <xs:element name="throughputController" type="throughputControllerType"/>
        <xs:element name="userTransports" type="transportListType"/>
        <xs:element name="useBuiltinTransports" type="boolType"/>
        <xs:element name="propertiesPolicy" type="propertyPolicyType"/>
        <xs:element name="name" type="stringType"/>
      </xs:all>
    </xs:complexType>*/

    if (nullptr == p_profile)
    {
        logError(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    if (nullptr == p_profile->Attribute(PROFILE_NAME))
    {
        logError(XMLPARSER, "Not found '" << PROFILE_NAME << "' attribute");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, participant_node);

    tinyxml2::XMLElement* p_element = p_profile->FirstChildElement(RTPS);
    if (nullptr == p_element)
    {
        logError(XMLPARSER, "Not found '" << RTPS << "' tag");
        return XMLP_ret::XML_ERROR;
    }

    uint8_t ident     = 1;
    tinyxml2::XMLElement* p_aux = nullptr;
    // defaultUnicastLocatorList
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_UNI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK !=
            getXMLLocatorList(p_aux, participant_node.get()->rtps.defaultUnicastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // defaultMulticastLocatorList
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_MULTI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK !=
            getXMLLocatorList(p_aux, participant_node.get()->rtps.defaultMulticastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // defaultOutLocatorList
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_OUT_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, participant_node.get()->rtps.defaultOutLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // defaultSendPort - uint32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_SEND_PORT)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux, &participant_node.get()->rtps.defaultSendPort, ident))
            return XMLP_ret::XML_ERROR;
    }
    // sendSocketBufferSize - uint32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(SEND_SOCK_BUF_SIZE)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux, &participant_node.get()->rtps.sendSocketBufferSize, ident))
            return XMLP_ret::XML_ERROR;
    }
    // listenSocketBufferSize - uint32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(LIST_SOCK_BUF_SIZE)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux, &participant_node.get()->rtps.listenSocketBufferSize, ident))
            return XMLP_ret::XML_ERROR;
    }
    // builtin
    if (nullptr != (p_aux = p_element->FirstChildElement(BUILTIN)))
    {
        if (XMLP_ret::XML_OK != getXMLBuiltinAttributes(p_aux, participant_node.get()->rtps.builtin, ident))
            return XMLP_ret::XML_ERROR;
    }
    // port
    if (nullptr != (p_aux = p_element->FirstChildElement(PORT)))
    {
        if (XMLP_ret::XML_OK != getXMLPortParameters(p_aux, participant_node.get()->rtps.port, ident))
            return XMLP_ret::XML_ERROR;
    }
    // TODO: userData
    if (nullptr != (p_aux = p_element->FirstChildElement(USER_DATA)))
    {
        if (XMLP_ret::XML_OK != getXMLOctetVector(p_aux, participant_node.get()->rtps.userData, ident))
            return XMLP_ret::XML_ERROR;
    }
    // participantID - int32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(PART_ID)))
    {
        if (XMLP_ret::XML_OK != getXMLInt(p_aux, &participant_node.get()->rtps.participantID, ident))
            return XMLP_ret::XML_ERROR;
    }
    // use_IP4_to_send - boolType
    if (nullptr != (p_aux = p_element->FirstChildElement(IP4_TO_SEND)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux, &participant_node.get()->rtps.use_IP4_to_send, ident))
            return XMLP_ret::XML_ERROR;
    }
    // use_IP6_to_send - boolType
    if (nullptr != (p_aux = p_element->FirstChildElement(IP6_TO_SEND)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux, &participant_node.get()->rtps.use_IP6_to_send, ident))
            return XMLP_ret::XML_ERROR;
    }
    // throughputController
    if (nullptr != (p_aux = p_element->FirstChildElement(THROUGHPUT_CONT)))
    {
        if (XMLP_ret::XML_OK !=
            getXMLThroughputController(p_aux, participant_node.get()->rtps.throughputController, ident))
            return XMLP_ret::XML_ERROR;
    }
    // userTransports
    if (nullptr != (p_aux = p_element->FirstChildElement(USER_TRANS)))
    {
        if (XMLP_ret::XML_OK != getXMLTransportList(p_aux, participant_node.get()->rtps.userTransports, ident))
            return XMLP_ret::XML_ERROR;
    }

    // useBuiltinTransports - boolType
    if (nullptr != (p_aux = p_element->FirstChildElement(USE_BUILTIN_TRANS)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux, &participant_node.get()->rtps.useBuiltinTransports, ident))
            return XMLP_ret::XML_ERROR;
    }
    // propertiesPolicy
    if (nullptr != (p_aux = p_element->FirstChildElement(PROPERTIES_POLICY)))
    {
        if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux, participant_node.get()->rtps.properties, ident))
            return XMLP_ret::XML_ERROR;
    }
    // name - stringType
    if (nullptr != (p_aux = p_element->FirstChildElement(NAME)))
    {
        std::string s;
        if (XMLP_ret::XML_OK != getXMLString(p_aux, &s, ident))
            return XMLP_ret::XML_ERROR;
        participant_node.get()->rtps.setName(s.c_str());
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::fillDataNode(tinyxml2::XMLElement* p_profile, DataNode<PublisherAttributes>& publisher_node)
{
    /*<xs:complexType name="publisherProfileType">
      <xs:all minOccurs="0">
        <xs:element name="topic" type="topicAttributesType"/>
        <xs:element name="qos" type="writerQosPoliciesType"/>
        <xs:element name="times" type="writerTimesType"/>
        <xs:element name="unicastLocatorList" type="locatorListType"/>
        <xs:element name="multicastLocatorList" type="locatorListType"/>
        <xs:element name="outLocatorList" type="locatorListType"/>
        <xs:element name="throughputController" type="throughputControllerType"/>
        <xs:element name="historyMemoryPolicy" type="historyMemoryPolicyType"/>
        <xs:element name="userDefinedID" type="int16Type"/>
        <xs:element name="entityID" type="int16Type"/>
      </xs:all>
      <xs:attribute name="profile_name" type="stringType" use="required"/>
    </xs:complexType>*/

    if (nullptr == p_profile)
    {
        logError(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }
    if (nullptr == p_profile->Attribute(PROFILE_NAME))
    {
        logError(XMLPARSER, "Not found '" << PROFILE_NAME << "' attribute");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, publisher_node);

    uint8_t ident     = 1;
    tinyxml2::XMLElement* p_aux = nullptr;
    // topic
    if (nullptr != (p_aux = p_profile->FirstChildElement(TOPIC)))
    {
        if (XMLP_ret::XML_OK != getXMLTopicAttributes(p_aux, publisher_node.get()->topic, ident))
            return XMLP_ret::XML_ERROR;
    }
    // qos
    if (nullptr != (p_aux = p_profile->FirstChildElement(QOS)))
    {
        if (XMLP_ret::XML_OK != getXMLWriterQosPolicies(p_aux, publisher_node.get()->qos, ident))
            return XMLP_ret::XML_ERROR;
    }
    // times
    if (nullptr != (p_aux = p_profile->FirstChildElement(TIMES)))
    {
        if (XMLP_ret::XML_OK != getXMLWriterTimes(p_aux, publisher_node.get()->times, ident))
            return XMLP_ret::XML_ERROR;
    }
    // unicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(UNI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, publisher_node.get()->unicastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // multicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(MULTI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, publisher_node.get()->multicastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // outLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(OUT_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, publisher_node.get()->outLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // throughputController
    if (nullptr != (p_aux = p_profile->FirstChildElement(THROUGHPUT_CONT)))
    {
        if (XMLP_ret::XML_OK !=
            getXMLThroughputController(p_aux, publisher_node.get()->throughputController, ident))
            return XMLP_ret::XML_ERROR;
    }
    // historyMemoryPolicy
    if (nullptr != (p_aux = p_profile->FirstChildElement(HIST_MEM_POLICY)))
    {
        if (XMLP_ret::XML_OK != getXMLHistoryMemoryPolicy(p_aux, publisher_node.get()->historyMemoryPolicy, ident))
            return XMLP_ret::XML_ERROR;
    }
    // propertiesPolicy
    if (nullptr != (p_aux = p_profile->FirstChildElement(PROPERTIES_POLICY)))
    {
        if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux, publisher_node.get()->properties, ident))
            return XMLP_ret::XML_ERROR;
    }
    // userDefinedID - int16type
    if (nullptr != (p_aux = p_profile->FirstChildElement(USER_DEF_ID)))
    {
        int i = 0;
        if (XMLP_ret::XML_OK != getXMLInt(p_aux, &i, ident) || i > 255)
            return XMLP_ret::XML_ERROR;
        publisher_node.get()->setUserDefinedID(static_cast<uint8_t>(i));
    }
    // entityID - int16Type
    if (nullptr != (p_aux = p_profile->FirstChildElement(ENTITY_ID)))
    {
        int i = 0;
        if (XMLP_ret::XML_OK != getXMLInt(p_aux, &i, ident) || i > 255)
            return XMLP_ret::XML_ERROR;
        publisher_node.get()->setEntityID(static_cast<uint8_t>(i));
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLParser::fillDataNode(tinyxml2::XMLElement* p_profile, DataNode<SubscriberAttributes>& subscriber_node)
{
    /*<xs:complexType name="subscriberProfileType">
      <xs:all minOccurs="0">
        <xs:element name="topic" type="topicAttributesType"/>
        <xs:element name="qos" type="readerQosPoliciesType"/>
        <xs:element name="times" type="readerTimesType"/>
        <xs:element name="unicastLocatorList" type="locatorListType"/>
        <xs:element name="multicastLocatorList" type="locatorListType"/>
        <xs:element name="outLocatorList" type="locatorListType"/>
        <xs:element name="expectsInlineQos" type="boolType"/>
        <xs:element name="historyMemoryPolicy" type="historyMemoryPolicyType"/>
        <xs:element name="propertiesPolicy" type="propertyPolicyType"/>
        <xs:element name="userDefinedID" type="int16Type"/>
        <xs:element name="entityID" type="int16Type"/>
      </xs:all>
      <xs:attribute name="profile_name" type="stringType" use="required"/>
    </xs:complexType>*/

    if (nullptr == p_profile)
    {
        logError(XMLPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }

    if (nullptr == p_profile->Attribute(PROFILE_NAME))
    {
        logError(XMLPARSER, "Not found '" << PROFILE_NAME << "' attribute");
        return XMLP_ret::XML_ERROR;
    }

    addAllAttributes(p_profile, subscriber_node);

    uint8_t ident     = 1;
    tinyxml2::XMLElement* p_aux = nullptr;
    // topic
    if (nullptr != (p_aux = p_profile->FirstChildElement(TOPIC)))
    {
        if (XMLP_ret::XML_OK != getXMLTopicAttributes(p_aux, subscriber_node.get()->topic, ident))
            return XMLP_ret::XML_ERROR;
    }
    // qos
    if (nullptr != (p_aux = p_profile->FirstChildElement(QOS)))
    {
        if (XMLP_ret::XML_OK != getXMLReaderQosPolicies(p_aux, subscriber_node.get()->qos, ident))
            return XMLP_ret::XML_ERROR;
    }
    // times
    if (nullptr != (p_aux = p_profile->FirstChildElement(TIMES)))
    {
        if (XMLP_ret::XML_OK != getXMLReaderTimes(p_aux, subscriber_node.get()->times, ident))
            return XMLP_ret::XML_ERROR;
    }
    // unicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(UNI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, subscriber_node.get()->unicastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // multicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(MULTI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, subscriber_node.get()->multicastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // outLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(OUT_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, subscriber_node.get()->outLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // expectsInlineQos - boolType
    if (nullptr != (p_aux = p_profile->FirstChildElement(EXP_INLINE_QOS)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux, &subscriber_node.get()->expectsInlineQos, ident))
            return XMLP_ret::XML_ERROR;
    }
    // historyMemoryPolicy
    if (nullptr != (p_aux = p_profile->FirstChildElement(HIST_MEM_POLICY)))
    {
        if (XMLP_ret::XML_OK != getXMLHistoryMemoryPolicy(p_aux, subscriber_node.get()->historyMemoryPolicy, ident))
            return XMLP_ret::XML_ERROR;
    }
    // propertiesPolicy
    if (nullptr != (p_aux = p_profile->FirstChildElement(PROPERTIES_POLICY)))
    {
        if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux, subscriber_node.get()->properties, ident))
            return XMLP_ret::XML_ERROR;
    }
    // userDefinedID - int16Type
    if (nullptr != (p_aux = p_profile->FirstChildElement(USER_DEF_ID)))
    {
        int i = 0;
        if (XMLP_ret::XML_OK != getXMLInt(p_aux, &i, ident) || i > 255)
            return XMLP_ret::XML_ERROR;
        subscriber_node.get()->setUserDefinedID(static_cast<uint8_t>(i));
    }
    // entityID - int16Type
    if (nullptr != (p_aux = p_profile->FirstChildElement(ENTITY_ID)))
    {
        int i = 0;
        if (XMLP_ret::XML_OK != getXMLInt(p_aux, &i, ident) || i > 255)
            return XMLP_ret::XML_ERROR;
        subscriber_node.get()->setEntityID(static_cast<uint8_t>(i));
    }
    return XMLP_ret::XML_OK;
}

} // namespace xmlparser
} // namespace fastrtps
} // namespace eprosima
