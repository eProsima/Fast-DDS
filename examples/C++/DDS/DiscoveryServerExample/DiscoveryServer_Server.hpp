#ifndef _DISCOVERYSERVER_SERVER_HPP_
#define _DISCOVERYSERVER_SERVER_HPP_

#include <string>

#include "../HelloWorldExample/HelloWorldPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

class DiscoveryServer_Server : public eprosima::fastdds::dds::DataWriterListener,
                            public eprosima::fastdds::dds::DataReaderListener
{
public:

    DiscoveryServer_Server();

    virtual ~DiscoveryServer_Server();

    bool init(
            const std::string& locator_str,
            const std::string& guid_prefix);

    void run();

    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;
        
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

private:

    void run_thread();

    HelloWorld received_hello_;
    HelloWorld sent_hello_;

    eprosima::fastdds::dds::DomainParticipant* server_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataWriter* writer_;
    eprosima::fastdds::dds::DataReader* reader_;
    eprosima::fastdds::dds::TypeSupport type_;

    bool stop_;
};

#endif // _DISCOVERYSERVER_SERVER_HPP_
