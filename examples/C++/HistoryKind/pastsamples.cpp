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

//Enums and configuration structure
enum Reliability_type { Best_Effort, Reliable };
enum Durability_type { Transient_Local, Volatile };
enum HistoryKind_type { Keep_Last, Keep_All };
enum Key_type { No_Key, With_Key};

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

    Pparam.topic.topicKind = NO_KEY;
    Pparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Pparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Pparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Pparam.topic.historyQos.depth =  50;
    Pparam.topic.resourceLimitsQos.max_samples = 100;
    Pparam.topic.resourceLimitsQos.max_instances = 1;
    Pparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Publisher..." << std::endl;
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

    //Keep All Sub
    SubscriberAttributes Rparam;
    Rparam.topic.topicDataType = sampleType.getName();
    Rparam.topic.topicName = "samplePubSubTopic";
    Rparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.topic.historyQos.depth =  50;
    Rparam.topic.resourceLimitsQos.max_samples = 100;
    Rparam.topic.resourceLimitsQos.max_instances = 1;
    Rparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Keep All Subscriber..." << std::endl;
    Subscriber* mySub1 = Domain::createSubscriber(PubParticipant, Rparam, nullptr);
    if (myPub == nullptr)
    {
        std::cout << "something went wrong while creating the Transient Local Subscriber..." << std::endl;
        return;
    }

    //Keep Last
    SubscriberAttributes Rparam2;
    Rparam2.topic.topicDataType = sampleType.getName();
    Rparam2.topic.topicName = "samplePubSubTopic";
    Rparam2.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Rparam2.topic.topicKind = NO_KEY;
    Rparam2.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam2.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Rparam2.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam2.topic.historyQos.depth =  10;
    Rparam2.topic.resourceLimitsQos.max_samples = 100;
    Rparam2.topic.resourceLimitsQos.max_instances = 1;
    Rparam2.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Keep Last Subscriber with depth 10..." << std::endl;
    Subscriber* mySub2 = Domain::createSubscriber(PubParticipant, Rparam2, nullptr);
    if (myPub == nullptr)
    {
        std::cout << "something went wrong while creating the Volatile Subscriber..." << std::endl;
        return;
    }

    //Send 20 samples
    std::cout << "Publishing 20 samples on the topic..." << std::endl;
    for (uint8_t j = 0; j < 20; j++)
    {
        my_sample.index(j + 1);
        my_sample.key_value(1);
        myPub->write(&my_sample);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    //Read the contents of both histories:
    std::cout << "The Keep All Subscriber holds: " << std::endl;
    while (mySub1->readNextData(&my_sample, &sample_info))
    {
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;
    std::cout << "The Keep Last (Depth 10) Subscriber holds: " << std::endl;
    while (mySub2->readNextData(&my_sample, &sample_info))
    {
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;

    Domain::removeParticipant(PubParticipant);
    Domain::removeParticipant(SubParticipant);
}
