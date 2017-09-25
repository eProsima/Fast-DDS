#include <tinyxml2.h>
#include <fastrtps/xmlparser/XMLParserCommon.h>
#include <fastrtps/xmlparser/XMLProfileParser.h>

namespace eprosima {
namespace fastrtps {
namespace xmlparser {

std::map<std::string, ParticipantAttributes> XMLProfileParser::m_participant_profiles;
ParticipantAttributes default_participant_attributes;
std::map<std::string, PublisherAttributes>   XMLProfileParser::m_publisher_profiles;
PublisherAttributes default_publisher_attributes;
std::map<std::string, SubscriberAttributes>  XMLProfileParser::m_subscriber_profiles;
SubscriberAttributes default_subscriber_attributes;
std::map<std::string, XMLP_ret>              XMLProfileParser::m_xml_files;

XMLP_ret XMLProfileParser::fillParticipantAttributes(const std::string &profile_name, ParticipantAttributes &atts)
{
    part_map_iterator_t it = m_participant_profiles.find(profile_name);
    if (it == m_participant_profiles.end())
    {
        logError(XMLPROFILEPARSER, "Profile '" << profile_name << "' not found '");
        return XMLP_ret::XML_ERROR;
    }
    atts = it->second;
    return XMLP_ret::XML_OK;
}

void XMLProfileParser::getDefaultParticipantAttributes(ParticipantAttributes& participant_attributes)
{
    participant_attributes = default_participant_attributes;
}

void XMLProfileParser::getDefaultPublisherAttributes(PublisherAttributes& publisher_attributes)
{
    publisher_attributes = default_publisher_attributes;
}

void XMLProfileParser::getDefaultSubscriberAttributes(SubscriberAttributes& subscriber_attributes)
{
    subscriber_attributes = default_subscriber_attributes;
}

XMLP_ret XMLProfileParser::fillPublisherAttributes(const std::string &profile_name, PublisherAttributes &atts)
{
    publ_map_iterator_t it = m_publisher_profiles.find(profile_name);
    if (it == m_publisher_profiles.end())
    {
        logError(XMLPROFILEPARSER, "Profile '" << profile_name << "' not found '");
        return XMLP_ret::XML_ERROR;
    }
    atts = it->second;
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileParser::fillSubscriberAttributes(const std::string &profile_name, SubscriberAttributes &atts)
{
    subs_map_iterator_t it = m_subscriber_profiles.find(profile_name);
    if (it == m_subscriber_profiles.end())
    {
        logError(XMLPROFILEPARSER, "Profile '" << profile_name << "' not found");
        return XMLP_ret::XML_ERROR;
    }
    atts = it->second;
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileParser::loadDefaultXMLFile()
{
    return loadXMLFile(DEFAULT_FASTRTPS_PROFILES);
}

XMLP_ret XMLProfileParser::loadXMLFile(const std::string &filename)
{
    if (filename.empty())
    {
        logError(XMLPROFILEPARSER, "Error loading XML file, filename empty");
        return XMLP_ret::XML_ERROR;
    }

    xmlfile_map_iterator_t it = m_xml_files.find(filename);
    if (it != m_xml_files.end() && XMLP_ret::XML_OK == it->second)
    {
        logInfo(XMLPROFILEPARSER, "XML file '" << filename << "' already parsed");
        return XMLP_ret::XML_OK;
    }

    tinyxml2::XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile(filename.c_str());

    if (XML_SUCCESS != eResult)
    {
        if (filename != std::string(DEFAULT_FASTRTPS_PROFILES))
            logError(XMLPROFILEPARSER, "Error opening '" << filename << "'");
        m_xml_files.emplace(filename, XMLP_ret::XML_ERROR);
        return XMLP_ret::XML_ERROR;
    }

    logInfo(XMLPROFILEPARSER, "File '" << filename << "' opened successfully");

    XMLElement* p_root = xmlDoc.FirstChildElement(PROFILES);
    if (nullptr == p_root)
    {
        logError(XMLPROFILEPARSER, "Not found 'profiles' root tag");
        return XMLP_ret::XML_ERROR;
    }

    std::string profile_name = "";
    unsigned int profileCount = 0u;
    XMLElement *p_profile = p_root->FirstChildElement();
    const char *tag = nullptr;
    while (nullptr != p_profile)
    {
        if (nullptr != (tag = p_profile->Value()))
        {
            bool is_default_profile = false;
            p_profile->QueryBoolAttribute("is_default_profile", &is_default_profile);

            // If profile parsing functions fails, log and continue.
            if (strcmp(tag, PARTICIPANT) == 0)
            {
                ParticipantAttributes participant_atts;
                if (XMLP_ret::XML_OK == parseXMLParticipantProf(p_profile, participant_atts, profile_name))
                {
                    if(is_default_profile)
                    {
                        default_participant_attributes = participant_atts;
                    }

                    if (false == m_participant_profiles.emplace(profile_name, participant_atts).second)
                    {
                        logError(XMLPROFILEPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
                    }
                    ++profileCount;
                }
                else
                {
                    logError(XMLPROFILEPARSER, "Error parsing participant profile");
                }
            }
            else if (strcmp(tag, PUBLISHER) == 0)
            {
                PublisherAttributes publisher_atts;
                if (XMLP_ret::XML_OK == parseXMLPublisherProf(p_profile, publisher_atts, profile_name))
                {
                    if(is_default_profile)
                    {
                        default_publisher_attributes = publisher_atts;
                    }

                    if (false == m_publisher_profiles.emplace(profile_name, publisher_atts).second)
                    {
                        logError(XMLPROFILEPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
                    }
                    ++profileCount;
                }
                else
                {
                    logError(XMLPROFILEPARSER, "Error parsing publisher profile");
                }
            }
            else if (strcmp(tag, SUBSCRIBER) == 0)
            {
                SubscriberAttributes subscriber_atts;
                parseXMLSubscriberProf(p_profile, subscriber_atts, profile_name);
                if (XMLP_ret::XML_OK == parseXMLSubscriberProf(p_profile, subscriber_atts, profile_name))
                {
                    if(is_default_profile)
                    {
                        default_subscriber_attributes = subscriber_atts;
                    }

                    if (false == m_subscriber_profiles.emplace(profile_name, subscriber_atts).second)
                    {
                        logError(XMLPROFILEPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
                    }
                    ++profileCount;
                }
                else
                {
                    logError(XMLPROFILEPARSER, "Error parsing subscriber profile");
                }
            }
            else
            {
                logError(XMLPROFILEPARSER, "Not expected tag: '" << tag << "'");
            }
        }
        p_profile = p_profile->NextSiblingElement();
    }

    if (0 == profileCount)
    {
        m_xml_files.emplace(filename, XMLP_ret::XML_ERROR);
        logError(XMLPROFILEPARSER, "Bad file '" << filename << "' content expected tag: '" << tag << "'");
        return XMLP_ret::XML_ERROR;

    }

    m_xml_files.emplace(filename, XMLP_ret::XML_OK);

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileParser::parseXMLParticipantProf(XMLElement *p_profile,
                                                   ParticipantAttributes &participant_atts,
                                                   std::string &profile_name)
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
        <!-- <xs:element name="userTransports" type="XXX"/> -->
        <xs:element name="useBuiltinTransports" type="boolType"/>
        <xs:element name="propertiesPolicy" type="propertyPolicyType"/>
        <xs:element name="name" type="stringType"/>
      </xs:all>
    </xs:complexType>*/

    if (nullptr == p_profile)
    {
        logError(XMLPROFILEPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }
    const char *prof_name = p_profile->Attribute(PROFILE_NAME);
    if (nullptr == prof_name)
    {
        logError(XMLPROFILEPARSER, "Not found '" << PROFILE_NAME << "' attribute");
        return XMLP_ret::XML_ERROR;
    }
    profile_name = prof_name;

    XMLElement *p_element = p_profile->FirstChildElement(RTPS);
    if (nullptr == p_element)
    {
        logError(XMLPROFILEPARSER, "Not found '" << RTPS << "' tag");
        return XMLP_ret::XML_ERROR;
    }

    uint8_t ident = 1;
    XMLElement *p_aux = nullptr;
    // defaultUnicastLocatorList
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_UNI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, participant_atts.rtps.defaultUnicastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // defaultMulticastLocatorList
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_MULTI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, participant_atts.rtps.defaultMulticastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // defaultOutLocatorList
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_OUT_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, participant_atts.rtps.defaultOutLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // defaultSendPort - uint32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_SEND_PORT)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux, &participant_atts.rtps.defaultSendPort, ident))
            return XMLP_ret::XML_ERROR;
    }
    // sendSocketBufferSize - uint32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(SEND_SOCK_BUF_SIZE)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux, &participant_atts.rtps.sendSocketBufferSize, ident))
            return XMLP_ret::XML_ERROR;
    }
    // listenSocketBufferSize - uint32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(LIST_SOCK_BUF_SIZE)))
    {
        if (XMLP_ret::XML_OK != getXMLUint(p_aux, &participant_atts.rtps.listenSocketBufferSize, ident))
            return XMLP_ret::XML_ERROR;
    }
    // builtin
    if (nullptr != (p_aux = p_element->FirstChildElement(BUILTIN)))
    {
        if (XMLP_ret::XML_OK != getXMLBuiltinAttributes(p_aux, participant_atts.rtps.builtin, ident))
            return XMLP_ret::XML_ERROR;
    }
    // port
    if (nullptr != (p_aux = p_element->FirstChildElement(PORT)))
    {
        if (XMLP_ret::XML_OK != getXMLPortParameters(p_aux, participant_atts.rtps.port, ident))
            return XMLP_ret::XML_ERROR;
    }
    // TODO: userData
    if (nullptr != (p_aux = p_element->FirstChildElement(USER_DATA)))
    {
        if (XMLP_ret::XML_OK != getXMLOctetVector(p_aux, participant_atts.rtps.userData, ident))
            return XMLP_ret::XML_ERROR;
    }
    // participantID - int32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(PART_ID)))
    {
        if (XMLP_ret::XML_OK != getXMLInt(p_aux, &participant_atts.rtps.participantID, ident))
            return XMLP_ret::XML_ERROR;
    }
    // use_IP4_to_send - boolType
    if (nullptr != (p_aux = p_element->FirstChildElement(IP4_TO_SEND)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux, &participant_atts.rtps.use_IP4_to_send, ident))
            return XMLP_ret::XML_ERROR;
    }
    // use_IP6_to_send - boolType
    if (nullptr != (p_aux = p_element->FirstChildElement(IP6_TO_SEND)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux, &participant_atts.rtps.use_IP6_to_send, ident))
            return XMLP_ret::XML_ERROR;
    }
    // throughputController
    if (nullptr != (p_aux = p_element->FirstChildElement(THROUGHPUT_CONT)))
    {
        if (XMLP_ret::XML_OK != getXMLThroughputController(p_aux, participant_atts.rtps.throughputController, ident))
            return XMLP_ret::XML_ERROR;
    }
    // TODO: userTransports
    if (nullptr != (p_aux = p_element->FirstChildElement(USER_TRANS)))
    {
        logError(XMLPROFILEPARSER, "Attribute '" << p_aux->Value() << "' do not supported for now");
    }

    // useBuiltinTransports - boolType
    if (nullptr != (p_aux = p_element->FirstChildElement(USE_BUILTIN_TRANS)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux, &participant_atts.rtps.useBuiltinTransports, ident))
            return XMLP_ret::XML_ERROR;
    }
    // propertiesPolicy
    if (nullptr != (p_aux = p_element->FirstChildElement(PROPERTIES_POLICY)))
    {
        if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux, participant_atts.rtps.properties, ident))
            return XMLP_ret::XML_ERROR;
    }
    // name - stringType
    if (nullptr != (p_aux = p_element->FirstChildElement(NAME)))
    {
        std::string s = "";
        if (XMLP_ret::XML_OK != getXMLString(p_aux, &s, ident)) return XMLP_ret::XML_ERROR;
        participant_atts.rtps.setName(s.c_str());
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileParser::parseXMLPublisherProf(XMLElement *p_profile,
                                                 PublisherAttributes &publisher_atts,
                                                 std::string &profile_name)
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
        logError(XMLPROFILEPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }
    const char *prof_name = p_profile->Attribute(PROFILE_NAME);
    if (nullptr == prof_name)
    {
        logError(XMLPROFILEPARSER, "Not found '" << PROFILE_NAME << "' attribute");
        return XMLP_ret::XML_ERROR;
    }
    profile_name = prof_name;

    uint8_t ident = 1;
    XMLElement *p_aux = nullptr;
    // topic
    if (nullptr != (p_aux = p_profile->FirstChildElement(TOPIC)))
    {
        if (XMLP_ret::XML_OK != getXMLTopicAttributes(p_aux, publisher_atts.topic, ident))
            return XMLP_ret::XML_ERROR;
    }
    // qos
    if (nullptr != (p_aux = p_profile->FirstChildElement(QOS)))
    {
        if (XMLP_ret::XML_OK != getXMLWriterQosPolicies(p_aux, publisher_atts.qos, ident))
            return XMLP_ret::XML_ERROR;
    }
    // times
    if (nullptr != (p_aux = p_profile->FirstChildElement(TIMES)))
    {
        if (XMLP_ret::XML_OK != getXMLWriterTimes(p_aux, publisher_atts.times, ident))
            return XMLP_ret::XML_ERROR;
    }
    // unicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(UNI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, publisher_atts.unicastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // multicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(MULTI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, publisher_atts.multicastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // outLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(OUT_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, publisher_atts.outLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // throughputController
    if (nullptr != (p_aux = p_profile->FirstChildElement(THROUGHPUT_CONT)))
    {
        if (XMLP_ret::XML_OK != getXMLThroughputController(p_aux, publisher_atts.throughputController, ident))
            return XMLP_ret::XML_ERROR;
    }
    // historyMemoryPolicy
    if (nullptr != (p_aux = p_profile->FirstChildElement(HIST_MEM_POLICY)))
    {
        if (XMLP_ret::XML_OK != getXMLHistoryMemoryPolicy(p_aux, publisher_atts.historyMemoryPolicy, ident))
            return XMLP_ret::XML_ERROR;
    }
    // propertiesPolicy
    if (nullptr != (p_aux = p_profile->FirstChildElement(PROPERTIES_POLICY)))
    {
        if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux, publisher_atts.properties, ident))
            return XMLP_ret::XML_ERROR;
    }
    // userDefinedID - int16type
    if (nullptr != (p_aux = p_profile->FirstChildElement(USER_DEF_ID)))
    {
        int i = 0;
        if (XMLP_ret::XML_OK != getXMLInt(p_aux, &i, ident) || i > 255) return XMLP_ret::XML_ERROR;
        publisher_atts.setUserDefinedID(static_cast<uint8_t>(i));
    }
    // entityID - int16Type
    if (nullptr != (p_aux = p_profile->FirstChildElement(ENTITY_ID)))
    {
        int i = 0;
        if (XMLP_ret::XML_OK != getXMLInt(p_aux, &i, ident) || i > 255) return XMLP_ret::XML_ERROR;
        publisher_atts.setEntityID(static_cast<uint8_t>(i));
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileParser::parseXMLSubscriberProf(XMLElement *p_profile,
                                                  SubscriberAttributes &subscriber_atts,
                                                  std::string &profile_name)
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
        logError(XMLPROFILEPARSER, "Bad parameters!");
        return XMLP_ret::XML_ERROR;
    }
    const char *prof_name = p_profile->Attribute(PROFILE_NAME);
    if (nullptr == prof_name)
    {
        logError(XMLPROFILEPARSER, "Not found '" << PROFILE_NAME << "' attribute");
        return XMLP_ret::XML_ERROR;
    }
    profile_name = prof_name;

    uint8_t ident = 1;
    XMLElement *p_aux = nullptr;
    // topic
    if (nullptr != (p_aux = p_profile->FirstChildElement(TOPIC)))
    {
        if (XMLP_ret::XML_OK != getXMLTopicAttributes(p_aux, subscriber_atts.topic, ident))
            return XMLP_ret::XML_ERROR;
    }
    // qos
    if (nullptr != (p_aux = p_profile->FirstChildElement(QOS)))
    {
        if (XMLP_ret::XML_OK != getXMLReaderQosPolicies(p_aux, subscriber_atts.qos, ident))
            return XMLP_ret::XML_ERROR;
    }
    // times
    if (nullptr != (p_aux = p_profile->FirstChildElement(TIMES)))
    {
        if (XMLP_ret::XML_OK != getXMLReaderTimes(p_aux, subscriber_atts.times, ident))
            return XMLP_ret::XML_ERROR;
    }
    // unicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(UNI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, subscriber_atts.unicastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // multicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(MULTI_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, subscriber_atts.multicastLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // outLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(OUT_LOC_LIST)))
    {
        if (XMLP_ret::XML_OK != getXMLLocatorList(p_aux, subscriber_atts.outLocatorList, ident))
            return XMLP_ret::XML_ERROR;
    }
    // expectsInlineQos - boolType
    if (nullptr != (p_aux = p_profile->FirstChildElement(EXP_INLINE_QOS)))
    {
        if (XMLP_ret::XML_OK != getXMLBool(p_aux, &subscriber_atts.expectsInlineQos, ident))
            return XMLP_ret::XML_ERROR;
    }
    // historyMemoryPolicy
    if (nullptr != (p_aux = p_profile->FirstChildElement(HIST_MEM_POLICY)))
    {
        if (XMLP_ret::XML_OK != getXMLHistoryMemoryPolicy(p_aux, subscriber_atts.historyMemoryPolicy, ident))
            return XMLP_ret::XML_ERROR;
    }
    // propertiesPolicy
    if (nullptr != (p_aux = p_profile->FirstChildElement(PROPERTIES_POLICY)))
    {
        if (XMLP_ret::XML_OK != getXMLPropertiesPolicy(p_aux, subscriber_atts.properties, ident))
            return XMLP_ret::XML_ERROR;
    }
    // userDefinedID - int16Type
    if (nullptr != (p_aux = p_profile->FirstChildElement(USER_DEF_ID)))
    {
        int i = 0;
        if (XMLP_ret::XML_OK != getXMLInt(p_aux, &i, ident) || i > 255) return XMLP_ret::XML_ERROR;
        subscriber_atts.setUserDefinedID(static_cast<uint8_t>(i));
    }
    // entityID - int16Type
    if (nullptr != (p_aux = p_profile->FirstChildElement(ENTITY_ID)))
    {
        int i = 0;
        if (XMLP_ret::XML_OK != getXMLInt(p_aux, &i, ident) || i > 255) return XMLP_ret::XML_ERROR;
        subscriber_atts.setEntityID(static_cast<uint8_t>(i));
    }
    return XMLP_ret::XML_OK;
}

} /* xmlparser  */
} /* namespace  */
} /* namespace eprosima */
