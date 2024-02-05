// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Server.hpp
 */

#ifndef _FASTDDS_DDS_RPC_SERVER_HPP_
#define _FASTDDS_DDS_RPC_SERVER_HPP_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>

#include <fastdds/dds/core/Entity.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Time_t.h>
#include <fastrtps/fastrtps_dll.h>
#include <fastrtps/rtps/common/WriteParams.h>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace rpc {

static void check_entity_creation(
        const Entity* entity)
{
    if (nullptr == entity)
    {
        throw std::runtime_error("Entity creation failed");
    }
}

/**
 * TODO(eduponz): Consider adding a base class and two derived classes for handling sync and async
 * servers.
 */

/**
 * @brief The Server class represents an RPC server.
 *
 * This class provides functionality for creating an RPC server that can handle incoming requests
 * and send back responses. It is templated on the types of the request and response data.
 *
 * @tparam RequestTypeSupport The type support class for the request data type.
 * @tparam ResponseTypeSupport The type support class for the response data type.
 */
template<
    typename RequestTypeSupport,
    typename ResponseTypeSupport
    >
class Server : protected DataReaderListener
{
public:

    using RequestType = typename RequestTypeSupport::type;
    using ResponseType = typename ResponseTypeSupport::type;
    using ServiceProcedure = std::function<ResponseType (const RequestType&)>;

    /*
     * TODO(eduponz): Add ctor overloads for instantiating Servers in different ways, e.g.:
     *    1. Only receive a Server name
     *    2. Do not receive a participant (would make the current ctor suitable to receive a ref)
     *    3. Set different QoS for the request and response DataReader and DataWriter respectively
     *    4. Optionally inject pools for reusing request and response samples.
     *    5. Provide the type support instances to avoid creating them in the ctor.
     */

    /**
     * @brief Constructs a Server object.
     *
     * @param participant The DomainParticipant object to use for communication. It is used to
     *                    create the necessary entities for the server. The listener is not
     *                    overriden, with the except of the on_data_available method, which is
     *                    overriden by the Server class, which in turn acts as a DataReaderListener
     *                    for the request DataReader. Consequently, the data_available bit of the
     *                    provided DomainParticipant status mask is disabled, as well as the
     *                    data_on_readers one. If set to nullptr, a DomainParticipant is created.
     * @param request_topic_name The name of the topic for receiving requests.
     * @param response_topic_name The name of the topic for sending responses.
     * @param max_ack_wait The maximum time to wait for acknowledgment of the response before
     *                     processing the next request.
     * @param service_procedure The service procedure to handle incoming requests.
     */
    RTPS_DllAPI Server(
            DomainParticipant* participant,
            const std::string& request_topic_name,
            const std::string& response_topic_name,
            Duration_t max_ack_wait,
            ServiceProcedure service_procedure)
        : service_procedure_(service_procedure)
        , participant_(participant)
        , request_type_support_(new RequestTypeSupport())
        , request_topic_(nullptr)
        , subscriber_(nullptr)
        , request_reader_(nullptr)
        , response_type_support_(new ResponseTypeSupport())
        , response_topic_(nullptr)
        , publisher_(nullptr)
        , response_writer_(nullptr)
        , max_ack_wait_(max_ack_wait)
        , stop_(false)
        , participant_ownership_(false)
    {
        std::unique_lock<std::mutex> lock(dds_mutex_);

        /* Create participant if not provided */
        if (nullptr == participant_)
        {
            // TODO(eduponz): Different ctor for this case
            participant_ = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
            check_entity_creation(participant_);
            participant_ownership_ = true;
        }
        // TODO(eduponz): StatusMask cannot be set on already constructed entities.

        /* Register types and create topics */
        request_type_support_.register_type(participant_, request_type_support_.get_type_name());
        response_type_support_.register_type(participant_, response_type_support_.get_type_name());

        response_topic_ = participant_->create_topic(
            response_topic_name,
            response_type_support_.get_type_name(),
            TOPIC_QOS_DEFAULT);
        check_entity_creation(response_topic_);

        request_topic_ = participant_->create_topic(
            request_topic_name,
            request_type_support_.get_type_name(),
            TOPIC_QOS_DEFAULT);
        check_entity_creation(request_topic_);

        /* Create response DataWriter */
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
        check_entity_creation(publisher_);

        // TODO(eduponz): Add Qos a profile for this
        response_writer_ = publisher_->create_datawriter(response_topic_, DATAWRITER_QOS_DEFAULT);
        check_entity_creation(response_writer_);
        EPROSIMA_LOG_INFO(RPC,
                "Created response writer in topic: " << response_topic_name << " [" << response_type_support_.get_type_name() <<
                "]");

        /* Create request DataReader */
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
        check_entity_creation(subscriber_);

        // TODO(eduponz): Add Qos a profile for this
        DataReaderQos reader_qos;
        reader_qos.reliability().kind = RELIABLE_RELIABILITY_QOS;
        request_reader_ =
                subscriber_->create_datareader(request_topic_, reader_qos, this, StatusMask::data_available());
        check_entity_creation(request_reader_);
        EPROSIMA_LOG_INFO(RPC,
                "Created request reader in topic: " << request_topic_name << " [" << request_type_support_.get_type_name() <<
                "]");
    }

    /**
     * @brief Runs the server.
     *
     * @pre This function can only be called once.
     * @pre This function cannot be called after stop has been called.
     *
     * This function starts the server and processes requests asynchronously by processing a
     * reception queue. It is a blocking function that will not return until the server is stopped,
     * and it that sense it is meant to be executed in a user-provided execution context (e.g. on
     * its onw thread). This function executes the provided service procedure for each incoming
     * request, and it is used to send the responses back to the clients.
     */
    RTPS_DllAPI void run()
    {
        while (!stop_)
        {
            // Block until a request is available or the server is stopped
            {
                std::unique_lock<std::mutex> lock(request_cv_mutex_);
                request_cv_.wait(lock, [this]
                        {
                            return !requests_queue_.empty() || stop_;
                        });
            }

            // Check if the server has been stopped
            if (requests_queue_.empty() && stop_)
            {
                break;
            }

            // Get the request from the queue
            RequestQueueElement request;
            {
                std::lock_guard<std::mutex> lock(request_queue_mutex_);
                request = requests_queue_.front();
                requests_queue_.pop();
            }

            // Run the service procedure to get the response
            ResponseType response = service_procedure_(request.first);

            // Fill the response related sample identity to that the client can differentiate
            // between the response to its request and other responses.
            eprosima::fastrtps::rtps::WriteParams write_params;
            write_params.related_sample_identity(request.second.sample_identity);

            // Send the response
            response_writer_->write(&response, write_params);
            response_writer_->wait_for_acknowledgments(max_ack_wait_);
        }
    }

    /**
     * @brief Stops the server.
     *
     * This function stops the server and terminates any ongoing operations.
     * If deletes all the server's entities and releases the resources, so the server instance
     * cannot be used after this function is called.
     *
     * @return true if the server was successfully stopped, false otherwise.
     */
    RTPS_DllAPI bool stop()
    {
        stop_.store(true);
        request_cv_.notify_one();

        std::unique_lock<std::mutex> lock(dds_mutex_);

        // Clear request queue
        {
            std::lock_guard<std::mutex> lock(request_queue_mutex_);
            RequestQueue empty_queue;
            std::swap(requests_queue_, empty_queue);
        }

        // Delete DDS entities
        ReturnCode_t ret = participant_->delete_contained_entities();

        if ((ret == ReturnCode_t::RETCODE_OK) && participant_ownership_)
        {
            ret = DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }

        if (ret != ReturnCode_t::RETCODE_OK)
        {
            EPROSIMA_LOG_ERROR(RPC, "Failed to delete contained entities");
        }

        return ret == ReturnCode_t::RETCODE_OK;
    }

protected:

    /**
     * @brief Callback function for handling incoming data.
     *
     * This function is called when new data becomes available on the DataReader.
     * It is responsible for processing the incoming requests.
     *
     * @param reader The DataReader object that received the data.
     */
    void on_data_available(
            DataReader* reader) override
    {
        std::unique_lock<std::mutex> lock(dds_mutex_);

        RequestType request;
        SampleInfo info;

        // TODO(eduponz): Use take() api instead to handle loans when possible
        while (ReturnCode_t::RETCODE_OK == reader->take_next_sample(&request, &info))
        {
            if (info.valid_data)
            {
                {
                    std::lock_guard<std::mutex> lock(request_queue_mutex_);
                    requests_queue_.push({request, info});
                }
                request_cv_.notify_one();
            }

            if (stop_)
            {
                break;
            }
        }
    }

    //! Callback function for handling status changes in the DataReader.
    ServiceProcedure service_procedure_;

    //! The DomainParticipant object used for communication.
    DomainParticipant* participant_;

    //! The type support object for the request data type.
    TypeSupport request_type_support_;

    //! The topic for receiving requests.
    Topic* request_topic_;

    //! The Subscriber for the request DataReader.
    Subscriber* subscriber_;

    //! The DataReader for receiving requests.
    DataReader* request_reader_;

    //! The type support object for the response data type.
    TypeSupport response_type_support_;

    //! The topic for sending responses.
    Topic* response_topic_;

    //! The Publisher for the response DataWriter.
    Publisher* publisher_;

    //! The DataWriter for sending responses.
    DataWriter* response_writer_;

    //! The maximum time to wait for response acknowledgments.
    Duration_t max_ack_wait_;

    //! Flag to indicate whether the server is stopped.
    std::atomic<bool> stop_;

    //! Mutex to protect the DDS entities.
    mutable std::mutex dds_mutex_;

    //! Flag to indicate whether the participant is owned by the server or provided by the user.
    bool participant_ownership_ = false;

    //! The request queue element type.
    using RequestQueueElement = std::pair<RequestType, SampleInfo>;

    //! The request queue type.
    using RequestQueue = std::queue<RequestQueueElement>;

    //! The queue for incoming requests.
    RequestQueue requests_queue_;

    //! Mutex to protect the request queue.
    mutable std::mutex request_queue_mutex_;

    //! Mutex to protect the request condition variable.
    mutable std::mutex request_cv_mutex_;

    //! Condition variable to notify the server of available requests.
    std::condition_variable request_cv_;
};

} // namespace rpc
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_RPC_SERVER_HPP_
