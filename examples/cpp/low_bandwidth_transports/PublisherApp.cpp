/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */
/**
 * @file PublisherApp.cpp
 *
 */

#include "PublisherApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/rtps/transport/low-bandwidth/PayloadCompressionTransportDescriptor.hpp>
#include <fastdds/rtps/transport/low-bandwidth/HeaderReductionTransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace low_bandwidth {

PublisherApp::PublisherApp(
        const CLIParser::publisher_config& config,
        const std::string& topic_name)
    : samples_(config.samples)
    , expected_matches_(config.matched)
{
    // Set up the data type with initial values
    hello_.index(0);
    hello_.message("Hello world");

    DomainParticipantQos participant_qos;

    auto udp_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();

#if HAVE_ZLIB || HAVE_BZIP2
    auto compress_transport =
            std::make_shared<eprosima::fastdds::rtps::PayloadCompressionTransportDescriptor>(udp_transport);
    participant_qos.properties().properties().emplace_back(eprosima::fastdds::rtps::Property(
                "rtps.payload_compression.compression_library", "AUTOMATIC"));

    auto header_reduction_transport =
            std::make_shared<eprosima::fastdds::rtps::HeaderReductionTransportDescriptor>(compress_transport);
#else
    auto header_reduction_transport =
            std::make_shared<eprosima::fastdds::rtps::HeaderReductionTransportDescriptor>(udp_transport);
#endif // if HAVE_ZLIB || HAVE_BZIP2
    participant_qos.properties().properties().emplace_back(eprosima::fastdds::rtps::Property(
                "rtps.header_reduction.remove_version", "true"));
    participant_qos.properties().properties().emplace_back(eprosima::fastdds::rtps::Property(
                "rtps.header_reduction.remove_vendor_id", "true"));
    participant_qos.properties().properties().emplace_back(eprosima::fastdds::rtps::Property(
                "rtps.header_reduction.submessage.combine_id_and_flags", "true"));
    participant_qos.properties().properties().emplace_back(eprosima::fastdds::rtps::Property(
                "rtps.header_reduction.submessage.compress_entitiy_ids", "16,16"));

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

    // Create the publisher
    PublisherQos pub_qos = PUBLISHER_QOS_DEFAULT;
    participant_->get_default_publisher_qos(pub_qos);
    publisher_ = participant_->create_publisher(pub_qos, nullptr, StatusMask::none());
    if (publisher_ == nullptr)
    {
        throw std::runtime_error("Publisher initialization failed");
    }

    // Create the topic
    TopicQos topic_qos = TOPIC_QOS_DEFAULT;
    participant_->get_default_topic_qos(topic_qos);
    topic_ = participant_->create_topic(topic_name, type_.get_type_name(), topic_qos);
    if (topic_ == nullptr)
    {
        throw std::runtime_error("Topic initialization failed");
    }

    // Create the data writer
    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    writer_qos.history().depth = 5;
    publisher_->get_default_datawriter_qos(writer_qos);
    writer_ = publisher_->create_datawriter(topic_, writer_qos, this, StatusMask::all());
    if (writer_ == nullptr)
    {
        throw std::runtime_error("DataWriter initialization failed");
    }
}

PublisherApp::~PublisherApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void PublisherApp::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Publisher matched." << std::endl;
        cv_.notify_one();
    }
    else if (info.current_count_change == -1)
    {
        matched_ = static_cast<int16_t>(info.current_count);
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void PublisherApp::run()
{
    while (!is_stopped() && ((samples_ == 0) || (hello_.index() < samples_)))
    {
        if (publish())
        {
            std::cout << "Message: '" << hello_.message() << "' with index: '" << hello_.index()
                      << "' SENT" << std::endl;

            if (hello_.index() == 1u)
            {
                ReturnCode_t acked = RETCODE_ERROR;
                do
                {
                    dds::Duration_t acked_wait{1, 0};
                    acked = writer_->wait_for_acknowledgments(acked_wait);
                }
                while (acked != RETCODE_OK);
            }
        }
        // Wait for period or stop event
        std::unique_lock<std::mutex> period_lock(mutex_);
        cv_.wait_for(period_lock, std::chrono::milliseconds(period_ms_), [&]()
                {
                    return is_stopped();
                });
    }
}

bool PublisherApp::publish()
{
    bool ret = false;
    // Wait for the data endpoints discovery
    std::unique_lock<std::mutex> matched_lock(mutex_);
    cv_.wait(matched_lock, [&]()
            {
                // at least one has been discovered
                return ((matched_ >= expected_matches_) || is_stopped());
            });

    if (!is_stopped())
    {
        hello_.index(hello_.index() + 1);
        ret = (RETCODE_OK == writer_->write(&hello_));
    }
    return ret;
}

bool PublisherApp::is_stopped()
{
    return stop_.load();
}

void PublisherApp::stop()
{
    stop_.store(true);
    cv_.notify_one();
}

} // namespace low_bandwidth
} // namespace examples
} // namespace fastdds
} // namespace eprosima
