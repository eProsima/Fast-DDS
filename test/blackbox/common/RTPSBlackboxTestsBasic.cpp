// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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


#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.hpp>
#include <fastdds/rtps/builtin/data/TopicDescription.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/flowcontrol/FlowControllerDescriptor.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "RTPSAsSocketReader.hpp"
#include "RTPSAsSocketWriter.hpp"
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS
};

class RTPS : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(library_settings);
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::rtps::RTPSDomain::set_library_settings(library_settings);
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

TEST_P(RTPS, RTPSAsNonReliableSocket)
{
    RTPSAsSocketReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::BEST_EFFORT).
            add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(RTPS, AsyncRTPSAsNonReliableSocket)
{
    RTPSAsSocketReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::BEST_EFFORT).
            add_to_multicast_locator_list(ip, global_port).
            asynchronously(eprosima::fastdds::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(RTPS, AsyncRTPSAsNonReliableSocketWithWriterSpecificFlowControl)
{
    RTPSAsSocketReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    uint32_t bytesPerPeriod = 440; // Roughly ten times the size of the payload being sent
    uint32_t periodMillisecs = 300;
    writer.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::BEST_EFFORT).
            add_to_multicast_locator_list(ip, global_port).
            asynchronously(eprosima::fastdds::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).
            add_flow_controller_descriptor_to_pparams(bytesPerPeriod, periodMillisecs).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(RTPS, RTPSAsReliableSocket)
{
    RTPSAsSocketReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).
            add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).
            add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(RTPS, AsyncRTPSAsReliableSocket)
{
    RTPSAsSocketReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).
            add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).
            add_to_multicast_locator_list(ip, global_port).
            asynchronously(eprosima::fastdds::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(RTPS, RTPSAsNonReliableWithRegistration)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::BEST_EFFORT).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(RTPS, AsyncRTPSAsNonReliableWithRegistration)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::BEST_EFFORT).
            asynchronously(eprosima::fastdds::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(2);
}

TEST_P(RTPS, RTPSAsReliableWithRegistration)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.add_to_multicast_locator_list(ip, global_port).
            reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

TEST_P(RTPS, AsyncRTPSAsReliableWithRegistration)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.add_to_multicast_locator_list(ip, global_port).
            reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.asynchronously(eprosima::fastdds::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}

// Regression test of Refs #2786, github issue #194
TEST_P(RTPS, RTPSAsReliableVolatileSocket)
{
    RTPSAsSocketReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSAsSocketWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    std::string ip("239.255.1.4");

    reader.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).
            add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).
            durability(eprosima::fastdds::rtps::DurabilityKind_t::VOLATILE).
            add_to_multicast_locator_list(ip, global_port).
            auto_remove_on_volatile().init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator();

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Wait for acks to be sent and check writer history is empty
    writer.wait_for_all_acked(std::chrono::seconds(100));

    ASSERT_TRUE(writer.is_history_empty());
}

TEST_P(RTPS, RTPSAsReliableWithRegistrationAndHolesInHistory)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // To simulate lossy conditions
    int gaps_to_drop = 2;
    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    test_transport->drop_gap_messages_filter_ = [&gaps_to_drop](rtps::CDRMessage_t& )
            {
                if (gaps_to_drop > 0)
                {
                    --gaps_to_drop;
                    return true;
                }
                return false;
            };
    test_transport->dropLogLength = 1;

    reader.
            durability(eprosima::fastdds::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.durability(eprosima::fastdds::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            disable_builtin_transport().
            add_user_transport_to_pparams(test_transport).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();
    auto send_data (data);

    reader.expected_data(data);
    reader.startReception();

    // Send data
    writer.send(send_data);
    // In this test all data should be sent.
    ASSERT_TRUE(send_data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
    // Block until all data is ACK'd
    writer.waitForAllAcked(std::chrono::seconds(10));

    // Make holes in history
    for (auto it = data.begin(); it != data.end();)
    {
        if ((it->index() % 2) == 0)
        {
            eprosima::fastdds::rtps::SequenceNumber_t seq {0, it->index()};
            writer.remove_change(seq);
            it = data.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Create a late joiner
    RTPSWithRegistrationReader<HelloWorldPubSubType> late_joiner(TEST_TOPIC_NAME);

    late_joiner.
            durability(eprosima::fastdds::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(late_joiner.isInitialized());

    // Wait for discovery.
    late_joiner.wait_discovery();

    // Block reader until reception finished or timeout.
    late_joiner.expected_data(data);
    late_joiner.startReception();
    late_joiner.block_for_all();
}

/*
 * This test checks that GAPs are properly sent when a writer is sending data to
 * each reader separately.
 */

TEST(RTPS, RTPSUnavailableSampleGapWhenSeparateSending)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // To simulate lossy conditions
    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();

    reader.
            durability(eprosima::fastdds::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            history_depth(3).
            reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    // set_separate_sending

    writer.durability(eprosima::fastdds::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            disable_builtin_transport().
            reliability(eprosima::fastdds::rtps::ReliabilityKind_t::RELIABLE).
            history_depth(3).
            set_separate_sending(true).
            add_user_transport_to_pparams(test_transport).init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    HelloWorld message;
    message.message("HelloWorld");

    std::list<HelloWorld> data;
    std::list<HelloWorld> expected;

    reader.startReception();

    // Send data
    uint16_t index = 0;
    message.index(++index);

    data.push_back(message);
    expected.push_back(message);
    reader.expected_data(expected);
    writer.send(data);

    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = true;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    message.index(++index);
    data.push_back(message);
    writer.send(data);

    message.index(++index);
    data.push_back(message);
    expected.push_back(message);
    reader.expected_data(expected);
    writer.send(data);

    writer.remove_change({0, 2});

    std::this_thread::sleep_for(std::chrono::seconds(1));

    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = false;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    message.index(++index);
    data.push_back(message);
    expected.push_back(message);
    reader.expected_data(expected);

    writer.send(data);

    // Block reader until reception finished or timeout.
    reader.block_for_all(std::chrono::seconds(1));
    // Block until all data is ACK'd
    writer.waitForAllAcked(std::chrono::seconds(1));

    EXPECT_EQ(reader.getReceivedCount(), static_cast<unsigned int>(expected.size()));
}

TEST_P(RTPS, RTPSAsReliableVolatileTwoWritersConsecutives)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.reliability(ReliabilityKind_t::RELIABLE).init();
    EXPECT_TRUE(reader.isInitialized());

    writer.reliability(ReliabilityKind_t::RELIABLE).init();
    EXPECT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto complete_data = default_helloworld_data_generator();

    reader.expected_data(complete_data);
    reader.startReception();

    // Send data
    writer.send(complete_data);
    EXPECT_TRUE(complete_data.empty());

    reader.block_for_all();
    reader.stopReception();

    writer.destroy();

    // Wait for undiscovery
    reader.wait_undiscovery();

    writer.reliability(ReliabilityKind_t::RELIABLE).init();

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    complete_data = default_helloworld_data_generator();

    reader.expected_data(complete_data, true);
    reader.startReception();

    writer.send(complete_data);
    EXPECT_TRUE(complete_data.empty());

    reader.block_for_all();

    reader.destroy();
    writer.destroy();
}

TEST_P(RTPS, RTPSAsReliableTransientLocalTwoWritersConsecutives)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.reliability(ReliabilityKind_t::RELIABLE).durability(DurabilityKind_t::TRANSIENT_LOCAL).init();
    EXPECT_TRUE(reader.isInitialized());

    writer.reliability(ReliabilityKind_t::RELIABLE).init();

    EXPECT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto complete_data = default_helloworld_data_generator();

    reader.expected_data(complete_data);
    reader.startReception();

    // Send data
    writer.send(complete_data);
    EXPECT_TRUE(complete_data.empty());

    reader.block_for_all();
    reader.stopReception();

    writer.destroy();

    // Wait for undiscovery
    reader.wait_undiscovery();

    writer.reliability(ReliabilityKind_t::RELIABLE).init();

    // Wait for discovery
    writer.wait_discovery();
    reader.wait_discovery();

    complete_data = default_helloworld_data_generator();

    reader.expected_data(complete_data, true);
    reader.startReception();

    writer.send(complete_data);
    EXPECT_TRUE(complete_data.empty());

    reader.block_for_all();

    reader.destroy();
    writer.destroy();
}

/**
 * This test checks the addition of network interfaces at run-time.
 *
 * After launching the reader with the network interfaces enabled,
 * the writer is launched with the transport simulating that there
 * are no interfaces.
 * No participant discovery occurs, nor is communication established.
 *
 * In a second step, the flag to simulate no interfaces is disabled and
 * RTPSParticipant::update_attributes() called to add the "new" interfaces.
 * Discovery is succesful and communication is established.
 */
TEST(RTPS, RTPSNetworkInterfaceChangesAtRunTime)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // reader is initialized with all the network interfaces
    reader.reliability(ReliabilityKind_t::RELIABLE).durability(DurabilityKind_t::TRANSIENT_LOCAL).init();
    ASSERT_TRUE(reader.isInitialized());

    // writer: launch without interfaces
    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    test_transport->test_transport_options->simulate_no_interfaces = true;
    writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport).init();
    ASSERT_TRUE(writer.isInitialized());

    // no discovery
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));
    EXPECT_EQ(writer.get_matched(), 0u);
    EXPECT_EQ(reader.get_matched(), 0u);

    // send data
    auto complete_data = default_helloworld_data_generator();
    size_t samples = complete_data.size();

    reader.expected_data(complete_data, true);
    reader.startReception();

    writer.send(complete_data);
    EXPECT_TRUE(complete_data.empty());

    // no data received
    reader.block_for_all(std::chrono::seconds(3));
    EXPECT_EQ(reader.getReceivedCount(), 0u);

    // enable interfaces
    test_transport->test_transport_options->simulate_no_interfaces = false;
    writer.participant_update_attributes();

    // Wait for discovery
    writer.wait_discovery(std::chrono::seconds(3));
    reader.wait_discovery(std::chrono::seconds(3));
    ASSERT_EQ(writer.get_matched(), 1u);
    ASSERT_EQ(reader.get_matched(), 1u);

    // data received
    reader.block_for_all();
    EXPECT_EQ(reader.getReceivedCount(), static_cast<unsigned int>(samples));

    reader.destroy();
    writer.destroy();
}

/**
 * Regression test for checking that a not enabled RTPSParticipant can be removed
 *
 * https://github.com/eProsima/Fast-DDS/pull/2171 introduced this regression since with it
 * the PDP is not enabled until calling BuiltinProtocols::enable(), which is called within
 * RTPSParticipant::enable(). However, during RTPSDomain::removeRTPSParticipant(), there is a call
 * to BuiltinProtocols::stopRTPSParticipantAnnouncement(), which in turn calls
 * PDP::stopRTPSParticipantAnnouncement(). That function ends up accessing a timed event pointer,
 * which is only instantiated on PDP::enable(). Since the RTPSParticipant was not enabled,
 * BuiltinProtocols and in turn PDP are not either, meaning that it is not safe to call
 * PDP::stopRTPSParticipantAnnouncement() on a not enabled PDP.
 *
 * The test checks that the necessary guards are in place so that it is safe to call
 * RTPSDomain::removeRTPSParticipant() o a not enabled RTPSParticipant.
 */
TEST(RTPS, RemoveDisabledParticipant)
{
    RTPSParticipantAttributes rtps_attr;
    RTPSParticipant* rtps_participant = RTPSDomain::createParticipant(
        (uint32_t)GET_PID() % 230, false, rtps_attr, nullptr);

    ASSERT_NE(nullptr, rtps_participant);
    ASSERT_TRUE(RTPSDomain::removeRTPSParticipant(rtps_participant));
}

/**
 * This test checks a race condition on initializing a writer's flow controller when creating
 * several RTPSWriters in parallel: https://eprosima.easyredmine.com/issues/15905
 *
 * The test creates a participant with 4 different flow controllers, and then creates 200 threads
 * which each create an RTPSWriter which uses one of the participant's flow controllers.
 * The threads wait for a command coming from the main thread to delete the writers. This is to
 * ensure that all threads are initialized prior to any of them starts deleting.
 */
TEST(RTPS, MultithreadedWriterCreation)
{
    /* Flow controller builder */
    using FlowControllerDescriptor_t = eprosima::fastdds::rtps::FlowControllerDescriptor;
    using SchedulerPolicy_t = eprosima::fastdds::rtps::FlowControllerSchedulerPolicy;
    auto create_flow_controller =
            [](const char* name, SchedulerPolicy_t scheduler,
                    int32_t max_bytes_per_period,
                    uint64_t period_ms) -> std::shared_ptr<FlowControllerDescriptor_t>
            {
                std::shared_ptr<FlowControllerDescriptor_t> descriptor = std::make_shared<FlowControllerDescriptor_t>();
                descriptor->name = name;
                descriptor->scheduler = scheduler;
                descriptor->max_bytes_per_period = max_bytes_per_period;
                descriptor->period_ms = period_ms;
                return descriptor;
            };

    /* Create participant */
    RTPSParticipantAttributes rtps_attr;
    // Create one flow controller of each kind to make things interesting
    const char* flow_controller_name = "fifo_controller";
    rtps_attr.flow_controllers.push_back(create_flow_controller("high_priority_controller",
            SchedulerPolicy_t::HIGH_PRIORITY, 200, 10));
    rtps_attr.flow_controllers.push_back(create_flow_controller("priority_with_reservation_controller",
            SchedulerPolicy_t::PRIORITY_WITH_RESERVATION, 200, 10));
    rtps_attr.flow_controllers.push_back(create_flow_controller("round_robin_controller",
            SchedulerPolicy_t::ROUND_ROBIN, 200, 10));
    rtps_attr.flow_controllers.push_back(create_flow_controller(flow_controller_name, SchedulerPolicy_t::FIFO, 200,
            10));
    RTPSParticipant* rtps_participant = RTPSDomain::createParticipant(
        (uint32_t)GET_PID() % 230, false, rtps_attr, nullptr);

    /* Test sync variables */
    std::mutex finish_mtx;
    std::condition_variable finish_cv;
    bool should_finish = false;

    /* Lambda function to create a writer with a flow controller, and to destroy it at command */
    auto thread_run = [rtps_participant, flow_controller_name, &finish_mtx, &finish_cv, &should_finish]()
            {
                /* Create writer history */
                eprosima::fastdds::rtps::HistoryAttributes hattr;
                eprosima::fastdds::rtps::WriterHistory* history = new eprosima::fastdds::rtps::WriterHistory(hattr);

                /* Create writer with a flow controller */
                eprosima::fastdds::rtps::WriterAttributes writer_attr;
                writer_attr.mode = RTPSWriterPublishMode::ASYNCHRONOUS_WRITER;
                writer_attr.flow_controller_name = flow_controller_name;
                eprosima::fastdds::rtps::RTPSWriter*  writer = eprosima::fastdds::rtps::RTPSDomain::createRTPSWriter(
                    rtps_participant, writer_attr, history, nullptr);

                TopicDescription topic_desc;
                topic_desc.type_name = "string";
                topic_desc.topic_name = "test_topic";
                /* Register writer in participant */
                eprosima::fastdds::dds::WriterQos writer_qos;
                ASSERT_EQ(rtps_participant->register_writer(writer, topic_desc, writer_qos), true);

                {
                    /* Wait for test completion request */
                    std::unique_lock<std::mutex> lock(finish_mtx);
                    finish_cv.wait(lock, [&should_finish]()
                            {
                                return should_finish;
                            });
                }

                /* Remove writer */
                ASSERT_TRUE(RTPSDomain::removeRTPSWriter(writer));
                delete history;
            };

    {
        /* Create test threads */
        constexpr size_t num_threads = 200;
        std::vector<std::thread> threads;
        for (size_t i = 0; i < num_threads; ++i)
        {
            threads.push_back(std::thread(thread_run));
        }

        /* Once all threads are created, we can start deleting them */
        {
            std::lock_guard<std::mutex> guard(finish_mtx);
            should_finish = true;
            finish_cv.notify_all();
        }

        /* Wait until are threads join */
        for (std::thread& thr : threads)
        {
            thr.join();
        }
    }

    /* Clean up */
    ASSERT_TRUE(RTPSDomain::removeRTPSParticipant(rtps_participant));
    ASSERT_NE(nullptr, rtps_participant);
    RTPSDomain::stopAll();
}

class CustomReaderDataFilter : public eprosima::fastdds::rtps::IReaderDataFilter
{
public:

    CustomReaderDataFilter() = default;

    ~CustomReaderDataFilter() = default;

    bool is_relevant(
            const eprosima::fastdds::rtps::CacheChange_t& change,
            const eprosima::fastdds::rtps::GUID_t& reader_guid) const override
    {
        static_cast<void>(reader_guid);
        if (change.sequenceNumber == SequenceNumber_t{0, 3})
        {
            return false;
        }
        return true;
    }

};

/*!
 * Auxiliary method
 */
void has_been_fully_delivered_test(
        ReliabilityKind_t reliability_mode)
{
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer_1(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer_2(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    CustomReaderDataFilter filter;
    writer_1.reliability(reliability_mode).init();
    // Once writer has been initialized (created) set filter
    writer_1.reader_data_filter(&filter);
    ASSERT_TRUE(writer_1.isInitialized());

    auto data = default_helloworld_data_generator(3);
    // No matched RTPSReaders: sample considered delivered
    eprosima::fastdds::rtps::CacheChange_t* change = writer_1.send_sample(data.front());
    EXPECT_TRUE(writer_1.has_been_fully_delivered(change->sequenceNumber));
    data.pop_front();

    // No matched RTPSReaders: sequence number not generated by RTPSWriter
    EXPECT_FALSE(writer_1.has_been_fully_delivered({10, 0}));

    writer_2.reliability(reliability_mode).asynchronously(RTPSWriterPublishMode::ASYNCHRONOUS_WRITER)
            .add_flow_controller_descriptor_to_pparams(eprosima::fastdds::rtps::FlowControllerSchedulerPolicy::FIFO,
            1, 1000).init();
    ASSERT_TRUE(writer_2.isInitialized());

    reader.reliability(reliability_mode).init();
    ASSERT_TRUE(reader.isInitialized());

    writer_1.wait_discovery();
    writer_2.wait_discovery();
    reader.wait_discovery();

    reader.expected_data(data);
    reader.startReception();

    change = writer_1.send_sample(data.front());
    if (reliability_mode == ReliabilityKind_t::BEST_EFFORT)
    {
        EXPECT_TRUE(writer_1.waitForAllAcked(std::chrono::milliseconds(150)));
    }
    else
    {
        EXPECT_TRUE(writer_1.waitForAllAcked(std::chrono::seconds::zero()));
    }
    EXPECT_TRUE(writer_1.has_been_fully_delivered(change->sequenceNumber));
    // Flow controller prevents data sending
    change = writer_2.send_sample(data.front());
    if (reliability_mode == ReliabilityKind_t::BEST_EFFORT)
    {
        EXPECT_TRUE(writer_2.waitForAllAcked(std::chrono::milliseconds(150)));
    }
    else
    {
        EXPECT_FALSE(writer_2.waitForAllAcked(std::chrono::milliseconds(150)));
    }
    EXPECT_FALSE(writer_2.has_been_fully_delivered(change->sequenceNumber));
    data.pop_front();

    // Irrelevant change is considered delivered
    change = writer_1.send_sample(data.front());
    EXPECT_TRUE(writer_1.waitForAllAcked(std::chrono::milliseconds(150)));
    EXPECT_TRUE(writer_1.has_been_fully_delivered(change->sequenceNumber));

    // Sequence number not generated by RTPSWriter
    change->sequenceNumber = SequenceNumber_t{2, 0};
    EXPECT_FALSE(writer_1.has_been_fully_delivered(change->sequenceNumber));

    if (reliability_mode == ReliabilityKind_t::RELIABLE)
    {
        reader.block_for_at_least(1);
        EXPECT_EQ(reader.getReceivedCount(), 1u);
    }
}

/*!
 * \test RTPS-HBD-01 Test `RTPSWriter::has_been_fully_delivered` with Best Effort Reliability
 */
TEST(RTPS, best_effort_has_been_fully_delivered)
{
    has_been_fully_delivered_test(ReliabilityKind_t::BEST_EFFORT);
}

/*!
 * \test RTPS-HBD-02 Test `RTPSWriter::has_been_fully_delivered` with Reliable Reliability
 */
TEST(RTPS, reliable_has_been_fully_delivered)
{
    has_been_fully_delivered_test(ReliabilityKind_t::RELIABLE);
}

/**
 * @test This test checks the ignore local endpoints feature on the RTPS layer when writer and
 *       reader are under the same participant. Corresponds with tests:
 *          * PART-IGNORE-LOCAL-TEST:04
 *          * PART-IGNORE-LOCAL-TEST:05
 *          * PART-IGNORE-LOCAL-TEST:06
 */
TEST(RTPS, participant_ignore_local_endpoints)
{
    class CustomLogConsumer : public eprosima::fastdds::dds::LogConsumer
    {
    public:

        void Consume(
                const eprosima::fastdds::dds::Log::Entry&) override
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
        {"PART-IGNORE-LOCAL-TEST:04", "true", 0, 0, 5, 0},
        {"PART-IGNORE-LOCAL-TEST:05", "false", 0, 1, 5, 5},
        {"PART-IGNORE-LOCAL-TEST:06", "asdfg", 1, 1, 5, 5}
    };

    for (Config test_config : tests_configs)
    {
        std::cout << std::endl;
        std::cout << "---------------------------------------" << std::endl;
        std::cout << "Running test: " << test_config.test_id << std::endl;
        std::cout << "---------------------------------------" << std::endl;

        /* Set up */
        eprosima::fastdds::dds::Log::Reset();
        eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);
        CustomLogConsumer* log_consumer = new CustomLogConsumer();
        std::unique_ptr<CustomLogConsumer> log_consumer_unique_ptr(log_consumer);
        eprosima::fastdds::dds::Log::RegisterConsumer(std::move(log_consumer_unique_ptr));

        // Create the RTPSParticipant with the appropriate value for the property
        eprosima::fastdds::rtps::RTPSParticipantAttributes patt;
        patt.properties.properties().emplace_back("fastdds.ignore_local_endpoints", test_config.property_value);
        eprosima::fastdds::rtps::RTPSParticipant* participant =
                eprosima::fastdds::rtps::RTPSDomain::createParticipant(static_cast<uint32_t>(GET_PID()) % 230, patt);
        ASSERT_NE(participant, nullptr);

        /* Procedure */
        // Create the RTPSWriter
        RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME, participant);
        writer.init();
        EXPECT_TRUE(writer.isInitialized());

        // Create the RTPSReader
        RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME, participant);
        reader.init();
        EXPECT_TRUE(reader.isInitialized());

        // Wait for discovery
        writer.wait_discovery(test_config.expected_matched_endpoints, std::chrono::seconds(1));
        reader.wait_discovery(test_config.expected_matched_endpoints, std::chrono::seconds(1));
        EXPECT_EQ(writer.get_matched(), test_config.expected_matched_endpoints);
        EXPECT_EQ(reader.get_matched(), test_config.expected_matched_endpoints);

        // Send samples
        auto samples = default_helloworld_data_generator(test_config.sent_samples);
        reader.expected_data(samples);
        reader.startReception(samples.size());
        writer.send(samples);
        EXPECT_TRUE(samples.empty());

        // Wait for reception
        reader.block_for_all(std::chrono::seconds(1));
        EXPECT_EQ(reader.getReceivedCount(), test_config.expected_received_samples);

        // Wait for log entries
        EXPECT_EQ(log_consumer->wait_for_entries(test_config.log_errors, std::chrono::seconds(
                    1)), test_config.log_errors);

        /* Tear-down */
        eprosima::fastdds::rtps::RTPSDomain::removeRTPSParticipant(participant);
        eprosima::fastdds::dds::Log::Reset();
    }
}

/**
 * @test This test checks the ignore local endpoints feature on the RTPS layer when writer and
 *       reader are under the different participant. Corresponds with test:
 *          * PART-IGNORE-LOCAL-TEST:08
 */
TEST(RTPS, participant_ignore_local_endpoints_two_participants)
{
    std::cout << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "Running test: PART-IGNORE-LOCAL-TEST:08" << std::endl;
    std::cout << "---------------------------------------" << std::endl;

    /* Set up */
    // Create the RTPSParticipants with the appropriate value for the property
    eprosima::fastdds::rtps::RTPSParticipantAttributes patt;
    patt.properties.properties().emplace_back("fastdds.ignore_local_endpoints", "true");
    eprosima::fastdds::rtps::RTPSParticipant* participant_writer =
            eprosima::fastdds::rtps::RTPSDomain::createParticipant(static_cast<uint32_t>(GET_PID()) % 230, patt);
    ASSERT_NE(participant_writer, nullptr);
    eprosima::fastdds::rtps::RTPSParticipant* participant_reader =
            eprosima::fastdds::rtps::RTPSDomain::createParticipant(static_cast<uint32_t>(GET_PID()) % 230, patt);
    ASSERT_NE(participant_reader, nullptr);

    /* Procedure */
    // Create the RTPSWriter
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME, participant_writer);
    writer.init();
    EXPECT_TRUE(writer.isInitialized());

    // Create the RTPSReader
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME, participant_reader);
    reader.init();
    EXPECT_TRUE(reader.isInitialized());

    // Wait for discovery
    writer.wait_discovery(1, std::chrono::seconds(1));
    reader.wait_discovery(1, std::chrono::seconds(1));
    EXPECT_EQ(writer.get_matched(), 1u);
    EXPECT_EQ(reader.get_matched(), 1u);

    // Send samples
    auto samples = default_helloworld_data_generator(5);
    reader.expected_data(samples);
    reader.startReception(samples.size());
    writer.send(samples);
    EXPECT_TRUE(samples.empty());

    // Wait for reception
    reader.block_for_all(std::chrono::seconds(1));
    EXPECT_EQ(reader.getReceivedCount(), 5u);

    /* Tear-down */
    eprosima::fastdds::rtps::RTPSDomain::removeRTPSParticipant(participant_writer);
    eprosima::fastdds::rtps::RTPSDomain::removeRTPSParticipant(participant_reader);
}

/* Maximum number of bytes allowed for an RTPS datagram generated by this participant. */
TEST(RTPS, max_output_message_size_participant)
{
    /* Set up */
    // Create the RTPSReader
    RTPSWithRegistrationReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    reader.init();
    EXPECT_TRUE(reader.isInitialized());

    // Create the RTPSParticipants with the appropriate value for the property
    auto test_transport =  std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    const uint32_t segment_size = 1470;
    std::string segment_size_str = std::to_string(segment_size);
    test_transport->messages_filter_ = [segment_size](eprosima::fastdds::rtps::CDRMessage_t& datagram)
            {
                EXPECT_LE(datagram.length, segment_size);
                // Never drop samples
                return false;
            };

    eprosima::fastdds::rtps::RTPSParticipantAttributes patt;
    patt.useBuiltinTransports = false;
    patt.userTransports.push_back(test_transport);
    patt.properties.properties().emplace_back("fastdds.max_message_size", segment_size_str);
    eprosima::fastdds::rtps::RTPSParticipant* participant_writer =
            eprosima::fastdds::rtps::RTPSDomain::createParticipant(static_cast<uint32_t>(GET_PID()) % 230, patt);
    ASSERT_NE(participant_writer, nullptr);

    // Create the RTPSWriter
    RTPSWithRegistrationWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME, participant_writer);
    writer.init();
    EXPECT_TRUE(writer.isInitialized());

    // Wait for discovery
    writer.wait_discovery(1, std::chrono::seconds(2));
    reader.wait_discovery(1, std::chrono::seconds(2));
    EXPECT_EQ(writer.get_matched(), 1u);
    EXPECT_EQ(reader.get_matched(), 1u);

    // Send samples
    auto samples = default_data16kb_data_generator(1);
    reader.expected_data(samples);
    reader.startReception();
    writer.send(samples);
    EXPECT_TRUE(samples.empty());

    // Wait for reception
    reader.block_for_all(std::chrono::seconds(1));
    EXPECT_EQ(reader.getReceivedCount(), 1u);

    /* Tear-down */
    eprosima::fastdds::rtps::RTPSDomain::removeRTPSParticipant(participant_writer);
}

/* Maximum number of bytes allowed for an RTPS datagram generated by this writer. */
TEST(RTPS, max_output_message_size_writer)
{
    const uint32_t segment_size = 1470;
    std::string segment_size_str = std::to_string(segment_size);

    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    test_transport->messages_filter_ = [segment_size](eprosima::fastdds::rtps::CDRMessage_t& datagram)
            {
                EXPECT_LE(datagram.length, segment_size);
                // Never drop samples
                return false;
            };
    RTPSWithRegistrationWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);
    writer.add_property("fastdds.max_message_size", segment_size_str).
            disable_builtin_transport().add_user_transport_to_pparams(test_transport).init();
    ASSERT_TRUE(writer.isInitialized());

    RTPSWithRegistrationReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);
    reader.init();
    EXPECT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    EXPECT_EQ(writer.get_matched(), 1u);
    EXPECT_EQ(reader.get_matched(), 1u);

    // Send samples
    auto samples = default_data16kb_data_generator(1);
    reader.expected_data(samples);
    reader.startReception();
    writer.send(samples);
    EXPECT_TRUE(samples.empty());

    // Wait for reception
    reader.block_for_all(std::chrono::seconds(1));
    EXPECT_EQ(reader.getReceivedCount(), 1u);

}

class DummyPool : public IPayloadPool
{
public:

    DummyPool(
            uint32_t payload_size,
            uint32_t num_endpoints,
            uint32_t num_samples)
        : payload_size_(payload_size)
    {
        for (uint32_t i = 0; i < num_samples * num_endpoints; ++i)
        {
            octet* payload = (octet*)calloc(payload_size_, sizeof(octet));

            all_payloads_.emplace(payload, 0u);
            free_payloads_.push_back(payload);
        }
    }

    ~DummyPool()
    {
        for (auto it : all_payloads_)
        {
            free(it.first);
        }
    }

    bool get_payload(
            uint32_t size,
            SerializedPayload_t& payload) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return do_get_payload(size, payload);
    }

    bool get_payload(
            const SerializedPayload_t& data,
            SerializedPayload_t& payload) override
    {
        std::lock_guard<std::mutex> lock(mutex_);

        octet* payload_buff = data.data;

        if (data.payload_owner == this)
        {
            uint32_t& refs = all_payloads_[payload_buff];
            EXPECT_LT(0u, refs);
            ++refs;
            ++num_reserves_;
            ++num_references_;

            payload.data = payload_buff;
            payload.length = data.length;
            payload.max_size = data.max_size;
            payload.payload_owner = this;
            return true;
        }

        if (!do_get_payload(data.max_size, payload))
        {
            return false;
        }

        ++num_copies_;
        payload.copy(&data, true);

        return true;
    }

    bool release_payload(
            SerializedPayload_t& payload) override
    {
        std::lock_guard<std::mutex> lock(mutex_);

        EXPECT_EQ(this, payload.payload_owner);

        octet* payload_buff = payload.data;
        uint32_t& refs = all_payloads_[payload_buff];

        EXPECT_GT(refs, 0u);

        ++num_releases_;
        if (0 == --refs)
        {
            free_payloads_.push_back(payload_buff);
        }

        payload.data = nullptr;
        payload.max_size = 0;
        payload.length = 0;
        payload.payload_owner = nullptr;

        return true;
    }

    size_t num_reserves() const
    {
        return num_reserves_;
    }

    size_t num_releases() const
    {
        return num_releases_;
    }

    size_t num_references() const
    {
        return num_references_;
    }

    size_t num_copies() const
    {
        return num_copies_;
    }

private:

    bool do_get_payload(
            uint32_t size,
            SerializedPayload_t& payload)
    {
        if (free_payloads_.empty())
        {
            return false;
        }

        EXPECT_LE(size, payload_size_);

        octet* payload_buff = free_payloads_.back();
        uint32_t& refs = all_payloads_[payload_buff];
        EXPECT_EQ(0u, refs);

        free_payloads_.pop_back();
        ++refs;
        ++num_reserves_;

        payload.data = payload_buff;
        payload.max_size = payload_size_;
        payload.length = 0;
        payload.pos = 0;
        payload.payload_owner = this;

        return true;
    }

    uint32_t payload_size_;

    size_t num_reserves_ = 0;
    size_t num_releases_ = 0;
    size_t num_references_ = 0;
    size_t num_copies_ = 0;

    std::mutex mutex_;
    std::map<octet*, uint32_t> all_payloads_;
    std::vector<octet*> free_payloads_;
};

/* Endpoint creation fails when entity id is incoherent. */
TEST(RTPS, endpoint_creation_fails_with_incoherent_entity_id)
{
    // create dummy payload pool for reader
    uint32_t payload_size = static_cast<uint32_t>(256);
    payload_size += static_cast<uint32_t>(eprosima::fastcdr::Cdr::alignment(payload_size, 4)); /* possible submessage alignment */
    payload_size += 4u; // encapsulation header

    std::shared_ptr<DummyPool> pool = std::make_shared<DummyPool>(payload_size, 10, 10);

    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    RTPSWithRegistrationWriter<HelloWorldPubSubType> wrong_writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> wrong_reader(TEST_TOPIC_NAME);

    RTPSWithRegistrationWriter<KeyedHelloWorldPubSubType> keyed_writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<KeyedHelloWorldPubSubType> keyed_reader(TEST_TOPIC_NAME);

    RTPSWithRegistrationWriter<KeyedHelloWorldPubSubType> wrong_keyed_writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<KeyedHelloWorldPubSubType> wrong_keyed_reader(TEST_TOPIC_NAME);

    const EntityId_t writer_entity_id(0x0003003);
    const EntityId_t keyed_writer_entity_id(0x0003002);

    const EntityId_t reader_entity_id(0x0003004);
    const EntityId_t keyed_reader_entity_id(0x0003007);

    writer.set_entity_id(writer_entity_id)
            .init();
    reader.set_entity_id(reader_entity_id)
            .payload_pool(pool)
            .init();

    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    wrong_writer.set_entity_id(keyed_writer_entity_id)
            .init();
    wrong_reader.set_entity_id(keyed_reader_entity_id)
            .payload_pool(pool)
            .init();

    ASSERT_FALSE(wrong_writer.isInitialized());
    ASSERT_FALSE(wrong_reader.isInitialized());

    keyed_writer.set_entity_id(keyed_writer_entity_id)
            .init();
    keyed_reader.set_entity_id(keyed_reader_entity_id)
            .payload_pool(pool)
            .init();

    ASSERT_TRUE(keyed_writer.isInitialized());
    ASSERT_TRUE(keyed_reader.isInitialized());

    wrong_keyed_writer.set_entity_id(writer_entity_id)
            .init();
    wrong_keyed_reader.set_entity_id(reader_entity_id)
            .payload_pool(pool)
            .init();

    ASSERT_FALSE(wrong_keyed_writer.isInitialized());
    ASSERT_FALSE(wrong_keyed_reader.isInitialized());
}

bool validate_publication_builtin_topic_data(
        const eprosima::fastdds::rtps::PublicationBuiltinTopicData& pubdata,
        const EntityId_t& writer_id,
        const dds::WriterQos& writer_qos,
        const TopicDescription& topic_desc,
        const GUID_t& participant_guid)
{
    bool ret = true;

    eprosima::fastdds::rtps::BuiltinTopicKey_t w_key, part_key;

    entity_id_to_builtin_topic_key(w_key, writer_id);
    guid_prefix_to_builtin_topic_key(part_key, participant_guid.guidPrefix);

    ret &= (0 == memcmp(pubdata.key.value, w_key.value, sizeof(eprosima::fastdds::rtps::BuiltinTopicKey_t)));
    ret &=
            (0 ==
            memcmp(pubdata.participant_key.value, part_key.value,
            sizeof(eprosima::fastdds::rtps::BuiltinTopicKey_t)));
    ret &= (pubdata.topic_name == topic_desc.topic_name);
    ret &= (pubdata.type_name == topic_desc.type_name);

    // Writer Qos
    ret &= (pubdata.durability == writer_qos.m_durability);
    ret &= (pubdata.durability_service == writer_qos.m_durabilityService);
    ret &= (pubdata.deadline == writer_qos.m_deadline);
    ret &= (pubdata.latency_budget == writer_qos.m_latencyBudget);
    ret &= (pubdata.liveliness == writer_qos.m_liveliness);
    ret &= (pubdata.reliability == writer_qos.m_reliability);
    ret &= (pubdata.lifespan == writer_qos.m_lifespan);
    ret &= (
        (pubdata.user_data.size() == writer_qos.m_userData.size()) &&
        (0 == memcmp(pubdata.user_data.data(), writer_qos.m_userData.data(), pubdata.user_data.size())));
    ret &= (pubdata.ownership == writer_qos.m_ownership);
    ret &= (pubdata.ownership_strength == writer_qos.m_ownershipStrength);
    ret &= (pubdata.destination_order == writer_qos.m_destinationOrder);

    // Publisher Qos
    ret &= (pubdata.presentation == writer_qos.m_presentation);
    ret &= (pubdata.partition.getNames() == writer_qos.m_partition.getNames());
    // ignore topic_data not implemented
    // ignore group_data

    return ret;
}

bool validate_subscription_builtin_topic_data(
        const eprosima::fastdds::rtps::SubscriptionBuiltinTopicData& subdata,
        const EntityId_t& reader_id,
        const dds::ReaderQos& reader_qos,
        const TopicDescription& topic_desc,
        const GUID_t& participant_guid)
{
    bool ret = true;

    eprosima::fastdds::rtps::BuiltinTopicKey_t r_key, part_key;

    entity_id_to_builtin_topic_key(r_key, reader_id);
    guid_prefix_to_builtin_topic_key(part_key, participant_guid.guidPrefix);

    ret &= (0 == memcmp(subdata.key.value, r_key.value, sizeof(eprosima::fastdds::rtps::BuiltinTopicKey_t)));
    ret &=
            (0 ==
            memcmp(subdata.participant_key.value, part_key.value,
            sizeof(eprosima::fastdds::rtps::BuiltinTopicKey_t)));
    ret &= (subdata.topic_name == topic_desc.topic_name);
    ret &= (subdata.type_name == topic_desc.type_name);

    // RTPS Reader
    ret &= (subdata.durability == reader_qos.m_durability);
    ret &= (subdata.deadline == reader_qos.m_deadline);
    ret &= (subdata.latency_budget == reader_qos.m_latencyBudget);
    ret &= (subdata.liveliness == reader_qos.m_liveliness);
    ret &= (subdata.reliability == reader_qos.m_reliability);
    ret &= (subdata.ownership == reader_qos.m_ownership);
    ret &= (subdata.destination_order == reader_qos.m_destinationOrder);
    ret &= (
        (subdata.user_data.size() == reader_qos.m_userData.size()) &&
        (0 == memcmp(subdata.user_data.data(), reader_qos.m_userData.data(), subdata.user_data.size())));
    // time based filter not implemented

    // Subscriber Qos
    ret &= (subdata.presentation == reader_qos.m_presentation);
    ret &= (subdata.partition.getNames() == reader_qos.m_partition.getNames());
    // ignore topic_data not implemented
    // ignore group_data

    return ret;
}

/**
 * @test RTPS-PART-API-GSI-GPI-01
 *
 * get_subscription/publication_info() must return false if the entity is not found.
 */
TEST(RTPS, rtps_participant_get_pubsub_info_negative)
{
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.init();
    reader.init();

    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    eprosima::fastdds::rtps::PublicationBuiltinTopicData pubdata;
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData subdata;

    // Get publication info from the reader participant and validate it
    GUID_t unknown_writer_guid = writer.guid();
    unknown_writer_guid.entityId.value[3] = 0x44;
    bool ret = reader.get_rtps_participant()->get_publication_info(pubdata, unknown_writer_guid);
    ASSERT_FALSE(ret);

    GUID_t unknown_reader_guid = reader.guid();
    unknown_reader_guid.entityId.value[3] = 0x44;
    // Get subscription info from the reader participant and validate it
    ret = writer.get_rtps_participant()->get_subscription_info(subdata, unknown_reader_guid);
    ASSERT_FALSE(ret);
}

/**
 * @test RTPS-PART-API-GSI-GPI-02
 *
 * get_subscription/publication_info() must succeed when the guid is known and correctly retrieve the publication/subscription data.
 * Parameterize the test for different transports (Transport, Datasharing and Intraprocess).
 */
TEST_P(RTPS, rtps_participant_get_pubsub_info)
{
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    std::vector<std::string> partitions{"*"};

    writer.user_data({'u', 's', 'e', 'r', 'd', 'a', 't', 'a'})
            .partitions(partitions)
            .init();
    reader.user_data({'u', 's', 'e', 'r', 'd', 'a', 't', 'a'})
            .partitions(partitions)
            .init();

    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    eprosima::fastdds::rtps::PublicationBuiltinTopicData pubdata;
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData subdata;

    // Get publication info from the reader participant and validate it
    bool ret = reader.get_rtps_participant()->get_publication_info(pubdata, writer.guid());
    ASSERT_TRUE(ret);
    ASSERT_TRUE(validate_publication_builtin_topic_data(pubdata, writer.guid().entityId,
            writer.get_writer_qos(), writer.get_topic_description(),  writer.get_rtps_participant()->getGuid()));

    // Get subscription info from the reader participant and validate it
    ret = writer.get_rtps_participant()->get_subscription_info(subdata, reader.guid());
    ASSERT_TRUE(ret);
    ASSERT_TRUE(validate_subscription_builtin_topic_data(subdata, reader.guid().entityId,
            reader.get_reader_qos(), reader.get_topic_description(), reader.get_rtps_participant()->getGuid()));
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(RTPS,
        RTPS,
        testing::Values(TRANSPORT, INTRAPROCESS),
        [](const testing::TestParamInfo<RTPS::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }

        });
