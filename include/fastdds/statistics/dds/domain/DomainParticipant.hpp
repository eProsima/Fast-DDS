// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file DomainParticipant.hpp
 *
 */

#ifndef _FASTDDS_STATISTICS_DDS_DOMAIN_DOMAINPARTICIPANT_HPP_
#define _FASTDDS_STATISTICS_DDS_DOMAIN_DOMAINPARTICIPANT_HPP_

#include <string>

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/types/TypesBase.h>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

/**
 * Class DomainParticipant: extends standard DDS DomainParticipant class to include specific methods for the Statistics
 * module
 * @ingroup STATISTICS_MODULE
 */
class DomainParticipant : public eprosima::fastdds::dds::DomainParticipant
{
    DomainParticipant() = delete;

public:

    /**
     * @brief This operation enables a Statistics DataWriter
     * @param topic_name Name of the topic associated to the Statistics DataWriter
     * @param dwqos DataWriterQos to be set
     * @return RETCODE_UNSUPPORTED if the FASTDDS_STATISTICS CMake option has not been set,
     * RETCODE_BAD_PARAMETER if the topic name provided does not correspond to any Statistics DataWriter,
     * RETCODE_INCONSISTENT_POLICY if the DataWriterQos provided is inconsistent,
     * RETCODE_OK if the DataWriter has been created or if it has been created previously,
     * and RETCODE_ERROR otherwise
     */
    RTPS_DllAPI ReturnCode_t enable_statistics_datawriter(
            const std::string& topic_name,
            const eprosima::fastdds::dds::DataWriterQos& dwqos);

    /**
     * @brief This operation disables a Statistics DataWriter
     * @param topic_name Name of the topic associated to the Statistics DataWriter
     * @return RETCODE_UNSUPPORTED if the FASTDDS_STATISTICS CMake option has not been set,
     * RETCODE_BAD_PARAMETER if the topic name provided does not correspond to any Statistics DataWriter,
     * RETCODE_OK if the DataWriter has been correctly deleted or does not exist,
     * and RETCODE_ERROR otherwise
     */
    RTPS_DllAPI ReturnCode_t disable_statistics_datawriter(
            const std::string& topic_name);

    /**
     * @brief This operation narrows the DDS DomainParticipant to the Statistics DomainParticipant
     * @param domain_participant Reference to the DDS DomainParticipant
     * @return Reference to the Statistics DomainParticipant if successful.
     * nullptr otherwise.
     */
    RTPS_DllAPI static DomainParticipant* narrow(
            eprosima::fastdds::dds::DomainParticipant* domain_participant);

    /**
     * @brief This operation narrows the DDS DomainParticipant to the Statistics DomainParticipant
     * @param domain_participant Constant reference to the DDS DomainParticipant
     * @return Constant reference to the Statistics DomainParticipant if successful.
     * nullptr otherwise.
     */
    RTPS_DllAPI static const DomainParticipant* narrow(
            const eprosima::fastdds::dds::DomainParticipant* domain_participant);

    /**
     * @brief This operation enables the statistics DomainParticipant
     * @return RETCODE_OK
     */
    RTPS_DllAPI ReturnCode_t enable() override;

protected:

    /**
     * Constructor
     */
    DomainParticipant(
            const eprosima::fastdds::dds::StatusMask& mask)
        : eprosima::fastdds::dds::DomainParticipant(mask)
    {
    }

    /**
     * Auxiliary function to create the statistics builtin entities.
     * @return true if succesfully created statistics DDS entities,
     * false otherwise.
     */
    void create_statistics_builtin_entities();

    /**
     * Auxiliary function to enable statistics builtin datawriters.
     * @param topic_list string with the semicolon separated list of statistics topics.
     */
    void enable_statistics_builtin_datawriters(
            const std::string& topic_list);

    /**
     * Auxiliary function to delete the statistics builtin entities.
     */
    void delete_statistics_builtin_entities();

    /**
     * Auxiliary function that transforms the topic alias to the topic name.
     * @param topic string with the statistic topic name or alias.
     * @return string with the corresponding topic name if an alias has been used.
     * Otherwise, the same string passed as parameter is returned.
     */
    const std::string transform_topic_name_alias(
            const std::string& topic);

    /**
     * Auxiliary function to check if the topic name provided is a valid one.
     * @param topic string with the statistic topic name.
     * @return true if the topic name provided is valid.
     * false otherwise.
     */
    bool check_statistics_topic_name(
            const std::string& topic);

    /**
     * Auxiliary function to register the statistics type depending on the statistics topic name.
     * It also creates the topic (or finds it if already created) and returns the pointer.
     * @param[out] topic pointer to the created topic pointer.
     * If the method returns false the parameter is not modified.
     * @param[in] topic_name string with the statistics topic name.
     * @return true if successful.
     * false in case there is incompatibility between the type associated to the Topic and the expected type.
     */
    bool register_statistics_type_and_topic(
            eprosima::fastdds::dds::Topic** topic,
            const std::string& topic_name);

    /**
     * Auxiliary function that checks if the topic is already created within the domain participant.
     * If it is not, it creates the topic. Else, it returns the pointer to the found topic.
     * @param[out] topic pointer to the topic found or created pointer.
     * @param[in] topic_name string with the topic name to find or create.
     * @param[in] type_name string with the type name to check that the topic is associated with the expected type.
     * @return false if the topic is found but uses another type different from the expected one.
     * true otherwise.
     */
    bool find_or_create_topic(
            eprosima::fastdds::dds::Topic** topic,
            const std::string& topic_name,
            const std::string& type_name);

    /**
     * Auxiliary function to deregister statistic type and topic.
     * @param topic_name string with the statistics topic name.
     * @return true if successful. False otherwise.
     */
    bool deregister_statistics_type_and_topic(
            const std::string& topic_name);

    /**
     * Auxiliary method to delete a topic and deregister a type after checking that they are consistent.
     * @param topic_name string with the topic name.
     * @param type_name string with the type name.
     * @return false if the type and topic do not match or if delete_topic fails. Otherwise true.
     */
    bool delete_topic_and_type(
            const std::string& topic_name,
            const std::string& type_name);

    /**
     * Auxiliary function to check that the topic has been registered with the expected type.
     * @param topic_desc pointer to the TopicDescription found in the participant.
     * @param topic_name string with the statistics topic name.
     * @param type_name string with the type name to check that the topic is associated with the expected type.
     * @return true if the topic and type are consistent. False otherwise.
     */
    bool check_statistics_topic_and_type(
            const eprosima::fastdds::dds::TopicDescription* topic_desc,
            const std::string& topic_name,
            const std::string& type_name);

    eprosima::fastdds::dds::Publisher* builtin_publisher_;

    friend class eprosima::fastdds::dds::DomainParticipantFactory;
};

/* Environment variable to specify a semicolon-separated list of topic names that define the statistics DataWriters that
 * the DomainParticipant will enable when enabled itself.
 * The topic names must conform to the topic names aliases defined in @ref topic_names.hpp.
 * For the variable to take any effect the CMake option FASTDDS_STATISTICS must be enabled.
 */
const char* const FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE = "FASTDDS_STATISTICS";

} // namespace dds
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_STATISTICS_DDS_DOMAIN_DOMAINPARTICIPANT_HPP_ */
