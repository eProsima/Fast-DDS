#include <iostream>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "samplePubSubTypes.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

//Enums and configuration structure
enum Reliability_type
{
    Best_Effort, Reliable
};
enum Durability_type
{
    Transient_Local, Volatile
};
enum HistoryKind_type
{
    Keep_Last, Keep_All
};
enum Key_type
{
    No_Key, With_Key
};

typedef struct
{
    Reliability_type reliability;
    Durability_type durability;
    HistoryKind_type historykind;
    Key_type keys;
    uint16_t history_size;
    uint8_t depth;
    uint8_t no_keys;
    uint16_t max_samples_per_key;
} example_configuration;

void pastsamples();

int main()
{
    pastsamples();
    return 0;
}

void pastsamples()
{

    TypeSupport sampleType(new samplePubSubType());
    sample my_sample;
    SampleInfo sample_info;

    //Create Publisher Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.name("PublisherParticipant");

    DomainParticipant* PubParticipant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (PubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Publisher Participant..." << std::endl;
        return;
    }

    //Register the type
    sampleType.register_type(PubParticipant);

    //Create Publisher
    Publisher* myPub = PubParticipant->create_publisher(PUBLISHER_QOS_DEFAULT);
    if (myPub == nullptr)
    {
        std::cout << "Something went wrong while creating the Publisher..." << std::endl;
        return;
    }

    //Create Topic
    Topic* myTopic = PubParticipant->create_topic("samplePubSubTopic", sampleType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (myTopic == nullptr)
    {
        std::cout << "Something went wrong while creating the Topic..." << std::endl;
        return;
    }

    //Create DataWriter
    DataWriterQos wqos;
    wqos.endpoint().history_memory_policy = DYNAMIC_RESERVE_MEMORY_MODE;
    wqos.history().kind = KEEP_ALL_HISTORY_QOS;
    wqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.history().depth =  50;
    wqos.resource_limits().max_samples = 100;
    wqos.resource_limits().max_instances = 1;
    wqos.resource_limits().max_samples_per_instance = 100;

    DataWriter* myWriter = myPub->create_datawriter(myTopic, wqos);

    std::cout << "Creating Publisher..." << std::endl;

    //Create Subscriber Participant
    DomainParticipantQos psqos;
    psqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    psqos.name("SubscriberParticipant");

    DomainParticipant* SubParticipant = DomainParticipantFactory::get_instance()->create_participant(0, psqos);
    if (SubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Subscriber Participant..." << std::endl;
        return;
    }

    //Register the Type
    sampleType.register_type(SubParticipant);

    //Keep All Sub
    Subscriber* mySub1 = SubParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    if (mySub1 == nullptr)
    {
        std::cout << "something went wrong while creating the Transient Local Subscriber..." << std::endl;
        return;
    }

    //Create Topic
    Topic* myTopic2 = SubParticipant->create_topic("samplePubSubTopic", sampleType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (myTopic2 == nullptr)
    {
        std::cout << "Something went wrong while creating the Topic..." << std::endl;
        return;
    }

    DataReaderQos rqos1;
    rqos1.endpoint().history_memory_policy = DYNAMIC_RESERVE_MEMORY_MODE;
    rqos1.history().kind = KEEP_ALL_HISTORY_QOS;
    rqos1.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    rqos1.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos1.history().depth =  50;
    rqos1.resource_limits().max_samples = 100;
    rqos1.resource_limits().max_instances = 1;
    rqos1.resource_limits().max_samples_per_instance = 100;

    DataReader* myReader1 = mySub1->create_datareader(myTopic2, rqos1);

    std::cout << "Creating Keep All Subscriber..." << std::endl;

    //Keep Last
    Subscriber* mySub2 = SubParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    if (mySub2 == nullptr)
    {
        std::cout << "something went wrong while creating the Volatile Subscriber..." << std::endl;
        return;
    }

    DataReaderQos rqos2;
    rqos2.endpoint().history_memory_policy = DYNAMIC_RESERVE_MEMORY_MODE;
    rqos2.history().kind = KEEP_LAST_HISTORY_QOS;
    rqos2.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    rqos2.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rqos2.history().depth =  10;
    rqos2.resource_limits().max_samples = 100;
    rqos2.resource_limits().max_instances = 1;
    rqos2.resource_limits().max_samples_per_instance = 100;

    DataReader* myReader2 = mySub2->create_datareader(myTopic2, rqos2);

    std::cout << "Creating Keep Last Subscriber with depth 10..." << std::endl;

    //Send 20 samples
    std::cout << "Publishing 20 samples on the topic..." << std::endl;
    for (uint8_t j = 0; j < 20; j++)
    {
        my_sample.index(j + 1);
        my_sample.key_value(1);
        myWriter->write(&my_sample);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    //Read the contents of both histories:
    std::cout << "The Keep All Subscriber holds: " << std::endl;
    while (myReader1->read_next_sample(&my_sample, &sample_info) == ReturnCode_t::RETCODE_OK)
    {
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;

    std::cout << "The Keep Last (Depth 10) Subscriber holds: " << std::endl;
    while (myReader2->read_next_sample(&my_sample, &sample_info) == ReturnCode_t::RETCODE_OK)
    {
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;

    std::cout << "Deleting Publisher" << std::endl;

    myPub->delete_datawriter(myWriter);
    PubParticipant->delete_publisher(myPub);
    PubParticipant->delete_topic(myTopic);
    DomainParticipantFactory::get_instance()->delete_participant(PubParticipant);

    std::cout << "Deleting Subscribers" << std::endl;

    mySub1->delete_datareader(myReader1);
    mySub2->delete_datareader(myReader2);
    SubParticipant->delete_subscriber(mySub1);
    SubParticipant->delete_subscriber(mySub2);
    SubParticipant->delete_topic(myTopic2);
    DomainParticipantFactory::get_instance()->delete_participant(SubParticipant);
}
