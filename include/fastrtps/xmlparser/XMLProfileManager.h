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
#ifndef XML_PROFILE_MANAGER_H_
#define XML_PROFILE_MANAGER_H_

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/xmlparser/XMLParserCommon.h>
#include <fastrtps/xmlparser/XMLParser.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilder.h>
#include <fastrtps/types/DynamicPubSubType.h>

#include <stdio.h>
#include <string>
#include <map>

namespace eprosima {
namespace fastrtps {
namespace xmlparser {

using participant_map_t = std::map<std::string, up_participant_t>;
using part_map_iterator_t = participant_map_t::iterator;
using publisher_map_t = std::map<std::string, up_publisher_t>;
using publ_map_iterator_t = publisher_map_t::iterator;
using subscriber_map_t = std::map<std::string, up_subscriber_t>;
using subs_map_iterator_t = subscriber_map_t::iterator;
using topic_map_t = std::map<std::string, up_topic_t>;
using topic_map_iterator_t = topic_map_t::iterator;
using requester_map_t = std::map<std::string, up_requester_t>;
using requester_map_iterator_t = requester_map_t::iterator;
using replier_map_t = std::map<std::string, up_replier_t>;
using replier_map_iterator_t = replier_map_t::iterator;
using xmlfiles_map_t = std::map<std::string, XMLP_ret>;
using xmlfile_map_iterator_t = xmlfiles_map_t::iterator;


/**
 * Class XMLProfileManager, used to make available profiles from XML file.
 * @ingroup XMLPROFILEMANAGER_MODULE
 */
class XMLProfileManager
{
public:

    /**
     * Load the default profiles XML file.
     */
    RTPS_DllAPI static void loadDefaultXMLFile();

    /**
     * Load a profiles XML file.
     * @param filename Name for the file to be loaded.
     * @return XMLP_ret::XML_OK if all profiles are correct, XMLP_ret::XML_NOK if some are and some are not,
     *         XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadXMLFile(
            const std::string& filename);

    /**
     * Load a profiles XML node.
     * @param doc Node to be loaded.
     * @return XMLP_ret::XML_OK if all profiles are correct, XMLP_ret::XML_NOK if some are and some are not,
     *         XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadXMLNode(
            tinyxml2::XMLDocument& doc);

    /**
     * Load a profiles XML node.
     * @param profiles Node to be loaded.
     * @return XMLP_ret::XML_OK if all profiles are correct, XMLP_ret::XML_NOK if some are and some are not,
     *         XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadXMLProfiles(
            tinyxml2::XMLElement& profiles);

    /**
     * Load a dynamic types XML node.
     * @param types Node to be loaded.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret loadXMLDynamicTypes(
            tinyxml2::XMLElement& types);

    /**
     * Library settings setter.
     * @param library_settings New value for library settings.
     */
    RTPS_DllAPI static void library_settings(
            const LibrarySettingsAttributes& library_settings);

    /**
     * Library settings getter.
     * @return const ref to current library settings.
     */
    RTPS_DllAPI static const LibrarySettingsAttributes& library_settings();

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @param log_error Flag to log an error if the profile_name is not found.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case. Defaults true.
     */
    RTPS_DllAPI static XMLP_ret fillParticipantAttributes(
            const std::string& profile_name,
            ParticipantAttributes& atts,
            bool log_error = true);

    //!Fills participant_attributes with the default values.
    RTPS_DllAPI static void getDefaultParticipantAttributes(
            ParticipantAttributes& participant_attributes);

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @param log_error Flag to log an error if the profile_name is not found.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case. Defaults true.
     */
    RTPS_DllAPI static XMLP_ret fillPublisherAttributes(
            const std::string& profile_name,
            PublisherAttributes& atts,
            bool log_error = true);

    //!Fills publisher_attributes with the default values.
    RTPS_DllAPI static void getDefaultPublisherAttributes(
            PublisherAttributes& publisher_attributes);

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @param log_error Flag to log an error if the profile_name is not found.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case. Defaults true.
     */
    RTPS_DllAPI static XMLP_ret fillSubscriberAttributes(
            const std::string& profile_name,
            SubscriberAttributes& atts,
            bool log_error = true);

    //!Fills subscriber_attributes with the default values.
    RTPS_DllAPI static void getDefaultSubscriberAttributes(
            SubscriberAttributes& subscriber_attributes);

    //!Add a new transport instance along with its id.
    RTPS_DllAPI static bool insertTransportById(
            const std::string& transport_id,
            sp_transport_t transport);

    //!Retrieves a transport instance by its id.
    RTPS_DllAPI static sp_transport_t getTransportById(
            const std::string& transport_id);

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret fillTopicAttributes(
            const std::string& profile_name,
            TopicAttributes& atts);

    //!Fills topic_attributes with the default values.
    RTPS_DllAPI static void getDefaultTopicAttributes(
            TopicAttributes& topic_attributes);

    //!Add a new dynamic type instance along with its name.
    RTPS_DllAPI static bool insertDynamicTypeByName(
            const std::string& type_name,
            p_dynamictypebuilder_t type);

    //!Retrieves a transport instance by its name.
    RTPS_DllAPI static p_dynamictypebuilder_t getDynamicTypeByName(
            const std::string& type_name);


    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret fillRequesterAttributes(
            const std::string& profile_name,
            RequesterAttributes& atts);

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    RTPS_DllAPI static XMLP_ret fillReplierAttributes(
            const std::string& profile_name,
            ReplierAttributes& atts);

    /**
     * Deletes the XMLProsileManager instance.
     * FastRTPS's Domain calls this method automatically on its destructor, but
     * if using XMLProfileManager outside of FastRTPS, it should be called manually.
     */
    RTPS_DllAPI static void DeleteInstance()
    {
        participant_profiles_.clear();
        publisher_profiles_.clear();
        subscriber_profiles_.clear();
        requester_profiles_.clear();
        replier_profiles_.clear();
        xml_files_.clear();
        transport_profiles_.clear();
    }

    /**
     * Retrieves a DynamicPubSubType for the given dynamic type name.
     * Any instance retrieve by calling this method must be deleted calling the
     * XMLProfileManager::DeleteDynamicPubSubType method.
     */
    RTPS_DllAPI static types::DynamicPubSubType* CreateDynamicPubSubType(
            const std::string& type_name)
    {
        if (dynamic_types_.find(type_name) != dynamic_types_.end())
        {
            return new types::DynamicPubSubType(dynamic_types_[type_name]->build());
        }
        return nullptr;
    }

    /**
     * Deletes the given DynamicPubSubType previously created by calling
     * XMLProfileManager::CreateDynamicPubSubType method.
     */
    RTPS_DllAPI static void DeleteDynamicPubSubType(
            types::DynamicPubSubType* type)
    {
        delete type;
    }

private:

    RTPS_DllAPI static XMLP_ret extractDynamicTypes(
            up_base_node_t properties,
            const std::string& filename);

    RTPS_DllAPI static XMLP_ret extractProfiles(
            up_base_node_t properties,
            const std::string& filename);

    RTPS_DllAPI static XMLP_ret extractParticipantProfile(
            up_base_node_t& profile,
            const std::string& filename);

    RTPS_DllAPI static XMLP_ret extractPublisherProfile(
            up_base_node_t& profile,
            const std::string& filename);

    RTPS_DllAPI static XMLP_ret extractSubscriberProfile(
            up_base_node_t& profile,
            const std::string& filename);

    RTPS_DllAPI static XMLP_ret extractTopicProfile(
            up_base_node_t& profile,
            const std::string& filename);

    RTPS_DllAPI static XMLP_ret extractRequesterProfile(
            up_base_node_t& profile,
            const std::string& filename);

    RTPS_DllAPI static XMLP_ret extractReplierProfile(
            up_base_node_t& profile,
            const std::string& filename);

    static BaseNode* root;

    static LibrarySettingsAttributes library_settings_;

    static participant_map_t participant_profiles_;

    static publisher_map_t publisher_profiles_;

    static subscriber_map_t subscriber_profiles_;

    static topic_map_t topic_profiles_;

    static requester_map_t requester_profiles_;

    static replier_map_t replier_profiles_;

    static xmlfiles_map_t xml_files_;

    static sp_transport_map_t transport_profiles_;

    static p_dynamictype_map_t dynamic_types_;
};

} /* xmlparser */
} /* namespace */
} /* namespace eprosima */

#endif // ifndef XML_PROFILE_MANAGER_H_
