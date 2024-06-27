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

#include <iostream>
#include <stdexcept>

#include <fastdds/dds/core/detail/DDSReturnCode.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include "Calculator.hpp"
#include "CalculatorPubSubTypes.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace request_reply {

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
{
    // Create the participant
    auto factory = DomainParticipantFactory::get_instance();

    if (nullptr == factory)
    {
        throw std::runtime_error("Failed to get participant factory instance");
    }

    participant_ = factory->create_participant_with_default_profile(nullptr, StatusMask::none());

    if (nullptr == participant_)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    /********** REQUEST ENTITIES **********/
    // Create the request TypeSupport
    request_type_.reset(new CalculatorRequestTypePubSubType());

    if (nullptr == request_type_)
    {
        throw std::runtime_error("Failed to create request type");
    }

    // Register the type
    if (RETCODE_OK != request_type_.register_type(participant_))
    {
        throw std::runtime_error("Failed to register request type");
    }

    // Create the request topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;

    if (RETCODE_OK != participant_->get_default_topic_qos(topic_qos))
    {
        throw std::runtime_error("Failed to get default topic qos");
    }

    request_topic_ = participant_->create_topic("rq/" + service_name, request_type_.get_type_name(), topic_qos);

    if (nullptr == request_topic_)
    {
        throw std::runtime_error("Request topic initialization failed");
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

    // Create the request reader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;

    if (RETCODE_OK != subscriber_->get_default_datareader_qos(reader_qos))
    {
        throw std::runtime_error("Failed to get default datareader qos");
    }

    request_reader_ = subscriber_->create_datareader(request_topic_, reader_qos, this, StatusMask::all());

    if (nullptr == request_reader_)
    {
        throw std::runtime_error("Request reader initialization failed");
    }

    /********** REPLY ENTITIES **********/
    // Create the reply TypeSupport
    reply_type_.reset(new CalculatorReplyTypePubSubType());

    if (nullptr == reply_type_)
    {
        throw std::runtime_error("Failed to create reply type");
    }

    // Register the type
    if (RETCODE_OK != reply_type_.register_type(participant_))
    {
        throw std::runtime_error("Failed to register reply type");
    }

    // Create the reply topic
    reply_topic_ = participant_->create_topic("rr/" + service_name, reply_type_.get_type_name(), topic_qos);

    if (nullptr == reply_topic_)
    {
        throw std::runtime_error("Reply topic initialization failed");
    }

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

    reply_writer_ = publisher_->create_datawriter(reply_topic_, writer_qos, this, StatusMask::all());

    if (nullptr == reply_writer_)
    {
        throw std::runtime_error("Reply writer initialization failed");
    }
}

ServerApp::~ServerApp()
{
}

void ServerApp::run()
{
}

void ServerApp::stop()
{
}

void ServerApp::on_publication_matched(
        DataWriter* writer,
        const PublicationMatchedStatus& info)
{
    static_cast<void>(writer);
    static_cast<void>(info);
}

void ServerApp::on_subscription_matched(
        DataReader* /* reader */,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Remote request writer matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Remote request writer unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void ServerApp::on_data_available(
        DataReader* reader)
{
    static_cast<void>(reader);
}

} // namespace request_reply
} // namespace examples
} // namespace fastdds
} // namespace eprosima
