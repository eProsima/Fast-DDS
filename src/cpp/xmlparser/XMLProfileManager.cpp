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
#include <xmlparser/XMLProfileManager.h>

#include <cstdlib>
#include <functional>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include <tinyxml2.h>

#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/log/Log.hpp>

#include <xmlparser/XMLTree.h>

using namespace eprosima::fastdds;
using namespace ::xmlparser;

LibrarySettings XMLProfileManager::library_settings_;
std::map<std::string, up_participantfactory_t> XMLProfileManager::participant_factory_profiles_;
dds::DomainParticipantFactoryQos default_participant_factory_qos;
std::map<std::string, up_participant_t> XMLProfileManager::participant_profiles_;
ParticipantAttributes default_participant_attributes;
std::map<std::string, up_publisher_t> XMLProfileManager::publisher_profiles_;
PublisherAttributes default_publisher_attributes;
std::map<std::string, up_subscriber_t> XMLProfileManager::subscriber_profiles_;
SubscriberAttributes default_subscriber_attributes;
std::map<std::string, up_topic_t> XMLProfileManager::topic_profiles_;
TopicAttributes default_topic_attributes;
std::map<std::string, up_requester_t> XMLProfileManager::requester_profiles_;
std::map<std::string, up_replier_t> XMLProfileManager::replier_profiles_;
std::map<std::string, XMLP_ret> XMLProfileManager::xml_files_;
sp_transport_map_t XMLProfileManager::transport_profiles_;
p_dynamictype_map_t XMLProfileManager::dynamic_types_;
BaseNode* XMLProfileManager::root = nullptr;

template<typename T>
struct AttributesTraits;

template<>
struct AttributesTraits<ParticipantAttributes>
{
    static constexpr NodeType node_type = NodeType::PARTICIPANT;
    using NodePtrType = p_node_participant_t;
    using NodeUniquePtrType = up_participant_t;

    static std::string name()
    {
        return "Participant";
    }

};

template<>
struct AttributesTraits<PublisherAttributes>
{
    static constexpr NodeType node_type = NodeType::PUBLISHER;
    using NodePtrType = p_node_publisher_t;
    using NodeUniquePtrType = up_publisher_t;

    static std::string name()
    {
        return "Publisher";
    }

};

template<>
struct AttributesTraits<SubscriberAttributes>
{
    static constexpr NodeType node_type = NodeType::SUBSCRIBER;
    using NodePtrType = p_node_subscriber_t;
    using NodeUniquePtrType = up_subscriber_t;

    static std::string name()
    {
        return "Subscriber";
    }

};

template<>
struct AttributesTraits<TopicAttributes>
{
    static constexpr NodeType node_type = NodeType::TOPIC;
    using NodePtrType = p_node_topic_t;
    using NodeUniquePtrType = up_topic_t;

    static std::string name()
    {
        return "Topic";
    }

};

template<>
struct AttributesTraits<RequesterAttributes>
{
    static constexpr NodeType node_type = NodeType::REQUESTER;
    using NodePtrType = p_node_requester_t;
    using NodeUniquePtrType = up_requester_t;

    static std::string name()
    {
        return "Requester";
    }

};

template<>
struct AttributesTraits<ReplierAttributes>
{
    static constexpr NodeType node_type = NodeType::REPLIER;
    using NodePtrType = p_node_replier_t;
    using NodeUniquePtrType = up_replier_t;

    static std::string name()
    {
        return "Replier";
    }

};

template <typename AttributesType>
XMLP_ret fill_attributes_from_xml(
        const std::string& xml,
        AttributesType& atts,
        const std::function<bool(typename AttributesTraits<AttributesType>::NodePtrType)>& node_filter,
        bool fulfill_xsd)
{
    using Traits = AttributesTraits<AttributesType>;

    up_base_node_t root_node;
    XMLP_ret loaded_ret = XMLParser::loadXML(xml.c_str(), xml.size(), root_node);

    if (!root_node || loaded_ret != XMLP_ret::XML_OK)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing string");
        return XMLP_ret::XML_ERROR;
    }

    // Process node function
    auto process_node = [&](const up_base_node_t& node_to_process, AttributesType& atts)
            {
                // If node type doesn't match, skip
                if (Traits::node_type != node_to_process->getType())
                {
                    return XMLP_ret::XML_NOK;
                }

                // Cast the node to the expected type
                typename Traits::NodePtrType node = dynamic_cast<typename Traits::NodePtrType>(node_to_process.get());
                if (!node)
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Error casting node");
                    return XMLP_ret::XML_ERROR;
                }

                // If node doesn't match the filter, skip
                if (!node_filter(node))
                {
                    return XMLP_ret::XML_NOK;
                }

                // Retrieve node data
                typename Traits::NodeUniquePtrType node_data = node->getData();
                if (!node_data)
                {
                    EPROSIMA_LOG_ERROR(XMLPARSER, "Error retrieving node data");
                    return XMLP_ret::XML_ERROR;
                }

                // Fill attributes
                atts = *node_data;
                return XMLP_ret::XML_OK;
            };

    // Recursive function to process the root node and its children
    std::function<XMLP_ret(const up_base_node_t&, AttributesType&)> process_node_recursive;
    process_node_recursive =
            [&process_node, &process_node_recursive](const up_base_node_t& node_to_process, AttributesType& atts)
            {
                XMLP_ret ret = process_node(node_to_process, atts);
                if (XMLP_ret::XML_OK == ret || XMLP_ret::XML_ERROR == ret)
                {
                    return ret;
                }

                for (auto&& child: node_to_process->getChildren())
                {
                    ret = process_node_recursive(child, atts);
                    if (XMLP_ret::XML_OK == ret || XMLP_ret::XML_ERROR == ret)
                    {
                        return ret;
                    }
                }
                return XMLP_ret::XML_NOK;
            };

    std::reference_wrapper<up_base_node_t> node_to_process = root_node;
    if (fulfill_xsd)
    {
        if (NodeType::ROOT == root_node->getType())
        {
            for (auto&& child: root_node->getChildren())
            {
                if (NodeType::PROFILES == child->getType())
                {
                    node_to_process = child;
                    break;
                }
            }
        }

        // Abort if profiles tag is not found according to XSD
        if (NodeType::PROFILES != node_to_process.get()->getType())
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Provided XML literal does not contain profiles");
            return XMLP_ret::XML_ERROR;
        }
    }

    if (XMLP_ret::XML_OK == process_node_recursive(node_to_process, atts))
    {
        return XMLP_ret::XML_OK;
    }

    EPROSIMA_LOG_ERROR(XMLPARSER, Traits::name() << " profile not found");
    return XMLP_ret::XML_ERROR;
}

template <typename AttributesType>
XMLP_ret fill_attributes_from_xml(
        const std::string& xml,
        AttributesType& atts,
        bool fulfill_xsd,
        const std::string& profile_name)
{
    auto node_filter = [&profile_name](typename AttributesTraits<AttributesType>::NodePtrType node) -> bool
            {
                if (!profile_name.empty())
                {
                    auto it = node->getAttributes().find(PROFILE_NAME);
                    return (it != node->getAttributes().end() && it->second == profile_name);
                }
                return true;
            };
    return fill_attributes_from_xml(xml, atts, node_filter, fulfill_xsd);
}

template <typename AttributesType>
XMLP_ret fill_default_attributes_from_xml(
        const std::string& xml,
        AttributesType& atts,
        bool fulfill_xsd)
{
    auto node_filter = [](typename AttributesTraits<AttributesType>::NodePtrType node) -> bool
            {
                auto it = node->getAttributes().find(DEFAULT_PROF);
                return (it != node->getAttributes().end() && it->second == "true");
            };
    return fill_attributes_from_xml(xml, atts, node_filter, fulfill_xsd);
}

XMLP_ret XMLProfileManager::fillParticipantAttributes(
        const std::string& profile_name,
        ParticipantAttributes& atts,
        bool log_error)
{
    part_map_iterator_t it = participant_profiles_.find(profile_name);
    if (it == participant_profiles_.end())
    {
        if (log_error)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Profile '" << profile_name << "' not found");
        }
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::fill_participant_attributes_from_xml(
        const std::string& xml,
        ParticipantAttributes& atts,
        bool fulfill_xsd,
        const std::string& profile_name)
{
    return fill_attributes_from_xml<ParticipantAttributes>(xml, atts, fulfill_xsd, profile_name);
}

XMLP_ret XMLProfileManager::fill_default_participant_attributes_from_xml(
        const std::string& xml,
        ParticipantAttributes& atts,
        bool fulfill_xsd)
{
    return fill_default_attributes_from_xml<ParticipantAttributes>(xml, atts, fulfill_xsd);
}

XMLP_ret XMLProfileManager::fillPublisherAttributes(
        const std::string& profile_name,
        PublisherAttributes& atts,
        bool log_error)
{
    publ_map_iterator_t it = publisher_profiles_.find(profile_name);
    if (it == publisher_profiles_.end())
    {
        if (log_error)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Profile '" << profile_name << "' not found");
        }
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::fill_publisher_attributes_from_xml(
        const std::string& xml,
        PublisherAttributes& atts,
        bool fulfill_xsd,
        const std::string& profile_name)
{
    return fill_attributes_from_xml<PublisherAttributes>(xml, atts, fulfill_xsd, profile_name);
}

XMLP_ret XMLProfileManager::fill_default_publisher_attributes_from_xml(
        const std::string& xml,
        PublisherAttributes& atts,
        bool fulfill_xsd)
{
    return fill_default_attributes_from_xml<PublisherAttributes>(xml, atts, fulfill_xsd);
}

XMLP_ret XMLProfileManager::fillSubscriberAttributes(
        const std::string& profile_name,
        SubscriberAttributes& atts,
        bool log_error)
{
    subs_map_iterator_t it = subscriber_profiles_.find(profile_name);
    if (it == subscriber_profiles_.end())
    {
        if (log_error)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Profile '" << profile_name << "' not found");
        }
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::fill_subscriber_attributes_from_xml(
        const std::string& xml,
        SubscriberAttributes& atts,
        bool fulfill_xsd,
        const std::string& profile_name)
{
    return fill_attributes_from_xml<SubscriberAttributes>(xml, atts, fulfill_xsd, profile_name);
}

XMLP_ret XMLProfileManager::fill_default_subscriber_attributes_from_xml(
        const std::string& xml,
        SubscriberAttributes& atts,
        bool fulfill_xsd)
{
    return fill_default_attributes_from_xml<SubscriberAttributes>(xml, atts, fulfill_xsd);
}

XMLP_ret XMLProfileManager::fillTopicAttributes(
        const std::string& profile_name,
        TopicAttributes& atts)
{
    topic_map_iterator_t it = topic_profiles_.find(profile_name);
    if (it == topic_profiles_.end())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Profile '" << profile_name << "' not found");
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::fill_topic_attributes_from_xml(
        const std::string& xml,
        TopicAttributes& atts,
        bool fulfill_xsd,
        const std::string& profile_name)
{
    return fill_attributes_from_xml<TopicAttributes>(xml, atts, fulfill_xsd, profile_name);
}

XMLP_ret XMLProfileManager::fill_default_topic_attributes_from_xml(
        const std::string& xml,
        TopicAttributes& atts,
        bool fulfill_xsd)
{
    return fill_default_attributes_from_xml<TopicAttributes>(xml, atts, fulfill_xsd);
}

XMLP_ret XMLProfileManager::fillRequesterAttributes(
        const std::string& profile_name,
        RequesterAttributes& atts)
{
    requester_map_iterator_t it = requester_profiles_.find(profile_name);
    if (it == requester_profiles_.end())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Profile '" << profile_name << "' not found");
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::fill_requester_attributes_from_xml(
        const std::string& xml,
        RequesterAttributes& atts,
        bool fulfill_xsd,
        const std::string& profile_name)
{
    return fill_attributes_from_xml<RequesterAttributes>(xml, atts, fulfill_xsd, profile_name);
}

XMLP_ret XMLProfileManager::fill_default_requester_attributes_from_xml(
        const std::string& xml,
        RequesterAttributes& atts,
        bool fulfill_xsd)
{
    return fill_default_attributes_from_xml<RequesterAttributes>(xml, atts, fulfill_xsd);
}

XMLP_ret XMLProfileManager::fillReplierAttributes(
        const std::string& profile_name,
        ReplierAttributes& atts)
{
    replier_map_iterator_t it = replier_profiles_.find(profile_name);
    if (it == replier_profiles_.end())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Profile '" << profile_name << "' not found");
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::fill_replier_attributes_from_xml(
        const std::string& xml,
        ReplierAttributes& atts,
        bool fulfill_xsd,
        const std::string& profile_name)
{
    return fill_attributes_from_xml<ReplierAttributes>(xml, atts, fulfill_xsd, profile_name);
}

XMLP_ret XMLProfileManager::fill_default_replier_attributes_from_xml(
        const std::string& xml,
        ReplierAttributes& atts,
        bool fulfill_xsd)
{
    return fill_default_attributes_from_xml<ReplierAttributes>(xml, atts, fulfill_xsd);
}

void XMLProfileManager::getDefaultParticipantAttributes(
        ParticipantAttributes& participant_attributes)
{
    participant_attributes = default_participant_attributes;
}

void XMLProfileManager::getDefaultPublisherAttributes(
        PublisherAttributes& publisher_attributes)
{
    publisher_attributes = default_publisher_attributes;
}

XMLP_ret XMLProfileManager::fillDomainParticipantFactoryQos(
        const std::string& profile_name,
        dds::DomainParticipantFactoryQos& qos,
        bool log_error)
{
    part_factory_map_iterator_t it = participant_factory_profiles_.find(profile_name);
    if (it == participant_factory_profiles_.end())
    {
        if (log_error)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Profile '" << profile_name << "' not found");
        }
        return XMLP_ret::XML_ERROR;
    }
    qos = *(it->second);
    return XMLP_ret::XML_OK;
}

void XMLProfileManager::getDefaultDomainParticipantFactoryQos(
        dds::DomainParticipantFactoryQos& qos)
{
    qos = default_participant_factory_qos;
}

void XMLProfileManager::getDefaultSubscriberAttributes(
        SubscriberAttributes& subscriber_attributes)
{
    subscriber_attributes = default_subscriber_attributes;
}

void XMLProfileManager::getDefaultTopicAttributes(
        TopicAttributes& topic_attributes)
{
    topic_attributes = default_topic_attributes;
}

void XMLProfileManager::loadDefaultXMLFile()
{
    // Try to load the default XML file set with an environment variable.
#ifdef _WIN32
    char file_path[MAX_PATH];
    char absolute_path[MAX_PATH];
    char current_directory[MAX_PATH];
    char** filename = {nullptr};
    size_t size = MAX_PATH;
    if (getenv_s(&size, file_path, size, DEFAULT_FASTDDS_ENV_VARIABLE) == 0 && size > 0)
    {
        // Use absolute path to ensure the file is loaded only once
        if (GetFullPathName(file_path, MAX_PATH, absolute_path, filename) == 0)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "GetFullPathName failed " << GetLastError());
        }
        else
        {
            loadXMLFile(absolute_path);
        }
    }

    // Should take into account '\0'
    char skip_xml[2];
    size = 2;

    // Try to load the default XML file if variable does not exist or is not set to '1'
    if (!(getenv_s(&size, skip_xml, size, SKIP_DEFAULT_XML_FILE) == 0 && skip_xml[0] == '1'))
    {
        // Try to load the default XML file.
        if (GetCurrentDirectory(MAX_PATH, current_directory) == 0)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "GetCurrentDirectory failed " << GetLastError());
        }
        else
        {
            strcat_s(current_directory, MAX_PATH, "\\");
            strcat_s(current_directory, MAX_PATH, DEFAULT_FASTDDS_PROFILES);
            loadXMLFile(current_directory, true);
        }
    }
#else
    char absolute_path[PATH_MAX];

    if (const char* file_path = std::getenv(DEFAULT_FASTDDS_ENV_VARIABLE))
    {
        char* res = realpath(file_path, absolute_path);
        if (res)
        {
            loadXMLFile(absolute_path);
        }
        else
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "realpath failed " << std::strerror(errno));
        }
    }

    const char* skip_xml = std::getenv(SKIP_DEFAULT_XML_FILE);

    // Try to load the default XML file if variable does not exist or is not set to '1'
    if (!(skip_xml != nullptr && skip_xml[0] == '1'))
    {
        if (getcwd(absolute_path, PATH_MAX) == NULL)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "getcwd failed " << std::strerror(errno));
        }
        else
        {
            strcat(absolute_path, "/");
            strcat(absolute_path, DEFAULT_FASTDDS_PROFILES);
            loadXMLFile(absolute_path, true);
        }
    }

#endif // ifdef _WIN32
}

XMLP_ret XMLProfileManager::loadXMLProfiles(
        tinyxml2::XMLElement& profiles)
{
    up_base_node_t root_node;
    if (strcmp(profiles.Name(), PROFILES) != 0)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "<profiles> element not found");
        return XMLP_ret::XML_ERROR;
    }

    if (XMLParser::loadXMLProfiles(profiles, root_node) == XMLP_ret::XML_OK)
    {
        EPROSIMA_LOG_INFO(XMLPARSER, "Node parsed successfully");
        return XMLProfileManager::extractProfiles(std::move(root_node), "-XML Node-");
    }
    else
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing profiles");
        return XMLP_ret::XML_ERROR;
    }
}

XMLP_ret XMLProfileManager::loadXMLDynamicTypes(
        tinyxml2::XMLElement& types)
{
    return XMLParser::loadXMLDynamicTypes(types);
}

XMLP_ret XMLProfileManager::loadXMLNode(
        tinyxml2::XMLDocument& doc)
{
    up_base_node_t root_node;
    XMLP_ret parse_ret;
    XMLP_ret loaded_ret = XMLParser::loadXML(doc, root_node);

    if (!root_node)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing node");
        return XMLP_ret::XML_ERROR;
    }

    EPROSIMA_LOG_INFO(XMLPARSER, "Node parsed successfully");

    if (NodeType::PROFILES == root_node->getType())
    {
        parse_ret = XMLProfileManager::extractProfiles(std::move(root_node), "-XML Node-");
        if (parse_ret == XMLP_ret::XML_OK && loaded_ret != XMLP_ret::XML_OK)
        {
            return XMLP_ret::XML_NOK;
        }
        else
        {
            return parse_ret;
        }
    }

    if (NodeType::ROOT == root_node->getType())
    {
        for (auto&& child: root_node->getChildren())
        {
            if (NodeType::PROFILES == child.get()->getType())
            {
                parse_ret = XMLProfileManager::extractProfiles(std::move(child), "-XML Node-");
                if (parse_ret == XMLP_ret::XML_OK && loaded_ret != XMLP_ret::XML_OK)
                {
                    return XMLP_ret::XML_NOK;
                }
                else
                {
                    return parse_ret;
                }
            }
        }
    }

    return XMLP_ret::XML_ERROR;
}

XMLP_ret XMLProfileManager::loadXMLFile(
        const std::string& filename)
{
    return loadXMLFile(filename, false);
}

XMLP_ret XMLProfileManager::loadXMLFile(
        const std::string& filename,
        bool is_default)
{
    if (filename.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error loading XML file, filename empty");
        return XMLP_ret::XML_ERROR;
    }

    xmlfile_map_iterator_t it = xml_files_.find(filename);
    if (it != xml_files_.end() && XMLP_ret::XML_OK == it->second)
    {
        EPROSIMA_LOG_INFO(XMLPARSER, "XML file '" << filename << "' already parsed");
        return XMLP_ret::XML_OK;
    }

    up_base_node_t root_node;
    XMLP_ret loaded_ret = XMLParser::loadXML(filename, root_node, is_default);
    if (!root_node || loaded_ret != XMLP_ret::XML_OK)
    {
        if (!is_default)
        {
            EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing '" << filename << "'");
        }
        xml_files_.emplace(filename, XMLP_ret::XML_ERROR);
        return XMLP_ret::XML_ERROR;
    }

    EPROSIMA_LOG_INFO(XMLPARSER, "File '" << filename << "' parsed successfully");

    if (NodeType::ROOT == root_node->getType())
    {
        for (auto&& child: root_node->getChildren())
        {
            if (NodeType::PROFILES == child.get()->getType())
            {
                return XMLProfileManager::extractProfiles(std::move(child), filename);
            }
        }
        return loaded_ret;
    }
    else if (NodeType::PROFILES == root_node->getType())
    {
        return XMLProfileManager::extractProfiles(std::move(root_node), filename);
    }

    return loaded_ret;
}

XMLP_ret XMLProfileManager::loadXMLString(
        const char* data,
        size_t length)
{
    up_base_node_t root_node;
    XMLP_ret loaded_ret = XMLParser::loadXML(data, length, root_node);
    if (!root_node || loaded_ret != XMLP_ret::XML_OK)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error parsing string");
        return XMLP_ret::XML_ERROR;
    }

    if (NodeType::ROOT == root_node->getType())
    {
        for (auto&& child: root_node->getChildren())
        {
            if (NodeType::PROFILES == child.get()->getType())
            {
                return XMLProfileManager::extractProfiles(std::move(child), "inmem");
            }
        }
        return loaded_ret;
    }
    else if (NodeType::PROFILES == root_node->getType())
    {
        return XMLProfileManager::extractProfiles(std::move(root_node), "inmem");
    }

    return loaded_ret;
}

XMLP_ret XMLProfileManager::extractProfiles(
        up_base_node_t profiles,
        const std::string& filename)
{
    assert(profiles != nullptr);

    unsigned int profile_count = 0u;

    XMLP_ret ret = XMLP_ret::XML_OK;
    for (auto&& profile: profiles->getChildren())
    {
        if (NodeType::DOMAINPARTICIPANT_FACTORY == profile->getType())
        {
            if (XMLP_ret::XML_OK == extractDomainParticipantFactoryProfile(profile, filename))
            {
                ++profile_count;
            }
            else
            {
                ret = XMLP_ret::XML_NOK;
            }
        }
        else if (NodeType::PARTICIPANT == profile->getType())
        {
            if (XMLP_ret::XML_OK == extractParticipantProfile(profile, filename))
            {
                ++profile_count;
            }
            else
            {
                ret = XMLP_ret::XML_NOK;
            }
        }
        else if (NodeType::PUBLISHER == profile.get()->getType())
        {
            if (XMLP_ret::XML_OK == extractPublisherProfile(profile, filename))
            {
                ++profile_count;
            }
            else
            {
                ret = XMLP_ret::XML_NOK;
            }
        }
        else if (NodeType::SUBSCRIBER == profile.get()->getType())
        {
            if (XMLP_ret::XML_OK == extractSubscriberProfile(profile, filename))
            {
                ++profile_count;
            }
            else
            {
                ret = XMLP_ret::XML_NOK;
            }
        }
        else if (NodeType::TOPIC == profile.get()->getType())
        {
            if (XMLP_ret::XML_OK == extractTopicProfile(profile, filename))
            {
                ++profile_count;
            }
            else
            {
                ret = XMLP_ret::XML_NOK;
            }
        }
        else if (NodeType::REQUESTER == profile.get()->getType())
        {
            if (XMLP_ret::XML_OK == extractRequesterProfile(profile, filename))
            {
                ++profile_count;
            }
            else
            {
                ret = XMLP_ret::XML_NOK;
            }
        }
        else if (NodeType::REPLIER == profile.get()->getType())
        {
            if (XMLP_ret::XML_OK == extractReplierProfile(profile, filename))
            {
                ++profile_count;
            }
            else
            {
                ret = XMLP_ret::XML_NOK;
            }
        }
    }

    profile_count += static_cast<unsigned int>(transport_profiles_.size()); // Count transport profiles

    if (profile_count == 0)
    {
        EPROSIMA_LOG_ERROR(XMLProfileManager, "Could not extract any profile");
        ret = XMLP_ret::XML_ERROR;
    }

    xml_files_.emplace(filename, ret);

    return ret;
}

XMLP_ret XMLProfileManager::extractDomainParticipantFactoryProfile(
        up_base_node_t& profile,
        const std::string& filename)
{
    static_cast<void>(filename);
    std::string profile_name = "";

    p_node_participantfactory_t node_factory = dynamic_cast<p_node_participantfactory_t>(profile.get());
    node_att_map_cit_t it = node_factory->getAttributes().find(PROFILE_NAME);
    if (it == node_factory->getAttributes().end() || it->second.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<part_factory_map_iterator_t, bool> emplace = participant_factory_profiles_.emplace(profile_name,
                    node_factory->getData());
    if (false == emplace.second)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    it = node_factory->getAttributes().find(DEFAULT_PROF);
    if (it != node_factory->getAttributes().end() && it->second == "true") // Set as default profile
    {
        default_participant_factory_qos = *(emplace.first->second.get());
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::extractParticipantProfile(
        up_base_node_t& profile,
        const std::string& filename)
{
    (void)(filename);
    std::string profile_name = "";

    p_node_participant_t node_part = dynamic_cast<p_node_participant_t>(profile.get());
    node_att_map_cit_t it = node_part->getAttributes().find(PROFILE_NAME);
    if (it == node_part->getAttributes().end() || it->second.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<part_map_iterator_t, bool> emplace = participant_profiles_.emplace(profile_name, node_part->getData());
    if (false == emplace.second)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    it = node_part->getAttributes().find(DEFAULT_PROF);
    if (it != node_part->getAttributes().end() && it->second == "true") // Set as default profile
    {
        // +V+ TODO: LOG ERROR IN SECOND ATTEMPT
        default_participant_attributes = *(emplace.first->second.get());
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::extractPublisherProfile(
        up_base_node_t& profile,
        const std::string& filename)
{
    (void)(filename);
    std::string profile_name = "";

    p_node_publisher_t node_part = dynamic_cast<p_node_publisher_t>(profile.get());
    node_att_map_cit_t it = node_part->getAttributes().find(PROFILE_NAME);
    if (it == node_part->getAttributes().end() || it->second.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<publ_map_iterator_t, bool> emplace = publisher_profiles_.emplace(profile_name, node_part->getData());
    if (false == emplace.second)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    it = node_part->getAttributes().find(DEFAULT_PROF);
    if (it != node_part->getAttributes().end() && it->second == "true") // Set as default profile
    {
        // +V+ TODO: LOG ERROR IN SECOND ATTEMPT
        default_publisher_attributes = *(emplace.first->second.get());
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::extractSubscriberProfile(
        up_base_node_t& profile,
        const std::string& filename)
{
    (void)(filename);
    std::string profile_name = "";

    p_node_subscriber_t node_part = dynamic_cast<p_node_subscriber_t>(profile.get());
    node_att_map_cit_t it = node_part->getAttributes().find(PROFILE_NAME);
    if (it == node_part->getAttributes().end() || it->second.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<subs_map_iterator_t, bool> emplace = subscriber_profiles_.emplace(profile_name, node_part->getData());
    if (false == emplace.second)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    it = node_part->getAttributes().find(DEFAULT_PROF);
    if (it != node_part->getAttributes().end() && it->second == "true") // Set as default profile
    {
        // +V+ TODO: LOG ERROR IN SECOND ATTEMPT
        default_subscriber_attributes = *(emplace.first->second.get());
    }
    return XMLP_ret::XML_OK;
}

bool XMLProfileManager::insertTransportById(
        const std::string& transport_id,
        sp_transport_t transport)
{
    if (transport_profiles_.find(transport_id) == transport_profiles_.end())
    {
        transport_profiles_[transport_id] = transport;
        return true;
    }
    EPROSIMA_LOG_ERROR(XMLPARSER,
            "Error adding the transport " << transport_id << ". There is other transport with the same id");
    return false;
}

const LibrarySettings& XMLProfileManager::library_settings()
{
    return library_settings_;
}

void XMLProfileManager::library_settings(
        const LibrarySettings& library_settings)
{
    library_settings_ = library_settings;
}

sp_transport_t XMLProfileManager::getTransportById(
        const std::string& transport_id)
{
    if (transport_profiles_.find(transport_id) != transport_profiles_.end())
    {
        return transport_profiles_[transport_id];
    }
    return nullptr;
}

bool XMLProfileManager::insertDynamicTypeBuilderByName(
        const std::string& type_name,
        const eprosima::fastdds::dds::DynamicTypeBuilder::_ref_type& type)
{
    if (dynamic_types_.find(type_name) == dynamic_types_.end())
    {
        dynamic_types_.emplace(std::make_pair(type_name, type));
        return true;
    }
    EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding the type " << type_name << ". There is other type with the same name.");
    return false;
}

XMLP_ret XMLProfileManager::getDynamicTypeBuilderByName(
        eprosima::fastdds::dds::DynamicTypeBuilder::_ref_type& dynamic_type,
        const std::string& type_name)
{
    if (dynamic_types_.find(type_name) != dynamic_types_.end())
    {
        dynamic_type = dynamic_types_[type_name];
        return XMLP_ret::XML_OK;
    }

    return XMLP_ret::XML_ERROR;
}

XMLP_ret XMLProfileManager::extractTopicProfile(
        up_base_node_t& profile,
        const std::string& filename)
{
    (void)(filename);
    std::string profile_name = "";

    p_node_topic_t node_topic = dynamic_cast<p_node_topic_t>(profile.get());
    node_att_map_cit_t it = node_topic->getAttributes().find(PROFILE_NAME);
    if (it == node_topic->getAttributes().end() || it->second.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<topic_map_iterator_t, bool> emplace = topic_profiles_.emplace(profile_name, node_topic->getData());
    if (false == emplace.second)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    it = node_topic->getAttributes().find(DEFAULT_PROF);
    if (it != node_topic->getAttributes().end() && it->second == "true")
    {
        // +V+ TODO: LOG ERROR IN SECOND ATTEMPT
        default_topic_attributes = *(emplace.first->second.get());
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::extractRequesterProfile(
        up_base_node_t& profile,
        const std::string& filename)
{
    (void)(filename);
    std::string profile_name = "";

    p_node_requester_t node_requester = dynamic_cast<p_node_requester_t>(profile.get());
    node_att_map_cit_t it = node_requester->getAttributes().find(PROFILE_NAME);
    if (it == node_requester->getAttributes().end() || it->second.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<requester_map_iterator_t, bool> emplace = requester_profiles_.emplace(profile_name,
                    node_requester->getData());
    if (false == emplace.second)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::extractReplierProfile(
        up_base_node_t& profile,
        const std::string& filename)
{
    (void)(filename);
    std::string profile_name = "";

    p_node_replier_t node_replier = dynamic_cast<p_node_replier_t>(profile.get());
    node_att_map_cit_t it = node_replier->getAttributes().find(PROFILE_NAME);
    if (it == node_replier->getAttributes().end() || it->second.empty())
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<replier_map_iterator_t, bool> emplace = replier_profiles_.emplace(profile_name, node_replier->getData());
    if (false == emplace.second)
    {
        EPROSIMA_LOG_ERROR(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}

void XMLProfileManager::DeleteInstance()
{
    participant_factory_profiles_.clear();
    participant_profiles_.clear();
    publisher_profiles_.clear();
    subscriber_profiles_.clear();
    requester_profiles_.clear();
    replier_profiles_.clear();
    topic_profiles_.clear();
    xml_files_.clear();
    transport_profiles_.clear();
    dynamic_types_.clear();
}
