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

#include "BlackboxTests.hpp"
#include "../api/dds-pim/PubSubReader.hpp"
#include "../api/dds-pim/PubSubWriter.hpp"
#include "../api/dds-pim/PubSubWriterReader.hpp"
#include "../types/FixedSized.h"
#include "../types/FixedSizedPubSubTypes.h"
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

/**
 * @test This test checks the ignore local endpoints feature on the RTPS layer when writer and
 *       reader are under the same participant. Corresponds with tests:
 *          * PART-IGNORE-LOCAL-TEST:01
 *          * PART-IGNORE-LOCAL-TEST:02
 *          * PART-IGNORE-LOCAL-TEST:03
 */
TEST(DDSBasic, participant_ignore_local_endpoints)
{
    class CustomLogConsumer : public LogConsumer
    {
    public:

        void Consume(
                const Log::Entry&) override
        {
            logs_consumed_++;
            cv_.notify_all();
        }

        size_t wait_for_entries(
                uint32_t amount,
                const std::chrono::duration<double>& max_wait)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait_for(lock, max_wait, [this, amount]() -> bool
                    {
                        return logs_consumed_ > 0 && logs_consumed_.load() == amount;
                    });
            return logs_consumed_.load();
        }

    protected:

        std::atomic<size_t> logs_consumed_{0u};
        std::mutex mtx_;
        std::condition_variable cv_;
    };

    struct Config
    {
        std::string test_id;
        std::string property_value;
        uint8_t log_errors;
        uint8_t expected_matched_endpoints;
        uint8_t sent_samples;
        uint8_t expected_received_samples;
    };

    std::vector<Config> tests_configs = {
        {"PART-IGNORE-LOCAL-TEST:01", "true", 0, 0, 5, 0},
        {"PART-IGNORE-LOCAL-TEST:02", "false", 0, 1, 5, 5},
        {"PART-IGNORE-LOCAL-TEST:03", "asdfg", 1, 1, 5, 5}
    };

    for (Config test_config : tests_configs)
    {
        std::cout << std::endl;
        std::cout << "---------------------------------------" << std::endl;
        std::cout << "Running test: " << test_config.test_id << std::endl;
        std::cout << "---------------------------------------" << std::endl;

        /* Set up */
        Log::Reset();
        Log::SetVerbosity(Log::Error);
        CustomLogConsumer* log_consumer = new CustomLogConsumer();
        std::unique_ptr<CustomLogConsumer> log_consumer_unique_ptr(log_consumer);
        Log::RegisterConsumer(std::move(log_consumer_unique_ptr));

        // Create the DomainParticipant with the appropriate value for the property
        PubSubWriterReader<HelloWorldPubSubType> writer_reader(TEST_TOPIC_NAME);
        eprosima::fastrtps::rtps::PropertyPolicy property_policy;
        property_policy.properties().emplace_back("fastdds.ignore_local_endpoints", test_config.property_value);
        writer_reader.property_policy(property_policy);

        /* Procedure */
        // Create the DataWriter & DataReader
        writer_reader.init();
        EXPECT_TRUE(writer_reader.isInitialized());

        // Wait for discovery
        writer_reader.wait_discovery(test_config.expected_matched_endpoints, std::chrono::seconds(1));
        EXPECT_EQ(writer_reader.get_publication_matched(), test_config.expected_matched_endpoints);
        EXPECT_EQ(writer_reader.get_subscription_matched(), test_config.expected_matched_endpoints);

        // Send samples
        auto samples = default_helloworld_data_generator(test_config.sent_samples);
        writer_reader.startReception(samples);
        writer_reader.send(samples);
        EXPECT_TRUE(samples.empty());

        // Wait for reception
        EXPECT_EQ(writer_reader.block_for_all(std::chrono::seconds(1)), test_config.expected_received_samples);

        // Wait for log entries
        EXPECT_EQ(log_consumer->wait_for_entries(test_config.log_errors, std::chrono::seconds(
                    1)), test_config.log_errors);

        /* Tear-down */
        Log::Reset();
    }
}

/**
 * @test This test checks the ignore local endpoints feature on the RTPS layer when writer and
 *       reader are under the different participant. Corresponds with test:
 *          * PART-IGNORE-LOCAL-TEST:07
 */
TEST(DDSBasic, participant_ignore_local_endpoints_two_participants)
{
    std::cout << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "Running test: PART-IGNORE-LOCAL-TEST:07" << std::endl;
    std::cout << "---------------------------------------" << std::endl;

    /* Set up */

    // Create the DomainParticipants with the appropriate value for the property
    eprosima::fastrtps::rtps::PropertyPolicy property_policy;
    property_policy.properties().emplace_back("fastdds.ignore_local_endpoints", "true");
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    writer.property_policy(property_policy);
    reader.property_policy(property_policy);

    /* Procedure */
    // Create the DataWriter & DataReader
    writer.init();
    EXPECT_TRUE(writer.isInitialized());
    reader.init();
    EXPECT_TRUE(reader.isInitialized());

    // Wait for discovery
    writer.wait_discovery(std::chrono::seconds(1));
    reader.wait_discovery(std::chrono::seconds(1));
    EXPECT_EQ(writer.get_matched(), 1u);
    EXPECT_EQ(reader.get_matched(), 1u);

    // Send samples
    auto samples = default_helloworld_data_generator(5);
    reader.startReception(samples);
    writer.send(samples);
    EXPECT_TRUE(samples.empty());

    // Wait for reception
    EXPECT_EQ(reader.block_for_all(std::chrono::seconds(1)), 5);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
