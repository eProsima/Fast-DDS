// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <atomic>
#include <condition_variable>
#include <gmock/gmock-matchers.h>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/core/policy/ParameterSerializer.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>

#include "BlackboxTests.hpp"
#include "../api/dds-pim/CustomPayloadPool.hpp"
#include "../api/dds-pim/PubSubReader.hpp"
#include "../api/dds-pim/PubSubWriter.hpp"
#include "../types/FixedSized.hpp"
#include "../types/FixedSizedPubSubTypes.hpp"
#include "../types/HelloWorldPubSubTypes.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

TEST(AcknackQos, DDSEnableUpdatabilityOfPositiveAcksPeriodDDSLayer)
{
    // This test checks the behaviour of disabling positive ACKs.
    // It also checks that only the positive ACKs
    // period is updatable at runtime through set_qos.

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    // Configure datapublisher_qos
    writer.keep_duration({1, 0});
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    writer.durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS);
    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Configure datasubscriber_qos
    reader.keep_duration({1, 0});
    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    reader.init();

    ASSERT_TRUE(reader.isInitialized());

    // Check correct initialitation
    eprosima::fastdds::dds::DataWriterQos get_att = writer.get_qos();
    EXPECT_TRUE(get_att.reliable_writer_qos().disable_positive_acks.enabled);
    EXPECT_EQ(get_att.reliable_writer_qos().disable_positive_acks.duration, eprosima::fastdds::dds::Duration_t({1, 0}));

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
    // Wait for all acked msgs
    EXPECT_TRUE(writer.waitForAllAcked(std::chrono::milliseconds(1200)));

    // Wait to disable timer because no new messages are sent
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    // Send a new message to check that timer is restarted correctly
    data = default_helloworld_data_generator(1);
    reader.startReception(data);
    writer.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();
    EXPECT_TRUE(writer.waitForAllAcked(std::chrono::milliseconds(1200)));

    // Update attributes on DDS layer
    eprosima::fastdds::dds::DataWriterQos w_att = writer.get_qos();
    w_att.reliable_writer_qos().disable_positive_acks.enabled = true;
    w_att.reliable_writer_qos().disable_positive_acks.duration = eprosima::fastdds::dds::Duration_t({2, 0});

    EXPECT_TRUE(writer.set_qos(w_att));

    // Check that period has been changed in DataWriterQos
    get_att = writer.get_qos();
    EXPECT_TRUE(get_att.reliable_writer_qos().disable_positive_acks.enabled);
    EXPECT_EQ(get_att.reliable_writer_qos().disable_positive_acks.duration, eprosima::fastdds::dds::Duration_t({2, 0}));

    data = default_helloworld_data_generator();

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
    // Check that period has been correctly updated
    EXPECT_FALSE(writer.waitForAllAcked(std::chrono::milliseconds(1200)));
    EXPECT_TRUE(writer.waitForAllAcked(std::chrono::milliseconds(1200)));

    // Try to disable positive_acks
    w_att.reliable_writer_qos().disable_positive_acks.enabled = false;

    // Check that is not possible to change disable_positive_acks at runtime
    EXPECT_FALSE(writer.set_qos(w_att));
}

TEST(AcknackQos, RecoverAfterLosingCommunicationWithDisablePositiveAck)
{
    // This test makes the writer send a few samples
    // and checks that those changes were received by the reader.
    // Then disconnects the communication and sends some more samples.
    // Reconnects and checks that the reader receives only the lost samples by the disconnection.

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // Number of samples written by writer
    uint32_t writer_samples = 15;

    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();

    writer.keep_duration({2, 0});
    //writer.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    //writer.history_depth(15);
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    //writer.lifespan_period(lifespan_s);
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(test_transport);
    writer.init();

    reader.keep_duration({1, 0});
    //reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    //reader.history_depth(15);
    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    //reader.lifespan_period(lifespan_s);
    reader.init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    std::list<HelloWorld> data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = true;

    data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    std::this_thread::sleep_for(std::chrono::seconds(1));

    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = false;

    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST(AcknackQos, NotRecoverAfterLosingCommunicationWithDisablePositiveAck)
{
    // This test makes the writer send a few samples
    // and checks that those changes were received by the reader.
    // Then disconnects the communication and sends some more samples.
    // Reconnects and checks that the reader receives only the lost samples by the disconnection.

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // Number of samples written by writer
    uint32_t writer_samples = 15;

    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();

    writer.keep_duration({1, 0});
    //writer.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    //writer.history_depth(15);
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    //writer.lifespan_period(lifespan_s);
    writer.disable_builtin_transport();
    writer.add_user_transport_to_pparams(test_transport);
    writer.init();

    reader.keep_duration({1, 0});
    //reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    //reader.history_depth(15);
    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    //reader.lifespan_period(lifespan_s);
    reader.init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    std::list<HelloWorld> data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = true;

    data = default_helloworld_data_generator(writer_samples);
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = false;

    // Block reader until reception finished or timeout.
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(1)), 0u);
}

/*!
 * @test Regresion test for Github #3323.
 */
TEST(AcknackQos, DisablePositiveAcksWithBestEffortReader)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.keep_duration({2, 0});
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    writer.durability_kind(eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS);
    writer.init();

    reader.keep_duration({1, 0});
    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
    reader.init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    std::list<HelloWorld> data = default_helloworld_data_generator();
    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}
