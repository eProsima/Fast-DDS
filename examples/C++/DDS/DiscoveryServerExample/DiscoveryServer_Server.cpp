#include "DiscoveryServer_Server.hpp"

#include <iostream>
#include <sstream>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/rtps/common/Locator.h>
#include <fastrtps/utils/IPLocator.h>

DiscoveryServer_Server::DiscoveryServer_Server()
    : server_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new HelloWorldPubSubType())
    , stop_(false)
{
}

bool DiscoveryServer_Server::init(
        const std::string& locator_str,
        const std::string& guid_prefix)
{
    eprosima::fastrtps::rtps::Locator_t locator;

    sent_hello_.index(0);
    sent_hello_.message("HelloWorld Server");

    size_t delimiter_pos = locator_str.find(":");
    std::string address = locator_str.substr(0, delimiter_pos);
    if (!eprosima::fastrtps::rtps::IPLocator::isIPv4(address))
    {
        auto response = eprosima::fastrtps::rtps::IPLocator::resolveNameDNS(address);
        if (response.first.size() > 0)
        {
            address = response.first.begin()->data();
        }
    } 
    if (!eprosima::fastrtps::rtps::IPLocator::setIPv4(locator, address))
    {
        return false;
    }
    if (std::string::npos != delimiter_pos)
    {
        locator.port = atoi(locator_str.substr(delimiter_pos + 1, locator_str.length()).c_str());
    }
    else
    {
        locator.port = eprosima::fastdds::rtps::DEFAULT_ROS2_SERVER_PORT;
    }

    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastrtps::rtps::DiscoveryProtocol::SERVER;
    std::istringstream(guid_prefix) >> pqos.wire_protocol().prefix;
    pqos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);
    // Other required Server parameters are set using Environment File
    server_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (nullptr == server_)
    {
        return false;
    }
    std::cout << "SERVER with GUID prefix " << pqos.wire_protocol().prefix << " and listening locator " << locator <<
            " CREATED" << std::endl;
    type_.register_type(server_);
    publisher_ = server_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);
    if (nullptr == publisher_)
    {
        return false;
    }
    subscriber_ = server_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (nullptr == subscriber_)
    {
        return false;
    }
    topic_ = server_->create_topic("HelloWorldTopic", type_.get_type_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    if (nullptr == topic_)
    {
        return false;
    }
    writer_ = publisher_->create_datawriter(topic_, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT, this);
    if (nullptr == writer_)
    {
        return false;
    }
    reader_ = subscriber_->create_datareader(topic_, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, this);
    if (nullptr == reader_)
    {
        return false;
    }
    return true;
}

DiscoveryServer_Server::~DiscoveryServer_Server()
{
    server_->delete_contained_entities();
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(server_);
}

void DiscoveryServer_Server::on_subscription_matched(
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

void DiscoveryServer_Server::on_data_available(
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

void DiscoveryServer_Server::on_publication_matched(
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

void DiscoveryServer_Server::run_thread()
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

void DiscoveryServer_Server::run()
{
    std::thread thread(&DiscoveryServer_Server::run_thread, this);
    std::cout << "Server running with DataWriter publishing. Please press any key to stop the Server at any time." <<
            std::endl;
    std::cin.ignore();
    stop_ = true;
    thread.join();
}
