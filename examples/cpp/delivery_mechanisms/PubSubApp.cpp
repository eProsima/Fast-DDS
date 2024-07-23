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
 * @file PubSubApp.cpp
 *
 */

#include "PubSubApp.hpp"

#include <condition_variable>
#include <stdexcept>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/TCPv6TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/rtps/transport/UDPv6TransportDescriptor.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "DeliveryMechanismsPubSubTypes.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace delivery_mechanisms {

PubSubApp::PubSubApp(
        const CLIParser::delivery_mechanisms_config& config,
        const std::string& topic_name)
    : participant_(nullptr)
    , publisher_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , writer_(nullptr)
    , type_(new DeliveryMechanismsPubSubType())
    , matched_(0)
    , index_of_last_sample_sent_(0)
    , received_samples_(0)
    , samples_(config.samples)
    , stop_(false)
{
    // Check that the generated type fulfils example constraints: it is plain and bounded
    if (!type_->is_plain(eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION) || !type_->is_bounded())
    {
        throw std::runtime_error(
                  "Example generated type does not fulfil the example constraints: it is not plain and/or bounded");
    }

    // Create the participant
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.name("DeliveryMechanisms_pubsub_participant");
    pqos.transport().use_builtin_transports = false;

    uint32_t max_samples = samples_;
    if (max_samples == 0)
    {
        max_samples = DATAWRITER_QOS_DEFAULT.resource_limits().max_samples_per_instance;
    }

    if (config.ignore_local_endpoints)
    {
        pqos.properties().properties().emplace_back(
            "fastdds.ignore_local_endpoints",
            "true");
    }

    // Transport default definitions
    pqos.transport().use_builtin_transports = false;
    LibrarySettings library_settings;
    library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;

    switch (config.delivery_mechanism)
    {
        case CLIParser::DeliveryMechanismKind::INTRA_PROCESS:
        {
            // No transport needed, but at least a transport needs to be declared to avoid participant creation failure
            pqos.transport().use_builtin_transports = true;
            library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
            break;
        }
        case CLIParser::DeliveryMechanismKind::SHM:
        case CLIParser::DeliveryMechanismKind::DATA_SHARING:
        {
            std::shared_ptr<SharedMemTransportDescriptor> shm_transport_ =
                    std::make_shared<SharedMemTransportDescriptor>();
            shm_transport_->segment_size(shm_transport_->max_message_size() * max_samples);
            pqos.transport().user_transports.push_back(shm_transport_);
            break;
        }
        case CLIParser::DeliveryMechanismKind::LARGE_DATA:
        {
            // Large Data is a builtin transport
            pqos.transport().use_builtin_transports = true;
            pqos.setup_transports(BuiltinTransports::LARGE_DATA);
            break;
        }
        case CLIParser::DeliveryMechanismKind::TCPv4:
        {
            std::shared_ptr<TCPv4TransportDescriptor> tcp_v4_transport_ = std::make_shared<TCPv4TransportDescriptor>();
            pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastdds::dds::c_TimeInfinite;
            pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = dds::Duration_t(5, 0);
            tcp_v4_transport_->sendBufferSize = 0;
            tcp_v4_transport_->receiveBufferSize = 0;
            std::string tcp_ip_address = "127.0.0.1";
            if (!config.tcp_ip_address.empty())
            {
                tcp_ip_address = config.tcp_ip_address;
            }
            // Set unicast locators
            Locator_t tcp_v4_locator_;
            tcp_v4_locator_.kind = LOCATOR_KIND_TCPv4;
            IPLocator::setIPv4(tcp_v4_locator_, tcp_ip_address);
            IPLocator::setPhysicalPort(tcp_v4_locator_, 5100);
            pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(tcp_v4_locator_);
            pqos.wire_protocol().default_unicast_locator_list.push_back(tcp_v4_locator_);
            tcp_v4_transport_->set_WAN_address(tcp_ip_address);
            tcp_v4_transport_->add_listener_port(5100);
            pqos.transport().user_transports.push_back(tcp_v4_transport_);
            Locator tcp_v4_initial_peers_locator_;
            tcp_v4_initial_peers_locator_.kind = LOCATOR_KIND_TCPv4;
            tcp_v4_initial_peers_locator_.port = 5100;
            IPLocator::setIPv4(tcp_v4_initial_peers_locator_, tcp_ip_address);
            pqos.wire_protocol().builtin.initialPeersList.push_back(tcp_v4_initial_peers_locator_);
            break;
        }
        case CLIParser::DeliveryMechanismKind::TCPv6:
        {
            std::shared_ptr<TCPv6TransportDescriptor> tcp_v6_transport_ = std::make_shared<TCPv6TransportDescriptor>();
            pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastdds::dds::c_TimeInfinite;
            pqos.wire_protocol().builtin.discovery_config.leaseDuration_announcementperiod = dds::Duration_t(5, 0);
            tcp_v6_transport_->sendBufferSize = 0;
            tcp_v6_transport_->receiveBufferSize = 0;
            std::string tcp_ip_address = "::1";
            if (!config.tcp_ip_address.empty())
            {
                tcp_ip_address = config.tcp_ip_address;
            }
            // Set unicast locators
            Locator_t tcp_v6_locator_;
            tcp_v6_locator_.kind = LOCATOR_KIND_TCPv6;
            IPLocator::setIPv6(tcp_v6_locator_, tcp_ip_address);
            IPLocator::setPhysicalPort(tcp_v6_locator_, 5100);
            pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(tcp_v6_locator_);
            pqos.wire_protocol().default_unicast_locator_list.push_back(tcp_v6_locator_);
            tcp_v6_transport_->add_listener_port(5100);
            pqos.transport().user_transports.push_back(tcp_v6_transport_);
            Locator tcp_v6_initial_peers_locator_;
            tcp_v6_initial_peers_locator_.kind = LOCATOR_KIND_TCPv6;
            tcp_v6_initial_peers_locator_.port = 5100;
            IPLocator::setIPv6(tcp_v6_initial_peers_locator_, tcp_ip_address);
            pqos.wire_protocol().builtin.initialPeersList.push_back(tcp_v6_initial_peers_locator_);
            break;
        }
        case CLIParser::DeliveryMechanismKind::UDPv4:
        {
            pqos.transport().user_transports.push_back(std::make_shared<UDPv4TransportDescriptor>());
            break;
        }
        case CLIParser::DeliveryMechanismKind::UDPv6:
        {
            pqos.transport().user_transports.push_back(std::make_shared<UDPv6TransportDescriptor>());
            break;
        }
        default:
        {
            pqos.transport().use_builtin_transports = true;
            break;
        }
    }

    auto factory = DomainParticipantFactory::get_instance();
    factory->set_library_settings(library_settings);
    participant_ = factory->create_participant(config.domain, pqos, nullptr, StatusMask::none());
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

    // Create the data reader and data writer
    DataReaderQos reader_qos = DATAREADER_QOS_DEFAULT;
    DataWriterQos writer_qos = DATAWRITER_QOS_DEFAULT;
    subscriber_->get_default_datareader_qos(reader_qos);
    publisher_->get_default_datawriter_qos(writer_qos);
    reader_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    writer_qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    reader_qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    writer_qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    reader_qos.history().kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
    writer_qos.history().kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
    reader_qos.history().depth = max_samples;
    writer_qos.history().depth = max_samples;
    reader_qos.resource_limits().max_samples_per_instance = max_samples;
    writer_qos.resource_limits().max_samples_per_instance = max_samples;
    reader_qos.resource_limits().max_samples = reader_qos.resource_limits().max_instances * max_samples;
    writer_qos.resource_limits().max_samples = writer_qos.resource_limits().max_instances * max_samples;
    if (CLIParser::DeliveryMechanismKind::DATA_SHARING == config.delivery_mechanism)
    {
        reader_qos.data_sharing().automatic();
        writer_qos.data_sharing().automatic();
    }
    else
    {
        reader_qos.data_sharing().off();
        writer_qos.data_sharing().off();
    }
    reader_ = subscriber_->create_datareader(topic_, reader_qos, this, StatusMask::all());
    if (reader_ == nullptr)
    {
        throw std::runtime_error("DataReader initialization failed");
    }
    writer_ = publisher_->create_datawriter(topic_, writer_qos, this, StatusMask::all());
    if (writer_ == nullptr)
    {
        throw std::runtime_error("DataWriter initialization failed");
    }
}

PubSubApp::~PubSubApp()
{
    if (nullptr != participant_)
    {
        // Delete DDS entities contained within the DomainParticipant
        participant_->delete_contained_entities();

        // Delete DomainParticipant
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void PubSubApp::on_publication_matched(
        DataWriter* /*writer*/,
        const PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Pub matched." << std::endl;
        cv_.notify_one();
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Pub unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void PubSubApp::on_subscription_matched(
        DataReader* /*reader*/,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "Sub matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "Sub unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void PubSubApp::on_data_available(
        DataReader* reader)
{
    FASTDDS_CONST_SEQUENCE(DataSeq, DeliveryMechanisms);

    DataSeq delivery_mechanisms_sequence;
    SampleInfoSeq info_sequence;
    while ((!is_stopped()) && ((samples_ == 0) || ((samples_ > 0) && (received_samples_ < samples_)))
            && (RETCODE_OK == reader->take(delivery_mechanisms_sequence, info_sequence)))
    {
        for (LoanableCollection::size_type i = 0; i < info_sequence.length(); ++i)
        {
            if ((info_sequence[i].instance_state == ALIVE_INSTANCE_STATE) && info_sequence[i].valid_data)
            {
                const DeliveryMechanisms& delivery_mechanisms_ = delivery_mechanisms_sequence[i];

                received_samples_++;
                std::cout << "Sample: '" << delivery_mechanisms_.message().data() << "' with index: '" <<
                    delivery_mechanisms_.index() << "' RECEIVED" << std::endl;
            }
        }
        reader->return_loan(delivery_mechanisms_sequence, info_sequence);
    }
}

void PubSubApp::run()
{
    while (!is_stopped())
    {
        // publisher does not send more samples, but app does not stop
        if ((samples_ == 0) || ((samples_ > 0) && (index_of_last_sample_sent_ < samples_)))
        {
            if (!publish() && !is_stopped())
            {
                std::cout << "Error sending sample with index: '" << index_of_last_sample_sent_ << "'" << std::endl;
            }
        }

        // Wait for period or stop event
        std::unique_lock<std::mutex> terminate_lock(mutex_);
        cv_.wait_for(terminate_lock, std::chrono::milliseconds(period_ms_), [&]()
                {
                    return is_stopped();
                });
        // check wether the app needs to be stopped
        if ((samples_ > 0) && (received_samples_ >= samples_) && (index_of_last_sample_sent_ >= samples_))
        {
            stop();
        }
    }
}

bool PubSubApp::publish()
{
    bool ret = false;
    // Wait for the data endpoints delivery
    std::unique_lock<std::mutex> matched_lock(mutex_);
    cv_.wait(matched_lock, [&]()
            {
                // at least one has been discovered
                return ((matched_ > 0) || is_stopped());
            });
    void* sample_ = nullptr;
    if (!is_stopped() && (RETCODE_OK == writer_->loan_sample(sample_)))
    {
        DeliveryMechanisms* delivery_mechanisms_ = static_cast<DeliveryMechanisms*>(sample_);
        delivery_mechanisms_->index() = ++index_of_last_sample_sent_;
        memcpy(delivery_mechanisms_->message().data(), "Delivery mechanisms", sizeof("Delivery mechanisms"));
        ret = (RETCODE_OK == writer_->write(sample_));
        std::cout << "Sample: '" << delivery_mechanisms_->message().data() << "' with index: '"
                  << delivery_mechanisms_->index() << "' SENT" << std::endl;
    }
    return ret;
}

bool PubSubApp::is_stopped()
{
    return stop_.load();
}

void PubSubApp::stop()
{
    stop_.store(true);
    cv_.notify_all();
}

} // namespace delivery_mechanisms
} // namespace examples
} // namespace fastdds
} // namespace eprosima
