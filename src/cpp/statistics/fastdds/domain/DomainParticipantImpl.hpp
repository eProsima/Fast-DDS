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
 * @file DomainParticipantImpl.hpp
 */

#ifndef _FASTDDS_STADISTICS_FASTDDS_DOMAIN_DOMAINPARTICIPANTIMPL_HPP_
#define _FASTDDS_STADISTICS_FASTDDS_DOMAIN_DOMAINPARTICIPANTIMPL_HPP_

#include <fastrtps/config.h>

#ifdef FASTDDS_STATISTICS

#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/types/TypesBase.h>

#include <fastdds/domain/DomainParticipantImpl.hpp>

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

class DomainParticipantImpl : public eprosima::fastdds::dds::DomainParticipantImpl
{
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
    ReturnCode_t enable_statistics_datawriter(
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
    ReturnCode_t disable_statistics_datawriter(
            const std::string& topic_name);

    virtual ReturnCode_t enable() override;

protected:

    /**
     * Constructor
     */
    DomainParticipantImpl(
            eprosima::fastdds::dds::DomainParticipant* dp,
            eprosima::fastdds::dds::DomainId_t did,
            const eprosima::fastdds::dds::DomainParticipantQos& qos,
            eprosima::fastdds::dds::DomainParticipantListener* listen = nullptr)
        : eprosima::fastdds::dds::DomainParticipantImpl(dp, did, qos, listen)
        , builtin_publisher_(nullptr)
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
            const std::string& topic) noexcept;

    /**
     * Auxiliary function to check if the topic name provided is a valid one.
     * @param topic string with the statistic topic name.
     * @return true if the topic name provided is valid.
     * false otherwise.
     */
    bool check_statistics_topic_name(
            const std::string& topic) noexcept;

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
            const std::string& topic_name) noexcept;

    /**
     * Auxiliary function that checks if the topic is already created within the domain participant.
     * If it is not, it creates the topic and registers the type. It also checks if the type can be registered.
     * If succesfull, it returns the pointer to the found or created topic.
     * @param[out] topic pointer to the found or created topic pointer.
     * @param[in] topic_name string with the topic name to find or create.
     * @param[in] type TypeSupport to register.
     * @return false if the topic is found but uses another type different from the expected one or if register_type
     * fails because there is another TypeSupport using the same name. true otherwise.
     */
    bool find_or_create_topic_and_type(
            eprosima::fastdds::dds::Topic** topic,
            const std::string& topic_name,
            const eprosima::fastdds::dds::TypeSupport& type) noexcept;

    /**
     * Auxiliary method to delete a topic and deregister a type after checking that they are consistent.
     * @param topic_name string with the topic name.
     * @param type_name string with the type name.
     * @return false if the type and topic do not match or if delete_topic fails. Otherwise true.
     */
    bool delete_topic_and_type(
            const std::string& topic_name) noexcept;

    eprosima::fastdds::dds::Publisher* builtin_publisher_;

    friend class eprosima::fastdds::dds::DomainParticipantFactory;
};

/* Environment variable to specify a semicolon-separated list of topic names that define the statistics DataWriters that
 * the DomainParticipant will enable when enabled itself.
 * The topic names must conform to the topic names aliases defined in @ref topic_names.hpp.
 * For the variable to take any effect the CMake option FASTDDS_STATISTICS must be enabled.
 */
const char* const FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE = "FASTDDS_STATISTICS";

} // dds
} // statistics
} // fastdds
} // eprosima

#endif // FASTDDS_STATISTICS

#endif // _FASTDDS_STADISTICS_FASTDDS_DOMAIN_DOMAINPARTICIPANTIMPL_HPP_
