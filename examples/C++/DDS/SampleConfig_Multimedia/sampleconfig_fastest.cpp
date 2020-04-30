#include <iostream>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "samplePubSubTypes.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

void fastest();

int main()
{
    fastest();
    return 0;
}

void fastest()
{

    TypeSupport sampleType(new samplePubSubType());
    sample my_sample;
    SampleInfo sample_info;

    //Create Publisher Participant
    DomainParticipantQos ppqos;
    ppqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    ppqos.name("PublisherParticipant");

    DomainParticipant* PubParticipant = DomainParticipantFactory::get_instance()->create_participant(0, ppqos);
    if (PubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Publisher Participant..." << std::endl;
        return;
    }
    //Register Type
    sampleType.register_type(PubParticipant);

    //Create Publisher
    std::cout << "Creating Best-Effort Publisher..." << std::endl;
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
    wqos.history().kind = KEEP_LAST_HISTORY_QOS;
    wqos.durability().kind = VOLATILE_DURABILITY_QOS;
    wqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    wqos.history().depth =  5;

    DataWriter* myWriter = myPub->create_datawriter(PubTopic, wqos);

    if (myWriter == nullptr)
    {
        std::cout << " Something went wrong while creating the Publisher DataWriter..." << std::endl;
        return;
    }


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
    //Register Type
    sampleType.register_type(SubParticipant);

    //Create Subscriber
    std::cout << "Creating Subscriber..." << std::endl;
    Subscriber* mySub = SubParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    if (mySub == nullptr)
    {
        std::cout << "something went wrong while creating the Subscriber..." << std::endl;
        return;
    }

    //Create Topic
    Topic* SubTopic = SubParticipant->create_topic("samplePubSubTopic", sampleType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (SubTopic == nullptr)
    {
        std::cout << "something went wrong while creating the Subscriber Topic..." << std::endl;
        return;
    }

    //Create DataReader
    DataReaderQos rqos;
    rqos.history().kind = KEEP_LAST_HISTORY_QOS;
    rqos.durability().kind = VOLATILE_DURABILITY_QOS;
    rqos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
    rqos.history().depth =  5;

    DataReader* myReader = mySub->create_datareader(SubTopic, rqos);

    if (myReader == nullptr)
    {
        std::cout << " Something went wrong while creating the Subscriber DataReader..." << std::endl;
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    //Send 20 samples
    std::cout << "Publishing 10 samples on the topic..." << std::endl;
    for (uint8_t j = 0; j < 10; j++)
    {
        my_sample.index(j + 1);
        my_sample.key_value(1);
        if (myWriter->write(&my_sample))
        {
            std::cout << std::to_string(my_sample.index()) << " ";
        }
    }
    std::cout << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    //Read the contents of both histories:
    std::cout << "The Reliable Subscriber (with a history depth of 5) holds: " << std::endl;
    while (myReader->read_next_sample(&my_sample, &sample_info) == ReturnCode_t::RETCODE_OK)
    {
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;

    myPub->delete_datawriter(myWriter);
    PubParticipant->delete_publisher(myPub);
    PubParticipant->delete_topic(PubTopic);
    DomainParticipantFactory::get_instance()->delete_participant(PubParticipant);

    mySub->delete_datareader(myReader);
    SubParticipant->delete_subscriber(mySub);
    SubParticipant->delete_topic(SubTopic);
    DomainParticipantFactory::get_instance()->delete_participant(SubParticipant);
}
