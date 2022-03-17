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
#include <thread>

#include <gtest/gtest.h>

#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/transport/test_UDPv4TransportDescriptor.h>
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
    auto testTransport = std::make_shared<rtps::test_UDPv4TransportDescriptor>();
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
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
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
