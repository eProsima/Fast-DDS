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
 * @file ClientApp.cpp
 *
 */

#include "ClientApp.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <stdexcept>

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>

#include "CLIParser.hpp"
#include "types/Calculator.hpp"
#include "types/CalculatorPubSubTypes.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace request_reply {

using namespace eprosima::fastdds::dds;

/******* HELPER FUNCTIONS DECLARATIONS *******/
namespace detail {

template<typename TypeSupportClass>
Topic* create_topic(
        const std::string& topic_name,
        DomainParticipant* participant,
        TypeSupport& type);

} // namespace detail

/******** CLIENTAPP CLASS DEFINITION ********/
ClientApp::ClientApp(
        const CLIParser::config& config,
        const std::string& service_name)
    : request_input_({config.x, config.y})
    , participant_(nullptr)
    , request_type_(nullptr)
    , request_topic_(nullptr)
    , publisher_(nullptr)
    , request_writer_(nullptr)
    , reply_type_(nullptr)
    , reply_topic_(nullptr)
    , reply_cf_topic_(nullptr)
    , reply_topic_filter_expression_("")
    , subscriber_(nullptr)
    , reply_reader_(nullptr)
    , stop_(false)
{
    create_participant();
    create_request_entities(service_name);
    create_reply_entities(service_name);

    request_reply_info("ClientApp", "Client initialized with ID: " << participant_->guid().guidPrefix);
}

ClientApp::~ClientApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_shared_instance()->delete_participant(participant_);
    }

    server_matched_status_.clear();
    reply_topic_filter_parameters_.clear();
}

void ClientApp::run()
{
    request_reply_debug("ClientApp", "Waiting for a server to be available");
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [&]()
                {
                    return server_matched_status_.is_any_server_matched() || is_stopped();
                });
    }

    if (!is_stopped())
    {
        request_reply_debug("ClientApp",
                "One server is available. Waiting for some time to ensure matching on the server side");

        // TODO(eduponz): This wait should be conditioned to upcoming fully-matched API on the endpoints
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait_for(lock, std::chrono::seconds(1), [&]()
                {
                    return is_stopped();
                });
    }

    if (!is_stopped())
    {
        if (!send_requests())
        {
            throw std::runtime_error("Failed to send request");
        }

        wait_for_replies();
    }
}

void ClientApp::stop()
{
    stop_.store(true);
    cv_.notify_all();
}

void ClientApp::on_participant_discovery(
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
        request_reply_debug("ClientApp", "Ignoring participant with invalid user data: "
                << remote_participant_guid_prefix);
    }

    if (!should_be_ignored)
    {
        CLIParser::EntityKind entity_kind = static_cast<CLIParser::EntityKind>(info.user_data.data_vec()[0]);
        if (CLIParser::EntityKind::SERVER != entity_kind)
        {
            should_be_ignored = true;
            request_reply_debug("ClientApp", "Ignoring " << status_str << " "
                                                         << CLIParser::parse_entity_kind(entity_kind)
                                                         << ": " << remote_participant_guid_prefix);
        }
    }

    if (!should_be_ignored)
    {
        std::string server_str = CLIParser::parse_entity_kind(CLIParser::EntityKind::SERVER);

        if (status == rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT)
        {
            request_reply_debug("ClientApp", server_str << " " << status_str << ": " << remote_participant_guid_prefix);
        }
        else if (status == rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT ||
                status == rtps::ParticipantDiscoveryStatus::DROPPED_PARTICIPANT)
        {
            request_reply_debug("ClientApp", server_str << " " << status_str << ": " << remote_participant_guid_prefix);
        }
    }
}

void ClientApp::on_publication_matched(
        DataWriter* /* writer */,
        const PublicationMatchedStatus& info)
{
    std::lock_guard<std::mutex> lock(mtx_);

    rtps::GuidPrefix_t server_guid_prefix = rtps::iHandle2GUID(info.last_subscription_handle).guidPrefix;

    if (info.current_count_change == 1)
    {
        request_reply_debug("ClientApp", "Remote request reader matched.");

        server_matched_status_.match_request_reader(server_guid_prefix, true);
    }
    else if (info.current_count_change == -1)
    {
        request_reply_debug("ClientApp", "Remote request reader unmatched.");
        server_matched_status_.match_request_reader(server_guid_prefix, false);
    }
    else
    {
        request_reply_error("ClientApp", info.current_count_change
                << " is not a valid value for SubscriptionMatchedStatus current count change");
    }
    cv_.notify_all();
}

void ClientApp::on_subscription_matched(
        DataReader* /* reader */,
        const SubscriptionMatchedStatus& info)
{
    std::lock_guard<std::mutex> lock(mtx_);

    rtps::GuidPrefix_t server_guid_prefix = rtps::iHandle2GUID(info.last_publication_handle).guidPrefix;

    if (info.current_count_change == 1)
    {
        request_reply_debug("ClientApp", "Remote reply writer matched.");

        server_matched_status_.match_reply_writer(server_guid_prefix, true);
    }
    else if (info.current_count_change == -1)
    {
        request_reply_debug("ClientApp", "Remote reply writer unmatched.");
        server_matched_status_.match_reply_writer(server_guid_prefix, false);
    }
    else
    {
        request_reply_error("ClientApp", info.current_count_change
                << " is not a valid value for SubscriptionMatchedStatus current count change");
    }
    cv_.notify_all();
}

void ClientApp::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    CalculatorReplyType reply;

    while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&reply, &info)))
    {
        if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
        {
            std::lock_guard<std::mutex> lock(mtx_);

            rtps::GuidPrefix_t server_guid_prefix = rtps::iHandle2GUID(info.publication_handle).guidPrefix;

            auto request_status = requests_status_.find(info.related_sample_identity);

            if (requests_status_.end() != request_status)
            {
                if (!request_status->second)
                {
                    request_status->second = true;
                    request_reply_info("ClientApp", "Reply received from server "
                            << server_guid_prefix << " to request with ID '" << request_status->first.sequence_number()
                            << "' with result: '" << reply.result() << "'");
                }
                else
                {
                    request_reply_debug("ClientApp", "Duplicate reply received from server "
                            << server_guid_prefix << " to request with ID '" << request_status->first.sequence_number()
                            << "' with result: '" << reply.result() << "'");
                    continue;
                }
            }
            else
            {
                request_reply_error("ClientApp",
                        "Reply received from server " << server_guid_prefix << " with unknown request ID '"
                                                      << info.related_sample_identity.sequence_number() << "'");
                continue;
            }

            // Check if all responses have been received
            if (requests_status_.size() == 4)
            {
                bool all_responses_received = true;

                for (auto status : requests_status_)
                {
                    all_responses_received &= status.second;
                }

                if (all_responses_received)
                {
                    stop();
                    break;
                }
            }
        }
    }
}

void ClientApp::create_participant()
{
    // Create the participant
    auto factory = DomainParticipantFactory::get_shared_instance();

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

    participant_qos.user_data().data_vec().push_back(static_cast<uint8_t>(CLIParser::EntityKind::CLIENT));

    participant_ = factory->create_participant(participant_qos.domainId(), participant_qos, this, participant_mask);

    if (nullptr == participant_)
    {
        throw std::runtime_error("Participant initialization failed");
    }
}

template<>
Topic* ClientApp::create_topic<CalculatorRequestTypePubSubType>(
        const std::string& topic_name,
        TypeSupport& type)
{
    return detail::create_topic<CalculatorRequestTypePubSubType>(topic_name, participant_, type);
}

template<>
Topic* ClientApp::create_topic<CalculatorReplyTypePubSubType>(
        const std::string& topic_name,
        TypeSupport& type)
{
    return detail::create_topic<CalculatorReplyTypePubSubType>(topic_name, participant_, type);
}

void ClientApp::create_request_entities(
        const std::string& service_name)
{
    // Create the request topic
    request_topic_ = create_topic<CalculatorRequestTypePubSubType>("rq/" + service_name, request_type_);

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

    // Create the writer
    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;

    if (RETCODE_OK != publisher_->get_default_datawriter_qos(writer_qos))
    {
        throw std::runtime_error("Failed to get default datawriter qos");
    }

    request_writer_ = publisher_->create_datawriter(request_topic_, writer_qos, nullptr, StatusMask::none());

    if (nullptr == request_writer_)
    {
        throw std::runtime_error("Request writer initialization failed");
    }
}

void ClientApp::create_reply_entities(
        const std::string& service_name)
{
    // Create the reply topic
    reply_topic_ = create_topic<CalculatorReplyTypePubSubType>("rr/" + service_name, reply_type_);

    reply_topic_filter_expression_ = "client_id = '" +
            TypeConverter::to_string(participant_->guid().guidPrefix) + "'";

    reply_cf_topic_ = participant_->create_contentfilteredtopic("rr/" + service_name + "_cft", reply_topic_,
                    reply_topic_filter_expression_,
                    reply_topic_filter_parameters_);

    if (nullptr == reply_cf_topic_)
    {
        throw std::runtime_error("Failed to create CFT");
    }

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

    // Create the reader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;

    if (RETCODE_OK != subscriber_->get_default_datareader_qos(reader_qos))
    {
        throw std::runtime_error("Failed to get default datareader qos");
    }

    reply_reader_ = subscriber_->create_datareader(reply_cf_topic_, reader_qos, nullptr, StatusMask::none());

    if (nullptr == reply_reader_)
    {
        throw std::runtime_error("Reply reader initialization failed");
    }
}

bool ClientApp::send_requests()
{
    CalculatorRequestType request;

    request.client_id(TypeConverter::to_string(participant_->guid().guidPrefix));
    request.x(request_input_.first);
    request.y(request_input_.second);

    request.operation(CalculatorOperationType::ADDITION);
    bool ret = send_request(request);

    if (ret)
    {
        request.operation(CalculatorOperationType::SUBTRACTION);
        ret = send_request(request);
    }

    if (ret)
    {
        request.operation(CalculatorOperationType::MULTIPLICATION);
        ret = send_request(request);
    }

    if (ret)
    {
        request.operation(CalculatorOperationType::DIVISION);
        ret = send_request(request);
    }

    return ret;
}

bool ClientApp::send_request(
        const CalculatorRequestType& request)
{
    // Taking the mutex here to avoid taking a reply on the on_data_available callback
    // coming from a very fast server who replied before the request_status_ entry was set
    std::lock_guard<std::mutex> lock(mtx_);

    rtps::WriteParams wparams;
    ReturnCode_t ret = request_writer_->write(&request, wparams);

    requests_status_[wparams.sample_identity()] = false;

    request_reply_info("ClientApp",
            "Request sent with ID '" << wparams.sample_identity().sequence_number() <<
            "': '" << TypeConverter::to_string(request) << "'");

    return (RETCODE_OK == ret);
}

bool ClientApp::is_stopped()
{
    return stop_.load();
}

void ClientApp::wait_for_replies()
{
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [&]()
            {
                return is_stopped();
            });
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
