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
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"


void set_ros_discovery_server_auto_env()
{
    /* Set environment variable */
#ifdef _WIN32
    _putenv_s("ROS_DISCOVERY_SERVER", "AUTO");
#else
    setenv("ROS_DISCOVERY_SERVER", "AUTO", 1);
#endif // _WIN32

}

void stop_background_servers()
{
    // Stop server(s)
    std::system("fastdds discovery stop all");
}

/**
 * Refers to FASTDDS-DSAUTO-TEST:01 from the test plan.
 *
 * Launching a participant client with the environment variable ROS_DISCOVERY_SERVER
 * set to AUTO correctly launches and discovers a Discovery Server in the expected
 * domain.
 *
 */
TEST(DSAutoMode, ros_discovery_server_auto_env_correctly_launches)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    // Setting ROS_DISCOVERY_SERVER to AUTO
    // Configures as SUPER_CLIENT SHM and TCP
    set_ros_discovery_server_auto_env();

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
}

/**
 * Refers to FASTDDS-DSAUTO-TEST:02 from the test plan.
 *
 * TCP and SHM are the transports used in ROS_DISCOVERY_SERVER=AUTO.
 */
TEST(DSAutoMode, ros_discovery_server_auto_env_correct_transports_are_used)
{
    PubSubWriter<HelloWorldPubSubType> writer_udp(TEST_TOPIC_NAME), writer_auto(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_auto(TEST_TOPIC_NAME);

    auto udpv4_transport = std::make_shared<UDPv4TransportDescriptor>();

    writer_udp.disable_builtin_transport()
            .add_user_transport_to_pparams(udpv4_transport)
            .init();

    ASSERT_TRUE(writer_udp.isInitialized());

    // Setting ROS_DISCOVERY_SERVER to AUTO
    // Configures as SUPER_CLIENT SHM and TCP
    set_ros_discovery_server_auto_env();

    std::atomic<bool> locators_match_ds_auto_transport(true);

    reader_auto.set_on_discovery_function(
        [&locators_match_ds_auto_transport](const eprosima::fastdds::rtps::ParticipantBuiltinTopicData& data,
        eprosima::fastdds::rtps::ParticipantDiscoveryStatus)
        {
            for (auto locator : data.metatraffic_locators.unicast)
            {
                locators_match_ds_auto_transport.store( locators_match_ds_auto_transport &&
                (locator.kind == LOCATOR_KIND_TCPv4 || locator.kind == LOCATOR_KIND_SHM));
            }

            if (!data.metatraffic_locators.multicast.empty())
            {
                locators_match_ds_auto_transport.store(false);
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

    ASSERT_TRUE(locators_match_ds_auto_transport.load());

    // Stop server
    stop_background_servers();
}

/**
 * Refers to FASTDDS-DSAUTO-TEST:03 from the test plan.
 *
 * Client participants are aware of the discovery
 * information of the rest of participants in the same domain.
 */
TEST(DSAutoMode, ros_discovery_server_auto_env_discovery_info)
{
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
        locator.kind = LOCATOR_KIND_TCPv4;

        eprosima::fastdds::rtps::PortParameters port_params;

        auto ds_auto_port = port_params.getDiscoveryServerPort((uint32_t)GET_PID() % 230);

        IPLocator::setPhysicalPort(locator, ds_auto_port);
        IPLocator::setLogicalPort(locator, ds_auto_port);
        IPLocator::setIPv4(locator, "127.0.0.1");

        // Point to the well known DS port in the corresponding domain
        wire_protocol_qos.builtin.discovery_config.m_DiscoveryServers.push_back(locator);

        writers.back()->set_wire_protocol_qos(wire_protocol_qos)
                .disable_builtin_transport()
                .setup_large_data_tcp()
                .init();

        ASSERT_TRUE(writers.back()->isInitialized());
    }

    // Setting ROS_DISCOVERY_SERVER to AUTO
    // Configures as SUPER_CLIENT SHM and TCP
    set_ros_discovery_server_auto_env();

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
}

/**
 * Refers to FASTDDS-DSAUTO-TEST:04 from the test plan.
 *
 * Launching participant clients in different domains with
 * ROS_DISCOVERY_SERVER set to AUTO correctly
 * launch and discover the Discovery Server in its domain.
 */
TEST(DSAutoMode, ros_discovery_server_auto_env_multiple_clients_multiple_domains)
{
    unsigned int num_writer_reader_pairs = 5;

    std::vector<std::shared_ptr<PubSubWriter<HelloWorldPubSubType>>> writers;
    std::vector<std::shared_ptr<PubSubReader<HelloWorldPubSubType>>> readers;

    // Setting ROS_DISCOVERY_SERVER to AUTO
    // Configures as SUPER_CLIENT SHM and TCP
    set_ros_discovery_server_auto_env();

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

        auto data = default_helloworld_data_generator();

        readers[i]->startReception(data);
        writers[i]->send(data);
        ASSERT_TRUE(data.empty());

        readers[i]->block_for_all();
    }

    // Stop servers
    stop_background_servers();
}
