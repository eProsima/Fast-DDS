/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

/**
 * @file SubscriberApp.cpp
 *
 */

#include "SubscriberApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/rtps/transport/low-bandwidth/PayloadCompressionTransportDescriptor.hpp>
#include <fastdds/rtps/transport/low-bandwidth/HeaderReductionTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>


namespace eprosima {
namespace fastdds {
namespace examples {
namespace low_bandwidth {

SubscriberApp::SubscriberApp(
        const CLIParser::subscriber_config& config,
        const std::string& topic_name)
    : samples_(config.samples)
{
    DomainParticipantQos participant_qos;

    auto udp_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();

#if HAVE_ZLIB || HAVE_BZIP2
    auto compress_transport =
            std::make_shared<eprosima::fastdds::rtps::PayloadCompressionTransportDescriptor>(udp_transport);
    participant_qos.properties().properties().emplace_back(eprosima::fastdds::rtps::Property(
                "rtps.payload_compression.compression_library",
                "AUTOMATIC"));

    auto header_reduction_transport = std::make_shared<eprosima::fastdds::rtps::HeaderReductionTransportDescriptor>(
        compress_transport);
#else
    auto header_reduction_transport = std::make_shared<eprosima::fastdds::rtps::HeaderReductionTransportDescriptor>(
        udp_transport);
#endif // if HAVE_ZLIB || HAVE_BZIP2
    participant_qos.properties().properties().emplace_back(eprosima::fastdds::rtps::Property(
                "rtps.header_reduction.remove_version", "true"));
    participant_qos.properties().properties().emplace_back(eprosima::fastdds::rtps::Property(
                "rtps.header_reduction.remove_vendor_id", "true"));
    participant_qos.properties().properties().emplace_back(eprosima::fastdds::rtps::Property(
                "rtps.header_reduction.submessage.combine_id_and_flags",
                "true"));
    participant_qos.properties().properties().emplace_back(eprosima::fastdds::rtps::Property(
                "rtps.header_reduction.submessage.compress_entitiy_ids",
                "16,16"));

    participant_qos.transport().use_builtin_transports = false;
    participant_qos.transport().user_transports.push_back(header_reduction_transport);

    // Create the participant
    auto factory = DomainParticipantFactory::get_instance();
    participant_ = factory->create_participant(0, participant_qos);
    if (participant_ == nullptr)
    {
        throw std::runtime_error("Participant initialization failed");
    }

    // Register the type
    type_.register_type(participant_);

    // Create the subscriber
    SubscriberQos sub_qos = SUBSCRIBER_QOS_DEFAULT;
    participant_->get_default_subscriber_qos(sub_qos);
    subscriber_ = participant_->create_subscriber(sub_qos, nullptr, StatusMask::none());
    if (subscriber_ == nullptr)
    {
        throw std::runtime_error("Subscriber initialization failed");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic(topic_name, type_.get_type_name(), topic_qos);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the reader
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    subscriber_->get_default_datareader_qos(reader_qos);
    reader_ = subscriber_->create_datareader(topic_, reader_qos, this, StatusMask::all());
    if (reader_ == nullptr)
    {
        throw std::runtime_error("DataReader initialization failed");
    }
}

SubscriberApp::~SubscriberApp()
{
    if (participant_ != nullptr)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();
        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void SubscriberApp::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Subscriber unmatched." << std::endl;
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
    while ((!is_stopped()) && (RETCODE_OK == reader->take_next_sample(&hello_, &info)))
    {
        if ((info.instance_state == ALIVE_INSTANCE_STATE) && info.valid_data)
        {
            received_samples_++;
            // Print Hello world message data
            std::cout << "Message: '" << hello_.message() << "' with index: '" << hello_.index()
                      << "' RECEIVED" << std::endl;
            if (samples_ > 0 && (received_samples_ >= samples_))
            {
                stop();
            }
        }
    }
}

void SubscriberApp::run()
{
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, [&]
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
    terminate_cv_.notify_all();
}

} // namespace low_bandwidth
} // namespace examples
} // namespace fastdds
} // namespace eprosima
