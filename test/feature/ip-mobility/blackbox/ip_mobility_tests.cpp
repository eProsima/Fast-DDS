/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <chrono>
#include <memory>

#include <gtest/gtest.h>

#include <fastdds/LibrarySettings.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>

#include <BlackboxTests.hpp>
#include <HelloWorldPubSubTypes.hpp>
#include <PubSubReader.hpp>
#include <PubSubWriter.hpp>
#include <RTPSWithRegistrationReader.hpp>
#include <RTPSWithRegistrationWriter.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * This test checks the addition of network interfaces at run-time.
 *
 * After launching the reader with the network interfaces enabled,
 * the writer is launched with the transport simulating that there
 * are no interfaces.
 * No participant discovery occurs, nor is communication established.
 *
 * In a second step, the flag to simulate no interfaces is disabled and
 * DomainParticipant::set_qos() called to add the "new" interfaces.
 * Discovery is succesful and communication is established.
 */
TEST(IPMobilityTests, DDSNetworkInterfaceChangesAtRunTime)
{
    using namespace eprosima::fastdds::rtps;

    PubSubWriter<HelloWorldPubSubType> datawriter(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> datareader(TEST_TOPIC_NAME);

    // datareader is initialized with all the network interfaces
    datareader.durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS).history_depth(100).reliability(
        RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(datareader.isInitialized());

    // datawriter: launch without interfaces
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    test_transport->test_transport_options->simulate_no_interfaces = true;
    datawriter.disable_builtin_transport().add_user_transport_to_pparams(test_transport).history_depth(100).init();
    ASSERT_TRUE(datawriter.isInitialized());

    // no discovery
    datawriter.wait_discovery(std::chrono::seconds(3));
    datareader.wait_discovery(std::chrono::seconds(3));
    EXPECT_EQ(datawriter.get_matched(), 0u);
    EXPECT_EQ(datareader.get_matched(), 0u);

    // send data
    auto complete_data = default_helloworld_data_generator();
    size_t samples = complete_data.size();

    datareader.startReception(complete_data);

    datawriter.send(complete_data);
    EXPECT_TRUE(complete_data.empty());

    // no data received
    EXPECT_EQ(datareader.block_for_all(std::chrono::seconds(3)), 0u);

    // enable interfaces
    test_transport->test_transport_options->simulate_no_interfaces = false;
    datawriter.participant_set_qos();

    // Wait for discovery
    datawriter.wait_discovery(std::chrono::seconds(3));
    datareader.wait_discovery(std::chrono::seconds(3));
    ASSERT_EQ(datawriter.get_matched(), 1u);
    ASSERT_EQ(datareader.get_matched(), 1u);

    // data received
    EXPECT_EQ(datareader.block_for_all(std::chrono::seconds(3)), samples);

    datareader.destroy();
    datawriter.destroy();
}

} // namespace dds

namespace rtps {

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
TEST(IPMobilityTests, RTPSNetworkInterfaceChangesAtRunTime)
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

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

bool enable_datasharing;
bool use_pull_mode;

class BlackboxEnvironment : public ::testing::Environment
{
public:

    void SetUp()
    {
        // Blackbox tests were designed with the assumption that intraprocess
        // and datasharing are both disabled. Most of them use TEST_P in order to
        // test with and without intraprocess and datasharing, but those who test
        // conditions related to network packets being lost should not use intraprocess
        // nor datasharing. Setting it off here ensures that intraprocess and
        // datasharing are only tested when required.
        eprosima::fastdds::LibrarySettings att;
        att.intraprocess_delivery = eprosima::fastdds::INTRAPROCESS_OFF;
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(att);
        enable_datasharing = false;
        use_pull_mode = false;
    }

    void TearDown()
    {
        eprosima::fastdds::dds::Log::KillThread();
    }

};

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new BlackboxEnvironment);

    return RUN_ALL_TESTS();
}
