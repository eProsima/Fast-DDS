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

void safest();

int main()
{
    safest();
    return 0;
}

void safest()
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
    Pparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Pparam.topic.topicKind = WITH_KEY;
    Pparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Pparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Pparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Pparam.topic.historyQos.depth =  50;

    std::cout << "Creating Reliable Publisher..." << std::endl;
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
    Rparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Rparam.topic.topicKind = WITH_KEY;
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.topic.historyQos.depth =  50;

    std::cout << "Creating Subscriber..." << std::endl;
    Subscriber* mySub1 = Domain::createSubscriber(PubParticipant, Rparam, nullptr);
    if (myPub == nullptr)
    {
        std::cout << "something went wrong while creating the Subscriber..." << std::endl;
        return;
    }

    //Send 4 samples on 2 keys
    std::cout << "Publishing 8 samples distributed on 2 keys..." << std::endl;
    my_sample.key_value(1);
    for (uint8_t j = 1; j <= 4; j++)
    {
        my_sample.index(j);
        myPub->write(&my_sample);
    }
    my_sample.key_value(2);
    for (uint8_t j = 5; j <= 8; j++)
    {
        my_sample.index(j);
        myPub->write(&my_sample);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    //Read the contents of both histories:
    std::cout << "The Reliable Subscriber holds: " << std::endl;
    while (mySub1->readNextData(&my_sample, &sample_info))
    {
        std::cout << std::to_string(my_sample.index()) << " (key " << static_cast<int>(my_sample.key_value()) << ")" <<
                std::endl;
    }
    std::cout << std::endl;

    Domain::stopAll();
}
