#ifndef XML_PROFILE_PARSER_H_
#define XML_PROFILE_PARSER_H_

#include "stdio.h"
#include <string>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/xmlparser/XMLProfileParserCommon.h>


namespace tinyxml2
{
    class XMLElement;
}

using namespace tinyxml2;
using namespace eprosima::fastrtps::rtps;

namespace eprosima{
namespace fastrtps{
namespace xmlparser{


enum class XMLP_ret
{
    ERROR,
    OK,
    NOK,
};


typedef std::map<std::string, ParticipantAttributes> participant_map_t;
typedef std::map<std::string, PublisherAttributes> publisher_map_t;
typedef std::map<std::string, SubscriberAttributes> subscriber_map_t;
typedef std::map<std::string, XMLP_ret> xmlfiles_map_t;
typedef std::map<std::string, ParticipantAttributes>::iterator part_map_iterator_t;
typedef std::map<std::string, PublisherAttributes>::iterator publ_map_iterator_t;
typedef std::map<std::string, SubscriberAttributes>::iterator subs_map_iterator_t;
typedef std::map<std::string, XMLP_ret>::iterator xmlfile_map_iterator_t;


class XMLProfileParser
{

public:

    RTPS_DllAPI static XMLP_ret loadDefaultXMLFile();
    RTPS_DllAPI static XMLP_ret loadXMLFile(const std::string filename);
    RTPS_DllAPI static XMLP_ret fillParticipantProfileFromXMLFile(const std::string filename,
                                                                  const std::string profile_name,
                                                                  ParticipantAttributes &atts);
    RTPS_DllAPI static XMLP_ret fillPublisherProfileFromXMLFile(const std::string filename,
                                                                const std::string profile_name,
                                                                PublisherAttributes &atts);
    RTPS_DllAPI static XMLP_ret fillSubscriberProfileFromXMLFile(const std::string filename,
                                                                 const std::string profile_name,
                                                                 SubscriberAttributes &atts);
    RTPS_DllAPI static XMLP_ret fillParticipantProfile(const std::string profile_name, ParticipantAttributes &atts);
    RTPS_DllAPI static XMLP_ret fillPublisherProfile(const std::string profile_name, PublisherAttributes &atts);
    RTPS_DllAPI static XMLP_ret fillSubscriberProfile(const std::string profile_name, SubscriberAttributes &atts);

protected:

    RTPS_DllAPI static XMLP_ret parseXMLProfile(XMLElement *p_profile);
    RTPS_DllAPI static XMLP_ret parseXMLParticipantProf(XMLElement *p_profile, ParticipantAttributes &participant_atts, std::string &profile_name);
    RTPS_DllAPI static XMLP_ret parseXMLPublisherProf(XMLElement *p_profile, PublisherAttributes &publisher_atts, std::string &profile_name);
    RTPS_DllAPI static XMLP_ret parseXMLSubscriberProf(XMLElement *p_profile, SubscriberAttributes &subscriber_atts, std::string &profile_name);
    RTPS_DllAPI static XMLP_ret getXMLPropertiesPolicy(XMLElement *elem, PropertyPolicy &propertiesPolicy, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLHistoryMemoryPolicy(XMLElement *elem, MemoryManagementPolicy_t &historyMemoryPolicy, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLLocatorList(XMLElement *elem, LocatorList_t &locatorList, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLWriterTimes(XMLElement *elem, WriterTimes &times, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLReaderTimes(XMLElement *elem, ReaderTimes &times, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLDuration(XMLElement *elem, Duration_t &duration, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLWriterQosPolicies(XMLElement *elem, WriterQos &qos, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLReaderQosPolicies(XMLElement *elem, ReaderQos &qos, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLPublishModeQos(XMLElement *elem, PublishModeQosPolicy &publishMode, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLGroupDataQos(XMLElement *elem, GroupDataQosPolicy &groupData, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLTopicDataQos(XMLElement *elem, TopicDataQosPolicy &topicData, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLPartitionQos(XMLElement *elem, PartitionQosPolicy &partition, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLPresentationQos(XMLElement *elem, PresentationQosPolicy &presentation, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLDestinationOrderQos(XMLElement *elem, DestinationOrderQosPolicy &destinationOrder, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLOwnershipStrengthQos(XMLElement *elem, OwnershipStrengthQosPolicy &ownershipStrength, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLOwnershipQos(XMLElement *elem, OwnershipQosPolicy &ownership, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLTimeBasedFilterQos(XMLElement *elem, TimeBasedFilterQosPolicy &timeBasedFilter, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLUserDataQos(XMLElement *elem, UserDataQosPolicy &userData, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLLifespanQos(XMLElement *elem, LifespanQosPolicy &lifespan, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLReliabilityQos(XMLElement *elem, ReliabilityQosPolicy &reliability, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLLivelinessQos(XMLElement *elem, LivelinessQosPolicy &liveliness, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLLatencyBudgetQos(XMLElement *elem, LatencyBudgetQosPolicy &latencyBudget, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLDeadlineQos(XMLElement *elem, DeadlineQosPolicy &deadline, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLDurabilityServiceQos(XMLElement *elem, DurabilityServiceQosPolicy &durabilityService, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLDurabilityQos(XMLElement *elem, DurabilityQosPolicy &durability, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLTopicAttributes(XMLElement *elem, TopicAttributes &topic, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLHistoryQosPolicy(XMLElement *elem, HistoryQosPolicy &historyQos, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLResourceLimitsQos(XMLElement *elem, ResourceLimitsQosPolicy &resourceLimitsQos, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLThroughputController(XMLElement *elem, ThroughputControllerDescriptor &throughputController, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLPortParameters(XMLElement *elem, PortParameters &port, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLBuiltinAttributes(XMLElement *elem, BuiltinAttributes &builtin, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLOctetVector(XMLElement *elem, std::vector<octet> &octetVector, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLInt(XMLElement *elem, int *i, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLUint(XMLElement *elem, unsigned int *ui, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLUint(XMLElement *elem, uint16_t *ui16, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLBool(XMLElement *elem, bool *b, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLString(XMLElement *elem, std::string *s, uint8_t ident);

private:

    static participant_map_t m_participant_profiles;
    static publisher_map_t   m_publisher_profiles;
    static subscriber_map_t  m_subscriber_profiles;
    static xmlfiles_map_t    m_xml_files;
};

} /* xmlparser */
} /* namespace */
} /* namespace eprosima */

#endif
