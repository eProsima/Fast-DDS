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

#include <stdio.h>
#include <fastrtps/transport/TransportDescriptorInterface.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/attributes/RequesterAttributes.hpp>
#include <fastrtps/attributes/ReplierAttributes.hpp>
#include <fastrtps/xmlparser/XMLParserCommon.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>
#include <fastrtps/attributes/LibrarySettingsAttributes.h>

#include <map>
#include <string>

namespace tinyxml2 {
class XMLElement;
class XMLDocument;
} // namespace tinyxml2

namespace eprosima {
namespace fastrtps {
namespace xmlparser {

class BaseNode;
template <class T> class DataNode;

typedef std::unique_ptr<BaseNode>              up_base_node_t;
typedef std::vector<up_base_node_t>            up_base_node_vector_t;
typedef std::map<std::string, std::string>     node_att_map_t;
typedef node_att_map_t::iterator node_att_map_it_t;
typedef node_att_map_t::const_iterator node_att_map_cit_t;

typedef std::shared_ptr<fastdds::rtps::TransportDescriptorInterface> sp_transport_t;
typedef std::map<std::string, sp_transport_t>  sp_transport_map_t;
typedef types::DynamicTypeBuilder*             p_dynamictypebuilder_t;
typedef std::map<std::string, p_dynamictypebuilder_t> p_dynamictype_map_t;

typedef std::unique_ptr<ParticipantAttributes> up_participant_t;
typedef DataNode<ParticipantAttributes>        node_participant_t;
typedef node_participant_t*                    p_node_participant_t;
typedef std::unique_ptr<node_participant_t>    up_node_participant_t;

typedef std::unique_ptr<PublisherAttributes>   up_publisher_t;
typedef DataNode<PublisherAttributes>          node_publisher_t;
typedef node_publisher_t*                      p_node_publisher_t;
typedef std::unique_ptr<node_publisher_t>      up_node_publisher_t;

typedef std::unique_ptr<SubscriberAttributes>  up_subscriber_t;
typedef DataNode<SubscriberAttributes>         node_subscriber_t;
typedef node_subscriber_t*                     p_node_subscriber_t;
typedef std::unique_ptr<node_subscriber_t>     up_node_subscriber_t;

typedef std::unique_ptr<TopicAttributes>       up_topic_t;
typedef DataNode<TopicAttributes>              node_topic_t;
typedef node_topic_t*                          p_node_topic_t;
typedef std::unique_ptr<node_topic_t>          up_node_topic_t;

typedef std::unique_ptr<RequesterAttributes>   up_requester_t;
typedef DataNode<RequesterAttributes>          node_requester_t;
typedef node_requester_t*                      p_node_requester_t;
typedef std::unique_ptr<node_requester_t>      up_node_requester_t;

typedef std::unique_ptr<ReplierAttributes>     up_replier_t;
typedef DataNode<ReplierAttributes>            node_replier_t;
typedef node_replier_t*                        p_node_replier_t;
typedef std::unique_ptr<node_replier_t>        up_node_replier_t;

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
    RTPS_DllAPI static XMLP_ret loadDefaultXMLFile(
            up_base_node_t& root);

    /**
     * Load a XML file.
     * @param filename Name for the file to be loaded.
     * @param root Root node.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadXML(
            const std::string& filename,
            up_base_node_t& root);

    /**
     * Load a XML data from buffer.
     * @param data XML data to load.
     * @param length Length of the XML data.
     * @param root Root node.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadXML(
            const char* data,
            size_t length,
            up_base_node_t& root);

    /**
     * Load a XML node.
     * @param xmlDoc Node to be loaded.
     * @param root Root node.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadXML(
            tinyxml2::XMLDocument& xmlDoc,
            up_base_node_t& root);

    /**
     * Load a XML node.
     * @param profiles Node to be loaded.
     * @param root Root node.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadXMLProfiles(
            tinyxml2::XMLElement& profiles,
            up_base_node_t& root);

    /**
     * Load a XML node.
     * @param types Node to be loaded.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadXMLDynamicTypes(
            tinyxml2::XMLElement& types);

protected:

    RTPS_DllAPI static XMLP_ret parseXML(
            tinyxml2::XMLDocument& xmlDoc,
            up_base_node_t& root);

    RTPS_DllAPI static XMLP_ret parseXMLProfiles(
            tinyxml2::XMLElement& profiles,
            up_base_node_t& root);

    RTPS_DllAPI static XMLP_ret parseProfiles(
            tinyxml2::XMLElement* p_root,
            BaseNode& profilesNode);

    RTPS_DllAPI static XMLP_ret parseRoot(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode);


    /**
     * Load a XML log node and parses it. It applies the configuration of the node directly.
     * @param p_root Node to be loaded.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret parseLogConfig(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLLibrarySettings(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLTransportsProf(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLParticipantProf(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode);

    RTPS_DllAPI static XMLP_ret parseXMLPublisherProf(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode);

    RTPS_DllAPI static XMLP_ret parseXMLSubscriberProf(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode);

    RTPS_DllAPI static XMLP_ret parseXMLTopicData(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode);

    RTPS_DllAPI static XMLP_ret parseXMLRequesterProf(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode);

    RTPS_DllAPI static XMLP_ret parseXMLReplierProf(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode);

    RTPS_DllAPI static XMLP_ret parseXMLTransportData(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLCommonTransportData(
            tinyxml2::XMLElement* p_root,
            sp_transport_t p_transport);

    RTPS_DllAPI static XMLP_ret parseXMLCommonTCPTransportData(
            tinyxml2::XMLElement* p_root,
            sp_transport_t p_transport);

    RTPS_DllAPI static XMLP_ret parseXMLCommonSharedMemTransportData(
            tinyxml2::XMLElement* p_root,
            sp_transport_t p_transport);

    RTPS_DllAPI static XMLP_ret parse_tls_config(
            tinyxml2::XMLElement* p_root,
            sp_transport_t tcp_transport);

    /**
     * Load a XML consumer node and parses it. Adds the parsed consumer to Log directly.
     * @param consumer Node to be loaded.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret parseXMLConsumer(
            tinyxml2::XMLElement& consumer);

    RTPS_DllAPI static XMLP_ret parseXMLDynamicTypes(
            tinyxml2::XMLElement& types);

    RTPS_DllAPI static XMLP_ret parseDynamicTypes(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLTypes(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLDynamicType(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLStructDynamicType(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLUnionDynamicType(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLEnumDynamicType(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLAliasDynamicType(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLBitsetDynamicType(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static XMLP_ret parseXMLBitmaskDynamicType(
            tinyxml2::XMLElement* p_root);

    RTPS_DllAPI static p_dynamictypebuilder_t parseXMLBitfieldDynamicType(
            tinyxml2::XMLElement* p_root,
            p_dynamictypebuilder_t p_dynamictype,
            types::MemberId mId,
            uint16_t& position);

    RTPS_DllAPI static XMLP_ret parseXMLBitvalueDynamicType(
            tinyxml2::XMLElement* p_root,
            p_dynamictypebuilder_t p_dynamictype,
            uint16_t& position);

    RTPS_DllAPI static p_dynamictypebuilder_t parseXMLMemberDynamicType(
            tinyxml2::XMLElement* p_root,
            p_dynamictypebuilder_t p_dynamictype,
            types::MemberId mId);

    RTPS_DllAPI static p_dynamictypebuilder_t parseXMLMemberDynamicType(
            tinyxml2::XMLElement* p_root,
            p_dynamictypebuilder_t p_dynamictype,
            types::MemberId mId,
            const std::string& values);

    RTPS_DllAPI static XMLP_ret fillDataNode(
            tinyxml2::XMLElement* p_profile,
            DataNode<ParticipantAttributes>& participant_node);

    RTPS_DllAPI static XMLP_ret fillDataNode(
            tinyxml2::XMLElement* p_profile,
            DataNode<PublisherAttributes>& publisher_node);

    RTPS_DllAPI static XMLP_ret fillDataNode(
            tinyxml2::XMLElement* p_profile,
            DataNode<SubscriberAttributes>& subscriber_node);

    RTPS_DllAPI static XMLP_ret fillDataNode(
            tinyxml2::XMLElement* node,
            DataNode<TopicAttributes>& topic_node);

    RTPS_DllAPI static XMLP_ret fillDataNode(
            tinyxml2::XMLElement* node,
            DataNode<RequesterAttributes>& requester_node);

    RTPS_DllAPI static XMLP_ret fillDataNode(
            tinyxml2::XMLElement* node,
            DataNode<ReplierAttributes>& replier_node);

    template <typename T>
    RTPS_DllAPI static void addAllAttributes(
            tinyxml2::XMLElement* p_profile,
            DataNode<T>& node);

    RTPS_DllAPI static XMLP_ret getXMLEnum(
            tinyxml2::XMLElement* elem,
            fastrtps::IntraprocessDeliveryType* e,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLPropertiesPolicy(
            tinyxml2::XMLElement* elem,
            rtps::PropertyPolicy& propertiesPolicy,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLHistoryMemoryPolicy(
            tinyxml2::XMLElement* elem,
            rtps::MemoryManagementPolicy_t& historyMemoryPolicy,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLLocatorList(
            tinyxml2::XMLElement* elem,
            rtps::LocatorList_t& locatorList,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLLocatorUDPv4(
            tinyxml2::XMLElement* elem,
            rtps::Locator_t& locator,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLLocatorUDPv6(
            tinyxml2::XMLElement* elem,
            rtps::Locator_t& locator,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLLocatorTCPv4(
            tinyxml2::XMLElement* elem,
            rtps::Locator_t& locator,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLLocatorTCPv6(
            tinyxml2::XMLElement* elem,
            rtps::Locator_t& locator,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLWriterTimes(
            tinyxml2::XMLElement* elem,
            rtps::WriterTimes& times,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLReaderTimes(
            tinyxml2::XMLElement* elem,
            rtps::ReaderTimes& times,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLDuration(
            tinyxml2::XMLElement* elem,
            Duration_t& duration,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLWriterQosPolicies(
            tinyxml2::XMLElement* elem,
            WriterQos& qos,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLReaderQosPolicies(
            tinyxml2::XMLElement* elem,
            ReaderQos& qos,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLPublishModeQos(
            tinyxml2::XMLElement* elem,
            PublishModeQosPolicy& publishMode,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLGroupDataQos(
            tinyxml2::XMLElement* elem,
            GroupDataQosPolicy& groupData,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLTopicDataQos(
            tinyxml2::XMLElement* elem,
            TopicDataQosPolicy& topicData,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLPartitionQos(
            tinyxml2::XMLElement* elem,
            PartitionQosPolicy& partition,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLPresentationQos(
            tinyxml2::XMLElement* elem,
            PresentationQosPolicy& presentation,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLDestinationOrderQos(
            tinyxml2::XMLElement* elem,
            DestinationOrderQosPolicy& destinationOrder,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLOwnershipStrengthQos(
            tinyxml2::XMLElement* elem,
            OwnershipStrengthQosPolicy& ownershipStrength,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLOwnershipQos(
            tinyxml2::XMLElement* elem,
            OwnershipQosPolicy& ownership,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLTimeBasedFilterQos(
            tinyxml2::XMLElement* elem,
            TimeBasedFilterQosPolicy& timeBasedFilter,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLUserDataQos(
            tinyxml2::XMLElement* elem,
            UserDataQosPolicy& userData,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLLifespanQos(
            tinyxml2::XMLElement* elem,
            LifespanQosPolicy& lifespan,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLReliabilityQos(
            tinyxml2::XMLElement* elem,
            ReliabilityQosPolicy& reliability,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLLivelinessQos(
            tinyxml2::XMLElement* elem,
            LivelinessQosPolicy& liveliness,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLLatencyBudgetQos(
            tinyxml2::XMLElement* elem,
            LatencyBudgetQosPolicy& latencyBudget,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLDeadlineQos(
            tinyxml2::XMLElement* elem,
            DeadlineQosPolicy& deadline,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLDurabilityServiceQos(
            tinyxml2::XMLElement* elem,
            DurabilityServiceQosPolicy& durabilityService,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLDurabilityQos(
            tinyxml2::XMLElement* elem,
            DurabilityQosPolicy& durability,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLTopicAttributes(
            tinyxml2::XMLElement* elem,
            TopicAttributes& topic,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLHistoryQosPolicy(
            tinyxml2::XMLElement* elem,
            HistoryQosPolicy& historyQos,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLResourceLimitsQos(
            tinyxml2::XMLElement* elem,
            ResourceLimitsQosPolicy& resourceLimitsQos,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLContainerAllocationConfig(
            tinyxml2::XMLElement* elem,
            ResourceLimitedContainerConfig& resourceLimitsQos,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLThroughputController(
            tinyxml2::XMLElement* elem,
            rtps::ThroughputControllerDescriptor& throughputController,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLPortParameters(
            tinyxml2::XMLElement* elem,
            rtps::PortParameters& port,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLParticipantAllocationAttributes(
            tinyxml2::XMLElement* elem,
            rtps::RTPSParticipantAllocationAttributes& allocation,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLRemoteLocatorsAllocationAttributes(
            tinyxml2::XMLElement* elem,
            rtps::RemoteLocatorsAllocationAttributes& allocation,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLSendBuffersAllocationAttributes(
            tinyxml2::XMLElement* elem,
            rtps::SendBuffersAllocationAttributes& allocation,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLDiscoverySettings(
            tinyxml2::XMLElement* elem,
            rtps::DiscoverySettings& settings,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLInitialAnnouncementsConfig(
            tinyxml2::XMLElement* elem,
            rtps::InitialAnnouncementConfig& config,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLBuiltinAttributes(
            tinyxml2::XMLElement* elem,
            rtps::BuiltinAttributes& builtin,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLOctetVector(
            tinyxml2::XMLElement* elem,
            std::vector<rtps::octet>& octetVector,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLInt(
            tinyxml2::XMLElement* elem,
            int* i,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLUint(
            tinyxml2::XMLElement* elem,
            unsigned int* ui,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLUint(
            tinyxml2::XMLElement* elem,
            uint16_t* ui16,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLBool(
            tinyxml2::XMLElement* elem,
            bool* b,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLEnum(
            tinyxml2::XMLElement* elem,
            rtps::DiscoveryProtocol_t* e,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLList(
            tinyxml2::XMLElement* elem,
            eprosima::fastdds::rtps::RemoteServerList_t& list,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLEnum(
            tinyxml2::XMLElement* elem,
            rtps::ParticipantFilteringFlags_t* e,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLRemoteServer(
            tinyxml2::XMLElement* elem,
            eprosima::fastdds::rtps::RemoteServerAttributes& server,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLString(
            tinyxml2::XMLElement* elem,
            std::string* s,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLTransports(
            tinyxml2::XMLElement* elem,
            std::vector<std::shared_ptr<fastdds::rtps::TransportDescriptorInterface>>& transports,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLDisablePositiveAcksQos(
            tinyxml2::XMLElement* elem,
            DisablePositiveACKsQosPolicy& disablePositiveAcks,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLguidPrefix(
            tinyxml2::XMLElement* elem,
            rtps::GuidPrefix_t& prefix,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLPublisherAttributes(
            tinyxml2::XMLElement* elem,
            PublisherAttributes& publisher,
            uint8_t ident);

    RTPS_DllAPI static XMLP_ret getXMLSubscriberAttributes(
            tinyxml2::XMLElement* elem,
            SubscriberAttributes& subscriber,
            uint8_t ident);
};

} // namespace xmlparser
} // namespace fastrtps
} // namespace eprosima

#endif // ifndef XML_PARSER_H_
