#include <iostream>
#include <string>

#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/Domain.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include "samplePubSubTypes.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

void fastest();

int main()
{
    fastest();
    return 0;
}

void fastest()
{

    samplePubSubType sampleType;
    sample my_sample;
    SampleInfo_t sample_info;

    ParticipantAttributes PparamPub;
    PparamPub.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PparamPub.rtps.setName("PublisherParticipant");

    Participant* PubParticipant = Domain::createParticipant(PparamPub);
    if (PubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Publisher Participant..." << std::endl;
        return;
    }
    Domain::registerType(PubParticipant, (TopicDataType*) &sampleType);


    //Publisher config
    PublisherAttributes Pparam;
    Pparam.topic.topicDataType = sampleType.getName();
    Pparam.topic.topicName = "samplePubSubTopic";

    Pparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Pparam.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Pparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    Pparam.topic.historyQos.depth =  5;

    std::cout << "Creating Best-Effort Publisher..." << std::endl;
    Publisher* myPub = Domain::createPublisher(PubParticipant, Pparam, nullptr);
    if (myPub == nullptr)
    {
        std::cout << "Something went wrong while creating the Publisher..." << std::endl;
        return;
    }


    ParticipantAttributes PparamSub;
    PparamSub.rtps.builtin.discovery_config.leaseDuration = c_TimeInfinite;
    PparamSub.rtps.setName("SubscriberParticipant");

    Participant* SubParticipant = Domain::createParticipant(PparamSub);
    if (SubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Subscriber Participant..." << std::endl;
        return;
    }
    Domain::registerType(SubParticipant, (TopicDataType*) &sampleType);

    // Sub
    SubscriberAttributes Rparam;
    Rparam.topic.topicDataType = sampleType.getName();
    Rparam.topic.topicName = "samplePubSubTopic";

    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Rparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    Rparam.topic.historyQos.depth =  5;

    std::cout << "Creating Subscriber..." << std::endl;
    Subscriber* mySub1 = Domain::createSubscriber(PubParticipant, Rparam, nullptr);
    if (myPub == nullptr)
    {
        std::cout << "something went wrong while creating the Subscriber..." << std::endl;
        return;
    }

    //Send 20 samples
    std::cout << "Publishing 10 samples on the topic..." << std::endl;
    for (uint8_t j = 0; j < 10; j++)
    {
        my_sample.index(j + 1);
        my_sample.key_value(1);
        myPub->write(&my_sample);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    //Read the contents of both histories:
    std::cout << "The Reliable Subscriber (with a history depth of 5) holds: " << std::endl;
    while (mySub1->readNextData(&my_sample, &sample_info))
    {
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;

    Domain::stopAll();
}
