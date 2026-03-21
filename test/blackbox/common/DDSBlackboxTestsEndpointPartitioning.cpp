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

#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastdds;

/**
 * This test checks that DataWriter/DataReaders match with each other one to one when the partition
 * configuration set on their profile matches. It also tests that this configuration is compatible with
 * Publisher/Subscriber partition configuration.
 */
TEST(EndpointPartitioning, SinglePartition)
{
    PubSubWriter<HelloWorldPubSubType> writer_a(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_a(TEST_TOPIC_NAME);

    // Positive test. Same partition. Match

    writer_a.set_xml_filename("partitions_profile.xml");
    writer_a.set_datawriter_profile("partition_a_writer");
    writer_a.init();
    EXPECT_TRUE(writer_a.isInitialized());

    reader_a.set_xml_filename("partitions_profile.xml");
    reader_a.set_datareader_profile("partition_a_reader");
    reader_a.init();
    EXPECT_TRUE(reader_a.isInitialized());

    writer_a.wait_discovery(std::chrono::seconds(3));
    reader_a.wait_discovery(std::chrono::seconds(3));

    ASSERT_TRUE(writer_a.is_matched());
    ASSERT_TRUE(reader_a.is_matched());

    // Negative test. Partition differs. No match.

    PubSubReader<HelloWorldPubSubType> reader_b(TEST_TOPIC_NAME);
    reader_b.set_xml_filename("partitions_profile.xml");
    reader_b.set_datareader_profile("partition_b_reader");
    reader_b.init();
    EXPECT_TRUE(reader_b.isInitialized());

    reader_b.wait_discovery(std::chrono::seconds(3));
    ASSERT_FALSE(reader_b.is_matched());

    // Partition interaction between DataWriter and Subscriber Qos. Match

    PubSubReader<HelloWorldPubSubType> reader_a_from_subscriber(TEST_TOPIC_NAME);
    reader_a_from_subscriber.partition("partition_a");
    reader_a_from_subscriber.init();

    EXPECT_TRUE(reader_a_from_subscriber.isInitialized());

    reader_a_from_subscriber.wait_discovery(std::chrono::seconds(3));
    ASSERT_TRUE(reader_a_from_subscriber.is_matched());

    // Partition interaction between Publisher and DataReader Qos. Match

    PubSubWriter<HelloWorldPubSubType> writer_b_from_publisher(TEST_TOPIC_NAME);
    writer_b_from_publisher.partition("partition_b");
    writer_b_from_publisher.init();

    reader_b.wait_discovery(std::chrono::seconds(3));
    writer_b_from_publisher.wait_discovery(std::chrono::seconds(3));

    ASSERT_TRUE(writer_b_from_publisher.is_matched());
    ASSERT_TRUE(reader_b.is_matched());

    writer_a.destroy();
    writer_b_from_publisher.destroy();

    reader_a.destroy();
    reader_b.destroy();
    reader_a_from_subscriber.destroy();
}

/**
 * This test checks that DataWriter/DataReader partition configuration takes precedence over the same configuration
 * done via the Publisher/Subscriber corresponding Qos parameter.
 */
TEST(EndpointPartitioning, QosOverride)
{
    // Partition configuration overriding. Endpoint configuration must take precedence.
    PubSubWriter<HelloWorldPubSubType> writer_a_qos_override(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_b_qos_override(TEST_TOPIC_NAME);

    writer_a_qos_override.set_xml_filename("partitions_profile.xml");
    writer_a_qos_override.set_datawriter_profile("partition_a_writer");

    // We change the PublisherQos so the partition matches. DataWriter partition stays the same (partition_a)
    writer_a_qos_override.partition("partition_b");
    writer_a_qos_override.init();
    EXPECT_TRUE(writer_a_qos_override.isInitialized());

    reader_b_qos_override.set_xml_filename("partitions_profile.xml");
    reader_b_qos_override.set_datareader_profile("partition_b_reader");
    reader_b_qos_override.init();
    EXPECT_TRUE(reader_b_qos_override.isInitialized());

    writer_a_qos_override.wait_discovery(std::chrono::seconds(3));
    reader_b_qos_override.wait_discovery(std::chrono::seconds(3));

    ASSERT_FALSE(writer_a_qos_override.is_matched());
    ASSERT_FALSE(reader_b_qos_override.is_matched());

    writer_a_qos_override.destroy();
    reader_b_qos_override.destroy();

    // Same test but trying to override DataReader configuration
    PubSubWriter<HelloWorldPubSubType> writer_b_qos_override(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_a_qos_override(TEST_TOPIC_NAME);

    writer_b_qos_override.set_xml_filename("partitions_profile.xml");
    writer_b_qos_override.set_datawriter_profile("partition_b_writer");
    writer_b_qos_override.partition("partition_a");
    writer_b_qos_override.init();
    EXPECT_TRUE(writer_b_qos_override.isInitialized());

    reader_a_qos_override.set_xml_filename("partitions_profile.xml");
    reader_a_qos_override.set_datareader_profile("partition_a_reader");
    reader_a_qos_override.init();
    EXPECT_TRUE(reader_a_qos_override.isInitialized());

    writer_b_qos_override.wait_discovery(std::chrono::seconds(3));
    reader_a_qos_override.wait_discovery(std::chrono::seconds(3));

    ASSERT_FALSE(writer_b_qos_override.is_matched());
    ASSERT_FALSE(reader_a_qos_override.is_matched());

    writer_b_qos_override.destroy();
    reader_a_qos_override.destroy();
}

/**
 * This test checks that multiple partitions can be defined properly on both DataReader and DataWriters
 * and that it matches with other endpoints with the same partitions.
 */
TEST(EndpointPartitioning, MultiplePartitions)
{
    // Multiple partition test
    PubSubReader<HelloWorldPubSubType> reader_a(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_b(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer_a_b(TEST_TOPIC_NAME);

    writer_a_b.set_xml_filename("partitions_profile.xml");
    writer_a_b.set_datawriter_profile("partition_a_b_writer");
    writer_a_b.init();
    EXPECT_TRUE(writer_a_b.isInitialized());

    reader_a.set_xml_filename("partitions_profile.xml");
    reader_a.set_datareader_profile("partition_a_reader");
    reader_a.init();
    EXPECT_TRUE(reader_a.isInitialized());

    reader_b.set_xml_filename("partitions_profile.xml");
    reader_b.set_datareader_profile("partition_b_reader");
    reader_b.init();
    EXPECT_TRUE(reader_b.isInitialized());

    writer_a_b.wait_discovery(std::chrono::seconds(3));
    reader_a.wait_discovery(std::chrono::seconds(3));
    reader_b.wait_discovery(std::chrono::seconds(3));

    ASSERT_TRUE(writer_a_b.is_matched());
    ASSERT_TRUE(reader_a.is_matched());
    ASSERT_TRUE(reader_b.is_matched());

    writer_a_b.destroy();
    reader_a.destroy();
    reader_b.destroy();

    PubSubWriter<HelloWorldPubSubType> writer_a(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer_b(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_a_b(TEST_TOPIC_NAME);

    writer_a.set_xml_filename("partitions_profile.xml");
    writer_a.set_datawriter_profile("partition_a_writer");
    writer_a.init();
    EXPECT_TRUE(writer_a.isInitialized());

    writer_b.set_xml_filename("partitions_profile.xml");
    writer_b.set_datawriter_profile("partition_b_writer");
    writer_b.init();
    EXPECT_TRUE(writer_b.isInitialized());

    reader_a_b.set_xml_filename("partitions_profile.xml");
    reader_a_b.set_datareader_profile("partition_a_b_reader");
    reader_a_b.init();
    EXPECT_TRUE(reader_a_b.isInitialized());

    writer_a.wait_discovery(std::chrono::seconds(3));
    writer_b.wait_discovery(std::chrono::seconds(3));
    reader_a_b.wait_discovery(std::chrono::seconds(3));

    ASSERT_TRUE(writer_a.is_matched());
    ASSERT_TRUE(writer_b.is_matched());
    ASSERT_TRUE(reader_a_b.is_matched());
}

/**
 * This test checks that partition configuration can be modified via the PropertyPolicyQos API
 */
TEST(EndpointPartitioning, PropertyQos)
{
    PubSubWriter<HelloWorldPubSubType> writer_a(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_a(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader_b(TEST_TOPIC_NAME);

    eprosima::fastdds::dds::PropertyPolicyQos writer_a_policy;
    writer_a_policy.properties().emplace_back("partitions", "partition_b");
    writer_a.entity_property_policy(writer_a_policy);
    writer_a.init();
    EXPECT_TRUE(writer_a.isInitialized());

    reader_a.set_xml_filename("partitions_profile.xml");
    reader_a.set_datareader_profile("partition_a_reader");
    reader_a.init();
    EXPECT_TRUE(reader_a.isInitialized());

    reader_b.set_xml_filename("partitions_profile.xml");
    reader_b.set_datareader_profile("partition_b_reader");
    reader_b.init();
    EXPECT_TRUE(reader_b.isInitialized());

    writer_a.wait_discovery(std::chrono::seconds(2));
    reader_a.wait_discovery(std::chrono::seconds(2));
    reader_b.wait_discovery(std::chrono::seconds(2));

    ASSERT_TRUE(writer_a.is_matched());
    ASSERT_FALSE(reader_a.is_matched());
    ASSERT_TRUE(reader_b.is_matched());

    writer_a.destroy();
    reader_a.destroy();
    reader_b.destroy();
}
