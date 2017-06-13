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

#include <fastrtps/utils/eClock.h>

#include "samplePubSubTypes.h"

//Enums and configuration structure
enum Reliability_type { Best_Effort, Reliable };
enum Durability_type { Transient_Local, Volatile };
enum HistoryKind_type { Keep_Last, Keep_All };
enum Key_type { No_Key, With_Key};

typedef struct{
    Reliability_type reliability;
    Durability_type durability;
    HistoryKind_type historykind;
    Key_type keys;
    uint16_t history_size;
    uint8_t depth;
    uint8_t no_keys;
    uint16_t max_samples_per_key;
} example_configuration;

void keys();

int main(){
    keys();
    return 0;
}

void keys(){

    samplePubSubType sampleType;
    sample my_sample;
    SampleInfo_t sample_info;

    ParticipantAttributes PparamPub;
    PparamPub.rtps.builtin.domainId = 0;
    PparamPub.rtps.builtin.leaseDuration = c_TimeInfinite;
    PparamPub.rtps.setName("PublisherParticipant");

    Participant *PubParticipant = Domain::createParticipant(PparamPub);
    if(PubParticipant == nullptr){
        std::cout << " Something went wrong while creating the Publisher Participant..." << std::endl;
        return;
    }
    Domain::registerType(PubParticipant,(TopicDataType*) &sampleType);


    //Publisher config
    PublisherAttributes Pparam;
    Pparam.topic.topicDataType = sampleType.getName();
    Pparam.topic.topicName = "samplePubSubTopic";
    Pparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Pparam.topic.topicKind = WITH_KEY;
    Pparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Pparam.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Pparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Pparam.topic.resourceLimitsQos.max_samples = 100;
    Pparam.topic.resourceLimitsQos.allocated_samples = 100;
    Pparam.topic.resourceLimitsQos.max_instances = 5;
    Pparam.topic.resourceLimitsQos.max_samples_per_instance = 20;

    std::cout << "Creating Publisher..." << std::endl;
    Publisher *myPub= Domain::createPublisher(PubParticipant, Pparam, nullptr);
    if(myPub == nullptr){
        std::cout << "Something went wrong while creating the Publisher..." << std::endl;
        return;
    }


    ParticipantAttributes PparamSub;
    PparamSub.rtps.builtin.domainId = 0;
    PparamSub.rtps.builtin.leaseDuration = c_TimeInfinite;
    PparamSub.rtps.setName("SubscriberParticipant");

    Participant *SubParticipant = Domain::createParticipant(PparamSub);
    if(SubParticipant == nullptr){
        std::cout << " Something went wrong while creating the Subscriber Participant..." << std::endl;
        return;
    }
    Domain::registerType(SubParticipant,(TopicDataType*) &sampleType);

    //Keep All Sub
    SubscriberAttributes Rparam;
    Rparam.topic.topicDataType = sampleType.getName();
    Rparam.topic.topicName = "samplePubSubTopic";
    Rparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Rparam.topic.topicKind = WITH_KEY;
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.topic.resourceLimitsQos.max_samples = 100;
    Rparam.topic.resourceLimitsQos.allocated_samples = 100;
    Rparam.topic.resourceLimitsQos.max_instances = 5;
    Rparam.topic.resourceLimitsQos.max_samples_per_instance = 20;

    std::cout << "Creating Subscriber..." << std::endl;
    Subscriber *mySub1= Domain::createSubscriber(PubParticipant, Rparam, nullptr);
    if(myPub == nullptr){
        std::cout << "Something went wrong while creating the Subscriber..." << std::endl;
        return;
    }

    //Send 10 samples
    std::cout << "Publishing 5 keys, 10 samples per key..." << std::endl;
    for(uint8_t i=0; i < 5; i++){
        for(uint8_t j=0; j < 10; j++){
            my_sample.index(j+1);
            my_sample.key_value(i+1);
            myPub->write(&my_sample);
        }
    }

    eClock::my_sleep(1500);

    std::cout << "Publishing 10 more samples on a key 3..." << std::endl;
    for(uint8_t j=0; j < 10; j++){
        my_sample.index(j+11);
        my_sample.key_value(3);
        myPub->write(&my_sample);
    }

    eClock::my_sleep(1500);

    //Read the contents of both histories:
    std::vector< std::pair<int,int> > sampleList;
        std::cout << "The Subscriber holds: " << std::endl;
    while(mySub1->readNextData(&my_sample, &sample_info)){
        sampleList.push_back(std::pair<int,int>(my_sample.index(),my_sample.key_value()));
    }
    for(int key=1;key<=5;key++){
        std::cout << "  On key " << std::to_string(key) << ": ";
        for(std::pair<int,int> n : sampleList){
            if(n.second == key)
               std::cout << std::to_string(n.first) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

}
