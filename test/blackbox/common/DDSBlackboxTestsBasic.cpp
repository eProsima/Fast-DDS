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
#include <fastdds/dds/log/Log.hpp>
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

#include <rtps/transport/test_UDPv4Transport.h>

#include "BlackboxTests.hpp"
#include "mock/BlackboxMockConsumer.h"
#include "../types/HelloWorldPubSubTypes.h"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

/**
 * This is a regression test for redmine issue #21060.
 *
 * It checks that when intraprocess delivery is set to full, there are no warnings in the desctructor of WriterProxy
 * when deleting a participant.
 */
TEST(DDSBasic, WarningOnDelete)
{
    using namespace eprosima::fastrtps;
    namespace dds = eprosima::fastdds::dds;
    auto factory = dds::DomainParticipantFactory::get_instance();

    // Set intraprocess delivery to full
    LibrarySettingsAttributes library_settings;
    library_settings = xmlparser::XMLProfileManager::library_settings();
    auto old_library_settings = library_settings;
    library_settings.intraprocess_delivery = INTRAPROCESS_FULL;
    xmlparser::XMLProfileManager::library_settings(library_settings);

    // Create participants
    auto participant_1 = factory->create_participant(0, dds::PARTICIPANT_QOS_DEFAULT);
    auto participant_2 = factory->create_participant(0, dds::PARTICIPANT_QOS_DEFAULT);

    /* Set up log */
    BlackboxMockConsumer* helper_consumer = new BlackboxMockConsumer();
    Log::ClearConsumers();  // Remove default consumers
    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(helper_consumer)); // Registering a consumer transfer ownership
    // Filter specific message
    dds::Log::SetErrorStringFilter(std::regex(".*~WriterProxy.*"));
    dds::Log::SetVerbosity(dds::Log::Warning);

    factory->delete_participant(participant_1);
    factory->delete_participant(participant_2);

    dds::Log::Flush();
    EXPECT_EQ(helper_consumer->ConsumedEntries().size(), 0u);
    helper_consumer->clear_entries();

    // Restore library settings
    xmlparser::XMLProfileManager::library_settings(old_library_settings);
}

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

/**
 * @test Set the maximum number of bytes allowed for a datagram generated by a DomainParticipant.
 */
TEST(DDSBasic, max_output_message_size_participant)
{
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    reader.init();
    EXPECT_TRUE(reader.isInitialized());

    auto testTransport =  std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    const uint32_t segment_size = 1470;
    std::string segment_size_str = std::to_string(segment_size);
    testTransport->messages_filter_ = [segment_size](eprosima::fastrtps::rtps::CDRMessage_t& datagram)
            {
                EXPECT_LE(datagram.length, segment_size);
                // Never drop samples
                return false;
            };

    // Create the DomainParticipants with the appropriate value for the property
    eprosima::fastrtps::rtps::PropertyPolicy property_policy;
    property_policy.properties().emplace_back("fastdds.max_message_size", segment_size_str);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);
    writer.property_policy(property_policy).disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport).init();
    EXPECT_TRUE(writer.isInitialized());

    // Wait for discovery
    writer.wait_discovery(std::chrono::seconds(2));
    reader.wait_discovery(std::chrono::seconds(2));
    EXPECT_EQ(writer.get_matched(), 1u);
    EXPECT_EQ(reader.get_matched(), 1u);

    // Send samples
    auto samples = default_data16kb_data_generator(1);
    reader.startReception(samples);
    writer.send(samples);
    EXPECT_TRUE(samples.empty());

    // Wait for reception
    reader.block_for_all(std::chrono::seconds(1));
    EXPECT_EQ(reader.getReceivedCount(), 1u);
}

/**
 * @test Set the maximum number of bytes allowed for a datagram generated by a DataWriter.
 */
TEST(DDSBasic, max_output_message_size_writer)
{
    const uint32_t segment_size = 1470;
    std::string segment_size_str = std::to_string(segment_size);

    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport->messages_filter_ = [segment_size](eprosima::fastrtps::rtps::CDRMessage_t& datagram)
            {
                EXPECT_LE(datagram.length, segment_size);
                // Never drop samples
                return false;
            };

    // Create the DataWriter with the appropriate value for the property
    eprosima::fastrtps::rtps::PropertyPolicy property_policy;
    property_policy.properties().emplace_back("fastdds.max_message_size", segment_size_str);
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);
    writer.entity_property_policy(property_policy).disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport).init();
    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    reader.init();
    EXPECT_TRUE(reader.isInitialized());

    // Wait for discovery
    writer.wait_discovery(std::chrono::seconds(2));
    reader.wait_discovery(std::chrono::seconds(2));

    EXPECT_EQ(writer.get_matched(), 1u);
    EXPECT_EQ(reader.get_matched(), 1u);

    // Send samples
    auto samples = default_data16kb_data_generator(1);
    reader.startReception(samples);
    writer.send(samples);
    EXPECT_TRUE(samples.empty());

    // Wait for reception
    reader.block_for_all(std::chrono::seconds(1));
    EXPECT_EQ(reader.getReceivedCount(), 1u);

}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
