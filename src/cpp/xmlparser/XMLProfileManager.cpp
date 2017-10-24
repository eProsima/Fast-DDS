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

namespace eprosima {
namespace fastrtps {
namespace xmlparser {

std::map<std::string, up_participant_t> XMLProfileManager::m_participant_profiles;
ParticipantAttributes default_participant_attributes;
std::map<std::string, up_publisher_t>   XMLProfileManager::m_publisher_profiles;
PublisherAttributes default_publisher_attributes;
std::map<std::string, up_subscriber_t>  XMLProfileManager::m_subscriber_profiles;
SubscriberAttributes default_subscriber_attributes;
std::map<std::string, XMLP_ret>         XMLProfileManager::m_xml_files;

BaseNode* XMLProfileManager::root = nullptr;

XMLP_ret XMLProfileManager::fillParticipantAttributes(const std::string &profile_name, ParticipantAttributes &atts)
{
    part_map_iterator_t it = m_participant_profiles.find(profile_name);
    if (it == m_participant_profiles.end())
    {
        logError(XMLPARSER, "Profile '" << profile_name << "' not found '");
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

void XMLProfileManager::getDefaultParticipantAttributes(ParticipantAttributes& participant_attributes)
{
    participant_attributes = default_participant_attributes;
}

void XMLProfileManager::getDefaultPublisherAttributes(PublisherAttributes& publisher_attributes)
{
    publisher_attributes = default_publisher_attributes;
}

void XMLProfileManager::getDefaultSubscriberAttributes(SubscriberAttributes& subscriber_attributes)
{
    subscriber_attributes = default_subscriber_attributes;
}

XMLP_ret XMLProfileManager::fillPublisherAttributes(const std::string &profile_name, PublisherAttributes &atts)
{
    publ_map_iterator_t it = m_publisher_profiles.find(profile_name);
    if (it == m_publisher_profiles.end())
    {
        logError(XMLPARSER, "Profile '" << profile_name << "' not found '");
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::fillSubscriberAttributes(const std::string &profile_name, SubscriberAttributes &atts)
{
    subs_map_iterator_t it = m_subscriber_profiles.find(profile_name);
    if (it == m_subscriber_profiles.end())
    {
        logError(XMLPARSER, "Profile '" << profile_name << "' not found");
        return XMLP_ret::XML_ERROR;
    }
    atts = *(it->second);
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::loadDefaultXMLFile()
{
    return loadXMLFile(DEFAULT_FASTRTPS_PROFILES);
}

XMLP_ret XMLProfileManager::loadXMLFile(const std::string& filename)
{
    if (filename.empty())
    {
        logError(XMLPARSER, "Error loading XML file, filename empty");
        return XMLP_ret::XML_ERROR;
    }

    xmlfile_map_iterator_t it = m_xml_files.find(filename);
    if (it != m_xml_files.end() && XMLP_ret::XML_OK == it->second)
    {
        logInfo(XMLPARSER, "XML file '" << filename << "' already parsed");
        return XMLP_ret::XML_OK;
    }

    up_base_node_t root_node;
    XMLParser::loadXML(filename, root_node);
    if (!root_node)
    {
        if (filename != std::string(DEFAULT_FASTRTPS_PROFILES))
        {
            logError(XMLPARSER, "Error parsing '" << filename << "'");
        }
        m_xml_files.emplace(filename, XMLP_ret::XML_ERROR);
        return XMLP_ret::XML_ERROR;
    }

    logInfo(XMLPARSER, "File '" << filename << "' parsed successfully");

    if (NodeType::PROFILES == root_node->getType())
    {
        return XMLProfileManager::extractProfiles(std::move(root_node), filename);
    }

    if (NodeType::ROOT == root_node->getType())
    {
        for (auto&& child: root_node->getChildren())
        {
            if (NodeType::PROFILES == child.get()->getType())
            {
                return XMLProfileManager::extractProfiles(std::move(child), filename);
            }
        }
    }

    return XMLP_ret::XML_ERROR;
}

XMLP_ret XMLProfileManager::extractProfiles(up_base_node_t profiles, const std::string& filename)
{
    if (nullptr == profiles)
    {
        logError(XMLPARSER, "Bad parameters");
        return XMLP_ret::XML_ERROR;
    }

    unsigned int profileCount = 0u;

    for (auto&& profile: profiles->getChildren())
    {
        if (NodeType::PARTICIPANT == profile->getType())
        {
            if (XMLP_ret::XML_OK == extractParticipantProfile(profile, filename))
            {
                ++profileCount;
            }
        }
        else if (NodeType::PUBLISHER == profile.get()->getType())
        {
            if (XMLP_ret::XML_OK == extractPublisherProfile(profile, filename))
            {
                ++profileCount;
            }
        }
        else if (NodeType::SUBSCRIBER == profile.get()->getType())
        {
            if (XMLP_ret::XML_OK == extractSubscriberProfile(profile, filename))
            {
                ++profileCount;
            }
        }
        else
        {
            logError(XMLPARSER, "Not expected tag");
        }
    }

    if (0 == profileCount)
    {
        m_xml_files.emplace(filename, XMLP_ret::XML_ERROR);
        logError(XMLPARSER, "Error, file '" << filename << "' bad content");
        return XMLP_ret::XML_ERROR;
    }

    m_xml_files.emplace(filename, XMLP_ret::XML_OK);

    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::extractParticipantProfile(up_base_node_t& profile, const std::string& filename)
{
    std::string profile_name = "";

    p_node_participant_t p_node_part = dynamic_cast<p_node_participant_t>(profile.get());
    node_att_map_cit_t it = p_node_part->getAttributes().find(PROFILE_NAME);
    if (it == p_node_part->getAttributes().end() || it->second.empty())
    {
        logError(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<part_map_iterator_t, bool> emplace = m_participant_profiles.emplace(profile_name, p_node_part->getData());
    if (false == emplace.second)
    {
        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    it = p_node_part->getAttributes().find(DEFAULT_PROF);
    if (it != p_node_part->getAttributes().end() && it->second == "true") // Set as default profile
    {
        // +V+ TODO: LOG ERROR IN SECOND ATTEMPT
        default_participant_attributes = *(emplace.first->second.get() );
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::extractPublisherProfile(up_base_node_t& profile, const std::string& filename)
{
    std::string profile_name = "";

    p_node_publisher_t p_node_part = dynamic_cast<p_node_publisher_t>(profile.get());
    node_att_map_cit_t it = p_node_part->getAttributes().find(PROFILE_NAME);
    if (it == p_node_part->getAttributes().end() || it->second.empty())
    {
        logError(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<publ_map_iterator_t, bool> emplace = m_publisher_profiles.emplace(profile_name, p_node_part->getData());
    if (false == emplace.second)
    {
        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    it = p_node_part->getAttributes().find(DEFAULT_PROF);
    if (it != p_node_part->getAttributes().end() && it->second == "true") // Set as default profile
    {
        // +V+ TODO: LOG ERROR IN SECOND ATTEMPT
        default_publisher_attributes = *(emplace.first->second.get() );
    }
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::extractSubscriberProfile(up_base_node_t& profile, const std::string& filename)
{
    std::string profile_name = "";

    p_node_subscriber_t p_node_part = dynamic_cast<p_node_subscriber_t>(profile.get());
    node_att_map_cit_t it = p_node_part->getAttributes().find(PROFILE_NAME);
    if (it == p_node_part->getAttributes().end() || it->second.empty())
    {
        logError(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
        return XMLP_ret::XML_ERROR;
    }

    profile_name = it->second;

    std::pair<subs_map_iterator_t, bool> emplace = m_subscriber_profiles.emplace(profile_name, p_node_part->getData());
    if (false == emplace.second)
    {
        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
        return XMLP_ret::XML_ERROR;
    }

    it = p_node_part->getAttributes().find(DEFAULT_PROF);
    if (it != p_node_part->getAttributes().end() && it->second == "true") // Set as default profile
    {
        // +V+ TODO: LOG ERROR IN SECOND ATTEMPT
        default_subscriber_attributes = *(emplace.first->second.get() );
    }
    return XMLP_ret::XML_OK;
}

} /* xmlparser  */
} /* namespace  */
} /* namespace eprosima */
