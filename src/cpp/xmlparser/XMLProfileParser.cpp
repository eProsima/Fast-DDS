#include <fastrtps/xmlparser/XMLProfileParser.h>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps {


std::map<std::string, ParticipantAttributes> XMLProfileParser::m_participant_profiles;
std::map<std::string, PublisherAttributes>   XMLProfileParser::m_publisher_profiles;
std::map<std::string, SubscriberAttributes>  XMLProfileParser::m_subscriber_profiles;
std::map<std::string, XMLP_ret>              XMLProfileParser::m_xml_files;

XMLP_ret XMLProfileParser::fillParticipantProfileFromXMLFile(const std::string filename,
                                           const std::string profile_name,
                                           ParticipantAttributes &atts)
{
    if (XMLP_ret::ERROR == XMLProfileParser::loadXMLFile(filename))
    {
        logError(XMLPROFILEPARSER, "Error parsing file: " << filename);
        return XMLP_ret::ERROR;
    }

    return fillParticipantProfile(profile_name, atts);
}

XMLP_ret XMLProfileParser::fillPublisherProfileFromXMLFile(const std::string filename,
                                         const std::string profile_name,
                                         PublisherAttributes &atts)
{
    if (XMLP_ret::ERROR == XMLProfileParser::loadXMLFile(filename))
    {
        logError(XMLPROFILEPARSER, "Error parsing file: " << filename);
        return XMLP_ret::ERROR;
    }

    return fillPublisherProfile(profile_name, atts);
}

XMLP_ret XMLProfileParser::fillSubscriberProfileFromXMLFile(const std::string filename,
                                          const std::string profile_name,
                                          SubscriberAttributes &atts)
{
    if (XMLP_ret::ERROR == XMLProfileParser::loadXMLFile(filename))
    {
        logError(XMLPROFILEPARSER, "Error parsing file: " << filename);
        return XMLP_ret::ERROR;
    }

    return fillSubscriberProfile(profile_name, atts);
}

XMLP_ret XMLProfileParser::fillParticipantProfile(const std::string profile_name, ParticipantAttributes &atts)
{
    part_map_iterator_t it = m_participant_profiles.find(profile_name);
    if (it == m_participant_profiles.end())
    {
        logError(XMLPROFILEPARSER, "Profile '" << profile_name << "' not founded '");
        return XMLP_ret::ERROR;
    }
    atts = it->second;
    return XMLP_ret::OK;
}

XMLP_ret XMLProfileParser::fillPublisherProfile(const std::string profile_name, PublisherAttributes &atts)
{
    publ_map_iterator_t it = m_publisher_profiles.find(profile_name);
    if (it == m_publisher_profiles.end())
    {
        logError(XMLPROFILEPARSER, "Profile '" << profile_name << "' not founded '");
        return XMLP_ret::ERROR;
    }
    atts = it->second;
    return XMLP_ret::OK;
}

XMLP_ret XMLProfileParser::fillSubscriberProfile(const std::string profile_name, SubscriberAttributes &atts)
{
    subs_map_iterator_t it = m_subscriber_profiles.find(profile_name);
    if (it == m_subscriber_profiles.end())
    {
        logError(XMLPROFILEPARSER, "Profile '" << profile_name << "' not founded '");
        return XMLP_ret::ERROR;
    }
    atts = it->second;
    return XMLP_ret::OK;
}

XMLP_ret XMLProfileParser::loadXMLFile(const std::string filename)
{
    if (m_xml_files.find(filename) != m_xml_files.end())
    {
        logInfo(XMLPROFILEPARSER, "XML file '" << filename << "' already parsed");
        return XMLP_ret::OK;
    }

    tinyxml2::XMLDocument xmlDoc;
    XMLError eResult = xmlDoc.LoadFile(filename.c_str());

    if (XML_SUCCESS != eResult)
    {
        logError(XMLPROFILEPARSER, "Error opening '" << filename << "'");
        m_xml_files.emplace(filename, XMLP_ret::ERROR);
        return XMLP_ret::ERROR;
    }

    logInfo(XMLPROFILEPARSER, "File '" << filename << "' opened successfully");

    XMLElement* p_root = xmlDoc.FirstChildElement(PROFILES);
    if (nullptr == p_root)
    {
        logError(XMLPROFILEPARSER, "Do not founded 'profiles' root tag");
        return XMLP_ret::ERROR;
    }

    std::string profile_name = "";
    unsigned int profileCount = 0u;
    XMLElement *p_profile = p_root->FirstChildElement();
    const char *tag = nullptr;
    while (nullptr != p_profile)
    {
        if (nullptr != (tag = p_profile->Value()))
        {
            // If profile parsing functions fails, log and continue.
            if (strcmp(tag, PARTICIPANT) == 0)
            {
                ParticipantAttributes participant_atts;
                if (XMLP_ret::OK == parseXMLParticipantProf(p_profile, participant_atts, profile_name))
                {
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
                if (XMLP_ret::OK == parseXMLPublisherProf(p_profile, publisher_atts, profile_name))
                {
                    if (false == m_publisher_profiles.emplace(profile_name, publisher_atts).second)
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
            else if (strcmp(tag, SUBSCRIBER) == 0)
            {
                SubscriberAttributes subscriber_atts;
                parseXMLSubscriberProf(p_profile, subscriber_atts, profile_name);
                if (XMLP_ret::OK == parseXMLSubscriberProf(p_profile, subscriber_atts, profile_name))
                {
                    if (false == m_subscriber_profiles.emplace(profile_name, subscriber_atts).second)
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
            else
            {
                logError(XMLPROFILEPARSER, "Not expected tag: '" << tag << "'");
            }
        }
        p_profile = p_profile->NextSiblingElement();
    }

    if (0 == profileCount)
    {
        m_xml_files.emplace(filename, XMLP_ret::ERROR);
        logError(XMLPROFILEPARSER, "Bad file '" << filename << "' content expected tag: '" << tag << "'");
        return XMLP_ret::ERROR;

    }

    m_xml_files.emplace(filename, XMLP_ret::OK);


    for (auto &profile: m_participant_profiles)
    {
        printf("%s\n", profile.first.c_str());
    }
    for (auto &profile: m_publisher_profiles)
    {
        printf("%s\n", profile.first.c_str());
    }
    for (auto &profile: m_subscriber_profiles)
    {
        printf("%s\n", profile.first.c_str());
    }

    return XMLP_ret::OK;
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
        return XMLP_ret::ERROR;
    }
    const char *prof_name = p_profile->Attribute(PROFILE_NAME);
    if (nullptr == prof_name)
    {
        logError(XMLPROFILEPARSER, "Do not founded '" << PROFILE_NAME << "' attribute");
        return XMLP_ret::ERROR;
    }
    profile_name = prof_name;

    XMLElement *p_element = p_profile->FirstChildElement(RTPS);
    if (nullptr == p_element)
    {
        logError(XMLPROFILEPARSER, "Do not founded '" << RTPS << "' tag");
        return XMLP_ret::ERROR;
    }

    uint8_t ident = 1;
    XMLElement *p_aux = nullptr;
    // defaultUnicastLocatorList
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_UNI_LOC_LIST)))
    {
        if (XMLP_ret::OK != getXMLLocatorList(p_aux, participant_atts.rtps.defaultUnicastLocatorList, ident))
            return XMLP_ret::ERROR;
    }
    // defaultMulticastLocatorList
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_MULTI_LOC_LIST)))
    {
        if (XMLP_ret::OK != getXMLLocatorList(p_aux, participant_atts.rtps.defaultMulticastLocatorList, ident))
            return XMLP_ret::ERROR;
    }
    // defaultOutLocatorList
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_OUT_LOC_LIST)))
    {
        if (XMLP_ret::OK != getXMLLocatorList(p_aux, participant_atts.rtps.defaultOutLocatorList, ident))
            return XMLP_ret::ERROR;
    }
    // defaultSendPort - uint32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(DEF_SEND_PORT)))
    {
        if (XMLP_ret::OK != getXMLUint(p_aux, &participant_atts.rtps.defaultSendPort, ident))
            return XMLP_ret::ERROR;
    }
    // sendSocketBufferSize - uint32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(SEND_SOCK_BUF_SIZE)))
    {
        if (XMLP_ret::OK != getXMLUint(p_aux, &participant_atts.rtps.sendSocketBufferSize, ident))
            return XMLP_ret::ERROR;
    }
    // listenSocketBufferSize - uint32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(LIST_SOCK_BUF_SIZE)))
    {
        if (XMLP_ret::OK != getXMLUint(p_aux, &participant_atts.rtps.listenSocketBufferSize, ident))
            return XMLP_ret::ERROR;
    }
    // builtin
    if (nullptr != (p_aux = p_element->FirstChildElement(BUILTIN)))
    {
        if (XMLP_ret::OK != getXMLBuiltinAttributes(p_aux, participant_atts.rtps.builtin, ident))
            return XMLP_ret::ERROR;
    }
    // port
    if (nullptr != (p_aux = p_element->FirstChildElement(PORT)))
    {
        if (XMLP_ret::OK != getXMLPortParameters(p_aux, participant_atts.rtps.port, ident))
            return XMLP_ret::ERROR;
    }
    // TODO: userData
    if (nullptr != (p_aux = p_element->FirstChildElement(USER_DATA)))
    {
        if (XMLP_ret::OK != getXMLOctetVector(p_aux, participant_atts.rtps.userData, ident))
            return XMLP_ret::ERROR;
    }
    // participantID - int32Type
    if (nullptr != (p_aux = p_element->FirstChildElement(PART_ID)))
    {
        if (XMLP_ret::OK != getXMLInt(p_aux, &participant_atts.rtps.participantID, ident))
            return XMLP_ret::ERROR;
    }
    // use_IP4_to_send - boolType
    if (nullptr != (p_aux = p_element->FirstChildElement(IP4_TO_SEND)))
    {
        if (XMLP_ret::OK != getXMLBool(p_aux, &participant_atts.rtps.use_IP4_to_send, ident))
            return XMLP_ret::ERROR;
    }
    // use_IP6_to_send - boolType
    if (nullptr != (p_aux = p_element->FirstChildElement(IP6_TO_SEND)))
    {
        if (XMLP_ret::OK != getXMLBool(p_aux, &participant_atts.rtps.use_IP6_to_send, ident))
            return XMLP_ret::ERROR;
    }
    // throughputController
    if (nullptr != (p_aux = p_element->FirstChildElement(THROUGHPUT_CONT)))
    {
        if (XMLP_ret::OK != getXMLThroughputController(p_aux, participant_atts.rtps.throughputController, ident))
            return XMLP_ret::ERROR;
    }
    // TODO: userTransports
    if (nullptr != (p_aux = p_element->FirstChildElement(USER_TRANS)))
    {
        logError(XMLPROFILEPARSER, "Attribute '" << p_aux->Value() << "' do not supported for now");
    }

    // useBuiltinTransports - boolType
    if (nullptr != (p_aux = p_element->FirstChildElement(USE_BUILTIN_TRANS)))
    {
        if (XMLP_ret::OK != getXMLBool(p_aux, &participant_atts.rtps.useBuiltinTransports, ident))
            return XMLP_ret::ERROR;
    }
    // propertiesPolicy
    if (nullptr != (p_aux = p_element->FirstChildElement(PROPERTIES_POLICY)))
    {
        if (XMLP_ret::OK != getXMLPropertiesPolicy(p_aux, participant_atts.rtps.properties, ident))
            return XMLP_ret::ERROR;
    }
    // name - stringType
    if (nullptr != (p_aux = p_element->FirstChildElement(NAME)))
    {
        std::string s = "";
        if (XMLP_ret::OK != getXMLString(p_aux, &s, ident)) return XMLP_ret::ERROR;
        participant_atts.rtps.setName(s.c_str());
    }
    return XMLP_ret::OK;
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
        return XMLP_ret::ERROR;
    }
    const char *prof_name = p_profile->Attribute(PROFILE_NAME);
    if (nullptr == prof_name)
    {
        logError(XMLPROFILEPARSER, "Do not founded '" << PROFILE_NAME << "' attribute");
        return XMLP_ret::ERROR;
    }
    profile_name = prof_name;

    uint8_t ident = 1;
    XMLElement *p_aux = nullptr;
    // topic
    if (nullptr != (p_aux = p_profile->FirstChildElement(TOPIC)))
    {
        if (XMLP_ret::OK != getXMLTopicAttributes(p_aux, publisher_atts.topic, ident))
            return XMLP_ret::ERROR;
    }
    // qos
    if (nullptr != (p_aux = p_profile->FirstChildElement(QOS)))
    {
        if (XMLP_ret::OK != getXMLWriterQosPolicies(p_aux, publisher_atts.qos, ident))
            return XMLP_ret::ERROR;
    }
    // times
    if (nullptr != (p_aux = p_profile->FirstChildElement(TIMES)))
    {
        if (XMLP_ret::OK != getXMLWriterTimes(p_aux, publisher_atts.times, ident))
            return XMLP_ret::ERROR;
    }
    // unicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(UNI_LOC_LIST)))
    {
        if (XMLP_ret::OK != getXMLLocatorList(p_aux, publisher_atts.unicastLocatorList, ident))
            return XMLP_ret::ERROR;
    }
    // multicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(MULTI_LOC_LIST)))
    {
        if (XMLP_ret::OK != getXMLLocatorList(p_aux, publisher_atts.multicastLocatorList, ident))
            return XMLP_ret::ERROR;
    }
    // outLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(OUT_LOC_LIST)))
    {
        if (XMLP_ret::OK != getXMLLocatorList(p_aux, publisher_atts.outLocatorList, ident))
            return XMLP_ret::ERROR;
    }
    // throughputController
    if (nullptr != (p_aux = p_profile->FirstChildElement(THROUGHPUT_CONT)))
    {
        if (XMLP_ret::OK != getXMLThroughputController(p_aux, publisher_atts.throughputController, ident))
            return XMLP_ret::ERROR;
    }
    // historyMemoryPolicy
    if (nullptr != (p_aux = p_profile->FirstChildElement(HIST_MEM_POLICY)))
    {
        if (XMLP_ret::OK != getXMLHistoryMemoryPolicy(p_aux, publisher_atts.historyMemoryPolicy, ident))
            return XMLP_ret::ERROR;
    }
    // userDefinedID - int16type
    if (nullptr != (p_aux = p_profile->FirstChildElement(USER_DEF_ID)))
    {
        int i = 0;
        if (XMLP_ret::OK != getXMLInt(p_aux, &i, ident)) return XMLP_ret::ERROR;
        publisher_atts.setUserDefinedID(i);
    }
    // entityID - int16Type
    if (nullptr != (p_aux = p_profile->FirstChildElement(ENTITY_ID)))
    {
        int i = 0;
        if (XMLP_ret::OK != getXMLInt(p_aux, &i, ident)) return XMLP_ret::ERROR;
        publisher_atts.setEntityID(i);
    }
    return XMLP_ret::OK;
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
        return XMLP_ret::ERROR;
    }
    const char *prof_name = p_profile->Attribute(PROFILE_NAME);
    if (nullptr == prof_name)
    {
        logError(XMLPROFILEPARSER, "Do not founded '" << PROFILE_NAME << "' attribute");
        return XMLP_ret::ERROR;
    }
    profile_name = prof_name;

    uint8_t ident = 1;
    XMLElement *p_aux = nullptr;
    // topic
    if (nullptr != (p_aux = p_profile->FirstChildElement(TOPIC)))
    {
        if (XMLP_ret::OK != getXMLTopicAttributes(p_aux, subscriber_atts.topic, ident))
            return XMLP_ret::ERROR;
    }
    // qos
    if (nullptr != (p_aux = p_profile->FirstChildElement(QOS)))
    {
        if (XMLP_ret::OK != getXMLReaderQosPolicies(p_aux, subscriber_atts.qos, ident))
            return XMLP_ret::ERROR;
    }
    // times
    if (nullptr != (p_aux = p_profile->FirstChildElement(TIMES)))
    {
        if (XMLP_ret::OK != getXMLReaderTimes(p_aux, subscriber_atts.times, ident))
            return XMLP_ret::ERROR;
    }
    // unicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(UNI_LOC_LIST)))
    {
        if (XMLP_ret::OK != getXMLLocatorList(p_aux, subscriber_atts.unicastLocatorList, ident))
            return XMLP_ret::ERROR;
    }
    // multicastLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(MULTI_LOC_LIST)))
    {
        if (XMLP_ret::OK != getXMLLocatorList(p_aux, subscriber_atts.multicastLocatorList, ident))
            return XMLP_ret::ERROR;
    }
    // outLocatorList
    if (nullptr != (p_aux = p_profile->FirstChildElement(OUT_LOC_LIST)))
    {
        if (XMLP_ret::OK != getXMLLocatorList(p_aux, subscriber_atts.outLocatorList, ident))
            return XMLP_ret::ERROR;
    }
    // expectsInlineQos - boolType
    if (nullptr != (p_aux = p_profile->FirstChildElement(EXP_INLINE_QOS)))
    {
        bool b = false;
        if (XMLP_ret::OK != getXMLBool(p_aux, &subscriber_atts.expectsInlineQos, ident))
            return XMLP_ret::ERROR;
    }
    // historyMemoryPolicy
    if (nullptr != (p_aux = p_profile->FirstChildElement(HIST_MEM_POLICY)))
    {
        if (XMLP_ret::OK != getXMLHistoryMemoryPolicy(p_aux, subscriber_atts.historyMemoryPolicy, ident))
            return XMLP_ret::ERROR;
    }
    // propertiesPolicy
    if (nullptr != (p_aux = p_profile->FirstChildElement(PROPERTIES_POLICY)))
    {
        if (XMLP_ret::OK != getXMLPropertiesPolicy(p_aux, subscriber_atts.properties, ident))
            return XMLP_ret::ERROR;
    }
    // userDefinedID - int16Type
    if (nullptr != (p_aux = p_profile->FirstChildElement(USER_DEF_ID)))
    {
        int i = 0;
        if (XMLP_ret::OK != getXMLInt(p_aux, &i, ident)) return XMLP_ret::ERROR;
        subscriber_atts.setUserDefinedID(i);
    }
    // entityID - int16Type
    if (nullptr != (p_aux = p_profile->FirstChildElement(ENTITY_ID)))
    {
        int i = 0;
        if (XMLP_ret::OK != getXMLInt(p_aux, &i, ident)) return XMLP_ret::ERROR;
        subscriber_atts.setEntityID(i);
    }
    return XMLP_ret::OK;
}


} /* namespace  */
} /* namespace eprosima */
