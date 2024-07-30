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

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/attributes/ThreadSettings.hpp>
#include <fastdds/rtps/transport/PortBasedTransportDescriptor.hpp>

#include <xmlparser/XMLParser.h>

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

using eprosima::fastdds::xmlparser::XMLP_ret;
using eprosima::fastdds::xmlparser::XMLParser;
using eprosima::fastdds::xmlparser::DataNode;
using eprosima::fastdds::xmlparser::BaseNode;
using eprosima::fastdds::xmlparser::sp_transport_t;
using eprosima::fastdds::xmlparser::up_participant_t;
using eprosima::fastdds::xmlparser::up_node_participant_t;
using eprosima::fastdds::xmlparser::node_participant_t;

// Class to test protected methods
class XMLParserTest : public XMLParser
{
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
            DataNode<xmlparser::ParticipantAttributes>& participant_node)
    {
        return fillDataNode(p_profile, participant_node);
    }

    static XMLP_ret getXMLDiscoverySettings_wrapper(
            tinyxml2::XMLElement* elem,
            DiscoverySettings& settings,
            uint8_t ident)
    {
        return getXMLDiscoverySettings(elem, settings, ident);
    }

    static XMLP_ret getXMLPortParameters_wrapper(
            tinyxml2::XMLElement* elem,
            PortParameters& port,
            uint8_t ident)
    {
        return getXMLPortParameters(elem, port, ident);
    }

    static XMLP_ret getXMLSubscriberAttributes_wrapper(
            tinyxml2::XMLElement* elem,
            xmlparser::SubscriberAttributes& subscriber,
            uint8_t ident)
    {
        return getXMLSubscriberAttributes(elem, subscriber, ident);
    }

    static XMLP_ret getXMLLifespanQos_wrapper(
            tinyxml2::XMLElement* elem,
            LifespanQosPolicy& lifespan,
            uint8_t ident)
    {
        return getXMLLifespanQos(elem, lifespan, ident);
    }

    static XMLP_ret getXMLDisablePositiveAcksQos_wrapper(
            tinyxml2::XMLElement* elem,
            DisablePositiveACKsQosPolicy& disablePositiveAcks,
            uint8_t ident)
    {
        return getXMLDisablePositiveAcksQos(elem, disablePositiveAcks, ident);
    }

    static XMLP_ret propertiesPolicy_wrapper(
            tinyxml2::XMLElement* elem,
            DataSharingQosPolicy& datasharingQos,
            uint8_t ident)
    {
        return getXMLDataSharingQos(elem, datasharingQos, ident);
    }

    static XMLP_ret propertiesPolicy_wrapper(
            tinyxml2::XMLElement* elem,
            OwnershipQosPolicy& ownershipQos,
            uint8_t ident)
    {
        return getXMLOwnershipQos(elem, ownershipQos, ident);
    }

    static XMLP_ret propertiesPolicy_wrapper(
            tinyxml2::XMLElement* elem,
            OwnershipStrengthQosPolicy& ownershipStrengthQos,
            uint8_t ident)
    {
        return getXMLOwnershipStrengthQos(elem, ownershipStrengthQos, ident);
    }

    static XMLP_ret propertiesPolicy_wrapper(
            tinyxml2::XMLElement* elem,
            PropertyPolicy& propertiesPolicy,
            uint8_t ident)
    {
        return getXMLPropertiesPolicy(elem, propertiesPolicy, ident);
    }

    static XMLP_ret getXMLTransports_wrapper(
            tinyxml2::XMLElement* elem,
            std::vector<std::shared_ptr<eprosima::fastdds::rtps::TransportDescriptorInterface>>& transports,
            uint8_t ident)
    {
        return getXMLTransports(elem, transports, ident);
    }

    static XMLP_ret getXMLBuiltinTransports_wrapper(
            tinyxml2::XMLElement* elem,
            eprosima::fastdds::rtps::BuiltinTransports* bt,
            uint8_t ident,
            eprosima::fastdds::rtps::BuiltinTransportsOptions* bt_opts = nullptr)
    {
        return getXMLBuiltinTransports(elem, bt, bt_opts, ident);
    }

    static XMLP_ret getXMLguidPrefix_wrapper(
            tinyxml2::XMLElement* elem,
            GuidPrefix_t& prefix,
            uint8_t ident)
    {
        return getXMLguidPrefix(elem, prefix, ident);
    }

    static XMLP_ret getXMLDuration_wrapper(
            tinyxml2::XMLElement* elem,
            dds::Duration_t& duration,
            uint8_t ident)
    {
        return getXMLDuration(elem, duration, ident);
    }

    static XMLP_ret getXMLString_wrapper(
            tinyxml2::XMLElement* elem,
            std::string* s,
            uint8_t ident)
    {
        return getXMLString(elem, s, ident);
    }

    static XMLP_ret getXMLBool_wrapper(
            tinyxml2::XMLElement* elem,
            bool* b,
            uint8_t ident)
    {
        return getXMLBool(elem, b, ident);
    }

    static XMLP_ret getXMLInt_wrapper(
            tinyxml2::XMLElement* elem,
            int* i,
            uint8_t ident)
    {
        return getXMLInt(elem, i, ident);
    }

    static XMLP_ret getXMLUint_wrapper(
            tinyxml2::XMLElement* elem,
            unsigned int* ui,
            uint8_t ident)
    {
        return getXMLUint(elem, ui, ident);
    }

    static XMLP_ret getXMLUint_wrapper(
            tinyxml2::XMLElement* elem,
            uint16_t* ui16,
            uint8_t ident)
    {
        return getXMLUint(elem, ui16, ident);
    }

    static XMLP_ret getXMLUint_wrapper(
            tinyxml2::XMLElement* elem,
            uint64_t* ui64,
            uint8_t ident)
    {
        return getXMLUint(elem, ui64, ident);
    }

    static XMLP_ret getXMLBuiltinAttributes_wrapper(
            tinyxml2::XMLElement* elem,
            BuiltinAttributes& builtin,
            uint8_t ident)
    {
        return getXMLBuiltinAttributes(elem, builtin, ident);
    }

    static XMLP_ret getXMLFlowControllerDescriptorList_wrapper(
            tinyxml2::XMLElement* elem,
            FlowControllerDescriptorList& flow_controller_descriptors,
            uint8_t ident)
    {
        return getXMLFlowControllerDescriptorList(elem, flow_controller_descriptors, ident);
    }

    static XMLP_ret getXMLTopicAttributes_wrapper(
            tinyxml2::XMLElement* elem,
            xmlparser::TopicAttributes& topic,
            uint8_t ident)
    {
        return getXMLTopicAttributes(elem, topic, ident);
    }

    static XMLP_ret getXMLResourceLimitsQos_wrapper(
            tinyxml2::XMLElement* elem,
            ResourceLimitsQosPolicy& resourceLimitsQos,
            uint8_t ident)
    {
        return getXMLResourceLimitsQos(elem, resourceLimitsQos, ident);
    }

    static XMLP_ret getXMLContainerAllocationConfig_wrapper(
            tinyxml2::XMLElement* elem,
            ResourceLimitedContainerConfig& allocation_config,
            uint8_t ident)
    {
        return getXMLContainerAllocationConfig(elem, allocation_config, ident);
    }

    static XMLP_ret getXMLHistoryQosPolicy_wrapper(
            tinyxml2::XMLElement* elem,
            HistoryQosPolicy& historyQos,
            uint8_t ident)
    {
        return getXMLHistoryQosPolicy(elem, historyQos, ident);
    }

    static XMLP_ret getXMLDurabilityQos_wrapper(
            tinyxml2::XMLElement* elem,
            DurabilityQosPolicy& durability,
            uint8_t ident)
    {
        return getXMLDurabilityQos(elem, durability, ident);
    }

    static XMLP_ret getXMLDeadlineQos_wrapper(
            tinyxml2::XMLElement* elem,
            DeadlineQosPolicy& deadline,
            uint8_t ident)
    {
        return getXMLDeadlineQos(elem, deadline, ident);
    }

    static XMLP_ret getXMLLatencyBudgetQos_wrapper(
            tinyxml2::XMLElement* elem,
            LatencyBudgetQosPolicy& latencyBudget,
            uint8_t ident)
    {
        return getXMLLatencyBudgetQos(elem, latencyBudget, ident);
    }

    static XMLP_ret getXMLReliabilityQos_wrapper(
            tinyxml2::XMLElement* elem,
            ReliabilityQosPolicy& reliability,
            uint8_t ident)
    {
        return getXMLReliabilityQos(elem, reliability, ident);
    }

    static XMLP_ret getXMLPartitionQos_wrapper(
            tinyxml2::XMLElement* elem,
            PartitionQosPolicy& partition,
            uint8_t ident)
    {
        return getXMLPartitionQos(elem, partition, ident);
    }

    static XMLP_ret getXMLWriterTimes_wrapper(
            tinyxml2::XMLElement* elem,
            WriterTimes& times,
            uint8_t ident)
    {
        return getXMLWriterTimes(elem, times, ident);
    }

    static XMLP_ret getXMLReaderTimes_wrapper(
            tinyxml2::XMLElement* elem,
            ReaderTimes& times,
            uint8_t ident)
    {
        return getXMLReaderTimes(elem, times, ident);
    }

    static XMLP_ret getXMLLocatorUDPv4_wrapper(
            tinyxml2::XMLElement* elem,
            Locator_t& locator,
            uint8_t ident)
    {
        return getXMLLocatorUDPv4(elem, locator, ident);
    }

    static XMLP_ret getXMLPublisherAttributes_wrapper(
            tinyxml2::XMLElement* elem,
            xmlparser::PublisherAttributes& publisher,
            uint8_t ident)
    {
        return getXMLPublisherAttributes(elem, publisher, ident);
    }

    static XMLP_ret getXMLHistoryMemoryPolicy_wrapper(
            tinyxml2::XMLElement* elem,
            MemoryManagementPolicy_t& historyMemoryPolicy,
            uint8_t ident)
    {
        return getXMLHistoryMemoryPolicy(elem, historyMemoryPolicy, ident);
    }

    static XMLP_ret getXMLLivelinessQos_wrapper(
            tinyxml2::XMLElement* elem,
            LivelinessQosPolicy& liveliness,
            uint8_t ident)
    {
        return getXMLLivelinessQos(elem, liveliness, ident);
    }

    static XMLP_ret getXMLPublishModeQos_wrapper(
            tinyxml2::XMLElement* elem,
            PublishModeQosPolicy& publishMode,
            uint8_t ident)
    {
        return getXMLPublishModeQos(elem, publishMode, ident);
    }

    static XMLP_ret getXMLParticipantAllocationAttributes_wrapper(
            tinyxml2::XMLElement* elem,
            RTPSParticipantAllocationAttributes& allocation,
            uint8_t ident)
    {
        return getXMLParticipantAllocationAttributes(elem, allocation, ident);
    }

    static XMLP_ret getXMLSendBuffersAllocationAttributes_wrapper(
            tinyxml2::XMLElement* elem,
            SendBuffersAllocationAttributes& allocation,
            uint8_t ident)
    {
        return getXMLSendBuffersAllocationAttributes(elem, allocation, ident);
    }

    static XMLP_ret getXMLRemoteLocatorsAllocationAttributes_wrapper(
            tinyxml2::XMLElement* elem,
            RemoteLocatorsAllocationAttributes& allocation,
            uint8_t ident)
    {
        return getXMLRemoteLocatorsAllocationAttributes(elem, allocation, ident);
    }

    static XMLP_ret getXMLEnum_wrapper(
            tinyxml2::XMLElement* elem,
            eprosima::fastdds::IntraprocessDeliveryType* e,
            uint8_t ident)
    {
        return getXMLEnum(elem, e, ident);
    }

    static XMLP_ret getXMLEnum_wrapper(
            tinyxml2::XMLElement* elem,
            DiscoveryProtocol* e,
            uint8_t ident)
    {
        return getXMLEnum(elem, e, ident);
    }

    static XMLP_ret getXMLEnum_wrapper(
            tinyxml2::XMLElement* elem,
            ParticipantFilteringFlags* e,
            uint8_t ident)
    {
        return getXMLEnum(elem, e, ident);
    }

    static XMLP_ret getXMLOctetVector_wrapper(
            tinyxml2::XMLElement* elem,
            std::vector<octet>& e,
            uint8_t ident)
    {
        return getXMLOctetVector(elem, e, ident);
    }

    static XMLP_ret parseXMLTypes_wrapper(
            tinyxml2::XMLElement* p_root)
    {
        return parseXMLTypes(p_root);
    }

    static XMLP_ret parseXMLTransportData_wrapper(
            tinyxml2::XMLElement* p_root)
    {
        return parseXMLTransportData(p_root);
    }

    static XMLP_ret parseXMLCommonTransportData_wrapper(
            tinyxml2::XMLElement* p_root,
            sp_transport_t p_transport)
    {
        return parseXMLCommonTransportData(p_root, p_transport);
    }

    static XMLP_ret parseXMLCommonSharedMemTransportData_wrapper(
            tinyxml2::XMLElement* p_root,
            sp_transport_t p_transport)
    {
        return parseXMLCommonSharedMemTransportData(p_root, p_transport);
    }

    static XMLP_ret parseXMLCommonTCPTransportData_wrapper(
            tinyxml2::XMLElement* p_root,
            eprosima::fastdds::xmlparser::sp_transport_t p_transport)
    {
        return parseXMLCommonTCPTransportData(p_root, p_transport);
    }

    static XMLP_ret parseXMLConsumer_wrapper(
            tinyxml2::XMLElement& p_root)
    {
        return parseXMLConsumer(p_root);
    }

    static XMLP_ret parseLogConfig_wrapper(
            tinyxml2::XMLElement* p_root)
    {
        return parseLogConfig(p_root);
    }

    static XMLP_ret parseXMLTransportsProf_wrapper(
            tinyxml2::XMLElement* p_root)
    {
        return parseXMLTransportsProf(p_root);
    }

    static XMLP_ret parseXML_wrapper(
            tinyxml2::XMLDocument& xmlDoc,
            eprosima::fastdds::xmlparser::up_base_node_t& root)
    {
        return parseXML(xmlDoc, root);
    }

    static XMLP_ret parseProfiles_wrapper(
            tinyxml2::XMLElement* p_root,
            BaseNode& profilesNode)
    {
        return parseProfiles(p_root, profilesNode);
    }

    static XMLP_ret parse_tls_config_wrapper(
            tinyxml2::XMLElement* p_root,
            eprosima::fastdds::xmlparser::sp_transport_t tcp_transport)
    {
        return parse_tls_config(p_root, tcp_transport);
    }

    static XMLP_ret parseXMLReceptionThreads_wrapper(
            tinyxml2::XMLElement& p_root,
            eprosima::fastdds::rtps::PortBasedTransportDescriptor::ReceptionThreadsConfigMap& reception_threads)
    {
        return parseXMLReceptionThreads(p_root, reception_threads);
    }

    static XMLP_ret parseXMLLibrarySettings_wrapper(
            tinyxml2::XMLElement* p_root)
    {
        return parseXMLLibrarySettings(p_root);
    }

    static XMLP_ret fillDataNode_wrapper(
            tinyxml2::XMLElement* p_profile,
            DataNode<xmlparser::PublisherAttributes>& publisher_node)
    {
        return fillDataNode(p_profile, publisher_node);
    }

    static XMLP_ret fillDataNode_wrapper(
            tinyxml2::XMLElement* p_profile,
            DataNode<xmlparser::SubscriberAttributes>& subscriber_node)
    {
        return fillDataNode(p_profile, subscriber_node);
    }

    static XMLP_ret fillDataNode_wrapper(
            tinyxml2::XMLElement* p_profile,
            DataNode<xmlparser::TopicAttributes>& topic_node)
    {
        return fillDataNode(p_profile, topic_node);
    }

    static XMLP_ret fillDataNode_wrapper(
            tinyxml2::XMLElement* p_profile,
            DataNode<xmlparser::RequesterAttributes>& requester_node)
    {
        return fillDataNode(p_profile, requester_node);
    }

    static XMLP_ret fillDataNode_wrapper(
            tinyxml2::XMLElement* p_profile,
            DataNode<xmlparser::ReplierAttributes>& replier_node)
    {
        return fillDataNode(p_profile, replier_node);
    }

    static XMLP_ret parseXMLParticipantProf_wrapper(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode)
    {
        return parseXMLParticipantProf(p_root, rootNode);
    }

    static XMLP_ret parseXMLPublisherProf_wrapper(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode)
    {
        return parseXMLPublisherProf(p_root, rootNode);
    }

    static XMLP_ret parseXMLSubscriberProf_wrapper(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode)
    {
        return parseXMLSubscriberProf(p_root, rootNode);
    }

    static XMLP_ret parseXMLRequesterProf_wrapper(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode)
    {
        return parseXMLRequesterProf(p_root, rootNode);
    }

    static XMLP_ret parseXMLReplierProf_wrapper(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode)
    {
        return parseXMLReplierProf(p_root, rootNode);
    }

    static XMLP_ret parseXMLTopicData_wrapper(
            tinyxml2::XMLElement* p_root,
            BaseNode& rootNode)
    {
        return parseXMLTopicData(p_root, rootNode);
    }

    static XMLP_ret getXMLThreadSettings_wrapper(
            tinyxml2::XMLElement* p_root,
            eprosima::fastdds::rtps::ThreadSettings& thread_settings)
    {
        return getXMLThreadSettings(*p_root, thread_settings);
    }

    static XMLP_ret getXMLThreadSettingsWithPort_wrapper(
            tinyxml2::XMLElement* p_root,
            eprosima::fastdds::rtps::ThreadSettings& thread_settings,
            uint32_t& port)
    {
        return getXMLThreadSettingsWithPort(*p_root, thread_settings, port);
    }

    static XMLP_ret getXMLEntityFactoryQos_wrapper(
            tinyxml2::XMLElement* p_root,
            eprosima::fastdds::dds::EntityFactoryQosPolicy& entity_factory)
    {
        return getXMLEntityFactoryQos(*p_root, entity_factory);
    }

};
