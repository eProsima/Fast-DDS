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

#include <fastdds/config.hpp>

#ifdef FASTDDS_STATISTICS

#include <string>

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/domain/DomainParticipantImpl.hpp>

#include "DomainParticipantStatisticsListener.hpp"
#include <statistics/rtps/monitor-service/Interfaces.hpp>

namespace efd = eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace statistics {

class MonitorServiceStatusData;

namespace dds {

class PublisherImpl;

class DomainParticipantImpl : public efd::DomainParticipantImpl,
    public rtps::IStatusQueryable
{
public:

    /**
     * @brief This operation enables a Statistics DataWriter
     *
     * @param [in] topic_name Name of the topic associated to the Statistics DataWriter
     * @param [in] dwqos DataWriterQos to be set
     * @return RETCODE_BAD_PARAMETER if the topic name provided does not correspond to any Statistics DataWriter,
     * RETCODE_INCONSISTENT_POLICY if the DataWriterQos provided is inconsistent,
     * RETCODE_OK if the DataWriter has been created or if it has been created previously,
     * and RETCODE_ERROR otherwise
     */
    efd::ReturnCode_t enable_statistics_datawriter(
            const std::string& topic_name,
            const efd::DataWriterQos& dwqos);

    /**
     * @brief This operation enables a Statistics DataWriter from a provided XML defined profile
     *
     * @param [in] profile_name Name for the profile to be used to fill the QoS structure.
     * @param [in] topic_name Name of the statistics topic to be enabled.
     * @return RETCODE_BAD_PARAMETER if the topic name provided does not correspond to any Statistics DataWriter,
     * RETCODE_INCONSISTENT_POLICY if the DataWriterQos provided is inconsistent,
     * RETCODE_OK if the DataWriter has been created or if it has been created previously,
     * and RETCODE_ERROR otherwise
     */
    efd::ReturnCode_t enable_statistics_datawriter_with_profile(
            const std::string& profile_name,
            const std::string& topic_name);

    /**
     * @brief This operation disables a Statistics DataWriter
     *
     * @param [in] topic_name Name of the topic associated to the Statistics DataWriter
     * @return RETCODE_UNSUPPORTED if the FASTDDS_STATISTICS CMake option has not been set,
     * RETCODE_BAD_PARAMETER if the topic name provided does not correspond to any Statistics DataWriter,
     * RETCODE_OK if the DataWriter has been correctly deleted or does not exist,
     * and RETCODE_ERROR otherwise
     */
    efd::ReturnCode_t disable_statistics_datawriter(
            const std::string& topic_name);

    /**
     * @brief This operation enables the DomainParticipantImpl
     *
     * @return RETCODE_OK if successful
     */
    efd::ReturnCode_t enable() override;

    void disable() override;

    /**
     * Auxiliary function that checks if a topic name corresponds to a statistics builtin topic name.
     * @param [in]   topic_name string with the topic name to check.
     * @return whether the input string corresponds to a builtin statistics topic name.
     */
    static bool is_statistics_topic_name(
            const std::string& topic_name) noexcept;

    /**
     * @brief This override calls the parent method and returns builtin publishers to nullptr
     *
     * @return RETCODE_OK if successful
     * @note This method is meant to be used followed by a deletion of the participant as it implies
     * the deletion of the builtin statistics publishers.
     */
    efd::ReturnCode_t delete_contained_entities() override;

    /**
     * Enables the monitor service in this DomainParticipant.
     *
     * @return RETCODE_OK if the monitor service could be correctly enabled.
     * @return RETCODE_ERROR if the monitor service could not be enabled properly.
     * @return RETCODE_UNSUPPORTED if FASTDDS_STATISTICS is not enabled.
     *
     */
    efd::ReturnCode_t enable_monitor_service();

    /**
     * Disables the monitor service in this DomainParticipant. Does nothing if the service was not enabled before.
     *
     * @return RETCODE_OK if the monitor service could be correctly disabled.
     * @return RETCODE_NOT_ENABLED if the monitor service was not previously enabled.
     * @return RETCODE_ERROR if the service could not be properly disabled.
     * @return RETCODE_UNSUPPORTED if FASTDDS_STATISTICS is not enabled.
     *
     */
    efd::ReturnCode_t disable_monitor_service();

    /**
     * fills in the ParticipantBuiltinTopicData from a MonitorService Message
     *
     * @param [out] data Proxy to fill
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return RETCODE_OK if the operation succeeds.
     * @return RETCODE_ERROR if the  operation fails.
     */
    efd::ReturnCode_t fill_discovery_data_from_cdr_message(
            fastdds::rtps::ParticipantBuiltinTopicData& data,
            const fastdds::statistics::MonitorServiceStatusData& msg);

    /**
     * fills in the WriterProxyData from a MonitorService Message
     *
     * @param [out] data Proxy to fill.
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return RETCODE_OK if the operation succeeds.
     * @return RETCODE_ERROR if the  operation fails.
     */
    efd::ReturnCode_t fill_discovery_data_from_cdr_message(
            fastdds::dds::PublicationBuiltinTopicData& data,
            const fastdds::statistics::MonitorServiceStatusData& msg);

    /**
     * fills in the SubscriptionBuiltinTopicData from a MonitorService Message
     *
     * @param [out] data Proxy to fill.
     * @param [in] msg MonitorService Message to get the proxy information from.
     *
     * @return RETCODE_OK if the operation succeeds.
     * @return RETCODE_ERROR if the  operation fails.
     */
    efd::ReturnCode_t fill_discovery_data_from_cdr_message(
            fastdds::dds::SubscriptionBuiltinTopicData& data,
            const fastdds::statistics::MonitorServiceStatusData& msg);

    /**
     * Gets the status observer for that entity
     *
     * @return status observer
     */

    const rtps::IStatusObserver* get_status_observer()
    {
        return status_observer_.load();
    }

protected:

    /**
     * Constructor
     */
    DomainParticipantImpl(
            efd::DomainParticipant* dp,
            efd::DomainId_t did,
            const efd::DomainParticipantQos& qos,
            efd::DomainParticipantListener* listen = nullptr)
        : efd::DomainParticipantImpl(dp, did, qos, listen)
        , builtin_publisher_(nullptr)
        , statistics_listener_(std::make_shared<DomainParticipantStatisticsListener>())
    {
    }

    efd::PublisherImpl* create_publisher_impl(
            const efd::PublisherQos& qos,
            efd::PublisherListener* listener) override;

    efd::SubscriberImpl* create_subscriber_impl(
            const efd::SubscriberQos& qos,
            efd::SubscriberListener* listener) override;

    /**
     * Auxiliary function to create the statistics builtin entities.
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
     * @param [in]   topic_name_or_alias string with the statistic topic name or alias.
     * @param [out]  topic_name          string with the corresponding topic name to use.
     * @param [out]  event_kind          statistics event kind corresponding to the topic name.
     * @return whether the input string corresponds to a valid topic name or alias.
     */
    bool transform_and_check_topic_name(
            const std::string& topic_name_or_alias,
            std::string& topic_name,
            uint32_t& event_kind) noexcept;

    /**
     * Auxiliary function to register the statistics type depending on the statistics topic name.
     * It also creates the topic (or finds it if already created) and returns the pointer.
     * @param [out] topic pointer to the created topic pointer.
     * If the method returns false the parameter is not modified.
     * @param [in] topic_name string with the statistics topic name.
     * @return true if successful.
     * false in case there is incompatibility between the type associated to the Topic and the expected type.
     */
    bool register_statistics_type_and_topic(
            efd::Topic** topic,
            const std::string& topic_name) noexcept;

    /**
     * Auxiliary function that checks if the topic is already created within the domain participant.
     * If it is not, it creates the topic and registers the type. It also checks if the type can be registered.
     * If succesfull, it returns the pointer to the found or created topic.
     * @param [out] topic pointer to the found or created topic pointer.
     * @param [in] topic_name string with the topic name to find or create.
     * @param [in] type TypeSupport to register.
     * @return false if the topic is found but uses another type different from the expected one or if register_type
     * fails because there is another TypeSupport using the same name. true otherwise.
     */
    bool find_or_create_topic_and_type(
            efd::Topic** topic,
            const std::string& topic_name,
            const efd::TypeSupport& type) noexcept;

    /**
     * Auxiliary method to delete a topic and deregister a type after checking that they are consistent.
     * @param topic_name string with the topic name.
     * @param type_name string with the type name.
     * @return false if the type and topic do not match or if delete_topic fails. Otherwise true.
     */
    bool delete_topic_and_type(
            const std::string& topic_name) noexcept;

    /**
     * @brief Implementation of the IStatusQueryable interface.
     */
    bool get_monitoring_status(
            const fastdds::rtps::GUID_t& entity_guid,
            eprosima::fastdds::statistics::MonitorServiceData&) override;

    efd::Publisher* builtin_publisher_ = nullptr;
    PublisherImpl* builtin_publisher_impl_ = nullptr;
    std::shared_ptr<DomainParticipantStatisticsListener> statistics_listener_;
    std::atomic<const rtps::IStatusObserver*> status_observer_{nullptr};

    friend class efd::DomainParticipantFactory;
};

/* Environment variable to specify a semicolon-separated list of topic names that define the statistics DataWriters that
 * the DomainParticipant will enable when enabled itself.
 * The topic names must conform to the topic names aliases defined in @ref topic_names.hpp.
 * For the variable to take any effect the CMake option FASTDDS_STATISTICS must be enabled.
 */
constexpr const char* FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE = "FASTDDS_STATISTICS";

} // dds
} // statistics
} // fastdds
} // eprosima

#endif // FASTDDS_STATISTICS

#endif // _FASTDDS_STADISTICS_FASTDDS_DOMAIN_DOMAINPARTICIPANTIMPL_HPP_
