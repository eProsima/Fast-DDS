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
#include <tinyxml2.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <fastrtps/xmlparser/XMLTree.h>
#include <fastdds/dds/log/Log.hpp>

#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#endif // ifdef _WIN32

using namespace eprosima::fastrtps;
using namespace ::xmlparser;

LibrarySettingsAttributes XMLProfileManager::library_settings_;
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
            logError(XMLPARSER, "Profile '" << profile_name << "' not found '");
        }
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
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
            logError(XMLPARSER, "Profile '" << profile_name << "' not found '");
        }
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
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
            logError(XMLPARSER, "Profile '" << profile_name << "' not found");
        }
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::fillTopicAttributes(
        const std::string& profile_name,
        TopicAttributes& atts)
{
    topic_map_iterator_t it = topic_profiles_.find(profile_name);
    if (it == topic_profiles_.end())
    {
        logError(XMLPARSER, "Profile '" << profile_name << "' not found");
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::fillRequesterAttributes(
        const std::string& profile_name,
        RequesterAttributes& atts)
{
    requester_map_iterator_t it = requester_profiles_.find(profile_name);
    if (it == requester_profiles_.end())
    {
        logError(XMLPARSER, "Profile '" << profile_name << "' not found");
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::fillReplierAttributes(
        const std::string& profile_name,
        ReplierAttributes& atts)
{
    replier_map_iterator_t it = replier_profiles_.find(profile_name);
    if (it == replier_profiles_.end())
    {
        logError(XMLPARSER, "Profile '" << profile_name << "' not found");
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
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
    size_t size = MAX_PATH;
    if (getenv_s(&size, file_path, size, DEFAULT_FASTRTPS_ENV_VARIABLE) == 0 && size > 0)
    {
        loadXMLFile(file_path);
    }

    // Should take into account '\0'
    char skip_xml[2];
    size = 2;

    // Try to load the default XML file if variable does not exist or is not set to '1'
    if (!(getenv_s(&size, skip_xml, size, "SKIP_DEFAULT_XML_FILE") == 0 && skip_xml[0] == '1'))
    {
        loadXMLFile(DEFAULT_FASTRTPS_PROFILES);
    }
#else

    if (const char* file_path = std::getenv(DEFAULT_FASTRTPS_ENV_VARIABLE))
    {
        loadXMLFile(file_path);
    }

    const char* skip_xml = std::getenv("SKIP_DEFAULT_XML_FILE");

    // Try to load the default XML file if variable does not exist or is not set to '1'
    if (!(skip_xml != nullptr && skip_xml[0] == '1'))
    {
        loadXMLFile(DEFAULT_FASTRTPS_PROFILES);
    }

#endif // ifdef _WIN32
}

XMLP_ret XMLProfileManager::loadXMLProfiles(
        tinyxml2::XMLElement& profiles)
{
    up_base_node_t root_node;
    XMLParser::loadXMLProfiles(profiles, root_node);

    if (!root_node)
    {
        logError(XMLPARSER, "Error parsing node");
        return XMLP_ret::XML_ERROR;
    }

    logInfo(XMLPARSER, "Node parsed successfully");

    if (NodeType::PROFILES == root_node->getType())
    {
        return XMLProfileManager::extractProfiles(std::move(root_node), "-XML Node-");
    }

    if (NodeType::ROOT == root_node->getType())
    {
        for (auto&& child: root_node->getChildren())
        {
            if (NodeType::PROFILES == child.get()->getType())
            {
                return XMLProfileManager::extractProfiles(std::move(child), "-XML Node-");
            }
        }
    }

    return XMLP_ret::XML_ERROR;
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
    XMLParser::loadXML(doc, root_node);

    if (!root_node)
    {
        logError(XMLPARSER, "Error parsing node");
        return XMLP_ret::XML_ERROR;
    }

    logInfo(XMLPARSER, "Node parsed successfully");

    if (NodeType::PROFILES == root_node->getType())
    {
        return XMLProfileManager::extractProfiles(std::move(root_node), "-XML Node-");
    }

    if (NodeType::ROOT == root_node->getType())
    {
        for (auto&& child: root_node->getChildren())
        {
            if (NodeType::PROFILES == child.get()->getType())
            {
                return XMLProfileManager::extractProfiles(std::move(child), "-XML Node-");
            }
        }
    }

    return XMLP_ret::XML_ERROR;
}

XMLP_ret XMLProfileManager::loadXMLFile(
        const std::string& filename)
{
    if (filename.empty())
    {
        logError(XMLPARSER, "Error loading XML file, filename empty");
        return XMLP_ret::XML_ERROR;
    }

    xmlfile_map_iterator_t it = xml_files_.find(filename);
    if (it != xml_files_.end() && XMLP_ret::XML_OK == it->second)
    {
        logInfo(XMLPARSER, "XML file '" << filename << "' already parsed");
        return XMLP_ret::XML_OK;
    }

    up_base_node_t root_node;
    XMLP_ret loaded_ret = XMLParser::loadXML(filename, root_node);
    if (!root_node || loaded_ret != XMLP_ret::XML_OK)
    {
        if (filename != std::string(DEFAULT_FASTRTPS_PROFILES))
        {
            logError(XMLPARSER, "Error parsing '" << filename << "'");
        }
        xml_files_.emplace(filename, XMLP_ret::XML_ERROR);
        return XMLP_ret::XML_ERROR;
    }

    logInfo(XMLPARSER, "File '" << filename << "' parsed successfully");

    if (NodeType::PROFILES == root_node->getType())
    {
        return XMLProfileManager::extractProfiles(std::move(root_node), filename);
    }

    if (NodeType::TYPES == root_node->getType())
    {
        return loaded_ret;
    }

    if (NodeType::LOG == root_node->getType())
    {
        return loaded_ret;
    }

    if (NodeType::ROOT == root_node->getType())
    {
        for (auto&& child: root_node->getChildren())
        {
            if (NodeType::PROFILES == child.get()->getType())
            {
                return XMLProfileManager::extractProfiles(std::move(child), filename);
            }
            // TODO Workaround when there is a ROOT tag without PROFILES. Return the corresponding error instead of
            // XMLP_ret::XML_ERROR. Only the type is checked so the objects do not need to be populated.
            else if (NodeType::TYPES == child.get()->getType())
            {
                return loaded_ret;
            }
            else if (NodeType::LOG == child.get()->getType())
            {
                return loaded_ret;
            }
        }
    }

    return XMLP_ret::XML_ERROR;
}

XMLP_ret XMLProfileManager::extractDynamicTypes(
        up_base_node_t profiles,
        const std::string& filename)
{
    if (nullptr == profiles)
    {
        logError(XMLPARSER, "Bad parameters");
        return XMLP_ret::XML_ERROR;
    }

    unsigned int profile_count = 0u;

    for (auto&& profile: profiles->getChildren())
    {
        if (NodeType::TYPE == profile->getType())
        {
            tinyxml2::XMLElement* node = dynamic_cast<tinyxml2::XMLElement*>(profile.get());
            if (XMLP_ret::XML_OK == XMLParser::loadXMLDynamicTypes(*node))
            {
                ++profile_count;
            }
        }
        else
        {
            logError(XMLPARSER, "Not expected tag");
        }
    }

    if (0 == profile_count)
    {
        xml_files_.emplace(filename, XMLP_ret::XML_ERROR);
        logError(XMLPARSER, "Error, file '" << filename << "' bad content");
        return XMLP_ret::XML_ERROR;
    }

    xml_files_.emplace(filename, XMLP_ret::XML_OK);

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::extractProfiles(
        up_base_node_t profiles,
        const std::string& filename)
{
    if (nullptr == profiles)
    {
        logError(XMLPARSER, "Bad parameters");
        return XMLP_ret::XML_ERROR;
    }

    unsigned int profile_count = 0u;

    XMLP_ret ret = XMLP_ret::XML_OK;
    for (auto&& profile: profiles->getChildren())
    {
        if (NodeType::PARTICIPANT == profile->getType())
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
        else
        {
            logError(XMLPARSER, "Not expected tag");
        }
    }

    profile_count += static_cast<unsigned int>(transport_profiles_.size()); // Count transport profiles

    if (ret != XMLP_ret::XML_OK && profile_count == 0)
    {
        // Could not extract any profile
        ret = XMLP_ret::XML_ERROR;
    }

    xml_files_.emplace(filename, ret);

    return ret;
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
        logError(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<part_map_iterator_t, bool> emplace = participant_profiles_.emplace(profile_name, node_part->getData());
    if (false == emplace.second)
    {
        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
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
        logError(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<publ_map_iterator_t, bool> emplace = publisher_profiles_.emplace(profile_name, node_part->getData());
    if (false == emplace.second)
    {
        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
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
        logError(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<subs_map_iterator_t, bool> emplace = subscriber_profiles_.emplace(profile_name, node_part->getData());
    if (false == emplace.second)
    {
        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
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
    logError(XMLPARSER, "Error adding the transport " << transport_id << ". There is other transport with the same id");
    return false;
}

const LibrarySettingsAttributes& XMLProfileManager::library_settings()
{
    return library_settings_;
}

void XMLProfileManager::library_settings(
        const LibrarySettingsAttributes& library_settings)
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

bool XMLProfileManager::insertDynamicTypeByName(
        const std::string& type_name,
        p_dynamictypebuilder_t type)
{
    if (dynamic_types_.find(type_name) == dynamic_types_.end())
    {
        dynamic_types_[type_name] = type;
        return true;
    }
    logError(XMLPARSER, "Error adding the type " << type_name << ". There is other type with the same name.");
    return false;
}

p_dynamictypebuilder_t XMLProfileManager::getDynamicTypeByName(
        const std::string& type_name)
{
    if (dynamic_types_.find(type_name) != dynamic_types_.end())
    {
        return dynamic_types_[type_name];
    }
    return nullptr;
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
        logError(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<topic_map_iterator_t, bool> emplace = topic_profiles_.emplace(profile_name, node_topic->getData());
    if (false == emplace.second)
    {
        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    it = node_topic->getAttributes().find(DEFAULT_PROF);
    if (false == emplace.second)
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
        logError(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<requester_map_iterator_t, bool> emplace = requester_profiles_.emplace(profile_name,
                    node_requester->getData());
    if (false == emplace.second)
    {
        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
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
        logError(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<replier_map_iterator_t, bool> emplace = replier_profiles_.emplace(profile_name, node_replier->getData());
    if (false == emplace.second)
    {
        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    return XMLP_ret::XML_OK;
}
