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

namespace eprosima {
namespace fastrtps {
namespace xmlparser {

std::map<std::string, ParticipantAttributes> XMLProfileManager::m_participant_profiles;
ParticipantAttributes default_participant_attributes;
std::map<std::string, PublisherAttributes>   XMLProfileManager::m_publisher_profiles;
PublisherAttributes default_publisher_attributes;
std::map<std::string, SubscriberAttributes>  XMLProfileManager::m_subscriber_profiles;
SubscriberAttributes default_subscriber_attributes;
std::map<std::string, XMLP_ret>              XMLProfileManager::m_xml_files;

BaseNode* XMLProfileManager::root = nullptr;

XMLP_ret XMLProfileManager::fillParticipantAttributes(const std::string &profile_name, ParticipantAttributes &atts)
{
    part_map_iterator_t it = m_participant_profiles.find(profile_name);
    if (it == m_participant_profiles.end())
    {
        logError(XMLPARSER, "Profile '" << profile_name << "' not found '");
        return XMLP_ret::XML_ERROR;
    }
    atts = it->second;
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
    atts = it->second;
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
    atts = it->second;
    return XMLP_ret::XML_OK;
}

XMLP_ret XMLProfileManager::loadDefaultXMLFile()
{
    return loadXMLFile(DEFAULT_FASTRTPS_PROFILES);
}

XMLP_ret XMLProfileManager::loadXMLFile(const std::string &filename)
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

    base_node_uptr_t root_node = XMLParser::parseXML(filename);
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
        return XMLProfileManager::extractProfiles(std::move(root_node));
    }

    if (NodeType::ROOT == root_node->getType())
    {
        for (auto&& child: root_node->getChildren())
        {
            if (NodeType::PROFILES == child.get()->getType())
            {
                return XMLProfileManager::extractProfiles(std::move(child));
            }
        }
    }

    return XMLP_ret::XML_ERROR;
}

XMLP_ret XMLProfileManager::extractProfiles(base_node_uptr_t profiles)
{
    if (nullptr == profiles)
    {
        logError(XMLPARSER, "Bad parameters");
        return XMLP_ret::XML_ERROR;
    }

    std::string profile_name = "";
    unsigned int profileCount = 0u;

    for (auto&& profile: profiles->getChildren())
    {
        if (NodeType::PARTICIPANT == profile->getType())
        {

            node_att_cit_t it = dynamic_cast<Node<ParticipantAttributes>*>(profile.get())->getAttributes().find(PROFILE_NAME);
            if (it == profile->attributes.end() || (*it).empty())
            {
                logError(XMLPARSER, "Error adding profile from file '" << filename << "': no name found");
                continue;
            }

            profile_name = *it;

            std::pair<part_map_iterator_t, bool> res = m_participant_profiles.emplace(profile_name, profile.getData());
            if (false ==res.second)
            {
                logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
                continue;
            }

            it = profile.attributes.find(DEFAULT_PROF);
            if (it != profile.attributes.end() && it->second == "true") // Set as default profile
            {
                // +V+ TODO: LOG ERROR IN SECOND ATTEMPT
                default_participant_attributes = *res.first;
            }

            ++profileCount;
        }
        else if (NodeType::PUBLISHER == profile.get()->getType())
        {

        }
        else if (NodeType::SUBSCRIBER == profile.get()->getType())
        {

        }
        else
        {
            logError(XMLPARSER, "Not expected tag: '" << tag << "'");
        }
    }


/*    root = new Node<int>(nullptr, NodeType::ROOT);
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
                        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
                    }
                    ++profileCount;
                }
                else
                {
                    logError(XMLPARSER, "Error parsing participant profile");
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
                        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
                    }
                    ++profileCount;
                }
                else
                {
                    logError(XMLPARSER, "Error parsing publisher profile");
                }
            }
            else if (strcmp(tag, SUBSCRIBER) == 0)
            {
                SubscriberAttributes subscriber_atts;
                if (XMLP_ret::XML_OK == parseXMLSubscriberProf(p_profile, subscriber_atts, profile_name))
                {
                    if(is_default_profile)
                    {
                        default_subscriber_attributes = subscriber_atts;
                    }

                    if (false == m_subscriber_profiles.emplace(profile_name, subscriber_atts).second)
                    {
                        logError(XMLPARSER, "Error adding profile '" << profile_name << "' from file '" << filename << "'");
                    }
                    ++profileCount;
                }
                else
                {
                    logError(XMLPARSER, "Error parsing subscriber profile");
                }
            }
            else
            {
                logError(XMLPARSER, "Not expected tag: '" << tag << "'");
            }
        }
        p_profile = p_profile->NextSiblingElement();
    }*/

    if (0 == profileCount)
    {
        m_xml_files.emplace(filename, XMLP_ret::XML_ERROR);
        logError(XMLPARSER, "Bad file '" << filename << "' content expected tag: '" << tag << "'");
        return XMLP_ret::XML_ERROR;

    }

    m_xml_files.emplace(filename, XMLP_ret::XML_OK);

    return XMLP_ret::XML_OK;
}

} /* xmlparser  */
} /* namespace  */
} /* namespace eprosima */
