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
 * @file SubscriberApp.cpp
 *
 */

#include "SubscriberApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>

#include "types/BenchmarkPubSubTypes.hpp"
#include "types/Benchmark_smallPubSubTypes.hpp"
#include "types/Benchmark_mediumPubSubTypes.hpp"
#include "types/Benchmark_bigPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace benchmark {

SubscriberApp::SubscriberApp(
        const CLIParser::subscriber_config& config)
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_pub_(nullptr)
    , writer_(nullptr)
    , subscriber_(nullptr)
    , topic_sub_(nullptr)
    , reader_(nullptr)
    , type_(nullptr)
    , samples_(config.samples)
    , matched_(0)
    , stop_(false)
    , msg_size_(config.msg_size)
{

    // Create the participant
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("Benchmark_sub_participant");
    auto factory = DomainParticipantFactory::get_instance();
    // Include Participant QoS
    pqos.setup_transports(config.transport);
    for (auto& transportDescriptor : pqos.transport().user_transports)
    {
        SocketTransportDescriptor* pT = dynamic_cast<SocketTransportDescriptor*>(transportDescriptor.get());
        if (pT)
        {
            pT->TTL = config.ttl;
        }
    }
    participant_ = factory->create_participant(config.domain, pqos);
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    // Register and set up the data type with initial values
    if (msg_size_ == CLIParser::MsgSizeKind::NONE)
    {
        benchmark_.index(0);
        type_ = TypeSupport(new BenchMarkPubSubType());
    }
    else if (msg_size_ == CLIParser::MsgSizeKind::SMALL)
    {
        benchmark_small_.index(0);
        type_ = TypeSupport(new BenchMarkSmallPubSubType());
    }
    else if (msg_size_ == CLIParser::MsgSizeKind::MEDIUM)
    {
        benchmark_medium_.index(0);
        type_ = TypeSupport(new BenchMarkMediumPubSubType());
    }
    else if (msg_size_ == CLIParser::MsgSizeKind::BIG)
    {
        benchmark_big_.index(0);
        type_ = TypeSupport(new BenchMarkBigPubSubType());
    }
    else
    {
        throw std::runtime_error("Type initialization failed");
    }
    type_.register_type(participant_);

    // Create the publisher
    PublisherQos pub_qos = PUBLISHER_QOS_DEFAULT;
    participant_->get_default_publisher_qos(pub_qos);
    publisher_ = participant_->create_publisher(pub_qos, nullptr, StatusMask::none());
    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Publisher initialization failed");
    }

    // Create the topics
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_pub_ = participant_->create_topic(config.topic_name + "_2", type_.get_type_name(), topic_qos);
    topic_sub_ = participant_->create_topic(config.topic_name + "_1", type_.get_type_name(), topic_qos);
    if (topic_pub_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the data writer
    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.reliability().kind = config.reliability;
    writer_qos.durability().kind = config.durability;
    publisher_->get_default_datawriter_qos(writer_qos);
    writer_ = publisher_->create_datawriter(topic_pub_, writer_qos, this, StatusMask::all());
    if (writer_ == nullptr)
    {
        throw std::runtime_error("DataWriter initialization failed");
    }

    // Create the subscriber
    SubscriberQos sub_qos = SUBSCRIBER_QOS_DEFAULT;
    participant_->get_default_subscriber_qos(sub_qos);
    subscriber_ = participant_->create_subscriber(sub_qos, nullptr, StatusMask::none());
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Subscriber initialization failed");
    }

    // Create the data reader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    subscriber_->get_default_datareader_qos(reader_qos);
    reader_qos.reliability().kind = config.reliability;
    reader_qos.durability().kind = config.durability;
    reader_ = subscriber_->create_datareader(topic_sub_, reader_qos, this, StatusMask::all());
    if (reader_ == nullptr)
    {
        throw std::runtime_error("DataReader initialization failed");
    }
}

SubscriberApp::~SubscriberApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void SubscriberApp::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_++;
        std::cout << "Publisher matched." << std::endl;
        cv_.notify_one();
    }
    else if (info.current_count_change == -1)
    {
        matched_--;
        std::cout << "Publisher unmatched." << std::endl;
        stop();
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void SubscriberApp::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_++;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_--;
        std::cout << "Subscriber unmatched." << std::endl;
        stop();
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void SubscriberApp::on_data_available(
        DataReader* reader)
{
    SampleInfo info;
    switch (msg_size_)
    {
        case CLIParser::MsgSizeKind::NONE:
            while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&benchmark_, &info)))
            {
                if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
                {
                    std::cout << "Sample with index: '" <<
                        benchmark_.index() << "' (Array 0 Bytes) RECEIVED" << std::endl;
                    benchmark_.index(benchmark_.index() + 1);
                    if (samples_ != 0 && benchmark_.index() >= samples_)
                    {
                        stop();
                        return;
                    }
                    if ((RETCODE_OK == writer_->write(&benchmark_)) == true)
                    {
                        std::cout << "Sample with index: '" <<
                            benchmark_.index() << "' (Array 0 Bytes) SENT" << std::endl;
                    }
                }
            }
            break;

        case CLIParser::MsgSizeKind::SMALL:
            while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&benchmark_small_, &info)))
            {
                if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
                {
                    std::cout << "Sample with index: '" <<
                        benchmark_small_.index() << "' (Array  " << static_cast<int>(benchmark_small_.array().size()) <<
                        " Bytes) RECEIVED" << std::endl;
                    benchmark_small_.index(benchmark_small_.index() + 1);
                    if (samples_ != 0 && benchmark_small_.index() >= samples_)
                    {
                        stop();
                        return;
                    }
                    if ((RETCODE_OK == writer_->write(&benchmark_small_)) == true)
                    {
                        std::cout << "Sample with index: '" <<
                            benchmark_small_.index() << "' (Array  " <<
                            static_cast<int>(benchmark_small_.array().size()) <<
                            " Bytes) SENT" << std::endl;
                    }
                }
            }
            break;

        case CLIParser::MsgSizeKind::MEDIUM:
            while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&benchmark_medium_, &info)))
            {
                if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
                {
                    std::cout << "Sample with index: '" <<
                        benchmark_medium_.index() << "' (Array  " <<
                        static_cast<int>(benchmark_medium_.data().size()) <<
                        " Bytes) RECEIVED" << std::endl;
                    benchmark_medium_.index(benchmark_medium_.index() + 1);
                    if (samples_ != 0 && benchmark_medium_.index() >= samples_)
                    {
                        stop();
                        return;
                    }
                    if ((RETCODE_OK == writer_->write(&benchmark_medium_)) == true)
                    {
                        std::cout << "Sample with index: '" <<
                            benchmark_medium_.index() << "' (Array  " <<
                            static_cast<int>(benchmark_medium_.data().size()) <<
                            " Bytes) SENT" << std::endl;
                    }
                }
            }
            break;

        case CLIParser::MsgSizeKind::BIG:
            while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&benchmark_big_, &info)))
            {
                if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
                {
                    std::cout << "Sample with index: '" <<
                        benchmark_big_.index() << "' (Array  " << static_cast<int>(benchmark_big_.data().size()) <<
                        " Bytes) RECEIVED" << std::endl;
                    benchmark_big_.index(benchmark_big_.index() + 1);
                    if (samples_ != 0 && benchmark_big_.index() >= samples_)
                    {
                        stop();
                        return;
                    }
                    if ((RETCODE_OK == writer_->write(&benchmark_big_)) == true)
                    {
                        std::cout << "Sample with index: '" <<
                            benchmark_big_.index() << "' (Array  " << static_cast<int>(benchmark_big_.data().size()) <<
                            " Bytes) SENT" << std::endl;
                    }
                }
            }
            break;

        default:
            throw std::runtime_error("Type invalid");
    }
}

void SubscriberApp::run()
{
    std::unique_lock<std::mutex> lock_(mutex_);
    cv_.wait(lock_, [&]
            {
                return is_stopped();
            });
}

bool SubscriberApp::is_stopped()
{
    return stop_.load();
}

void SubscriberApp::stop()
{
    stop_.store(true);
    cv_.notify_one();
}

} // namespace benchmark
} // namespace examples
} // namespace fastdds
} // namespace eprosima
