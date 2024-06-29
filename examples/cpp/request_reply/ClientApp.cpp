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
    : request_input_(config)
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

    request_reply_info("Client initialized with ID: " << participant_->guid().guidPrefix);
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
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [&]()
                {
                    return server_matched_status_.is_any_server_matched() || is_stopped();
                });
    }

    if (!is_stopped())
    {
        // Give some time for all connections to be matched on both ends
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait_for(lock, std::chrono::seconds(1), [&]()
                {
                    return is_stopped();
                });
    }

    if (!is_stopped())
    {
        if (!send_request())
        {
            throw std::runtime_error("Failed to send request");
        }

        wait_for_reply();
    }
}

void ClientApp::stop()
{
    stop_.store(true);
    cv_.notify_all();
}

void ClientApp::on_publication_matched(
        DataWriter* /* writer */,
        const PublicationMatchedStatus& info)
{
    std::lock_guard<std::mutex> lock(mtx_);

    rtps::GuidPrefix_t server_guid_prefix = rtps::iHandle2GUID(info.last_subscription_handle).guidPrefix;

    if (info.current_count_change == 1)
    {
        request_reply_info("Remote request reader matched.");

        server_matched_status_.match_request_reader(server_guid_prefix, true);
    }
    else if (info.current_count_change == -1)
    {
        request_reply_info("Remote request reader unmatched.");
        server_matched_status_.match_request_reader(server_guid_prefix, false);
    }
    else
    {
        request_reply_error(info.current_count_change
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
        request_reply_info("Remote reply writer matched.");

        server_matched_status_.match_reply_writer(server_guid_prefix, true);
    }
    else if (info.current_count_change == -1)
    {
        request_reply_info("Remote reply writer unmatched.");
        server_matched_status_.match_reply_writer(server_guid_prefix, false);
    }
    else
    {
        request_reply_error(info.current_count_change
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
            rtps::GuidPrefix_t server_guid_prefix = rtps::iHandle2GUID(info.publication_handle).guidPrefix;
            request_reply_info(
                "Reply received from server " << server_guid_prefix << " with result: " << reply.result());

            stop();
            break;
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

    participant_ = factory->create_participant_with_default_profile(nullptr, StatusMask::none());

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

    request_writer_ = publisher_->create_datawriter(request_topic_, writer_qos, this, StatusMask::all());

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
            TypeConverter::to_calculator_type(participant_->guid().guidPrefix) + "'";

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

    reply_reader_ = subscriber_->create_datareader(reply_cf_topic_, reader_qos, this, StatusMask::all());

    if (nullptr == reply_reader_)
    {
        throw std::runtime_error("Reply reader initialization failed");
    }
}

bool ClientApp::send_request()
{
    CalculatorRequestType request;

    request.client_id(TypeConverter::to_calculator_type(participant_->guid().guidPrefix));
    request.operation(TypeConverter::to_calculator_type(request_input_.operation));
    request.x(request_input_.x);
    request.y(request_input_.y);

    request_reply_info("Sending request");

    return request_writer_->write(&request);
}

bool ClientApp::is_stopped()
{
    return stop_.load();
}

void ClientApp::wait_for_reply()
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
