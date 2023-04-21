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

#include "BlackboxTests.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/flowcontrol/FlowControllerDescriptor.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.h>
#include <fastrtps/log/Log.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include "RTPSAsSocketReader.hpp"
#include "RTPSAsSocketWriter.hpp"
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"
#include <rtps/transport/test_UDPv4Transport.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using test_UDPv4Transport = eprosima::fastdds::rtps::test_UDPv4Transport;

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
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_OFF;
                xmlparser::XMLProfileManager::library_settings(library_settings);
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

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
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

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
            add_to_multicast_locator_list(ip, global_port).
            asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

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
    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
            add_to_multicast_locator_list(ip, global_port).
            asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).
            add_throughput_controller_descriptor_to_pparams(bytesPerPeriod, periodMillisecs).init();

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

    reader.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
            add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
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

    reader.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
            add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
            add_to_multicast_locator_list(ip, global_port).
            asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

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

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).init();

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

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::BEST_EFFORT).
            asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

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
            reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).init();

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
            reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.asynchronously(eprosima::fastrtps::rtps::RTPSWriterPublishMode::ASYNCHRONOUS_WRITER).init();

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

    reader.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
            add_to_multicast_locator_list(ip, global_port).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
            durability(eprosima::fastrtps::rtps::DurabilityKind_t::VOLATILE).
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
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport->drop_gap_messages_filter_ = [&gaps_to_drop](rtps::CDRMessage_t& )
            {
                if (gaps_to_drop > 0)
                {
                    --gaps_to_drop;
                    return true;
                }
                return false;
            };
    testTransport->dropLogLength = 1;

    reader.
            durability(eprosima::fastrtps::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.durability(eprosima::fastrtps::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            disable_builtin_transport().
            add_user_transport_to_pparams(testTransport).init();

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
            eprosima::fastrtps::rtps::SequenceNumber_t seq {0, it->index()};
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
            durability(eprosima::fastrtps::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).init();

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
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();

    reader.
            durability(eprosima::fastrtps::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            history_depth(3).
            reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    // set_separate_sending

    writer.durability(eprosima::fastrtps::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            disable_builtin_transport().
            reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).
            history_depth(3).
            add_user_transport_to_pparams(testTransport).init();

    ASSERT_TRUE(writer.isInitialized());

    writer.set_separate_sending(true);

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

    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = true;

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

    test_UDPv4Transport::test_UDPv4Transport_ShutdownAllNetwork = false;

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
    test_UDPv4Transport::simulate_no_interfaces = true;
    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
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
    test_UDPv4Transport::simulate_no_interfaces = false;
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
                eprosima::fastrtps::rtps::HistoryAttributes hattr;
                eprosima::fastrtps::rtps::WriterHistory* history = new eprosima::fastrtps::rtps::WriterHistory(hattr);
                eprosima::fastrtps::TopicAttributes topic_attr;

                /* Create writer with a flow controller */
                eprosima::fastrtps::rtps::WriterAttributes writer_attr;
                writer_attr.mode = RTPSWriterPublishMode::ASYNCHRONOUS_WRITER;
                writer_attr.flow_controller_name = flow_controller_name;
                eprosima::fastrtps::rtps::RTPSWriter*  writer = eprosima::fastrtps::rtps::RTPSDomain::createRTPSWriter(
                    rtps_participant, writer_attr, history, nullptr);

                /* Register writer in participant */
                eprosima::fastrtps::WriterQos writer_qos;
                ASSERT_EQ(rtps_participant->registerWriter(writer, topic_attr, writer_qos), true);

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

/* Regression Test for improving gaps processing
 *  https://github.com/eProsima/Fast-DDS/pull/3343
 */
TEST(RTPS, RTPSCorrectGAPProcessing)
{
    RTPSWithRegistrationReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    RTPSWithRegistrationWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    reader.durability(eprosima::fastrtps::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.durability(eprosima::fastrtps::rtps::DurabilityKind_t::TRANSIENT_LOCAL).
            reliability(eprosima::fastrtps::rtps::ReliabilityKind_t::RELIABLE).init();

    ASSERT_TRUE(writer.isInitialized());

    reader.wait_discovery();
    writer.wait_discovery();

    SequenceNumberSet_t seq_set(SequenceNumber_t(0, 0));

    //! GAP Message check
    RTPSReader& native_reader = reader.get_native_reader();
    ASSERT_NO_FATAL_FAILURE(native_reader.processGapMsg(writer.guid(), {0, 0}, seq_set));
}

class CustomReaderDataFilter : public eprosima::fastdds::rtps::IReaderDataFilter
{
public:

    CustomReaderDataFilter() = default;

    ~CustomReaderDataFilter() = default;

    bool is_relevant(
            const eprosima::fastrtps::rtps::CacheChange_t& change,
            const eprosima::fastrtps::rtps::GUID_t& reader_guid) const override
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
    eprosima::fastrtps::rtps::CacheChange_t* change = writer_1.send_sample(data.front());
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
        Log::Reset();
        Log::SetVerbosity(Log::Error);
        CustomLogConsumer* log_consumer = new CustomLogConsumer();
        std::unique_ptr<CustomLogConsumer> log_consumer_unique_ptr(log_consumer);
        Log::RegisterConsumer(std::move(log_consumer_unique_ptr));

        // Create the RTPSParticipant with the appropriate value for the property
        eprosima::fastrtps::rtps::RTPSParticipantAttributes patt;
        patt.properties.properties().emplace_back("fastdds.ignore_local_endpoints", test_config.property_value);
        eprosima::fastrtps::rtps::RTPSParticipant* participant =
                eprosima::fastrtps::rtps::RTPSDomain::createParticipant(static_cast<uint32_t>(GET_PID()) % 230, patt);
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
        eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(participant);
        Log::Reset();
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
    eprosima::fastrtps::rtps::RTPSParticipantAttributes patt;
    patt.properties.properties().emplace_back("fastdds.ignore_local_endpoints", "true");
    eprosima::fastrtps::rtps::RTPSParticipant* participant_writer =
            eprosima::fastrtps::rtps::RTPSDomain::createParticipant(static_cast<uint32_t>(GET_PID()) % 230, patt);
    ASSERT_NE(participant_writer, nullptr);
    eprosima::fastrtps::rtps::RTPSParticipant* participant_reader =
            eprosima::fastrtps::rtps::RTPSDomain::createParticipant(static_cast<uint32_t>(GET_PID()) % 230, patt);
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
    eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(participant_writer);
    eprosima::fastrtps::rtps::RTPSDomain::removeRTPSParticipant(participant_reader);
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
