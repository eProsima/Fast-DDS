#include "DiscoveryServer_Client.hpp"

#include <iostream>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/types/TypesBase.h>

DiscoveryServer_Client::DiscoveryServer_Client()
    : client_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new HelloWorldPubSubType)
    , stop_(false)
{
}

bool DiscoveryServer_Client::init()
{
    sent_hello_.index(0);
    sent_hello_.message("HelloWorld Client");

    // discoveryProtocol SIMPLE so the environment variable takes effect
    client_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0,
            eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    if (nullptr == client_)
    {
        return false;
    }
    type_.register_type(client_);
    publisher_ = client_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);
    if (nullptr == publisher_)
    {
        return false;
    }
    subscriber_ = client_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (nullptr == subscriber_)
    {
        return false;
    }
    topic_ = client_->create_topic("HelloWorldTopic", type_.get_type_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    if (nullptr == topic_)
    {
        return false;
    }
    reader_ = subscriber_->create_datareader(topic_, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, this);
    if (nullptr == reader_)
    {
        return false;
    }
    writer_ = publisher_->create_datawriter(topic_, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT, this);
    if (nullptr == writer_)
    {
        return false;
    }
    return true;
}

DiscoveryServer_Client::~DiscoveryServer_Client()
{
    client_->delete_contained_entities();
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(client_);
}

void DiscoveryServer_Client::on_subscription_matched(
        eprosima::fastdds::dds::DataReader*,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "DataReader matched" << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "DataReader unmatched" << std::endl;
    }
}

void DiscoveryServer_Client::on_data_available(
        eprosima::fastdds::dds::DataReader* reader)
{
    eprosima::fastdds::dds::SampleInfo info;
    if (eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK == reader->take_next_sample(&received_hello_, &info))
    {
        if (info.valid_data && info.publication_handle != writer_->get_instance_handle())
        {
            std::cout << "Message " << received_hello_.message() << " RECEIVED in topic " << topic_->get_name() <<
                    " with index: " << received_hello_.index() << std::endl;
        }
    }
}

void DiscoveryServer_Client::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        std::cout << "DataWriter matched" << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        std::cout << "DataWriter unmatched" << std::endl;
    }
}

void DiscoveryServer_Client::run_thread()
{
    while (!stop_)
    {
        sent_hello_.index(++sent_hello_.index());
        writer_->write(&sent_hello_);
        std::cout << "Message: " << sent_hello_.message() << " SENT in topic " << topic_->get_name() << " with index: " <<
                sent_hello_.index() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void DiscoveryServer_Client::run()
{
    std::thread thread(&DiscoveryServer_Client::run_thread, this);
    std::cout << "Client running. Please press any key to stop the client at any time." << std::endl;
    std::cin.ignore();
    stop_ = true;
    thread.join();
}
