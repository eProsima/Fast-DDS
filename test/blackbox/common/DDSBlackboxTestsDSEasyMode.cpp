// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <gtest/gtest.h>

#include <fastdds/dds/builtin/topic/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/common/PortParameters.hpp>
#include <fastdds/rtps/common/Types.hpp>
#include <fastdds/rtps/RTPSDomain.hpp>

#include "BlackboxTests.hpp"
#include "PubSubParticipant.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"


void set_easy_discovery_mode_env(
        const std::string& ip = "127.0.0.1")
{
    /* Set environment variable which will contain the ip of an external point of discovery*/
#ifdef _WIN32
    _putenv_s("ROS2_EASY_MODE", ip.c_str());
#else
    setenv("ROS2_EASY_MODE", ip.c_str(), 1);
#endif // _WIN32

}

void stop_background_servers()
{
    // Stop server(s)
    int res = std::system("fastdds discovery stop");
    ASSERT_EQ(res, 0);
}

/**
 * Refers to FASTDDS-EASYMODE-TEST:01 from the test plan.
 *
 * Launching a participant client with the environment variable ROS2_EASY_MODE
 * correctly spawns and discovers a Discovery Server in the expected
 * domain.
 *
 */
TEST(DSEasyMode, easy_discovery_mode_env_correctly_launches)
{
#ifndef _WIN32 // The feature is not supported on Windows yet
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    // Setting ROS2_EASY_MODE
    // Configures as SUPER_CLIENT SHM and TCP
    set_easy_discovery_mode_env();

    std::atomic<bool> writer_background_ds_discovered(false);
    std::atomic<bool> reader_background_ds_discovered(false);

    writer.set_on_discovery_function(
        [&writer_background_ds_discovered](
            const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& data,
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus)
        {
            if (data.participant_name == "DiscoveryServerAuto")
            {
                writer_background_ds_discovered.store(true);
            }
            return true;
        });
    writer.init();

    reader.set_on_discovery_function(
        [&reader_background_ds_discovered](const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& data,
        eprosima::fastdds::rtps::ParticipantDiscoveryStatus)
        {
            if (data.participant_name == "DiscoveryServerAuto")
            {
                reader_background_ds_discovered.store(true);
            }
            return true;
        });
    reader.init();

    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    // Wait for endpoint discovery first
    writer.wait_discovery();
    reader.wait_discovery();

    // Two participants are expected to have been discovered:
    // Backgroud DS and the other reader/writer
    ASSERT_GE(writer.get_participants_matched(), 2u);
    ASSERT_GE(reader.get_participants_matched(), 2u);
    ASSERT_TRUE(writer_background_ds_discovered.load());
    ASSERT_TRUE(reader_background_ds_discovered.load());

    auto data = default_helloworld_data_generator();

    reader.startReception(data);
    writer.send(data);
    ASSERT_TRUE(data.empty());

    reader.block_for_all();

    // Stop server
    stop_background_servers();
#endif // _WIN32
}

/**
 * Refers to FASTDDS-EASYMODE-TEST:02 from the test plan.
 *
 * TCP and SHM are the transports used in ROS2_EASY_MODE.
 */
TEST(DSEasyMode, easy_discovery_mode_env_correct_transports_are_used)
{
#ifndef _WIN32 // The feature is not supported on Windows yet
    PubSubWriter<HelloWorldPubSubType> writer_udp(TEST_TOPIC_NAME), writer_auto(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_auto(TEST_TOPIC_NAME);

    auto udpv4_transport = std::make_shared<UDPv4TransportDescriptor>();

    writer_udp.disable_builtin_transport()
            .add_user_transport_to_pparams(udpv4_transport)
            .init();

    ASSERT_TRUE(writer_udp.isInitialized());

    // Setting ROS2_EASY_MODE configures as SUPER_CLIENT SHM and TCP
    set_easy_discovery_mode_env();

    std::atomic<bool> locators_match_p2p_transport(true);

    reader_auto.set_on_discovery_function(
        [&locators_match_p2p_transport](const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& data,
        eprosima::fastdds::rtps::ParticipantDiscoveryStatus)
        {
            for (auto locator : data.metatraffic_locators.unicast)
            {
                locators_match_p2p_transport.store( locators_match_p2p_transport &&
                (locator.kind == LOCATOR_KIND_UDPv4 || locator.kind == LOCATOR_KIND_SHM));
            }

            if (!data.metatraffic_locators.multicast.empty())
            {
                locators_match_p2p_transport.store(false);
            }

            return true;
        });
    reader_auto.init();

    ASSERT_TRUE(reader_auto.isInitialized());

    // Discovery shall not happen
    writer_udp.wait_discovery(std::chrono::seconds(1));
    reader_auto.wait_discovery(std::chrono::seconds(1));

    ASSERT_FALSE(writer_udp.is_matched());
    ASSERT_FALSE(reader_auto.is_matched());

    // Now launch another DS AUTO participant writer
    writer_auto.init();

    writer_auto.wait_discovery();
    reader_auto.wait_discovery();

    ASSERT_TRUE(locators_match_p2p_transport.load());

    // Stop server
    stop_background_servers();
#endif // _WIN32
}

/**
 * Refers to FASTDDS-EASYMODE-TEST:03 from the test plan.
 *
 * Client participants are aware of the discovery
 * information of the rest of participants in the same domain.
 */
TEST(DSEasyMode, easy_discovery_mode_env_discovery_info)
{
#ifndef _WIN32 // The feature is not supported on Windows yet
    unsigned int num_writers = 3;
    std::vector<std::shared_ptr<PubSubWriter<HelloWorldPubSubType>>> writers;
    writers.reserve(num_writers);
    PubSubReader<HelloWorldPubSubType> reader_auto(TEST_TOPIC_NAME + "_auto");

    for (std::size_t i = 0; i < num_writers; ++i)
    {
        writers.emplace_back(std::make_shared<PubSubWriter<HelloWorldPubSubType>>(TEST_TOPIC_NAME + std::to_string(i)));

        eprosima::fastdds::dds::WireProtocolConfigQos wire_protocol_qos;

        wire_protocol_qos.builtin.discovery_config.discoveryProtocol =
                eprosima::fastdds::rtps::DiscoveryProtocol::CLIENT;

        eprosima::fastdds::rtps::Locator_t locator;
        locator.kind = LOCATOR_KIND_UDPv4;

        eprosima::fastdds::rtps::PortParameters port_params;

        auto domain_port = port_params.get_discovery_server_port((uint32_t)GET_PID() % 230);

        locator.port = domain_port;
        IPLocator::setIPv4(locator, "127.0.0.1");

        // Point to the well known DS port in the corresponding domain
        wire_protocol_qos.builtin.discovery_config.m_DiscoveryServers.push_back(locator);

        writers.back()->set_wire_protocol_qos(wire_protocol_qos)
                .setup_p2p_transports()
                .init();

        ASSERT_TRUE(writers.back()->isInitialized());
    }

    // Setting ROS2_EASY_MODE
    // Configures as SUPER_CLIENT SHM and TCP
    set_easy_discovery_mode_env();

    reader_auto.init();
    ASSERT_TRUE(reader_auto.isInitialized());

    // Discovery DS in Domain 0 + num_writers
    reader_auto.wait_participant_discovery(num_writers + 1);

    // This participant shall discover all the other participants
    // Despite not sharing a common topic with them (SUPER_CLIENT)
    ASSERT_EQ(reader_auto.get_participants_matched(), num_writers + 1u);

    for (auto& writer : writers)
    {
        // Writers shall discover SERVER participant only
        ASSERT_LE(writer->get_participants_matched(), 1u);
    }

    // Stop server
    stop_background_servers();
#endif // _WIN32
}

/**
 * Refers to FASTDDS-EASYMODE-TEST:04 from the test plan.
 *
 * Launching participant clients in different domains with
 * ROS_DISCOVERY_SERVER set to AUTO correctly
 * launches and discovers the Discovery Server in its domain.
 */
TEST(DSEasyMode, easy_discovery_mode_env_multiple_clients_multiple_domains)
{
#ifndef _WIN32 // The feature is not supported on Windows yet
    unsigned int num_writer_reader_pairs = 5;

    std::vector<std::shared_ptr<PubSubWriter<HelloWorldPubSubType>>> writers;
    std::vector<std::shared_ptr<PubSubReader<HelloWorldPubSubType>>> readers;

    // Setting ROS2_EASY_MODE
    // Configures as SUPER_CLIENT SHM and TCP
    set_easy_discovery_mode_env();

    for (std::size_t i = 10; i < 10 + num_writer_reader_pairs; ++i)
    {
        writers.emplace_back(std::make_shared<PubSubWriter<HelloWorldPubSubType>>(TEST_TOPIC_NAME + "_domain_" +
                std::to_string(i)));
        readers.emplace_back(std::make_shared<PubSubReader<HelloWorldPubSubType>>(TEST_TOPIC_NAME + "_domain_" +
                std::to_string(i)));

        writers.back()->set_domain_id((uint32_t)i)
                .init();
        readers.back()->set_domain_id((uint32_t)i)
                .init();
    }

    for (std::size_t i = 0; i < num_writer_reader_pairs; ++i)
    {
        writers[i]->wait_discovery();
        readers[i]->wait_discovery();

        ASSERT_EQ(writers[i]->get_matched(), 1u);
        ASSERT_EQ(readers[i]->get_matched(), 1u);

        auto data = default_helloworld_data_generator();

        readers[i]->startReception(data);
        writers[i]->send(data);
        ASSERT_TRUE(data.empty());

        readers[i]->block_for_all();
    }

    // Stop servers
    stop_background_servers();
#endif // _WIN32
}

/**
 * Launching a second participant in the same domain with different
 * ROS2_EASY_MODE ip value shall log an error.
 */
TEST(DSEasyMode, easy_discovery_mode_env_inconsistent_ip)
{
#ifndef _WIN32 // The feature is not supported on Windows yet

    using Log = eprosima::fastdds::dds::Log;
    using LogConsumer = eprosima::fastdds::dds::LogConsumer;

    // A LogConsumer accounting for any log errors
    struct TestConsumer : public LogConsumer
    {
        TestConsumer(
                std::atomic_size_t& n_logs_ref)
            : n_logs_(n_logs_ref)
        {
        }

        void Consume(
                const Log::Entry&) override
        {
            ++n_logs_;
        }

    private:

        std::atomic_size_t& n_logs_;
    };

    // Counter for log entries
    std::atomic<size_t>n_logs{};

    // Prepare Log module to check that at least one DOMAIN error is produced
    Log::SetCategoryFilter(std::regex("DOMAIN"));
    Log::SetVerbosity(Log::Kind::Error);
    Log::RegisterConsumer(std::unique_ptr<LogConsumer>(new TestConsumer(n_logs)));

    // Set ROS2_EASY_MODE to localhost
    set_easy_discovery_mode_env();
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    // Set ROS2_EASY_MODE to another address in the same domain
    set_easy_discovery_mode_env("192.168.1.100");
    PubSubWriter<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    reader.init();
    ASSERT_TRUE(n_logs.load() > 0);

    // Stop servers
    stop_background_servers();
#endif // _WIN32
}

/**
 * Check that the environment variable ROS2_EASY_MODE is ignored if
 * it does not have a valid IPv4 format.
 */
TEST(DSEasyMode, easy_discovery_mode_env_invalid)
{
#ifndef _WIN32 // The feature is not supported on Windows yet

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    // Set ROS2_EASY_MODE to an invalid string value
    set_easy_discovery_mode_env("Foo");

    std::atomic<bool> writer_background_ds_discovered(false);
    std::atomic<bool> reader_background_ds_discovered(false);

    writer.set_on_discovery_function(
        [&writer_background_ds_discovered](
            const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& data,
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus)
        {
            if (data.participant_name == "DiscoveryServerAuto")
            {
                writer_background_ds_discovered.store(true);
            }
            return true;
        });
    writer.init();

    reader.set_on_discovery_function(
        [&reader_background_ds_discovered](const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& data,
        eprosima::fastdds::rtps::ParticipantDiscoveryStatus)
        {
            if (data.participant_name == "DiscoveryServerAuto")
            {
                reader_background_ds_discovered.store(true);
            }
            return true;
        });
    reader.init();

    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    // Wait for endpoint discovery first
    writer.wait_discovery();
    reader.wait_discovery();

    // Check that no Background DS was discovered,
    // only the other reader or writer
    ASSERT_GE(writer.get_participants_matched(), 1u);
    ASSERT_GE(reader.get_participants_matched(), 1u);
    ASSERT_FALSE(writer_background_ds_discovered.load());
    ASSERT_FALSE(reader_background_ds_discovered.load());
#endif // _WIN32
}

/**
 * Test that Easy Mode launches correctly when setting it with code
 * using WireProtocolConfigQos.
 */
TEST(DSEasyMode, easy_mode_from_qos_valid)
{
#ifndef _WIN32 // The feature is not supported on Windows yet

    // Create one participant for each endpoint
    PubSubParticipant<HelloWorldPubSubType> participant_writer(1u, 0u, 1u, 0u);
    PubSubParticipant<HelloWorldPubSubType> participant_reader(0u, 1u, 0u, 1u);

    // Setting Easy Mode manually using WireProtocolConfigQos
    eprosima::fastdds::dds::WireProtocolConfigQos wire_protocol;
    wire_protocol.easy_mode("127.0.0.1");
    participant_writer.wire_protocol(wire_protocol);
    participant_writer.pub_topic_name(TEST_TOPIC_NAME);
    participant_reader.wire_protocol(wire_protocol);
    participant_reader.sub_topic_name(TEST_TOPIC_NAME);

    std::atomic<bool> participant_writer_background_ds_discovered(false);
    std::atomic<bool> participant_reader_background_ds_discovered(false);

    participant_writer.set_on_discovery_function(
        [&participant_writer_background_ds_discovered](
            const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& data)
        {
            if (data.participant_name == "DiscoveryServerAuto")
            {
                participant_writer_background_ds_discovered.store(true);
            }
            return true;
        });

    participant_reader.set_on_discovery_function(
        [&participant_reader_background_ds_discovered](
            const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& data)
        {
            if (data.participant_name == "DiscoveryServerAuto")
            {
                participant_reader_background_ds_discovered.store(true);
            }
            return true;
        });

    ASSERT_TRUE(participant_writer.init_participant());
    ASSERT_TRUE(participant_writer.init_publisher(0));
    ASSERT_TRUE(participant_reader.init_participant());
    ASSERT_TRUE(participant_reader.init_subscriber(0));

    // Two DomainParticipants should be discovered:
    // Background DS and the other reader/writer
    participant_writer.wait_discovery(std::chrono::seconds::zero(), 2);
    participant_reader.wait_discovery(std::chrono::seconds::zero(), 2);

    ASSERT_TRUE(participant_writer_background_ds_discovered.load());
    ASSERT_TRUE(participant_reader_background_ds_discovered.load());

    participant_writer.pub_wait_discovery();
    participant_reader.sub_wait_discovery();

    size_t num_samples = 10;
    auto data = default_helloworld_data_generator(num_samples);

    for (auto& sample : data)
    {
        ASSERT_TRUE(participant_writer.send_sample(sample, 0));
    }

    participant_reader.sub_wait_data_received(num_samples);

    // Stop server
    stop_background_servers();
#endif // _WIN32
}

/**
 * Check that the configuration is ignored when set with invalid values using `WireProtocolConfigQos`.
 */
TEST(DSEasyMode, easy_mode_from_qos_invalid)
{
#ifndef _WIN32 // The feature is not supported on Windows yet

    // Create one participant for each endpoint
    PubSubParticipant<HelloWorldPubSubType> participant_writer(1u, 0u, 1u, 0u);
    PubSubParticipant<HelloWorldPubSubType> participant_reader(0u, 1u, 0u, 1u);

    // Trying to set Easy Mode manually using a non-valid IPv4 address
    eprosima::fastdds::dds::WireProtocolConfigQos wire_protocol;
    wire_protocol.easy_mode("Foo");
    participant_writer.wire_protocol(wire_protocol);
    participant_writer.pub_topic_name(TEST_TOPIC_NAME);
    participant_reader.wire_protocol(wire_protocol);
    participant_reader.sub_topic_name(TEST_TOPIC_NAME);

    std::atomic<bool> participant_writer_background_ds_discovered(false);
    std::atomic<bool> participant_reader_background_ds_discovered(false);

    participant_writer.set_on_discovery_function(
        [&participant_writer_background_ds_discovered](
            const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& data)
        {
            if (data.participant_name == "DiscoveryServerAuto")
            {
                participant_writer_background_ds_discovered.store(true);
            }
            return true;
        });

    participant_reader.set_on_discovery_function(
        [&participant_reader_background_ds_discovered](
            const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& data)
        {
            if (data.participant_name == "DiscoveryServerAuto")
            {
                participant_reader_background_ds_discovered.store(true);
            }
            return true;
        });

    ASSERT_TRUE(participant_writer.init_participant());
    ASSERT_TRUE(participant_reader.init_participant());

    // Only one DomainParticipant should be discovered:
    // Participant associated to the other reader/writer
    // No Background DS Participant should be discovered
    std::chrono::seconds timeout(5);
    // Timeout should be reached as no Background DS is expected
    participant_writer.wait_discovery(timeout, 2);
    participant_reader.wait_discovery(timeout, 2);
    ASSERT_FALSE(participant_writer_background_ds_discovered.load());
    ASSERT_FALSE(participant_reader_background_ds_discovered.load());

#endif // _WIN32
}
