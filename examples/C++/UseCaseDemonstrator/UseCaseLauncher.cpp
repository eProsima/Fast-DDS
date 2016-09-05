/***
 * Use Case Demonstrator for eProsima Fast RTPS
 * --------------------------------------------
 *
 *  This is an interactive program designed to show the effect of different configuration parameters on the behaviour of eProsima Fast RTPS
 *  
 ***/

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

void pastsamples();
void latejoiners();
void keys();
void incompatible();
void safest();
void fastest();
void triggers();

int main(){

    bool validinput;
    std::string userchoice;
    std::cout << "Welcome to eProsima Fast RTPS Use Case Demonstrator" << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Choose your example:" << std::endl;
    std::cout << " 1 - Past samples storage" << std::endl;
    std::cout << " 2 - Late Joiners" << std::endl;
    std::cout << " 3 - Keys" << std::endl;
    std::cout << " 4 - Incompatible configuration" << std::endl;
    std::cout << " 5 - Sample configuration: Safest" << std::endl;
    std::cout << " 6 - Sample configuration: Fastest" << std::endl;
    std::cout << " 7 - Sample configuration: Triggers" << std::endl;

    std::cout << "Make your choice (1 to 7, q to quit): ";
    validinput = false;
    while(!validinput){
        std::cin >> userchoice;
        if(userchoice == std::string("q"))
                return 0;
        int choice;
        try{
            choice = std::stoi(userchoice);
        }catch(std::invalid_argument){
            std::cout << "Please input a valid argument" << std::endl;
            continue;
        }
        validinput=true;
        std::cout << "---------------------------------------------------" << std::endl;
        switch(choice){
            case 1:
                pastsamples();
                break;
            case 2:
                latejoiners();
                break;
            case 3:
                keys();
                break;
            case 4:
                incompatible();
                break;
            case 5:
                safest();
                break;
            case 6:
                fastest();
                break;
            case 7:
                triggers();
                break;
            default:
                validinput = false;
                std::cout << "Please enter a valid option" << std::endl;
                break;
        }
    }
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Exiting..." << std::endl;
    return 0;
}


void latejoiners(){
    
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

    Pparam.topic.topicKind = NO_KEY;
    Pparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Pparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Pparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Pparam.topic.historyQos.depth =  50;
    Pparam.topic.resourceLimitsQos.max_samples = 100;
    Pparam.topic.resourceLimitsQos.max_instances = 1;
    Pparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Publisher..." << std::endl; 
    Publisher *myPub= Domain::createPublisher(PubParticipant, Pparam, nullptr);
    if(myPub == nullptr){
        std::cout << "Something went wrong while creating the Publisher..." << std::endl;
        return;
    }
  
    //Send 20 samples
    std::cout << "Publishing 20 samples on the topic" << std::endl;
    for(uint8_t j=0; j < 20; j++){
        my_sample.index(j+1); 
        my_sample.key_value(1);
        myPub->write(&my_sample);
    }

    eClock::my_sleep(1500);

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

    //Transient Local Sub
    SubscriberAttributes Rparam;
    Rparam.topic.topicDataType = sampleType.getName();
    Rparam.topic.topicName = "samplePubSubTopic";
    Rparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.topic.historyQos.depth =  50;
    Rparam.topic.resourceLimitsQos.max_samples = 100;
    Rparam.topic.resourceLimitsQos.max_instances = 1;
    Rparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Transient Local Subscriber..." << std::endl;
    Subscriber *mySub1= Domain::createSubscriber(PubParticipant, Rparam, nullptr);
    if(myPub == nullptr){
        std::cout << "something went wrong while creating the Transient Local Subscriber..." << std::endl;
        return;
    }

    //Volatile Sub
    SubscriberAttributes Rparam2;
    Rparam2.topic.topicDataType = sampleType.getName();
    Rparam2.topic.topicName = "samplePubSubTopic";
    Rparam2.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Rparam2.topic.topicKind = NO_KEY;
    Rparam2.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam2.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Rparam2.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam2.topic.historyQos.depth =  50;
    Rparam2.topic.resourceLimitsQos.max_samples = 100;
    Rparam2.topic.resourceLimitsQos.max_instances = 1;
    Rparam2.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Volatile Subscriber..." << std::endl;
    Subscriber *mySub2= Domain::createSubscriber(PubParticipant, Rparam2, nullptr);
    if(myPub == nullptr){
        std::cout << "something went wrong while creating the Volatile Subscriber..." << std::endl;
        return;
    }
    
    //Send 20 samples
    std::cout << "Publishing 20 samples on the topic..." << std::endl;
    for(uint8_t j=0; j < 20; j++){
        my_sample.index(j+21); 
        my_sample.key_value(1);
        myPub->write(&my_sample);
    }

    eClock::my_sleep(1500);

    //Read the contents of both histories:
        std::cout << "The Transient Local Subscriber holds: " << std::endl;
    while(mySub1->readNextData(&my_sample, &sample_info)){
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;
    std::cout << "The Volatile Subscriber holds: " << std::endl;
    while(mySub2->readNextData(&my_sample, &sample_info)){
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;

}
void pastsamples(){
    
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

    Pparam.topic.topicKind = NO_KEY;
    Pparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Pparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Pparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Pparam.topic.historyQos.depth =  50;
    Pparam.topic.resourceLimitsQos.max_samples = 100;
    Pparam.topic.resourceLimitsQos.max_instances = 1;
    Pparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

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

    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.topic.historyQos.depth =  50;
    Rparam.topic.resourceLimitsQos.max_samples = 100;
    Rparam.topic.resourceLimitsQos.max_instances = 1;
    Rparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Keep All Subscriber..." << std::endl;
    Subscriber *mySub1= Domain::createSubscriber(PubParticipant, Rparam, nullptr);
    if(myPub == nullptr){
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
    Subscriber *mySub2= Domain::createSubscriber(PubParticipant, Rparam2, nullptr);
    if(myPub == nullptr){
        std::cout << "something went wrong while creating the Volatile Subscriber..." << std::endl;
        return;
    }
    
    //Send 20 samples
    std::cout << "Publishing 20 samples on the topic..." << std::endl;
    for(uint8_t j=0; j < 20; j++){
        my_sample.index(j+1); 
        my_sample.key_value(1);
        myPub->write(&my_sample);
    }

    eClock::my_sleep(1500);

    //Read the contents of both histories:
        std::cout << "The Keep All Subscriber holds: " << std::endl;
    while(mySub1->readNextData(&my_sample, &sample_info)){
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;
    std::cout << "The Keep Last (Depth 10) Subscriber holds: " << std::endl;
    while(mySub2->readNextData(&my_sample, &sample_info)){
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;

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
    Pparam.topic.historyQos.depth =  10;
    Pparam.topic.resourceLimitsQos.max_samples = 50;
    Pparam.topic.resourceLimitsQos.max_instances = 5;
    Pparam.topic.resourceLimitsQos.max_samples_per_instance = 10;

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
    Rparam.topic.historyQos.depth =  10;
    Rparam.topic.resourceLimitsQos.max_samples = 100;
    Rparam.topic.resourceLimitsQos.max_instances = 5;
    Rparam.topic.resourceLimitsQos.max_samples_per_instance = 20;

    std::cout << "Creating Subscriber..." << std::endl;
    Subscriber *mySub1= Domain::createSubscriber(PubParticipant, Rparam, nullptr);
    if(myPub == nullptr){
        std::cout << "Something went wrong while creating the Subscriber..." << std::endl;
        return;
    }
    
    //Send 20 samples
    std::cout << "Publishing 5 keys, 10 samples per key..." << std::endl;
    for(uint8_t i=0; i < 5; i++){
        for(uint8_t j=0; j < 20; j++){
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
void incompatible(){
    
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

    Pparam.topic.topicKind = NO_KEY;
    Pparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Pparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Pparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    Pparam.topic.historyQos.depth =  50;
    Pparam.topic.resourceLimitsQos.max_samples = 100;
    Pparam.topic.resourceLimitsQos.max_instances = 1;
    Pparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Best-Effort Publisher..." << std::endl; 
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

    //Reliable Sub
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

    std::cout << "Creating Reliable Subscriber..." << std::endl;
    Subscriber *mySub1= Domain::createSubscriber(PubParticipant, Rparam, nullptr);
    if(myPub == nullptr){
        std::cout << "something went wrong while creating the Reliable Subscriber..." << std::endl;
        return;
    }

    //Best Effort Sub
    SubscriberAttributes Rparam2;
    Rparam2.topic.topicDataType = sampleType.getName();
    Rparam2.topic.topicName = "samplePubSubTopic";
    Rparam2.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Rparam2.topic.topicKind = NO_KEY;
    Rparam2.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam2.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Rparam2.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    Rparam2.topic.historyQos.depth =  10;
    Rparam2.topic.resourceLimitsQos.max_samples = 100;
    Rparam2.topic.resourceLimitsQos.max_instances = 1;
    Rparam2.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Best Effort Subscriber with depth 10..." << std::endl;
    Subscriber *mySub2= Domain::createSubscriber(PubParticipant, Rparam2, nullptr);
    if(myPub == nullptr){
        std::cout << "Something went wrong while creating the Best Effort Subscriber..." << std::endl;
        return;
    }
    
    //Send 20 samples
    std::cout << "Publishing 10 samples on the topic..." << std::endl;
    for(uint8_t j=0; j < 10; j++){
        my_sample.index(j+1); 
        my_sample.key_value(1);
        myPub->write(&my_sample);
    }

    eClock::my_sleep(1500);

    //Read the contents of both histories:
        std::cout << "The Reliable Subscriber holds: " << std::endl;
    while(mySub1->readNextData(&my_sample, &sample_info)){
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;
    std::cout << "TheBest Effort Subscriber holds: " << std::endl;
    while(mySub2->readNextData(&my_sample, &sample_info)){
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;

}
void safest(){
    
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

    Pparam.topic.topicKind = NO_KEY;
    Pparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Pparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Pparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Pparam.topic.historyQos.depth =  50;
    Pparam.topic.resourceLimitsQos.max_samples = 100;
    Pparam.topic.resourceLimitsQos.max_instances = 1;
    Pparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Best-Effort Publisher..." << std::endl; 
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

    // Sub
    SubscriberAttributes Rparam;
    Rparam.topic.topicDataType = sampleType.getName();
    Rparam.topic.topicName = "samplePubSubTopic";
    Rparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.topic.historyQos.depth =  50;
    Rparam.topic.resourceLimitsQos.max_samples = 100;
    Rparam.topic.resourceLimitsQos.max_instances = 1;
    Rparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Subscriber..." << std::endl;
    Subscriber *mySub1= Domain::createSubscriber(PubParticipant, Rparam, nullptr);
    if(myPub == nullptr){
        std::cout << "something went wrong while creating the Subscriber..." << std::endl;
        return;
    }
    
    //Send 20 samples
    std::cout << "Publishing 10 samples on the topic..." << std::endl;
    for(uint8_t j=0; j < 10; j++){
        my_sample.index(j+1); 
        my_sample.key_value(1);
        myPub->write(&my_sample);
    }

    eClock::my_sleep(1500);

    //Read the contents of both histories:
        std::cout << "The Reliable Subscriber holds: " << std::endl;
    while(mySub1->readNextData(&my_sample, &sample_info)){
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;

}

void fastest(){

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

    Pparam.topic.topicKind = NO_KEY;
    Pparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Pparam.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Pparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    Pparam.topic.historyQos.depth =  50;
    Pparam.topic.resourceLimitsQos.max_samples = 100;
    Pparam.topic.resourceLimitsQos.max_instances = 1;
    Pparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Best-Effort Publisher..." << std::endl; 
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

    // Sub
    SubscriberAttributes Rparam;
    Rparam.topic.topicDataType = sampleType.getName();
    Rparam.topic.topicName = "samplePubSubTopic";
    Rparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Rparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    Rparam.topic.historyQos.depth =  50;
    Rparam.topic.resourceLimitsQos.max_samples = 100;
    Rparam.topic.resourceLimitsQos.max_instances = 1;
    Rparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Subscriber..." << std::endl;
    Subscriber *mySub1= Domain::createSubscriber(PubParticipant, Rparam, nullptr);
    if(myPub == nullptr){
        std::cout << "something went wrong while creating the Subscriber..." << std::endl;
        return;
    }
    
    //Send 20 samples
    std::cout << "Publishing 10 samples on the topic..." << std::endl;
    for(uint8_t j=0; j < 10; j++){
        my_sample.index(j+1); 
        my_sample.key_value(1);
        myPub->write(&my_sample);
    }

    eClock::my_sleep(1500);

    //Read the contents of both histories:
        std::cout << "The Reliable Subscriber holds: " << std::endl;
    while(mySub1->readNextData(&my_sample, &sample_info)){
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;




}
void triggers(){

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

    Pparam.topic.topicKind = NO_KEY;
    Pparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Pparam.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Pparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Pparam.topic.historyQos.depth =  50;
    Pparam.topic.resourceLimitsQos.max_samples = 100;
    Pparam.topic.resourceLimitsQos.max_instances = 1;
    Pparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Best-Effort Publisher..." << std::endl; 
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

    // Sub
    SubscriberAttributes Rparam;
    Rparam.topic.topicDataType = sampleType.getName();
    Rparam.topic.topicName = "samplePubSubTopic";
    Rparam.historyMemoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;

    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.qos.m_durability.kind = VOLATILE_DURABILITY_QOS;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.topic.historyQos.depth =  50;
    Rparam.topic.resourceLimitsQos.max_samples = 100;
    Rparam.topic.resourceLimitsQos.max_instances = 1;
    Rparam.topic.resourceLimitsQos.max_samples_per_instance = 100;

    std::cout << "Creating Subscriber..." << std::endl;
    Subscriber *mySub1= Domain::createSubscriber(PubParticipant, Rparam, nullptr);
    if(myPub == nullptr){
        std::cout << "something went wrong while creating the Subscriber..." << std::endl;
        return;
    }
    
    //Send 20 samples
    std::cout << "Publishing 10 samples on the topic..." << std::endl;
    for(uint8_t j=0; j < 10; j++){
        my_sample.index(j+1); 
        my_sample.key_value(1);
        myPub->write(&my_sample);
    }

    eClock::my_sleep(1500);

    //Read the contents of both histories:
        std::cout << "The Subscriber holds: " << std::endl;
    while(mySub1->readNextData(&my_sample, &sample_info)){
        std::cout << std::to_string(my_sample.index()) << " ";
    }
    std::cout << std::endl;
}

