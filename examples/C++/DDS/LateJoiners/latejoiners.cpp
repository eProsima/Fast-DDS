#include <iostream>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "samplePubSubTypes.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

void latejoiners();

int main()
{
    latejoiners();
    return 0;
}

void latejoiners()
{

    TypeSupport sampleType(new samplePubSubType());
    sample my_sample;
    SampleInfo sample_info;

    //Create Participant
    DomainParticipantQos pqos;
    pqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos.name("PublisherParticipant");

    DomainParticipant* PubParticipant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (PubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Publisher Participant..." << std::endl;
        return;
    }

    //Register Type
    sampleType.register_type(PubParticipant);

    //Create Publisher
    std::cout << "Creating Publisher..." << std::endl;
    Publisher* myPub = PubParticipant->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (myPub == nullptr)
    {
        std::cout << "Something went wrong while creating the Publisher..." << std::endl;
        return;
    }

    //Create Topic
    Topic* PubTopic = PubParticipant->create_topic("samplePubSubTopic", sampleType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (PubTopic == nullptr)
    {
        std::cout << "Something went wrong while creating the Publisher Topic..." << std::endl;
        return;
    }

    //Create DataWriter
    DataWriterQos wqos;
    wqos.history().kind = KEEP_ALL_HISTORY_QOS;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    DataWriter* myWriter = myPub->create_datawriter(PubTopic, wqos);

    //Send 20 samples
    std::cout << "Publishing 20 samples on the topic" << std::endl;
    for (uint8_t j = 0; j < 20; j++)
    {
        my_sample.index(j + 1);
        my_sample.key_value(1);
        myWriter->write(&my_sample);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    //Create Subscriber Participant
    DomainParticipantQos pqos1;
    pqos1.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    pqos1.name("SubscriberParticipant");

    DomainParticipant* SubParticipant = DomainParticipantFactory::get_instance()->create_participant(0, pqos1);
    if (SubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Subscriber Participant..." << std::endl;
        return;
    }

    //Register Type
    sampleType.register_type(SubParticipant);

    //Transient Local Sub
    std::cout << "Creating Transient Local Subscriber..." << std::endl;
    Subscriber* mySub1 = SubParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    if (mySub1 == nullptr)
    {
        std::cout << "something went wrong while creating the Transient Local Subscriber..." << std::endl;
        return;
    }

    //Create Topic
    Topic* SubTopic = SubParticipant->create_topic("samplePubSubTopic", sampleType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (SubTopic == nullptr)
    {
        std::cout << "something went wrong while creating the Subscribers Topic..." << std::endl;
        return;
    }

    //Create DataReader
    DataReaderQos rtqos;
    rtqos.history().kind = KEEP_ALL_HISTORY_QOS;
    rtqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rtqos.durability().kind = TRANSIENT_LOCAL_DURABILITY_QOS;

    DataReader* myReader1 = mySub1->create_datareader(SubTopic, rtqos);

    if (myReader1 == nullptr)
    {
        std::cout << "something went wrong while creating the Transient Local Subscriber DataReader..." << std::endl;
        return;
    }

    //Volatile Sub
    std::cout << "Creating Volatile Subscriber..." << std::endl;
    Subscriber* mySub2 = SubParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    if (mySub2 == nullptr)
    {
        std::cout << "something went wrong while creating the Volatile Subscriber..." << std::endl;
        return;
    }

    //Create DataReader
    DataReaderQos rvqos;
    rvqos.history().kind = KEEP_ALL_HISTORY_QOS;
    rvqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    rvqos.durability().kind = VOLATILE_DURABILITY_QOS;

    DataReader* myReader2 = mySub2->create_datareader(SubTopic, rvqos);

    if (myReader2 == nullptr)
    {
        std::cout << "something went wrong while creating the Volatile Subscriber DataReader..." << std::endl;
        return;
    }

    //Send 20 samples
    std::cout << "Publishing 20 samples on the topic..." << std::endl;
    for (uint8_t j = 0; j < 20; j++)
    {
        my_sample.index(j + 21);
        my_sample.key_value(1);
        myWriter->write(&my_sample);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    //Read the contents of both histories:
    std::cout << "The Transient Local Subscriber holds: " << std::endl;
    while (myReader1->read_next_sample(&my_sample, &sample_info) == ReturnCode_t::RETCODE_OK)
    {
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;
    std::cout << "The Volatile Subscriber holds: " << std::endl;
    while (myReader2->read_next_sample(&my_sample, &sample_info) == ReturnCode_t::RETCODE_OK)
    {
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;

    mySub2->delete_datareader(myReader2);
    mySub1->delete_datareader(myReader1);
    SubParticipant->delete_subscriber(mySub1);
    SubParticipant->delete_subscriber(mySub2);
    SubParticipant->delete_topic(SubTopic);
    DomainParticipantFactory::get_instance()->delete_participant(SubParticipant);

    myPub->delete_datawriter(myWriter);
    PubParticipant->delete_publisher(myPub);
    PubParticipant->delete_topic(PubTopic);
    DomainParticipantFactory::get_instance()->delete_participant(PubParticipant);
}
