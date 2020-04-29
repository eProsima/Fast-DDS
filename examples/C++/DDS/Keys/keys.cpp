#include <iostream>
#include <string>
#include <condition_variable>

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

DomainParticipant* PubParticipant;

DomainParticipant* SubParticipant;

Publisher* myPub;

Subscriber* mySub;

Topic* PubTopic;

Topic* SubTopic;

class PubListener : public eprosima::fastdds::dds::DataWriterListener
{
public:

    PubListener()
        : n_matched(0)
    {
    }

    ~PubListener() override
    {
    }

    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter*,
            const eprosima::fastdds::dds::PublicationMatchedStatus& info)
    {
        if (info.current_count_change == 1)
        {
            n_matched = info.total_count;
        }
        else if (info.current_count_change == -1)
        {
            n_matched = info.total_count;
        }
        else
        {
            std::cout << info.current_count_change
                      << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
        }
        cv_.notify_all();
    }

    int n_matched;

    std::condition_variable cv_;

    std::mutex mutex_;
};

class SubListener : public eprosima::fastdds::dds::DataReaderListener
{
public:

    SubListener()
        : n_matched(0)
        , n_samples(0)
    {
    }

    ~SubListener() override
    {
    }

    void on_subscription_matched(
            DataReader*,
            const SubscriptionMatchedStatus& info)
    {
        if (info.current_count_change == 1)
        {
            n_matched = info.total_count;
        }
        else if (info.current_count_change == -1)
        {
            n_matched = info.total_count;
        }
        else
        {
            std::cout << info.current_count_change
                      << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
        }
        cv_.notify_all();
    }

    void on_data_available(
            DataReader* reader)
    {
        SampleInfo info;
        if (reader->take_next_sample(&m_sample, &info) == ReturnCode_t::RETCODE_OK)
        {
            if (info.instance_state == eprosima::fastdds::dds::ALIVE)
            {
                this->n_samples++;
                // Print your structure data here.
                std::cout << this->n_samples << ": Message " << (uint32_t)(m_sample.index()) << " RECEIVED" <<
                    std::endl;
            }
        }
    }

    sample m_sample;

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

DataWriter* initPublisher(
        TypeSupport& sampleType,
        PubListener& listener)
{
    //Create Publisher Participant
    DomainParticipantQos ppqos;
    ppqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    ppqos.name("PublisherParticipant");

    PubParticipant = DomainParticipantFactory::get_instance()->create_participant(0, ppqos);
    if (PubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Publisher Participant..." << std::endl;
        return nullptr;
    }

    //Register Type
    sampleType.register_type(PubParticipant);

    //Publisher config
    std::cout << "Creating Publisher..." << std::endl;
    myPub = PubParticipant->create_publisher(PUBLISHER_QOS_DEFAULT);

    if (myPub == nullptr)
    {
        std::cout << "Something went wrong while creating the Publisher..." << std::endl;
    }

    //Create Topic
    PubTopic = PubParticipant->create_topic("samplePubSubTopic", sampleType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (PubTopic == nullptr)
    {
        std::cout << " Something went wrong while creating the Publisher Topic..." << std::endl;
        return nullptr;
    }

    DataWriterQos wqos;
    wqos.endpoint().history_memory_policy = DYNAMIC_RESERVE_MEMORY_MODE;
    wqos.history().kind = KEEP_ALL_HISTORY_QOS;
    wqos.durability().kind = VOLATILE_DURABILITY_QOS;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    wqos.resource_limits().max_samples = 100;
    wqos.resource_limits().allocated_samples = 100;
    wqos.resource_limits().max_instances = 5;
    wqos.resource_limits().max_samples_per_instance = 20;

    DataWriter* myWriter = myPub->create_datawriter(PubTopic, wqos, &listener);

    if (myWriter == nullptr)
    {
        std::cout << " Something went wrong while creating the Publisher DataWriter..." << std::endl;
        return nullptr;
    }

    return myWriter;
}

DataReader* initSubscriber(
        TypeSupport& sampleType,
        SubListener* listener)
{
    //Create Subscriber Participant
    DomainParticipantQos psqos;
    psqos.wire_protocol().builtin.discovery_config.leaseDuration = eprosima::fastrtps::c_TimeInfinite;
    psqos.name("SubscriberParticipant");

    SubParticipant = DomainParticipantFactory::get_instance()->create_participant(0, psqos);
    if (SubParticipant == nullptr)
    {
        std::cout << " Something went wrong while creating the Subscriber Participant..." << std::endl;
        return nullptr;
    }

    //Register Type
    sampleType.register_type(SubParticipant);

    //Keep All Sub
    std::cout << "Creating Subscriber..." << std::endl;
    mySub = SubParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    if (mySub == nullptr)
    {
        std::cout << "Something went wrong while creating the Subscriber..." << std::endl;
        return nullptr;
    }

    //Create Topic
    SubTopic = SubParticipant->create_topic("samplePubSubTopic", sampleType.get_type_name(), TOPIC_QOS_DEFAULT);

    if (SubTopic == nullptr)
    {
        std::cout << "Something went wrong while creating the Subscriber Topic..." << std::endl;
        return nullptr;
    }

    DataReaderQos rqos;
    rqos.endpoint().history_memory_policy = DYNAMIC_RESERVE_MEMORY_MODE;
    rqos.history().kind = KEEP_ALL_HISTORY_QOS;
    rqos.durability().kind = VOLATILE_DURABILITY_QOS;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;

    rqos.resource_limits().max_samples = 100;
    rqos.resource_limits().allocated_samples = 100;
    rqos.resource_limits().max_instances = 5;
    rqos.resource_limits().max_samples_per_instance = 20;

    DataReader* myReader = mySub->create_datareader(SubTopic, rqos, listener);

    return myReader;
}

void keys()
{
    eprosima::fastdds::dds::TypeSupport sampleType(new samplePubSubType());
    sample my_sample;
    SampleInfo sample_info;
    PubListener pubListener;

    DataWriter* myWriter = initPublisher(sampleType, pubListener);
    DataReader* myReader = initSubscriber(sampleType, nullptr);

    // wait for the connection
    std::unique_lock<std::mutex> lock(pubListener.mutex_);
    pubListener.cv_.wait(lock, [&pubListener]()
    {
        return pubListener.n_matched > 0;
    });

    //Send 10 samples
    std::cout << "Publishing 5 keys, 10 samples per key..." << std::endl;
    for (uint8_t i = 0; i < 5; i++)
    {
        for (uint8_t j = 0; j < 10; j++)
        {
            my_sample.index(j + 1);
            my_sample.key_value(i + 1);
            myWriter->write(&my_sample);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    std::cout << "Publishing 10 more samples on a key 3..." << std::endl;
    for (uint8_t j = 0; j < 10; j++)
    {
        my_sample.index(j + 11);
        my_sample.key_value(3);
        myWriter->write(&my_sample);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    //Read the contents of both histories:
    std::vector< std::pair<int, int> > sampleList;
    std::cout << "The Subscriber holds: " << std::endl;
    while (myReader->read_next_sample(&my_sample, &sample_info) == ReturnCode_t::RETCODE_OK)
    {
        sampleList.push_back(std::pair<int, int>(my_sample.index(), my_sample.key_value()));
    }

    for (int key = 1; key <= 5; key++)
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
}

void publisherKeys()
{
    eprosima::fastdds::dds::TypeSupport sampleType(new samplePubSubType());
    sample my_sample;
    PubListener pubListener;

    DataWriter* myWriter = initPublisher(sampleType, pubListener);

    // wait for the connection
    std::unique_lock<std::mutex> lock(pubListener.mutex_);
    pubListener.cv_.wait(lock, [&pubListener]()
    {
        return pubListener.n_matched > 0;
    });

    //Send 10 samples
    std::cout << "Publishing 5 keys, 10 samples per key..." << std::endl;
    for (uint8_t i = 0; i < 5; i++)
    {
        for (uint8_t j = 0; j < 10; j++)
        {
            my_sample.index(j + 1);
            my_sample.key_value(i + 1);
            myWriter->write(&my_sample);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    std::cout << "Publishing 10 more samples on a key 3..." << std::endl;
    for (uint8_t j = 0; j < 10; j++)
    {
        my_sample.index(j + 11);
        my_sample.key_value(3);
        myWriter->write(&my_sample);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    myPub->delete_datawriter(myWriter);
    PubParticipant->delete_publisher(myPub);
    PubParticipant->delete_topic(PubTopic);
    DomainParticipantFactory::get_instance()->delete_participant(PubParticipant);

}

void subscriberKeys()
{
    eprosima::fastdds::dds::TypeSupport sampleType(new samplePubSubType());
    SubListener subListener;

    initSubscriber(sampleType, &subListener);

    std::unique_lock<std::mutex> lock(subListener.mutex_);
    subListener.cv_.wait(lock, [&subListener]()
    {
        return subListener.n_matched > 0;
    });

    std::cin.ignore();

    SubParticipant->delete_subscriber(mySub);
    SubParticipant->delete_topic(SubTopic);
    DomainParticipantFactory::get_instance()->delete_participant(SubParticipant);
}
