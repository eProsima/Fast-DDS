// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <stdlib.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <gtest/gtest.h>

#include "../types/statistics/types.hpp"
#include "../types/statistics/typesPubSubTypes.hpp"
#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"

class WriterReaderDataTest : public eprosima::fastdds::statistics::WriterReaderData
{
public:

    bool operator ==(
            const WriterReaderDataTest& x) const
    {
        if (this->writer_guid().guidPrefix().value() == x.writer_guid().guidPrefix().value() &&
                this->writer_guid().entityId().value() == x.writer_guid().entityId().value() &&
                this->reader_guid().guidPrefix().value() == x.reader_guid().guidPrefix().value() &&
                this->reader_guid().entityId().value() == x.reader_guid().entityId().value() &&
                this->data() == x.data())
        {
            return true;
        }
        return false;
    }

};

class Locator2LocatorDataTest : public eprosima::fastdds::statistics::Locator2LocatorData
{
public:

    bool operator ==(
            const Locator2LocatorDataTest& x) const
    {
        if (this->src_locator().kind() == x.src_locator().kind() &&
                this->src_locator().port() == x.src_locator().port() &&
                this->src_locator().address() == x.src_locator().address() &&
                this->dst_locator().kind() == x.dst_locator().kind() &&
                this->dst_locator().port() == x.dst_locator().port() &&
                this->dst_locator().address() == x.dst_locator().address() &&
                this->data() == x.data())
        {
            return true;
        }
        return false;
    }

};

class Entity2LocatorTrafficTest : public eprosima::fastdds::statistics::Entity2LocatorTraffic
{
public:

    bool operator ==(
            const Entity2LocatorTrafficTest& x) const
    {
        if (this->src_guid().guidPrefix().value() == x.src_guid().guidPrefix().value() &&
                this->src_guid().entityId().value() == x.src_guid().entityId().value() &&
                this->dst_locator().kind() == x.dst_locator().kind() &&
                this->dst_locator().port() == x.dst_locator().port() &&
                this->dst_locator().address() == x.dst_locator().address() &&
                this->packet_count() == x.packet_count() &&
                this->byte_count() == x.byte_count() &&
                this->byte_magnitude_order() == x.byte_magnitude_order())
        {
            return true;
        }
        return false;
    }

};

class DiscoveryTimeTest : public eprosima::fastdds::statistics::DiscoveryTime
{
public:

    bool operator ==(
            const DiscoveryTimeTest& x) const
    {
        if (this->local_participant_guid().guidPrefix().value() == x.local_participant_guid().guidPrefix().value() &&
                this->local_participant_guid().entityId().value() == x.local_participant_guid().entityId().value() &&
                this->remote_entity_guid().guidPrefix().value() == x.remote_entity_guid().guidPrefix().value() &&
                this->remote_entity_guid().entityId().value() == x.remote_entity_guid().entityId().value() &&
                this->time() == x.time())
        {
            return true;
        }
        return false;
    }

};

class PhysicalDataTest : public eprosima::fastdds::statistics::PhysicalData
{
public:

    bool operator ==(
            const PhysicalDataTest& x) const
    {
        if (this->participant_guid().guidPrefix().value() == x.participant_guid().guidPrefix().value() &&
                this->participant_guid().entityId().value() == x.participant_guid().entityId().value() &&
                this->host() == x.host() &&
                this->user() == x.user() &&
                this->process() == x.process())
        {
            return true;
        }
        return false;
    }

};

class EntityDataTest : public eprosima::fastdds::statistics::EntityData
{
public:

    bool operator ==(
            const EntityDataTest& x) const
    {
        if (this->guid().guidPrefix().value() == x.guid().guidPrefix().value() &&
                this->guid().entityId().value() == x.guid().entityId().value() &&
                this->data() == x.data())
        {
            return true;
        }
        return false;
    }

};

class EntityCountTest : public eprosima::fastdds::statistics::EntityCount
{
public:

    bool operator ==(
            const EntityCountTest& x) const
    {
        if (this->guid().guidPrefix().value() == x.guid().guidPrefix().value() &&
                this->guid().entityId().value() == x.guid().entityId().value() &&
                this->count() == x.count())
        {
            return true;
        }
        return false;
    }

};

class SampleIdentityCountTest : public eprosima::fastdds::statistics::SampleIdentityCount
{
public:

    bool operator ==(
            const SampleIdentityCountTest& x) const
    {
        if (this->sample_id().writer_guid().guidPrefix().value() == x.sample_id().writer_guid().guidPrefix().value() &&
                this->sample_id().writer_guid().entityId().value() == x.sample_id().writer_guid().entityId().value() &&
                this->sample_id().sequence_number().high() == x.sample_id().sequence_number().high() &&
                this->sample_id().sequence_number().low() == x.sample_id().sequence_number().low() &&
                this->count() == x.count())
        {
            return true;
        }
        return false;
    }

};

class WriterReaderDataPubSubTypeTest : public eprosima::fastdds::statistics::WriterReaderDataPubSubType
{
public:

    typedef WriterReaderDataTest type;
};

class Locator2LocatorDataPubSubTypeTest : public eprosima::fastdds::statistics::Locator2LocatorDataPubSubType
{
public:

    typedef Locator2LocatorDataTest type;
};

class Entity2LocatorTrafficPubSubTypeTest : public eprosima::fastdds::statistics::Entity2LocatorTrafficPubSubType
{
public:

    typedef Entity2LocatorTrafficTest type;
};

class DiscoveryTimePubSubTypeTest : public eprosima::fastdds::statistics::DiscoveryTimePubSubType
{
public:

    typedef DiscoveryTimeTest type;
};

class PhysicalDataPubSubTypeTest : public eprosima::fastdds::statistics::PhysicalDataPubSubType
{
public:

    typedef PhysicalDataTest type;
};

class EntityDataPubSubTypeTest : public eprosima::fastdds::statistics::EntityDataPubSubType
{
public:

    typedef EntityDataTest type;
};

class EntityCountPubSubTypeTest : public eprosima::fastdds::statistics::EntityCountPubSubType
{
public:

    typedef EntityCountTest type;
};

class SampleIdentityCountPubSubTypeTest : public eprosima::fastdds::statistics::SampleIdentityCountPubSubType
{
public:

    typedef SampleIdentityCountTest type;
};

/*
 * This test checks that when the environment variable is correctly set, the proper statistics DataWriters are created
 * and enabled.
 * This test is only valid when FASTDDS_STATISTICS CMake option is set. Otherwise the test is empty.
 * 0. Setup test: create DDS entities needed for the test: TypeSupports, DataReaders...
 * 1. Set environment variable and create participant using a Qos set by code.
 * 2. Check that topics, types and datawriters have been created correctly.
 * 3. Check that those topics and types that have not been enabled does not exist.
 */
TEST(StatisticsDomainParticipant, CreateParticipant)
{
#ifdef FASTDDS_STATISTICS
    // 0. Setup test
    // Create TypeSupports
    eprosima::fastdds::dds::TypeSupport history_latency_type(
        new eprosima::fastdds::statistics::WriterReaderDataPubSubType);
    history_latency_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport network_latency_type(
        new eprosima::fastdds::statistics::Locator2LocatorDataPubSubType);
    network_latency_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport throughput_type(
        new eprosima::fastdds::statistics::EntityDataPubSubType);
    throughput_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport rtps_traffic_type(
        new eprosima::fastdds::statistics::Entity2LocatorTrafficPubSubType);
    rtps_traffic_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport count_type(
        new eprosima::fastdds::statistics::EntityCountPubSubType);
    count_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport discovery_type(
        new eprosima::fastdds::statistics::DiscoveryTimePubSubType);
    discovery_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport sample_identity_count_type(
        new eprosima::fastdds::statistics::SampleIdentityCountPubSubType);
    sample_identity_count_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport physical_data_type(
        new eprosima::fastdds::statistics::PhysicalDataPubSubType);
    physical_data_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport null_type(nullptr);

    // Create statistics DataReaders
    PubSubReader<WriterReaderDataPubSubTypeTest> history_latency_reader(
        eprosima::fastdds::statistics::HISTORY_LATENCY_TOPIC, true, true);
    PubSubReader<Locator2LocatorDataPubSubTypeTest> network_latency_reader(
        eprosima::fastdds::statistics::NETWORK_LATENCY_TOPIC, true, true);
    PubSubReader<Entity2LocatorTrafficPubSubTypeTest> rtps_lost_reader(
        eprosima::fastdds::statistics::RTPS_LOST_TOPIC, true, true);
    PubSubReader<Entity2LocatorTrafficPubSubTypeTest> rtps_sent_reader(
        eprosima::fastdds::statistics::RTPS_SENT_TOPIC, true, true);
    PubSubReader<EntityCountPubSubTypeTest> gap_count_reader(
        eprosima::fastdds::statistics::GAP_COUNT_TOPIC, true, true);
    PubSubReader<EntityCountPubSubTypeTest> pdp_reader(
        eprosima::fastdds::statistics::PDP_PACKETS_TOPIC, true, true);
    PubSubReader<EntityCountPubSubTypeTest> edp_reader(
        eprosima::fastdds::statistics::EDP_PACKETS_TOPIC, true, true);
    PubSubReader<DiscoveryTimePubSubTypeTest> discovery_reader(
        eprosima::fastdds::statistics::DISCOVERY_TOPIC, true, true);
    PubSubReader<PhysicalDataPubSubTypeTest> physical_data_reader(
        eprosima::fastdds::statistics::PHYSICAL_DATA_TOPIC, true, true);

    // 1. Set environment variable and create participant using Qos set by code
    const char* value = "HISTORY_LATENCY_TOPIC;NETWORK_LATENCY_TOPIC;RTPS_LOST_TOPIC;RTPS_SENT_TOPIC;GAP_COUNT_TOPIC";
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s("FASTDDS_STATISTICS", value));
#else
    ASSERT_EQ(0, setenv("FASTDDS_STATISTICS", value, 1));
#endif // ifdef _WIN32

    // There is no problem if some topic name is repeated.
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.properties().properties().emplace_back("fastdds.statistics",
            "HISTORY_LATENCY_TOPIC;DISCOVERY_TOPIC;PHYSICAL_DATA_TOPIC;PDP_PACKETS_TOPIC;EDP_PACKETS_TOPIC");

    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant((uint32_t)GET_PID() % 230, pqos);
    ASSERT_NE(participant, nullptr);

    // 2. Check that the statistics topics and types related to the statistics DataWriters enabled using the
    // environment variable exist.
    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::HISTORY_LATENCY_TOPIC));
    EXPECT_TRUE(history_latency_type == participant->find_type(history_latency_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::NETWORK_LATENCY_TOPIC));
    EXPECT_TRUE(network_latency_type == participant->find_type(network_latency_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::RTPS_LOST_TOPIC));
    EXPECT_TRUE(rtps_traffic_type == participant->find_type(rtps_traffic_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::RTPS_SENT_TOPIC));
    EXPECT_TRUE(rtps_traffic_type == participant->find_type(rtps_traffic_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::GAP_COUNT_TOPIC));
    EXPECT_TRUE(count_type == participant->find_type(count_type.get_type_name()));

    // Check that the statistics topics and types related to the statistics DataWriters set with the PropertyPolicyQos
    // have been created correctly.
    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::DISCOVERY_TOPIC));
    EXPECT_TRUE(discovery_type == participant->find_type(discovery_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::PHYSICAL_DATA_TOPIC));
    EXPECT_TRUE(physical_data_type == participant->find_type(physical_data_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::PDP_PACKETS_TOPIC));
    EXPECT_TRUE(count_type == participant->find_type(count_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::EDP_PACKETS_TOPIC));
    EXPECT_TRUE(count_type == participant->find_type(count_type.get_type_name()));

    // Before initializing the DataReaders the environment variable should be unset.
    // Otherwise each domainParticipant (each DataReader is launched in its own domainParticipant) will also enable
    // the statistics DataWriters set with the environment variable.
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s("FASTDDS_STATISTICS", ""));
#else
    ASSERT_EQ(0, unsetenv("FASTDDS_STATISTICS"));
#endif // ifdef _WIN32

    // Check that the statistics DataWriters has been created by matching a corresponding DataReader on those topics
    history_latency_reader.init();
    network_latency_reader.init();
    rtps_lost_reader.init();
    rtps_sent_reader.init();
    gap_count_reader.init();
    pdp_reader.init();
    edp_reader.init();
    discovery_reader.init();
    physical_data_reader.init();

    ASSERT_TRUE(history_latency_reader.isInitialized());
    ASSERT_TRUE(network_latency_reader.isInitialized());
    ASSERT_TRUE(rtps_lost_reader.isInitialized());
    ASSERT_TRUE(rtps_sent_reader.isInitialized());
    ASSERT_TRUE(gap_count_reader.isInitialized());
    ASSERT_TRUE(pdp_reader.isInitialized());
    ASSERT_TRUE(edp_reader.isInitialized());
    ASSERT_TRUE(discovery_reader.isInitialized());
    ASSERT_TRUE(physical_data_reader.isInitialized());

    // Wait for discovery. If there is discovery the test will pass. Otherwise, the test will timeout.
    history_latency_reader.wait_discovery();
    network_latency_reader.wait_discovery();
    rtps_lost_reader.wait_discovery();
    rtps_sent_reader.wait_discovery();
    gap_count_reader.wait_discovery();
    pdp_reader.wait_discovery();
    edp_reader.wait_discovery();
    discovery_reader.wait_discovery();
    physical_data_reader.wait_discovery();

    // 3. Check that those topics and types related to the statistics DataWriters not enabled does not exist.
    EXPECT_EQ(null_type, participant->find_type(throughput_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(sample_identity_count_type.get_type_name()));

    EXPECT_EQ(nullptr, participant->lookup_topicdescription(
                eprosima::fastdds::statistics::PUBLICATION_THROUGHPUT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(
                eprosima::fastdds::statistics::SUBSCRIPTION_THROUGHPUT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::RESENT_DATAS_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::HEARTBEAT_COUNT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::ACKNACK_COUNT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::NACKFRAG_COUNT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::DATA_COUNT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::SAMPLE_DATAS_TOPIC));

    // 4. Remove DDS entities
    history_latency_reader.destroy();
    network_latency_reader.destroy();
    rtps_lost_reader.destroy();
    rtps_sent_reader.destroy();
    gap_count_reader.destroy();
    pdp_reader.destroy();
    edp_reader.destroy();
    discovery_reader.destroy();
    physical_data_reader.destroy();

    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant),
            eprosima::fastdds::dds::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}

/*
 * This test is similar to the previous test. In this case the Qos are read from an XML file instead of setting them
 * directly in the code.
 * This test is only valid when FASTDDS_STATISTICS CMake option is set. Otherwise the test is empty.
 * 0. Setup test: create DDS entities needed for the test: TypeSupports, DataReaders...
 * 1. Set environment variable and create participant using a Qos set by XML.
 * 2. Check that topics, types and datawriters have been created correctly.
 * 3. Check that those topics and types that have not been enabled does not exist.
 */
TEST(StatisticsDomainParticipant, CreateParticipantUsingXML)
{
#ifdef FASTDDS_STATISTICS
    // 0. Setup test
    std::string xml_file = "StatisticsDomainParticipant_profile.xml";
    std::string participant_profile_name = "statistics_participant";

    // Create TypeSupports
    eprosima::fastdds::dds::TypeSupport history_latency_type(
        new eprosima::fastdds::statistics::WriterReaderDataPubSubType);
    history_latency_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport network_latency_type(
        new eprosima::fastdds::statistics::Locator2LocatorDataPubSubType);
    network_latency_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport throughput_type(
        new eprosima::fastdds::statistics::EntityDataPubSubType);
    throughput_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport rtps_traffic_type(
        new eprosima::fastdds::statistics::Entity2LocatorTrafficPubSubType);
    rtps_traffic_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport count_type(
        new eprosima::fastdds::statistics::EntityCountPubSubType);
    count_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport discovery_type(
        new eprosima::fastdds::statistics::DiscoveryTimePubSubType);
    discovery_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport sample_identity_count_type(
        new eprosima::fastdds::statistics::SampleIdentityCountPubSubType);
    sample_identity_count_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport physical_data_type(
        new eprosima::fastdds::statistics::PhysicalDataPubSubType);
    physical_data_type->register_type_object_representation();
    eprosima::fastdds::dds::TypeSupport null_type(nullptr);

    // Create statistics DataReaders
    PubSubReader<EntityDataPubSubTypeTest> publication_throughput_reader(
        eprosima::fastdds::statistics::PUBLICATION_THROUGHPUT_TOPIC, true, true);
    PubSubReader<EntityDataPubSubTypeTest> subscription_throughput_reader(
        eprosima::fastdds::statistics::SUBSCRIPTION_THROUGHPUT_TOPIC, true, true);
    PubSubReader<EntityCountPubSubTypeTest> resent_reader(
        eprosima::fastdds::statistics::RESENT_DATAS_TOPIC, true, true);
    PubSubReader<EntityCountPubSubTypeTest> heartbeat_count_reader(
        eprosima::fastdds::statistics::HEARTBEAT_COUNT_TOPIC, true, true);
    PubSubReader<EntityCountPubSubTypeTest> acknack_reader(
        eprosima::fastdds::statistics::ACKNACK_COUNT_TOPIC, true, true);
    PubSubReader<EntityCountPubSubTypeTest> data_count_reader(
        eprosima::fastdds::statistics::DATA_COUNT_TOPIC, true, true);
    PubSubReader<EntityCountPubSubTypeTest> nackfrag_count_reader(
        eprosima::fastdds::statistics::NACKFRAG_COUNT_TOPIC, true, true);
    PubSubReader<SampleIdentityCountPubSubTypeTest> sample_datas_reader(
        eprosima::fastdds::statistics::SAMPLE_DATAS_TOPIC, true, true);

    // 1. Set environment variable and create participant using Qos set by code
    const char* value = "PUBLICATION_THROUGHPUT_TOPIC;HEARTBEAT_COUNT_TOPIC;RESENT_DATAS_TOPIC;ACKNACK_COUNT_TOPIC";
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s("FASTDDS_STATISTICS", value));
#else
    ASSERT_EQ(0, setenv("FASTDDS_STATISTICS", value, 1));
#endif // ifdef _WIN32

    // Load XML profiles
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_file(xml_file);

    eprosima::fastdds::dds::DomainParticipant* participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->
                    create_participant_with_profile((uint32_t)GET_PID() % 230, participant_profile_name);
    ASSERT_NE(participant, nullptr);

    // 2. Check that the statistics topics and types related to the statistics DataWriters enabled using the
    // environment variable exist.
    EXPECT_NE(nullptr, participant->lookup_topicdescription(
                eprosima::fastdds::statistics::PUBLICATION_THROUGHPUT_TOPIC));
    EXPECT_TRUE(throughput_type == participant->find_type(throughput_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::HEARTBEAT_COUNT_TOPIC));
    EXPECT_TRUE(count_type == participant->find_type(count_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::RESENT_DATAS_TOPIC));
    EXPECT_TRUE(count_type == participant->find_type(count_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::ACKNACK_COUNT_TOPIC));
    EXPECT_TRUE(count_type == participant->find_type(count_type.get_type_name()));

    // Check that the statistics topics and types related to the statistics DataWriters set with the PropertyPolicyQos
    // have been created correctly.
    EXPECT_NE(nullptr, participant->lookup_topicdescription(
                eprosima::fastdds::statistics::SUBSCRIPTION_THROUGHPUT_TOPIC));
    EXPECT_TRUE(throughput_type == participant->find_type(throughput_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::NACKFRAG_COUNT_TOPIC));
    EXPECT_TRUE(count_type == participant->find_type(count_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::SAMPLE_DATAS_TOPIC));
    EXPECT_TRUE(sample_identity_count_type == participant->find_type(sample_identity_count_type.get_type_name()));

    EXPECT_NE(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::DATA_COUNT_TOPIC));
    EXPECT_TRUE(count_type == participant->find_type(count_type.get_type_name()));

    // Before initializing the DataReaders the environment variable should be unset.
    // Otherwise each domainParticipant (each DataReader is launched in its own domainParticipant) will also enable
    // the statistics DataWriters set with the environment variable.
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s("FASTDDS_STATISTICS", ""));
#else
    ASSERT_EQ(0, unsetenv("FASTDDS_STATISTICS"));
#endif // ifdef _WIN32

    // Check that the statistics DataWriters has been created by matching a corresponding DataReader on those topics
    publication_throughput_reader.init();
    subscription_throughput_reader.init();
    resent_reader.init();
    heartbeat_count_reader.init();
    acknack_reader.init();
    data_count_reader.init();
    nackfrag_count_reader.init();
    sample_datas_reader.init();

    ASSERT_TRUE(publication_throughput_reader.isInitialized());
    ASSERT_TRUE(subscription_throughput_reader.isInitialized());
    ASSERT_TRUE(resent_reader.isInitialized());
    ASSERT_TRUE(heartbeat_count_reader.isInitialized());
    ASSERT_TRUE(acknack_reader.isInitialized());
    ASSERT_TRUE(data_count_reader.isInitialized());
    ASSERT_TRUE(nackfrag_count_reader.isInitialized());
    ASSERT_TRUE(sample_datas_reader.isInitialized());

    // Wait for discovery. If there is discovery the test will pass. Otherwise, the test will timeout.
    publication_throughput_reader.wait_discovery();
    subscription_throughput_reader.wait_discovery();
    resent_reader.wait_discovery();
    heartbeat_count_reader.wait_discovery();
    acknack_reader.wait_discovery();
    data_count_reader.wait_discovery();
    nackfrag_count_reader.wait_discovery();
    sample_datas_reader.wait_discovery();

    // 3. Check that those topics and types related to the statistics DataWriters not enabled does not exist.
    EXPECT_EQ(null_type, participant->find_type(history_latency_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(network_latency_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(rtps_traffic_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(discovery_type.get_type_name()));
    EXPECT_EQ(null_type, participant->find_type(physical_data_type.get_type_name()));

    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::HISTORY_LATENCY_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::NETWORK_LATENCY_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::RTPS_SENT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::RTPS_LOST_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::GAP_COUNT_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::PDP_PACKETS_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::EDP_PACKETS_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::DISCOVERY_TOPIC));
    EXPECT_EQ(nullptr, participant->lookup_topicdescription(eprosima::fastdds::statistics::PHYSICAL_DATA_TOPIC));

    // 4. Remove DDS entities
    publication_throughput_reader.destroy();
    subscription_throughput_reader.destroy();
    resent_reader.destroy();
    heartbeat_count_reader.destroy();
    acknack_reader.destroy();
    data_count_reader.destroy();
    nackfrag_count_reader.destroy();
    sample_datas_reader.destroy();

    EXPECT_EQ(eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant),
            eprosima::fastdds::dds::RETCODE_OK);
#endif // FASTDDS_STATISTICS
}
