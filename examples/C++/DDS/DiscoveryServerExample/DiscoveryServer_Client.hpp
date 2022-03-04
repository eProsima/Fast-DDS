#ifndef _DISCOVERYSERVER_CLIENT_HPP_
#define _DISCOVERYSERVER_CLIENT_HPP_

#include "../HelloWorldExample/HelloWorldPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

class DiscoveryServer_Client : public eprosima::fastdds::dds::DataReaderListener,
                            public eprosima::fastdds::dds::DataWriterListener
{
public:

    DiscoveryServer_Client();

    virtual ~DiscoveryServer_Client();

    bool init();

    void run();

    void on_subscription_matched(
            eprosima::fastdds::dds::DataReader* reader,
            const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;
        
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* writer,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

private:

    void run_thread();

    HelloWorld received_hello_;
    HelloWorld sent_hello_;

    eprosima::fastdds::dds::DomainParticipant* client_;
    eprosima::fastdds::dds::Publisher* publisher_;
    eprosima::fastdds::dds::Subscriber* subscriber_;
    eprosima::fastdds::dds::Topic* topic_;
    eprosima::fastdds::dds::DataReader* reader_;
    eprosima::fastdds::dds::DataWriter* writer_;
    eprosima::fastdds::dds::TypeSupport type_;

    bool stop_;
};

#endif // _DISCOVERYSERVER_CLIENT_HPP_
