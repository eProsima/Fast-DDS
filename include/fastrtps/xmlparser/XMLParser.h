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
#ifndef XML_PARSER_H_
#define XML_PARSER_H_

#include "stdio.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/xmlparser/XMLParserCommon.h>

#include <map>
#include <string>

namespace tinyxml2 {
class XMLElement;
class XMLDocument;
} // namespace tinyxml2

using namespace tinyxml2;
using namespace eprosima::fastrtps::rtps;

namespace eprosima{
namespace fastrtps{
namespace xmlparser{

class BaseNode;
template <class T> class DataNode;

typedef std::unique_ptr<BaseNode> base_node_uptr_t;
typedef std::vector<base_node_uptr_t> base_node_uvtr_t;
typedef std::unique_ptr<ParticipantAttributes> up_participant_att_t;
typedef std::unique_ptr<PublisherAttributes>   up_publisher_att_t;
typedef std::unique_ptr<SubscriberAttributes>  up_subscriber_att_t;
typedef std::map<std::string, std::string>     att_map_t;
typedef att_map_t::iterator                    att_map_it_t;
typedef att_map_t::const_iterator              att_map_cit_t;
typedef DataNode<ParticipantAttributes>*           p_node_participant_t;

/**
 * Class XMLParser, used to load XML data.
 * @ingroup XMLPARSER_MODULE
 */
class XMLParser
{

  public:
    /**
     * Load the default XML file.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadDefaultXMLFile();

    /**
     * Load a XML file.
     * @param filename Name for the file to be loaded.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadXMLFile(const std::string& filename);

    /**
     * Load a XML data from buffer.
     * @param data XML data to load.
     * @param length Length of the XML data.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadXML(const char* data, size_t length);

  protected:
    RTPS_DllAPI static XMLP_ret parseXML(XMLDocument& xmlDoc);
    RTPS_DllAPI static XMLP_ret parseProfiles(XMLElement* p_root, BaseNode& profilesNode);
    RTPS_DllAPI static XMLP_ret parseRoot(XMLElement* p_root, BaseNode& rootNode);

    RTPS_DllAPI static XMLP_ret parseXMLParticipantProf(XMLElement* p_root, BaseNode& rootNode);
    RTPS_DllAPI static XMLP_ret parseXMLPublisherProf(XMLElement* p_root, BaseNode& rootNode);
    RTPS_DllAPI static XMLP_ret parseXMLSubscriberProf(XMLElement* p_root, BaseNode& rootNode);

    RTPS_DllAPI static XMLP_ret fillDataNode(XMLElement* p_profile, DataNode<ParticipantAttributes>& participant_node);
    RTPS_DllAPI static XMLP_ret fillDataNode(XMLElement* p_profile, DataNode<PublisherAttributes>& publisher_node);
    RTPS_DllAPI static XMLP_ret fillDataNode(XMLElement* p_profile, DataNode<SubscriberAttributes>& subscriber_node);

    template <typename T>
    RTPS_DllAPI static void addAllAttributes(XMLElement* p_profile, DataNode<T>& node);

    RTPS_DllAPI static XMLP_ret getXMLPropertiesPolicy(XMLElement* elem, PropertyPolicy& propertiesPolicy,
                                                       uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLHistoryMemoryPolicy(XMLElement* elem,
                                                          MemoryManagementPolicy_t& historyMemoryPolicy, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLLocatorList(XMLElement* elem, LocatorList_t& locatorList, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLWriterTimes(XMLElement* elem, WriterTimes& times, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLReaderTimes(XMLElement* elem, ReaderTimes& times, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLDuration(XMLElement* elem, Duration_t& duration, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLWriterQosPolicies(XMLElement* elem, WriterQos& qos, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLReaderQosPolicies(XMLElement* elem, ReaderQos& qos, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLPublishModeQos(XMLElement* elem, PublishModeQosPolicy& publishMode,
                                                     uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLGroupDataQos(XMLElement* elem, GroupDataQosPolicy& groupData, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLTopicDataQos(XMLElement* elem, TopicDataQosPolicy& topicData, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLPartitionQos(XMLElement* elem, PartitionQosPolicy& partition, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLPresentationQos(XMLElement* elem, PresentationQosPolicy& presentation,
                                                      uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLDestinationOrderQos(XMLElement* elem, DestinationOrderQosPolicy& destinationOrder,
                                                          uint8_t ident);
    RTPS_DllAPI static XMLP_ret
    getXMLOwnershipStrengthQos(XMLElement* elem, OwnershipStrengthQosPolicy& ownershipStrength, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLOwnershipQos(XMLElement* elem, OwnershipQosPolicy& ownership, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLTimeBasedFilterQos(XMLElement* elem, TimeBasedFilterQosPolicy& timeBasedFilter,
                                                         uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLUserDataQos(XMLElement* elem, UserDataQosPolicy& userData, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLLifespanQos(XMLElement* elem, LifespanQosPolicy& lifespan, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLReliabilityQos(XMLElement* elem, ReliabilityQosPolicy& reliability,
                                                     uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLLivelinessQos(XMLElement* elem, LivelinessQosPolicy& liveliness, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLLatencyBudgetQos(XMLElement* elem, LatencyBudgetQosPolicy& latencyBudget,
                                                       uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLDeadlineQos(XMLElement* elem, DeadlineQosPolicy& deadline, uint8_t ident);
    RTPS_DllAPI static XMLP_ret
    getXMLDurabilityServiceQos(XMLElement* elem, DurabilityServiceQosPolicy& durabilityService, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLDurabilityQos(XMLElement* elem, DurabilityQosPolicy& durability, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLTopicAttributes(XMLElement* elem, TopicAttributes& topic, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLHistoryQosPolicy(XMLElement* elem, HistoryQosPolicy& historyQos, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLResourceLimitsQos(XMLElement* elem, ResourceLimitsQosPolicy& resourceLimitsQos,
                                                        uint8_t ident);
    RTPS_DllAPI static XMLP_ret
    getXMLThroughputController(XMLElement* elem, ThroughputControllerDescriptor& throughputController, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLPortParameters(XMLElement* elem, PortParameters& port, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLBuiltinAttributes(XMLElement* elem, BuiltinAttributes& builtin, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLOctetVector(XMLElement* elem, std::vector<octet>& octetVector, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLInt(XMLElement* elem, int* i, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLUint(XMLElement* elem, unsigned int* ui, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLUint(XMLElement* elem, uint16_t* ui16, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLBool(XMLElement* elem, bool* b, uint8_t ident);
    RTPS_DllAPI static XMLP_ret getXMLString(XMLElement* elem, std::string* s, uint8_t ident);

  private:
    static BaseNode* root;
    static participant_map_t m_participant_profiles;
    static publisher_map_t m_publisher_profiles;
    static subscriber_map_t m_subscriber_profiles;
    static xmlfiles_map_t m_xml_files;
};

} // namespace xmlparser
} // namespace fastrtps
} // namespace eprosima

#endif
