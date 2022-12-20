// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <condition_variable>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/types/TypesBase.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "BlackboxTests.hpp"
#include "../types/HelloWorldPubSubTypes.h"

namespace eprosima {
namespace fastdds {
namespace dds {

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

/**
 * This test checks whether it is safe to delete not enabled DDS entities *
 */
TEST(DDSBasic, DeleteDisabledEntities)
{
    // Set DomainParticipantFactory to create disabled entities
    DomainParticipantFactoryQos factory_qos;
    factory_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);
    factory->set_qos(factory_qos);
    DomainParticipantFactoryQos factory_qos_check;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, factory->get_qos(factory_qos_check));
    ASSERT_EQ(false, factory_qos_check.entity_factory().autoenable_created_entities);

    // Create a disabled DomainParticipant, setting it to in turn create disable entities
    DomainParticipantQos participant_qos;
    participant_qos.entity_factory().autoenable_created_entities = false;
    DomainParticipant* participant = factory->create_participant((uint32_t)GET_PID() % 230, participant_qos);
    ASSERT_NE(nullptr, participant);
    DomainParticipantQos participant_qos_check;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->get_qos(participant_qos_check));
    ASSERT_EQ(false, participant_qos_check.entity_factory().autoenable_created_entities);

    // Create a disabled Publisher, setting it to in turn create disable entities
    PublisherQos publisher_qos;
    publisher_qos.entity_factory().autoenable_created_entities = false;
    Publisher* publisher = participant->create_publisher(publisher_qos);
    ASSERT_NE(nullptr, publisher);
    PublisherQos publisher_qos_check;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, publisher->get_qos(publisher_qos_check));
    ASSERT_EQ(false, publisher_qos_check.entity_factory().autoenable_created_entities);

    // Create a disabled Subscriber, setting it to in turn create disable entities
    SubscriberQos subscriber_qos;
    subscriber_qos.entity_factory().autoenable_created_entities = false;
    Subscriber* subscriber = participant->create_subscriber(subscriber_qos);
    ASSERT_NE(nullptr, subscriber);
    SubscriberQos subscriber_qos_check;
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, subscriber->get_qos(subscriber_qos_check));
    ASSERT_EQ(false, subscriber_qos_check.entity_factory().autoenable_created_entities);

    // Register type
    TypeSupport type_support;
    type_support.reset(new HelloWorldPubSubType());
    type_support.register_type(participant);
    ASSERT_NE(nullptr, type_support);

    // Create Topic
    Topic* topic = participant->create_topic("HelloWorld", type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic);

    // Create a disabled DataWriter
    DataWriter* datawriter = publisher->create_datawriter(topic, DATAWRITER_QOS_DEFAULT);
    ASSERT_NE(nullptr, datawriter);

    // Create a disabled DataReader
    DataReader* datareader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(nullptr, datareader);

    // Delete all entities
    publisher->delete_datawriter(datawriter);
    subscriber->delete_datareader(datareader);
    participant->delete_publisher(publisher);
    participant->delete_subscriber(subscriber);
    participant->delete_topic(topic);
    factory->delete_participant(participant);
}

/**
 * This test checks a race condition when calling DomainParticipantImpl::create_instance_handle()
 * from different threads simultaneously. This was resulting in a `free(): invalid pointer` crash
 * when deleting publishers created this way, as there was a clash in their respective instance
 * handles. Not only did the crash occur, but it was also reported by TSan.
 *
 * The test spawns 200 threads, each creating a publisher and then waiting on a command from the
 * main thread to delete them (so all of them at deleted at the same time).
 */
TEST(DDSBasic, MultithreadedPublisherCreation)
{
    /* Get factory */
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);

    /* Create participant */
    DomainParticipant* participant = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);

    /* Test synchronization variables */
    std::mutex finish_mtx;
    std::condition_variable finish_cv;
    bool should_finish = false;

    /* Function to create publishers, deleting them on command */
    auto thread_run =
            [participant, &finish_mtx, &finish_cv, &should_finish]()
            {
                /* Create publisher */
                Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
                ASSERT_NE(nullptr, publisher);

                {
                    /* Wait for test completion request */
                    std::unique_lock<std::mutex> lock(finish_mtx);
                    finish_cv.wait(lock, [&should_finish]()
                            {
                                return should_finish;
                            });
                }

                /* Delete publisher */
                ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_publisher(publisher));
            };

    {
        /* Create threads */
        std::vector<std::thread> threads;
        for (size_t i = 0; i < 200; i++)
        {
            threads.push_back(std::thread(thread_run));
        }

        /* Command threads to delete their publishers */
        {
            std::lock_guard<std::mutex> guard(finish_mtx);
            should_finish = true;
            finish_cv.notify_all();
        }

        /* Wait for all threads to join */
        for (std::thread& thr : threads)
        {
            thr.join();
        }
    }

    /* Clean up */
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, factory->delete_participant(participant));
}

TEST(DDSBasic, MultithreadedReaderCreationDoesNotDeadlock)
{
    // Get factory
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);

    // Create participant
    DomainParticipant* participant = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant);

    // Register type
    TypeSupport type_support;
    type_support.reset(new FixedSizedPubSubType());
    type_support.register_type(participant);
    ASSERT_NE(nullptr, type_support);

    // Create subscriber
    Subscriber* subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);

    // Create publisher
    Publisher* publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher);

    // Create Topic
    Topic* topic = participant->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic);

    // Set QoS
    DataSharingQosPolicy dsp;
    dsp.off();

    DataWriterQos dw_qos;
    DataReaderQos dr_qos;
    dw_qos.data_sharing(dsp);
    dr_qos.data_sharing(dsp);

    // Create DataWriter
    DataWriter* writer = publisher->create_datawriter(topic, dw_qos);
    ASSERT_NE(nullptr, writer);

    std::mutex mtx;
    std::condition_variable cv;
    bool should_finish = false;

    auto thread_run = [subscriber, topic, &mtx, &cv, &should_finish, &dr_qos]()
            {
                // Create reader
                DataReader* reader = subscriber->create_datareader(topic, dr_qos);
                ASSERT_NE(nullptr, reader);

                // Wait for test completion request
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&should_finish]()
                        {
                            return should_finish;
                        });

                ASSERT_EQ(ReturnCode_t::RETCODE_OK, subscriber->delete_datareader(reader));
            };

    {
        std::vector<std::thread> threads;
        for (size_t i = 0; i < 10; ++i)
        {
            threads.push_back(std::thread(thread_run));
        }

        {
            std::lock_guard<std::mutex> guard(mtx);
            should_finish = true;
            cv.notify_all();
        }

        for (std::thread& thr : threads)
        {
            thr.join();
        }
    }

    ASSERT_EQ(ReturnCode_t::RETCODE_OK, publisher->delete_datawriter(writer));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_publisher(publisher));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_subscriber(subscriber));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, participant->delete_topic(topic));
    ASSERT_EQ(ReturnCode_t::RETCODE_OK, factory->delete_participant(participant));
}

TEST(DDSBasic, DataSharingDataPersistenceToWriter)
{
    // Wait for reader to have notified the data
    std::condition_variable cv;
    std::mutex cv_mutex;
    std::atomic<int> received(0);
    int messages_to_send = 1;

    // Get factory
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);

    // Create participant
    DomainParticipant* participant_pub = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_pub);
    DomainParticipant* participant_sub = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_sub);

    // Register type
    TypeSupport type_support;
    type_support.reset(new FixedSizedPubSubType());
    type_support.register_type(participant_pub);
    type_support.register_type(participant_sub);
    ASSERT_NE(nullptr, type_support);

    // Create subscriber
    Subscriber* subscriber = participant_sub->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);

    // Create publisher
    Publisher* publisher = participant_pub->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher);

    // Create Topic
    Topic* topic_pub = participant_pub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    Topic* topic_sub = participant_sub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic_pub);
    ASSERT_NE(nullptr, topic_sub);

    // Set QoS
    DataWriterQos dw_qos;
    DataReaderQos dr_qos;

    dw_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    dr_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;

    dw_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    dr_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

    struct CustomReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        CustomReaderListener(std::condition_variable& cv, std::mutex& cv_mutex, std::atomic<int>& received)
            : cv_(cv)
            , cv_mutex_(cv_mutex)
            , received_(received)
        {}

        void on_data_available(
            DataReader*) override
        {
            std::lock_guard<std::mutex> guard(cv_mutex_);
            received_++;
            cv_.notify_all();
        }

        std::condition_variable& cv_;
        std::mutex& cv_mutex_;
        std::atomic<int>& received_;
    };
    CustomReaderListener listener(cv, cv_mutex, received);

    // Create DataWriter
    DataWriter* writer = publisher->create_datawriter(topic_pub, dw_qos);
    ASSERT_NE(nullptr, writer);

    // Create reader
    DataReader* reader = subscriber->create_datareader(topic_sub, dr_qos, &listener);
    ASSERT_NE(nullptr, reader);

    // Give time to get matched
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Create data and send
    for (int i=0; i<messages_to_send; i++)
    {
        FixedSized data;
        writer->write(&data);
    }

    // Wait to reader notifying the data
    std::unique_lock<std::mutex> lock(cv_mutex);
    cv.wait(lock, [&received, messages_to_send](){ return received.load() >= messages_to_send; });
    ASSERT_EQ(reader->get_unread_count(), messages_to_send);

    // Remove Data Writer
    publisher->delete_datawriter(writer);

    // Give time to be removed correctly
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Read from Reader
    FixedSized data_received;
    eprosima::fastdds::dds::SampleInfo info;
    auto result = reader->take_next_sample(&data_received, &info);
    ASSERT_EQ(result, ReturnCode_t::RETCODE_OK);

    // TODO close everything correctly
}

TEST(DDSBasic, DataSharingFirstMessage)
{
    // Wait for reader to have notified the data
    std::condition_variable cv;
    std::mutex cv_mutex;
    std::atomic<int> received(0);
    int messages_to_send = 1;

    // Get factory
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);

    // Create participant
    DomainParticipant* participant_pub = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_pub);
    DomainParticipant* participant_sub = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_sub);

    // Register type
    TypeSupport type_support;
    type_support.reset(new FixedSizedPubSubType());
    type_support.register_type(participant_pub);
    type_support.register_type(participant_sub);
    ASSERT_NE(nullptr, type_support);

    // Create subscriber
    Subscriber* subscriber = participant_sub->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);

    // Create publisher
    Publisher* publisher = participant_pub->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher);

    // Create Topic
    Topic* topic_pub = participant_pub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    Topic* topic_sub = participant_sub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic_pub);
    ASSERT_NE(nullptr, topic_sub);

    // Set QoS
    DataWriterQos dw_qos;
    DataReaderQos dr_qos;

    dw_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    dr_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;

    dw_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    dr_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

    struct CustomReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        CustomReaderListener(std::condition_variable& cv, std::mutex& cv_mutex, std::atomic<int>& received)
            : cv_(cv)
            , cv_mutex_(cv_mutex)
            , received_(received)
        {}

        void on_data_available(
            DataReader*) override
        {
            std::lock_guard<std::mutex> guard(cv_mutex_);
            received_++;
            cv_.notify_all();
        }

        std::condition_variable& cv_;
        std::mutex& cv_mutex_;
        std::atomic<int>& received_;
    };
    CustomReaderListener listener(cv, cv_mutex, received);

    // Create DataWriter
    DataWriter* writer = publisher->create_datawriter(topic_pub, dw_qos);
    ASSERT_NE(nullptr, writer);

    // Create reader
    DataReader* reader = subscriber->create_datareader(topic_sub, dr_qos, &listener);
    ASSERT_NE(nullptr, reader);

    // Create data and send
    for (int i=0; i<messages_to_send; i++)
    {
        FixedSized data;
        writer->write(&data);
    }

    // Wait to reader notifying the data
    std::unique_lock<std::mutex> lock(cv_mutex);
    cv.wait(lock, [&received, messages_to_send](){ return received.load() >= messages_to_send; });
    ASSERT_EQ(reader->get_unread_count(), messages_to_send);

    // Remove Data Writer
    publisher->delete_datawriter(writer);

    // Give time to be removed correctly
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Read from Reader
    FixedSized data_received;
    eprosima::fastdds::dds::SampleInfo info;
    auto result = reader->take_next_sample(&data_received, &info);
    ASSERT_EQ(result, ReturnCode_t::RETCODE_OK);

    // TODO close everything correctly
}

TEST(DDSBasic, IntraprocessGetUnreadCountUpdate)
{
    // Wait for reader to have notified the data
    std::condition_variable cv;
    std::mutex cv_mutex;
    std::atomic<int> received(0);
    int messages_to_send = 2;

    // Get factory
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);

    // Create participant
    DomainParticipant* participant_pub = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_pub);
    DomainParticipant* participant_sub = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_sub);

    // Register type
    TypeSupport type_support;
    type_support.reset(new FixedSizedPubSubType());
    type_support.register_type(participant_pub);
    type_support.register_type(participant_sub);
    ASSERT_NE(nullptr, type_support);

    // Create subscriber
    Subscriber* subscriber = participant_sub->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);

    // Create publisher
    Publisher* publisher = participant_pub->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher);

    // Create Topic
    Topic* topic_pub = participant_pub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    Topic* topic_sub = participant_sub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic_pub);
    ASSERT_NE(nullptr, topic_sub);

    // Set QoS
    DataWriterQos dw_qos;
    DataReaderQos dr_qos;

    dw_qos.data_sharing().off();
    dr_qos.data_sharing().off();

    dw_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    dr_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;

    dw_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    dr_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

    dw_qos.history().length = messages_to_send * 2;
    dr_qos.history().length = messages_to_send * 2;

    struct CustomReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        CustomReaderListener(std::condition_variable& cv, std::mutex& cv_mutex, std::atomic<int>& received)
            : cv_(cv)
            , cv_mutex_(cv_mutex)
            , received_(received)
        {}

        void on_data_available(
            DataReader* reader) override
        {
            {
                std::lock_guard<std::mutex> guard(cv_mutex_);
                received_++;
            }
            cv_.notify_all();
            ASSERT_EQ(reader->get_unread_count(), received_);
        }

        std::condition_variable& cv_;
        std::mutex& cv_mutex_;
        std::atomic<int>& received_;
    };
    CustomReaderListener listener(cv, cv_mutex, received);

    // Create DataWriter
    DataWriter* writer = publisher->create_datawriter(topic_pub, dw_qos);
    ASSERT_NE(nullptr, writer);

    // Create reader
    DataReader* reader = subscriber->create_datareader(topic_sub, dr_qos, &listener);
    ASSERT_NE(nullptr, reader);

    // Give time to match correctly
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Create data and send
    for (int i=0; i<messages_to_send; i++)
    {
        FixedSized data;
        writer->write(&data);
    }

    // Wait to reader notifying the data
    std::unique_lock<std::mutex> lock(cv_mutex);
    cv.wait(lock, [&received, messages_to_send](){ return received.load() >= messages_to_send; });
    ASSERT_EQ(reader->get_unread_count(), messages_to_send);

    // Remove Data Writer
    publisher->delete_datawriter(writer);

    // Give time to be removed correctly
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Read from Reader
    for (int i=0; i<messages_to_send; i++)
    {
        FixedSized data_received;
        eprosima::fastdds::dds::SampleInfo info;
        auto result = reader->take_next_sample(&data_received, &info);
        ASSERT_EQ(result, ReturnCode_t::RETCODE_OK);
    }

    // TODO close everything correctly
}

TEST(DDSBasic, IntraprocessReadAfterNotify)
{
    // Wait for reader to have notified the data
    std::condition_variable cv;
    std::mutex cv_mutex;
    std::atomic<int> received(0);
    int messages_to_send = 2;

    // Get factory
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);

    // Create participant
    DomainParticipant* participant_pub = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_pub);
    DomainParticipant* participant_sub = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_sub);

    // Register type
    TypeSupport type_support;
    type_support.reset(new FixedSizedPubSubType());
    type_support.register_type(participant_pub);
    type_support.register_type(participant_sub);
    ASSERT_NE(nullptr, type_support);

    // Create subscriber
    Subscriber* subscriber = participant_sub->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);

    // Create publisher
    Publisher* publisher = participant_pub->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher);

    // Create Topic
    Topic* topic_pub = participant_pub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    Topic* topic_sub = participant_sub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic_pub);
    ASSERT_NE(nullptr, topic_sub);

    // Set QoS
    DataWriterQos dw_qos;
    DataReaderQos dr_qos;

    dw_qos.data_sharing().off();
    dr_qos.data_sharing().off();

    dw_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    dr_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;

    dw_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    dr_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

    dw_qos.history().length = messages_to_send * 2;
    dr_qos.history().length = messages_to_send * 2;

    struct CustomReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        CustomReaderListener(std::condition_variable& cv, std::mutex& cv_mutex, std::atomic<int>& received)
            : cv_(cv)
            , cv_mutex_(cv_mutex)
            , received_(received)
        {}

        void on_data_available(
            DataReader*) override
        {
            {
                std::unique_lock<std::mutex> lock(cv_mutex_);
                received_++;
            }
            cv_.notify_all();
        }

        std::condition_variable& cv_;
        std::mutex& cv_mutex_;
        std::atomic<int>& received_;
    };
    CustomReaderListener listener(cv, cv_mutex, received);

    // Create DataWriter
    DataWriter* writer = publisher->create_datawriter(topic_pub, dw_qos);
    ASSERT_NE(nullptr, writer);

    // Create reader
    DataReader* reader = subscriber->create_datareader(topic_sub, dr_qos, &listener);
    ASSERT_NE(nullptr, reader);

    // Give time to match correctly
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Create data
    for (int i=0; i<messages_to_send; i++)
    {
        FixedSized data;
        writer->write(&data);
    }

    // Wait to reader notifying the data
    std::unique_lock<std::mutex> lock(cv_mutex);
    cv.wait(lock, [&received, messages_to_send, reader](){
        return received.load() >= messages_to_send;
    });

    for (int i=0; i<messages_to_send; i++)
    {
        FixedSized data_received;
        eprosima::fastdds::dds::SampleInfo info;
        auto result = reader->take_next_sample(&data_received, &info);
        ASSERT_EQ(result, ReturnCode_t::RETCODE_OK);
    }

    // TODO close everything correctly
}

TEST(DDSBasic, NonIntraprocessReadAfterNotify)
{
    // Wait for reader to have notified the data
    std::condition_variable cv;
    std::mutex cv_mutex;
    std::atomic<int> received(0);
    int messages_to_send = 2;

    // Disable intraprocess
    eprosima::fastrtps::LibrarySettingsAttributes att;
    att.intraprocess_delivery = eprosima::fastrtps::IntraprocessDeliveryType::INTRAPROCESS_OFF;
    eprosima::fastrtps::xmlparser::XMLProfileManager::library_settings(att);

    // Get factory
    DomainParticipantFactory* factory = DomainParticipantFactory::get_instance();
    ASSERT_NE(nullptr, factory);

    // Charge XML
    // auto xml_res = factory->load_XML_profiles_file("deactivate_intraprocess.xml");
    // ASSERT_EQ(xml_res, ReturnCode_t::RETCODE_OK);

    // Create participant
    DomainParticipant* participant_pub = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_pub);
    DomainParticipant* participant_sub = factory->create_participant((uint32_t)GET_PID() % 230, PARTICIPANT_QOS_DEFAULT);
    ASSERT_NE(nullptr, participant_sub);

    // Register type
    TypeSupport type_support;
    type_support.reset(new FixedSizedPubSubType());
    type_support.register_type(participant_pub);
    type_support.register_type(participant_sub);
    ASSERT_NE(nullptr, type_support);

    // Create subscriber
    Subscriber* subscriber = participant_sub->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    ASSERT_NE(nullptr, subscriber);

    // Create publisher
    Publisher* publisher = participant_pub->create_publisher(PUBLISHER_QOS_DEFAULT);
    ASSERT_NE(nullptr, publisher);

    // Create Topic
    Topic* topic_pub = participant_pub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    Topic* topic_sub = participant_sub->create_topic(TEST_TOPIC_NAME, type_support.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(nullptr, topic_pub);
    ASSERT_NE(nullptr, topic_sub);

    // Set QoS
    DataWriterQos dw_qos;
    DataReaderQos dr_qos;

    dw_qos.data_sharing().off();
    dr_qos.data_sharing().off();

    dw_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
    dr_qos.reliability().kind = eprosima::fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;

    dw_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    dr_qos.durability().kind = eprosima::fastdds::dds::DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;

    dw_qos.history().length = messages_to_send * 2;
    dr_qos.history().length = messages_to_send * 2;

    struct CustomReaderListener : public eprosima::fastdds::dds::DataReaderListener
    {
        CustomReaderListener(std::condition_variable& cv, std::mutex& cv_mutex, std::atomic<int>& received)
            : cv_(cv)
            , cv_mutex_(cv_mutex)
            , received_(received)
        {}

        void on_data_available(
            DataReader*) override
        {
            {
                std::unique_lock<std::mutex> lock(cv_mutex_);
                received_++;
            }
            cv_.notify_all();
        }

        std::condition_variable& cv_;
        std::mutex& cv_mutex_;
        std::atomic<int>& received_;
    };
    CustomReaderListener listener(cv, cv_mutex, received);

    // Create DataWriter
    DataWriter* writer = publisher->create_datawriter(topic_pub, dw_qos);
    ASSERT_NE(nullptr, writer);

    // Create reader
    DataReader* reader = subscriber->create_datareader(topic_sub, dr_qos, &listener);
    ASSERT_NE(nullptr, reader);

    // Give time to match correctly
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Create data
    for (int i=0; i<messages_to_send; i++)
    {
        FixedSized data;
        writer->write(&data);
    }

    // Wait to reader notifying the data
    std::unique_lock<std::mutex> lock(cv_mutex);
    cv.wait(lock, [&received, messages_to_send, reader](){
        return received.load() >= messages_to_send;
    });

    for (int i=0; i<messages_to_send; i++)
    {
        FixedSized data_received;
        eprosima::fastdds::dds::SampleInfo info;
        auto result = reader->take_next_sample(&data_received, &info);
        ASSERT_EQ(result, ReturnCode_t::RETCODE_OK);
    }

    // TODO close everything correctly
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
