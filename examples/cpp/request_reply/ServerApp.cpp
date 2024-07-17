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
 * @file ServerApp.cpp
 *
 */

#include "ServerApp.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantExtendedQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>

#include "CLIParser.hpp"
#include "types/Calculator.hpp"
#include "types/CalculatorPubSubTypes.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace request_reply {

/******* HELPER FUNCTIONS DECLARATIONS *******/
namespace detail {

template<typename TypeSupportClass>
Topic* create_topic(
        const std::string& topic_name,
        DomainParticipant* participant,
        TypeSupport& type);

} // namespace detail

/******** SERVERAPP CLASS DEFINITION ********/
ServerApp::ServerApp(
        const std::string& service_name)
    : participant_(nullptr)
    , request_type_(nullptr)
    , request_topic_(nullptr)
    , subscriber_(nullptr)
    , request_reader_(nullptr)
    , reply_type_(nullptr)
    , reply_topic_(nullptr)
    , publisher_(nullptr)
    , reply_writer_(nullptr)
    , stop_(false)
{
    // Spawn the reply thread
    reply_thread_ = std::thread(&ServerApp::reply_routine, this);

    // Create the DDS entities
    create_participant();
    create_request_entities(service_name);
    create_reply_entities(service_name);

    request_reply_info("ServerApp", "Server initialized with ID: " << participant_->guid().guidPrefix);
}

ServerApp::~ServerApp()
{
    // Join reply thread
    if (reply_thread_.joinable())
    {
        reply_thread_.join();
    }

    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    // Cleanup the pending requests
    while (requests_.size() > 0)
    {
        requests_.pop();
    }

    client_matched_status_.clear();
}

void ServerApp::run()
{
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]() -> bool
            {
                return is_stopped();
            });
}

void ServerApp::stop()
{
    stop_.store(true);
    cv_.notify_all();
}

void ServerApp::on_participant_discovery(
        DomainParticipant* /* participant */,
        rtps::ParticipantDiscoveryStatus status,
        const ParticipantBuiltinTopicData& info,
        bool& should_be_ignored)
{
    std::lock_guard<std::mutex> lock(mtx_);

    should_be_ignored = false;

    rtps::GuidPrefix_t remote_participant_guid_prefix = info.guid.guidPrefix;
    std::string status_str = TypeConverter::to_string(status);

    if (info.user_data.data_vec().size() != 1)
    {
        should_be_ignored = true;
        request_reply_debug("ServerApp", "Ignoring participant with invalid user data: "
                << remote_participant_guid_prefix);
    }

    if (!should_be_ignored)
    {
        CLIParser::EntityKind entity_kind = static_cast<CLIParser::EntityKind>(info.user_data.data_vec()[0]);
        if (CLIParser::EntityKind::CLIENT != entity_kind)
        {
            should_be_ignored = true;
            request_reply_debug("ServerApp", "Ignoring " << status_str << " "
                                                         << CLIParser::parse_entity_kind(entity_kind)
                                                         << ": " << remote_participant_guid_prefix);
        }
    }

    if (!should_be_ignored)
    {
        std::string client_str = CLIParser::parse_entity_kind(CLIParser::EntityKind::CLIENT);

        if (status == rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT)
        {
            request_reply_debug("ServerApp", client_str << " " << status_str << ": " << remote_participant_guid_prefix);
        }
        else if (status == rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT ||
                status == rtps::ParticipantDiscoveryStatus::DROPPED_PARTICIPANT)
        {
            client_matched_status_.match_reply_reader(remote_participant_guid_prefix, false);
            client_matched_status_.match_request_writer(remote_participant_guid_prefix, false);

            request_reply_debug("ServerApp", client_str << " " << status_str << ": " << remote_participant_guid_prefix);
        }
    }
}

void ServerApp::on_publication_matched(
        DataWriter* /* writer */,
        const PublicationMatchedStatus& info)
{
    std::lock_guard<std::mutex> lock(mtx_);

    rtps::GuidPrefix_t client_guid_prefix = rtps::iHandle2GUID(info.last_subscription_handle).guidPrefix;

    if (info.current_count_change == 1)
    {
        request_reply_debug("ServerApp", "Remote reply reader matched with client " << client_guid_prefix);
        client_matched_status_.match_reply_reader(client_guid_prefix, true);
    }
    else if (info.current_count_change == -1)
    {
        request_reply_debug("ServerApp", "Remote reply reader unmatched from client " << client_guid_prefix);
        client_matched_status_.match_reply_reader(client_guid_prefix, false);

        // Remove old replies since no one is waiting for them
        if (client_matched_status_.no_client_matched())
        {
            std::size_t removed;
            reply_writer_->clear_history(&removed);
        }
    }
    else
    {
        request_reply_error("ServerApp", info.current_count_change
                << " is not a valid value for PublicationMatchedStatus current count change");
    }
}

void ServerApp::on_subscription_matched(
        DataReader* /* reader */,
        const SubscriptionMatchedStatus& info)
{
    std::lock_guard<std::mutex> lock(mtx_);

    rtps::GuidPrefix_t client_guid_prefix = rtps::iHandle2GUID(info.last_publication_handle).guidPrefix;

    if (info.current_count_change == 1)
    {
        request_reply_debug("ServerApp", "Remote request writer matched with client " << client_guid_prefix);
        client_matched_status_.match_request_writer(client_guid_prefix, true);
    }
    else if (info.current_count_change == -1)
    {
        request_reply_debug("ServerApp", "Remote request writer unmatched from client " << client_guid_prefix);
        client_matched_status_.match_request_writer(client_guid_prefix, false);

        // Remove old replies since no one is waiting for them
        if (client_matched_status_.no_client_matched())
        {
            std::size_t removed;
            reply_writer_->clear_history(&removed);
        }
    }
    else
    {
        request_reply_error("ServerApp",
                info.current_count_change <<
                " is not a valid value for SubscriptionMatchedStatus current count change");
    }
}

void ServerApp::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    auto request = std::make_shared<CalculatorRequestType>();

    while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(request.get(), &info)))
    {
        if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
        {
            rtps::GuidPrefix_t client_guid_prefix = rtps::iHandle2GUID(info.publication_handle).guidPrefix;
            rtps::SequenceNumber_t request_id = info.sample_identity.sequence_number();

            request_reply_info("ServerApp",
                    "Request with ID '" << request_id << "' received from client " << client_guid_prefix);

            {
                // Only lock to push the request into the queue so that the consumer thread gets
                // a chance to process it in between subsequent takes
                std::lock_guard<std::mutex> lock(mtx_);
                requests_.push({info, request});
                cv_.notify_all();
            }
        }
    }
}

void ServerApp::create_participant()
{
    // Create the participant
    auto factory = DomainParticipantFactory::get_instance();

    if (nullptr == factory)
    {
        throw std::runtime_error("Failed to get participant factory instance");
    }

    StatusMask participant_mask = StatusMask::none();
    participant_mask << StatusMask::publication_matched();
    participant_mask << StatusMask::subscription_matched();
    participant_mask << StatusMask::data_available();

    DomainParticipantExtendedQos participant_qos;
    factory->get_participant_extended_qos_from_default_profile(participant_qos);

    participant_qos.user_data().data_vec().push_back(static_cast<uint8_t>(CLIParser::EntityKind::SERVER));

    participant_ = factory->create_participant(participant_qos.domainId(), participant_qos, this, participant_mask);

    if (nullptr == participant_)
    {
        throw std::runtime_error("Participant initialization failed");
    }
}

template<>
Topic* ServerApp::create_topic<CalculatorRequestTypePubSubType>(
        const std::string& topic_name,
        TypeSupport& type)
{
    return detail::create_topic<CalculatorRequestTypePubSubType>(topic_name, participant_, type);
}

template<>
Topic* ServerApp::create_topic<CalculatorReplyTypePubSubType>(
        const std::string& topic_name,
        TypeSupport& type)
{
    return detail::create_topic<CalculatorReplyTypePubSubType>(topic_name, participant_, type);
}

void ServerApp::create_request_entities(
        const std::string& service_name)
{
    request_topic_ = create_topic<CalculatorRequestTypePubSubType>("rq/" + service_name, request_type_);

    // Create the subscriber
    SubscriberQos sub_qos = SUBSCRIBER_QOS_DEFAULT;

    if (RETCODE_OK != participant_->get_default_subscriber_qos(sub_qos))
    {
        throw std::runtime_error("Failed to get default subscriber qos");
    }

    subscriber_ = participant_->create_subscriber(sub_qos, nullptr, StatusMask::none());

    if (nullptr == subscriber_)
    {
        throw std::runtime_error("Subscriber initialization failed");
    }

    // Create the request reader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;

    if (RETCODE_OK != subscriber_->get_default_datareader_qos(reader_qos))
    {
        throw std::runtime_error("Failed to get default datareader qos");
    }

    request_reader_ = subscriber_->create_datareader(request_topic_, reader_qos, nullptr, StatusMask::none());

    if (nullptr == request_reader_)
    {
        throw std::runtime_error("Request reader initialization failed");
    }
}

void ServerApp::create_reply_entities(
        const std::string& service_name)
{
    reply_topic_ = create_topic<CalculatorReplyTypePubSubType>("rr/" + service_name, reply_type_);

    // Create the publisher
    PublisherQos pub_qos = PUBLISHER_QOS_DEFAULT;

    if (RETCODE_OK != participant_->get_default_publisher_qos(pub_qos))
    {
        throw std::runtime_error("Failed to get default publisher qos");
    }

    publisher_ = participant_->create_publisher(pub_qos, nullptr, StatusMask::none());

    if (nullptr == publisher_)
    {
        throw std::runtime_error("Publisher initialization failed");
    }

    // Create the reply writer
    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;

    if (RETCODE_OK != publisher_->get_default_datawriter_qos(writer_qos))
    {
        throw std::runtime_error("Failed to get default datawriter qos");
    }

    reply_writer_ = publisher_->create_datawriter(reply_topic_, writer_qos, nullptr, StatusMask::none());

    if (nullptr == reply_writer_)
    {
        throw std::runtime_error("Reply writer initialization failed");
    }
}

bool ServerApp::is_stopped()
{
    return stop_.load();
}

void ServerApp::reply_routine()
{
    while (!is_stopped())
    {
        // Wait for a request to arrive
        std::unique_lock<std::mutex> lock(mtx_);

        cv_.wait(lock, [this]() -> bool
                {
                    return requests_.size() > 0 || is_stopped();
                });

        if (!is_stopped())
        {
            // Process one request per iteration
            Request request = requests_.front();
            requests_.pop();

            rtps::GuidPrefix_t client_guid_prefix = rtps::iHandle2GUID(request.info.publication_handle).guidPrefix;
            request_reply_debug("ServerApp", "Processing request from client " << client_guid_prefix);

            // If none of the client's endpoints are matched, ignore the request as the client is gone
            if (!client_matched_status_.is_fully_unmatched(client_guid_prefix))
            {
                request_reply_info("ServerApp", "Ignoring request from already gone client " << client_guid_prefix);
                continue;
            }

            // If the request's client is not fully matched, save it for later
            if (!client_matched_status_.is_matched(client_guid_prefix))
            {
                request_reply_debug("ServerApp",
                        "Client " << client_guid_prefix << " not fully matched, saving request for later");
                requests_.push(request);
                continue;
            }

            // Calculate the result
            std::int32_t result;

            // If the calculation fails, ignore the request, as the failure cause is a malformed request
            if (!calculate(*request.request, result))
            {
                request_reply_error("ServerApp",
                        "Failed to calculate result for request from client " << client_guid_prefix);
                continue;
            }

            // Prepare the reply
            CalculatorReplyType reply;
            reply.client_id(request.request->client_id());
            reply.result(result);

            // Prepare the WriteParams to link the reply to the request
            rtps::WriteParams write_params;
            rtps::SequenceNumber_t request_id = request.info.sample_identity.sequence_number();
            write_params.related_sample_identity().writer_guid(request.info.sample_identity.writer_guid());
            write_params.related_sample_identity().sequence_number(request_id);

            // Send the reply
            if (RETCODE_OK != reply_writer_->write(&reply, write_params))
            {
                // In case of failure, save the request for a later retry
                request_reply_error("ServerApp",
                        "Failed to send reply to request with ID '" << request_id << "' to client " <<
                        client_guid_prefix);
                requests_.push(request);
            }
            else
            {
                request_reply_info("ServerApp",
                        "Reply to request with ID '" << request_id << "' sent to client " << client_guid_prefix);
            }
        }
    }
}

bool ServerApp::calculate(
        const CalculatorRequestType& request,
        std::int32_t& result)
{
    bool success = true;

    switch (request.operation())
    {
        case CalculatorOperationType::ADDITION:
        {
            result = request.x() + request.y();
            break;
        }
        case CalculatorOperationType::SUBTRACTION:
        {
            result = request.x() - request.y();
            break;
        }
        case CalculatorOperationType::MULTIPLICATION:
        {
            result = request.x() * request.y();
            break;
        }
        case CalculatorOperationType::DIVISION:
        {
            if (0 == request.y())
            {
                request_reply_error("ServerApp", "Division by zero request received");
                success = false;
            }
            else
            {
                result = request.x() / request.y();
            }
            break;
        }
        default:
        {
            request_reply_error("ServerApp", "Unknown operation received");
            success = false;
            break;
        }
    }

    return success;
}

/******* HELPER FUNCTIONS DEFINITIONS *******/
namespace detail {

template<typename TypeSupportClass>
Topic* create_topic(
        const std::string& topic_name,
        DomainParticipant* participant,
        TypeSupport& type)
{
    assert(nullptr != participant);
    assert(nullptr == type.get());

    Topic* topic = nullptr;

    // Create the TypeSupport
    type.reset(new TypeSupportClass());

    if (nullptr == type)
    {
        throw std::runtime_error("Failed to create type");
    }

    // Register the type
    if (RETCODE_OK != type.register_type(participant))
    {
        throw std::runtime_error("Failed to register type");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;

    if (RETCODE_OK != participant->get_default_topic_qos(topic_qos))
    {
        throw std::runtime_error("Failed to get default topic qos");
    }

    topic = participant->create_topic(topic_name, type.get_type_name(), topic_qos);

    if (nullptr == topic)
    {
        throw std::runtime_error("Request topic initialization failed");
    }

    return topic;
}

} // namespace detail
} // namespace request_reply
} // namespace examples
} // namespace fastdds
} // namespace eprosima
