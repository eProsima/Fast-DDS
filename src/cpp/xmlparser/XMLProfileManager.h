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

#include <cstdio>
#include <map>
#include <string>

#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/LibrarySettings.hpp>

#include <xmlparser/attributes/ParticipantAttributes.hpp>
#include <xmlparser/attributes/PublisherAttributes.hpp>
#include <xmlparser/attributes/SubscriberAttributes.hpp>
#include <xmlparser/XMLParser.h>
#include <xmlparser/XMLParserCommon.h>

namespace eprosima {
namespace fastdds {
namespace xmlparser {

using participant_factory_map_t = std::map<std::string, up_participantfactory_t>;
using part_factory_map_iterator_t = participant_factory_map_t::iterator;
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
 * @ingroup XMLPARSER_MODULE
 */
class XMLProfileManager
{
public:

    /**
     * Load the default profiles XML file.
     */
    static void loadDefaultXMLFile();

    /**
     * Load a profiles XML file.
     * @param filename Name for the file to be loaded.
     * @return XMLP_ret::XML_OK if all profiles are correct, XMLP_ret::XML_NOK if some are and some are not,
     *         XMLP_ret::XML_ERROR in other case.
     */
    static XMLP_ret loadXMLFile(
            const std::string& filename);

    /**
     * Load a profiles XML file.
     * @param filename Name for the file to be loaded.
     * @param is_default Flag to indicate if the file is a default profiles file.
     * @return XMLP_ret::XML_OK if all profiles are correct, XMLP_ret::XML_NOK if some are and some are not,
     *         XMLP_ret::XML_ERROR in other case.
     */

    static XMLP_ret loadXMLFile(
            const std::string& filename,
            bool is_default);

    /**
     * Load a profiles XML string.
     * @param data Buffer containing the data.
     * @param length Length of data.
     * @return XMLP_ret::XML_OK if all profiles are correct, XMLP_ret::XML_NOK if some are and some are not,
     *         XMLP_ret::XML_ERROR in other case.
     */
    static XMLP_ret loadXMLString(
            const char* data,
            size_t length);

    /**
     * Load a profiles XML node.
     * @param doc Node to be loaded.
     * @return XMLP_ret::XML_OK if all profiles are correct, XMLP_ret::XML_NOK if some are and some are not,
     *         XMLP_ret::XML_ERROR in other case.
     */
    static XMLP_ret loadXMLNode(
            tinyxml2::XMLDocument& doc);

    /**
     * Load a profiles XML node.
     * @param profiles Node to be loaded.
     * @return XMLP_ret::XML_OK if all profiles are correct, XMLP_ret::XML_NOK if some are and some are not,
     *         XMLP_ret::XML_ERROR in other case.
     */
    static XMLP_ret loadXMLProfiles(
            tinyxml2::XMLElement& profiles);

    /**
     * Load a dynamic types XML node.
     * @param types Node to be loaded.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    static XMLP_ret loadXMLDynamicTypes(
            tinyxml2::XMLElement& types);

    /**
     * Library settings setter.
     * @param library_settings New value for library settings.
     */
    static void library_settings(
            const fastdds::LibrarySettings& library_settings);

    /**
     * Library settings getter.
     * @return const ref to current library settings.
     */
    static const fastdds::LibrarySettings& library_settings();

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @param log_error Flag to log an error if the profile_name is not found.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case. Defaults true.
     */
    static XMLP_ret fillParticipantAttributes(
            const std::string& profile_name,
            fastdds::ParticipantAttributes& atts,
            bool log_error = true);

    //!Fills participant_attributes with the default values.
    static void getDefaultParticipantAttributes(
            fastdds::ParticipantAttributes& participant_attributes);

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param qos Structure to be filled.
     * @param log_error Flag to log an error if the profile_name is not found. Defaults true.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    static XMLP_ret fillDomainParticipantFactoryQos(
            const std::string& profile_name,
            fastdds::dds::DomainParticipantFactoryQos& qos,
            bool log_error = true);

    /**
     * Fills input domain participant factory qos with the default values.
     * @param qos Structure to be filled.
     */
    static void getDefaultDomainParticipantFactoryQos(
            fastdds::dds::DomainParticipantFactoryQos& qos);

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @param log_error Flag to log an error if the profile_name is not found.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case. Defaults true.
     */
    static XMLP_ret fillPublisherAttributes(
            const std::string& profile_name,
            fastdds::PublisherAttributes& atts,
            bool log_error = true);

    //!Fills publisher_attributes with the default values.
    static void getDefaultPublisherAttributes(
            fastdds::PublisherAttributes& publisher_attributes);

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @param log_error Flag to log an error if the profile_name is not found.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case. Defaults true.
     */
    static XMLP_ret fillSubscriberAttributes(
            const std::string& profile_name,
            fastdds::SubscriberAttributes& atts,
            bool log_error = true);

    //!Fills subscriber_attributes with the default values.
    static void getDefaultSubscriberAttributes(
            fastdds::SubscriberAttributes& subscriber_attributes);

    //!Add a new transport instance along with its id.
    static bool insertTransportById(
            const std::string& transport_id,
            sp_transport_t transport);

    //!Retrieves a transport instance by its id.
    static sp_transport_t getTransportById(
            const std::string& transport_id);

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    static XMLP_ret fillTopicAttributes(
            const std::string& profile_name,
            TopicAttributes& atts);

    //!Fills topic_attributes with the default values.
    static void getDefaultTopicAttributes(
            TopicAttributes& topic_attributes);

    //!Add a new dynamic type instance along with its name.
    static bool insertDynamicTypeByName(
            const std::string& type_name,
            const fastdds::dds::DynamicType::_ref_type& type);

    static XMLP_ret getDynamicTypeByName(
            fastdds::dds::DynamicType::_ref_type& dynamic_type,
            const std::string& type_name);

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    static XMLP_ret fillRequesterAttributes(
            const std::string& profile_name,
            fastdds::RequesterAttributes& atts);

    /**
     * Search for the profile specified and fill the structure.
     * @param profile_name Name for the profile to be used to fill the structure.
     * @param atts Structure to be filled.
     * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
     */
    static XMLP_ret fillReplierAttributes(
            const std::string& profile_name,
            fastdds::ReplierAttributes& atts);

    /**
     * Deletes the XMLProfileManager instance.
     * FastDDS's Domain calls this method automatically on its destructor, but
     * if using XMLProfileManager outside of FastDDS, it should be called manually.
     */
    FASTDDS_EXPORTED_API static void DeleteInstance();

private:

    static XMLP_ret extractProfiles(
            up_base_node_t properties,
            const std::string& filename);

    static XMLP_ret extractDomainParticipantFactoryProfile(
            up_base_node_t& profile,
            const std::string& filename);

    static XMLP_ret extractParticipantProfile(
            up_base_node_t& profile,
            const std::string& filename);

    static XMLP_ret extractPublisherProfile(
            up_base_node_t& profile,
            const std::string& filename);

    static XMLP_ret extractSubscriberProfile(
            up_base_node_t& profile,
            const std::string& filename);

    static XMLP_ret extractTopicProfile(
            up_base_node_t& profile,
            const std::string& filename);

    static XMLP_ret extractRequesterProfile(
            up_base_node_t& profile,
            const std::string& filename);

    static XMLP_ret extractReplierProfile(
            up_base_node_t& profile,
            const std::string& filename);

    static BaseNode* root;

    static fastdds::LibrarySettings library_settings_;

    static participant_factory_map_t participant_factory_profiles_;

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
