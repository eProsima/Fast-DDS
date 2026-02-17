// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_RPC__SERVICEIMPL_HPP
#define FASTDDS_RPC__SERVICEIMPL_HPP

#include <mutex>

#include <fastdds/dds/domain/qos/ReplierQos.hpp>
#include <fastdds/dds/domain/qos/RequesterQos.hpp>
#include <fastdds/dds/rpc/Service.hpp>
#include <fastdds/dds/topic/ContentFilteredTopic.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "../domain/DomainParticipantImpl.hpp"
#include "../publisher/PublisherImpl.hpp"
#include "../subscriber/SubscriberImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

class ReplierImpl;
class RequesterImpl;

/**
 * @brief Class that represents the implementation of a Service entity
 */
class ServiceImpl : public Service
{

public:

    /**
     * @brief Constructor
     * Don't use it directly, use create_service from DomainParticipant instead
     */
    ServiceImpl(
            const std::string& service_name,
            const std::string& service_type_name,
            DomainParticipantImpl* participant,
            PublisherImpl* service_publisher,
            SubscriberImpl* service_subscriber);

    /**
     * @brief Destructor
     */
    virtual ~ServiceImpl();

    /**
     * @brief Get the service name
     *
     * @return Service name
     */
    const std::string& get_service_name() const override
    {
        return service_name_;
    }

    /**
     * @brief Get the service type name
     *
     * @return Service type name
     */
    const std::string& get_service_type_name() const override
    {
        return service_type_name_;
    }

    /**
     * @brief Remove a requester from the service
     *
     * @param requester Requester to remove
     * @return RETCODE_OK if the requester was removed successfully, an specific error code otherwise
     */
    ReturnCode_t remove_requester(
            RequesterImpl* requester);

    /**
     * @brief Remove a replier from the service
     *
     * @param replier replier to remove
     * @return RETCODE_OK if the requester was removed successfully, an specific error code otherwise
     */
    ReturnCode_t remove_replier(
            ReplierImpl* replier);

    /**
     * @brief Create a requester for the service
     *
     * @param qos Requester QoS
     * @return A pointer to the created requester or nullptr if an error occurred
     */
    RequesterImpl* create_requester(
            const RequesterQos& qos);

    /**
     * @brief Create a replier for the service
     *
     * @param qos Replier QoS
     * @return A pointer to the created replier or nullptr if an error occurred
     */
    ReplierImpl* create_replier(
            const ReplierQos& qos);

    /**
     * @brief Enable the service
     *
     * @return RETCODE_OK if the topics were created successfully, an specific error code otherwise
     * It will also try to enable all internal Requesters and Repliers
     */
    ReturnCode_t enable() override;

    /**
     * @brief Disable the service
     *
     * @return RETCODE_OK if all topics were deleted and all internal requesters/repliers were disabled,
     * an specific error code otherwise
     */
    ReturnCode_t close() override;

    /**
     * @brief Check if the service is enabled
     */
    inline bool is_enabled() const override
    {
        return enabled_;
    }

    bool service_type_in_use(
            const std::string& service_type_name) const
    {
        return service_type_name_ == service_type_name;
    }

    /**
     * @brief Check if the service is empty (i.e: it has neither requesters nor repliers)
     */
    inline bool is_empty() const
    {
        return repliers_.empty() && requesters_.empty();
    }

    /**
     * @brief Validate the requester/replier's QoS. They should be consistent with the service configuration
     *
     * @param qos QoS to validate
     * @return True if the parameters are valid, false otherwise
     */
    template <typename T>
    bool validate_qos(
            const T& qos)
    {

        bool valid = true;

        if (qos.writer_qos.reliability().kind != RELIABLE_RELIABILITY_QOS)
        {
            EPROSIMA_LOG_ERROR(SERVICE, "Writer QoS reliability must be RELIABLE_RELIABILITY_QOS");
            valid = false;
        }

        if (qos.reader_qos.reliability().kind != RELIABLE_RELIABILITY_QOS)
        {
            EPROSIMA_LOG_ERROR(SERVICE, "Reader QoS reliability must be RELIABLE_RELIABILITY_QOS");
            valid = false;
        }

        return valid;
    }

    DomainParticipantImpl* get_participant()
    {
        return participant_;
    }

    PublisherImpl* get_publisher()
    {
        return service_publisher_;
    }

    SubscriberImpl* get_subscriber()
    {
        return service_subscriber_;
    }

    Topic* get_request_topic()
    {
        return request_topic_;
    }

    Topic* get_reply_topic()
    {
        return reply_topic_;
    }

    ContentFilteredTopic* get_reply_filtered_topic()
    {
        return reply_filtered_topic_;
    }

private:

    /**
     * @brief Create request and reply topics for the service
     *
     * @return RETCODE_OK if request/reply topics were created successfully, an specific error code otherwise
     */
    ReturnCode_t create_request_reply_topics();

    /**
     * @brief Delete all internal Requester and Replier entities
     *
     * @return RETCODE_OK if all entities were deleted successfully, an specific error code otherwise
     */
    ReturnCode_t delete_contained_entities();

    //! Service name
    std::string service_name_;

    //! Service type name
    std::string service_type_name_;

    //! Request topic info
    std::string request_topic_name_;
    std::string request_type_name_;

    //! Reply topic info
    std::string reply_topic_name_;
    std::string reply_type_name_;
    std::string reply_filtered_topic_name_;

    //! DDS Participant associated with the service
    DomainParticipantImpl* participant_;

    //! DDS Publisher used to create DataWriters for Requesters and Repliers
    PublisherImpl* service_publisher_;

    //! DDS Subscriber used to create DataReaders for Requesters and Repliers
    SubscriberImpl* service_subscriber_;

    //! Vector of repliers attached to the service
    std::vector<ReplierImpl*> repliers_;

    //! Mutex to protect the repliers list
    std::mutex mtx_repliers_;

    //! Vector of requesters attached to the service
    std::vector<RequesterImpl*> requesters_;

    //! Mutex to protect the requesters list
    std::mutex mtx_requesters_;

    //! Request and Reply topics associated with the service
    // NOTE: These topics do not filter samples. They are used to create the content filtered topics
    // If we use these topics to publish/subscribe in a multiple requester - single replier service scenario,
    // The requesters will receive all the replies, not only the ones that match their requests
    Topic* request_topic_;
    Topic* reply_topic_;

    //! Request and Reply filtered topics associated with the service.
    // In a multiple requester - single replier service scenario, each requester will discard the received replies destinated to another requester
    ContentFilteredTopic* reply_filtered_topic_;

    bool enabled_;

};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RPC__SERVICEIMPL_HPP