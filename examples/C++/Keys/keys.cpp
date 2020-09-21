#include <iostream>
#include <string>
#include <condition_variable>

#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/Domain.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include "samplePubSubTypes.h"

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

constexpr uint8_t max_instances = 5;

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

class PubListener : public eprosima::fastrtps::PublisherListener
{
public:

    PubListener()
        : n_matched(0)
        , firstConnected(false)
    {
    }

    ~PubListener()
    {
    }

    void onPublicationMatched(
            eprosima::fastrtps::Publisher* /*pub*/,
            eprosima::fastrtps::rtps::MatchingInfo& info)
    {
        if (info.status == MATCHED_MATCHING)
        {
            n_matched++;
        }
        else
        {
            n_matched--;
        }
        cv_.notify_all();
    }

    int n_matched;
    bool firstConnected;
    std::condition_variable cv_;
    std::mutex mutex_;
};

class SubListener : public eprosima::fastrtps::SubscriberListener
{
public:

    SubListener()
        : n_matched(0)
        , n_samples(0)
    {
    }

    ~SubListener()
    {
    }

    void onSubscriptionMatched(
            eprosima::fastrtps::Subscriber* /*sub*/,
            eprosima::fastrtps::rtps::MatchingInfo& info)
    {
        if (info.status == MATCHED_MATCHING)
        {
            n_matched++;
        }
        else
        {
            n_matched--;
        }
        cv_.notify_all();
    }

    void onNewDataMessage(
            eprosima::fastrtps::Subscriber* sub)
    {
        if (sub->takeNextData((void*)&m_sample, &m_info))
        {
            if (m_info.sampleKind == ALIVE)
            {
                this->n_samples++;
                // Print your structure data here.
                std::cout << this->n_samples << "Message " << (uint32_t)(m_sample.index()) << " RECEIVED" << std::endl;
            }
        }
    }

    sample m_sample;
    eprosima::fastrtps::SampleInfo_t m_info;
    int n_matched;
    uint32_t n_samples;
    std::condition_variable cv_;
    std::mutex mutex_;
};

void keys();

void publisherKeys();

void subscriberKeys();

Publisher* initPublisher(
        samplePubSubType& sampleType,
        PubListener& listener);

Subscriber* initSubscriber(
        samplePubSubType& sampleType,
        SubListener* listener);

int main(
        int argc,
        char** argv)
{
    int iMode = -1;
    if (argc > 1)
    {
        if (strcmp(argv[1], "publisher") == 0)
        {
            iMode = 1;
        }
        else if (strcmp(argv[1], "subscriber") == 0)
        {
            iMode = 2;
        }
    }

    switch (iMode)
    {
        case 1:
            publisherKeys();
            break;
        case 2:
            subscriberKeys();
            break;
        default:
            keys();
            break;
    }
    return 0;
}

Publisher* initPublisher(
        samplePubSubType& sampleType,
        PubListener& listener)
{
    ParticipantAttributes PparamPub;
    PparamPub.rtps.setName("PublisherParticipant");

    Participant* PubParticipant = Domain::createParticipant(PparamPub);
    if (PubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Publisher Participant..." << std::endl;
        return nullptr;
    }
    Domain::registerType(PubParticipant, (TopicDataType*)&sampleType);

    //Publisher config
    PublisherAttributes Pparam;
    Pparam.topic.topicDataType = sampleType.getName();
    Pparam.topic.topicName = "samplePubSubTopic";

    Pparam.topic.topicKind = WITH_KEY;
    Pparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Pparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    std::cout << "Creating Publisher..." << std::endl;
    Publisher* myPub = Domain::createPublisher(PubParticipant, Pparam, &listener);
    if (myPub == nullptr)
    {
        std::cout << "Something went wrong while creating the Publisher..." << std::endl;
    }
    return myPub;
}

Subscriber* initSubscriber(
        samplePubSubType& sampleType,
        SubListener* listener)
{
    ParticipantAttributes PparamSub;
    PparamSub.rtps.setName("SubscriberParticipant");

    Participant* SubParticipant = Domain::createParticipant(PparamSub);
    if (SubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Subscriber Participant..." << std::endl;
        return nullptr;
    }
    Domain::registerType(SubParticipant, (TopicDataType*)&sampleType);

    //Keep All Sub
    SubscriberAttributes Rparam;
    Rparam.topic.topicDataType = sampleType.getName();
    Rparam.topic.topicName = "samplePubSubTopic";
    Rparam.topic.topicKind = WITH_KEY;
    Rparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

    std::cout << "Creating Subscriber..." << std::endl;
    Subscriber* mySub = Domain::createSubscriber(SubParticipant, Rparam, listener);
    if (mySub == nullptr)
    {
        std::cout << "Something went wrong while creating the Subscriber..." << std::endl;
    }
    return mySub;
}

void keys()
{
    samplePubSubType sampleType;
    sample my_sample;
    SampleInfo_t sample_info;
    PubListener pubListener;
    InstanceHandle_t key_instances[max_instances];

    Publisher* myPub = initPublisher(sampleType, pubListener);
    Subscriber* mySub = initSubscriber(sampleType, nullptr);

    // wait for the connection
    std::unique_lock<std::mutex> lock(pubListener.mutex_);
    pubListener.cv_.wait(lock, [&pubListener]()
            {
                return pubListener.n_matched > 0;
            });

    // Registering 'max_instances' instances.
    for (uint8_t i = 0; i < max_instances; i++)
    {
        my_sample.key_value(i + 1);
        key_instances[i] = myPub->register_instance(&my_sample);
    }

    //Send 10 samples
    std::cout << "Publishing " << max_instances << " keys, 10 samples per key..." << std::endl;
    for (uint8_t i = 0; i < max_instances; i++)
    {
        for (uint8_t j = 0; j < 10; j++)
        {
            my_sample.index(j + 1);
            my_sample.key_value(i + 1);
            myPub->write(&my_sample);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    std::cout << "Publishing 10 more samples on a key 3..." << std::endl;
    for (uint8_t j = 0; j < 10; j++)
    {
        my_sample.index(j + 11);
        my_sample.key_value(3);
        myPub->write(&my_sample);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    //Read the contents of both histories:
    std::vector< std::pair<int, int> > sampleList;
    std::cout << "The Subscriber holds: " << std::endl;
    while (mySub->readNextData(&my_sample, &sample_info))
    {
        sampleList.push_back(std::pair<int, int>(my_sample.index(), my_sample.key_value()));
    }

    for (uint8_t key = 1; key <= max_instances; key++)
    {
        std::cout << "  On key " << std::to_string(key) << ": ";
        for (std::pair<int, int> n : sampleList)
        {
            if (n.second == key)
            {
                std::cout << std::to_string(n.first) << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Disposing all keys ..." << std::endl;
    for (uint8_t i = 0; i < max_instances; i++)
    {
        my_sample.key_value(i + 1);
        myPub->dispose(&my_sample, key_instances[i]);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    while (mySub->readNextData(&my_sample, &sample_info))
    {
        if (ChangeKind_t::NOT_ALIVE_DISPOSED == sample_info.sampleKind)
        {
            for (uint8_t i = 0; i < max_instances; i++)
            {
                if (key_instances[i] == sample_info.iHandle)
                {
                    std::cout << "  Key " << std::to_string(i + 1) << " was disposed" << std::endl;
                    break;
                }
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    Domain::stopAll();
}

void publisherKeys()
{
    samplePubSubType sampleType;
    sample my_sample;
    SampleInfo_t sample_info;
    PubListener pubListener;

    Publisher* myPub = initPublisher(sampleType, pubListener);

    // wait for the connection
    std::unique_lock<std::mutex> lock(pubListener.mutex_);
    pubListener.cv_.wait(lock, [&pubListener]()
            {
                return pubListener.n_matched > 0;
            });

    // Registering 'max_instances' instances.
    for (uint8_t i = 0; i < max_instances; i++)
    {
        my_sample.key_value(i + 1);
        myPub->register_instance(&my_sample);
    }

    //Send 10 samples
    std::cout << "Publishing " << max_instances << " keys, 10 samples per key..." << std::endl;
    for (uint8_t i = 0; i < max_instances; i++)
    {
        for (uint8_t j = 0; j < 10; j++)
        {
            my_sample.index(j + 1);
            my_sample.key_value(i + 1);
            myPub->write(&my_sample);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    std::cout << "Publishing 10 more samples on a key 3..." << std::endl;
    for (uint8_t j = 0; j < 10; j++)
    {
        my_sample.index(j + 11);
        my_sample.key_value(3);
        myPub->write(&my_sample);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    Domain::stopAll();
}

void subscriberKeys()
{
    samplePubSubType sampleType;
    sample my_sample;
    SampleInfo_t sample_info;
    SubListener subListener;

    initSubscriber(sampleType, &subListener);

    std::unique_lock<std::mutex> lock(subListener.mutex_);
    subListener.cv_.wait(lock, [&subListener]()
            {
                return subListener.n_matched > 0;
            });

    std::cin.ignore();

    Domain::stopAll();
}
