// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastrtps/log/Log.h>

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include <fastrtps/transport/test_UDPv4TransportDescriptor.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <sstream>
#include <fstream>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

bool check_shared_file (
        const char* shared_dir,
        const eprosima::fastrtps::rtps::GUID_t& guid)
{
    bool result;
    std::stringstream file_name;
    std::fstream file_stream;

    file_name << shared_dir << "/fast_datasharing_" << guid.guidPrefix << "_" << guid.entityId;
    file_stream.open(file_name.str(), std::ios::in);
    result = file_stream.is_open();
    file_stream.close();
    return result;
}

TEST(DDSDataSharing, BasicCommunication)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    reader.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".").loan_sample_validation()
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".")
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_TRUE(check_shared_file(".", writer.datawriter_guid()));

    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Destroy reader and writer and see if there are dangling files
    reader.destroy();
    writer.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", writer.datawriter_guid()));
}


TEST(DDSDataSharing, TransientReader)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    constexpr int writer_history_depth = 2;
    constexpr int writer_sent_data = 4;

    writer.history_depth(writer_history_depth)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".")
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", writer.datawriter_guid()));

    // Send the data to fill the history and overwrite old changes
    // The reader only receives the last changes
    std::list<FixedSized> data = default_fixed_sized_data_generator(writer_sent_data);
    std::list<FixedSized> received_data;
    auto data_it = data.begin();
    std::advance(data_it, writer_sent_data - writer_history_depth);
    std::copy(data_it, data.end(), std::back_inserter(received_data));

    writer.send(data);
    ASSERT_TRUE(data.empty());

    reader.history_depth(writer_sent_data)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".")
            .reliability(BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", reader.datareader_guid()));

    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(received_data);
    reader.block_for_all();

    // Destroy reader and writer and see if there are dangling files
    reader.destroy();
    writer.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", writer.datawriter_guid()));
}


TEST(DDSDataSharing, BestEffortDirtyPayloads)
{
    // The writer's pool is smaller than the reader history.
    // The number of samples is larger than the pool size, so some payloads get reused
    // leaving dirty payloads in the reader
    PubSubReader<FixedSizedPubSubType> read_reader(TEST_TOPIC_NAME, false);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    constexpr int writer_history_depth = 2;
    constexpr int writer_sent_data = 5;

    writer.history_depth(writer_history_depth)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".")
            .reliability(BEST_EFFORT_RELIABILITY_QOS)
            .resource_limits_extra_samples(1).init();

    ASSERT_TRUE(writer.isInitialized());

    read_reader.history_depth(writer_sent_data)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".")
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(read_reader.isInitialized());

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", read_reader.datareader_guid()));
    ASSERT_TRUE(check_shared_file(".", writer.datawriter_guid()));

    writer.wait_discovery();
    read_reader.wait_discovery();

    std::list<FixedSized> data = default_fixed_sized_data_generator(writer_sent_data);
    std::list<FixedSized> valid_data;
    auto data_it = data.begin();
    std::advance(data_it, writer_sent_data - writer_history_depth - 1);
    std::copy(data_it, data.end(), std::back_inserter(valid_data));

    // Send the data to fill the history and overwrite old changes
    writer.send(data, 100);
    ASSERT_TRUE(data.empty());

    // The reader has overridden payloads in the history. Only the valid ones are returned to the user
    read_reader.startReception(valid_data);
    read_reader.block_for_all();

    // Destroy reader and writer and see if there are dangling files
    read_reader.destroy();
    writer.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", read_reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", writer.datawriter_guid()));
}

TEST(DDSDataSharing, ReliableDirtyPayloads)
{
    // The writer's pool is smaller than the reader history.
    // The number of samples is larger than the pool size, so some payloads get rused
    // leaving dirty payloads in the reader
    PubSubReader<FixedSizedPubSubType> read_reader(TEST_TOPIC_NAME, false);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    constexpr int writer_history_depth = 2;
    constexpr int writer_sent_data = 5;

    writer.history_depth(writer_history_depth)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".")
            .reliability(RELIABLE_RELIABILITY_QOS)
            .resource_limits_extra_samples(1).init();

    ASSERT_TRUE(writer.isInitialized());

    read_reader.history_depth(writer_sent_data)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".")
            .reliability(RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(read_reader.isInitialized());

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", read_reader.datareader_guid()));
    ASSERT_TRUE(check_shared_file(".", writer.datawriter_guid()));

    writer.wait_discovery();
    read_reader.wait_discovery();

    std::list<FixedSized> data = default_fixed_sized_data_generator(writer_sent_data);
    std::list<FixedSized> valid_data;
    auto data_it = data.begin();
    std::advance(data_it, writer_sent_data - writer_history_depth - 1);
    std::copy(data_it, data.end(), std::back_inserter(valid_data));

    writer.send(data, 100);
    ASSERT_TRUE(data.empty());

    // Send the data to fill the history and overwrite old changes
    // The reader will receive and process all changes so that the writer can reuse them,
    // but will keep them in the history.
    read_reader.startReception(valid_data);
    read_reader.block_for_all();

    // Destroy reader and writer and see if there are dangling files
    read_reader.destroy();
    writer.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", read_reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", writer.datawriter_guid()));
}

TEST(DDSDataSharing, DataSharingWriter_DifferentDomainReaders)
{
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> datasharing_reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> non_datasharing_reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> auto_reader(TEST_TOPIC_NAME);

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    std::vector<uint16_t> reader_ids;
    reader_ids.push_back(10);

    std::vector<uint16_t> writer_ids;
    writer_ids.push_back(20);

    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".", writer_ids)
            .resource_limits_extra_samples(5).init();
    ASSERT_TRUE(writer.isInitialized());

    datasharing_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".", reader_ids).init();
    ASSERT_TRUE(datasharing_reader.isInitialized());

    non_datasharing_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_off().init();
    ASSERT_TRUE(non_datasharing_reader.isInitialized());

    auto_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_auto(".", reader_ids).init();
    ASSERT_TRUE(auto_reader.isInitialized());

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", datasharing_reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", non_datasharing_reader.datareader_guid()));
    ASSERT_TRUE(check_shared_file(".", auto_reader.datareader_guid()));
    ASSERT_TRUE(check_shared_file(".", writer.datawriter_guid()));

    writer.wait_discovery(3);
    datasharing_reader.wait_discovery();
    non_datasharing_reader.wait_discovery();
    auto_reader.wait_discovery();

    std::list<FixedSized> data = default_fixed_sized_data_generator(4);
    datasharing_reader.startReception(data);
    non_datasharing_reader.startReception(data);
    auto_reader.startReception(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());

    ASSERT_EQ(datasharing_reader.block_for_all(std::chrono::seconds(2)), 0u);
    ASSERT_EQ(non_datasharing_reader.block_for_all(std::chrono::seconds(2)), 0u);
    ASSERT_EQ(auto_reader.block_for_all(std::chrono::seconds(2)), 0u);

    // Destroy reader and writer and see if there are dangling files
    datasharing_reader.destroy();
    non_datasharing_reader.destroy();
    auto_reader.destroy();
    writer.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", datasharing_reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", non_datasharing_reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", auto_reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", writer.datawriter_guid()));
}

TEST(DDSDataSharing, DataSharingWriter_CommonDomainReaders)
{
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> datasharing_reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> non_datasharing_reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> auto_reader(TEST_TOPIC_NAME);

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    std::vector<uint16_t> reader_ids;
    reader_ids.push_back(10);
    reader_ids.push_back(15);

    std::vector<uint16_t> writer_ids;
    writer_ids.push_back(20);
    writer_ids.push_back(15);

    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".", writer_ids)
            .resource_limits_extra_samples(5).init();
    ASSERT_TRUE(writer.isInitialized());

    datasharing_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".", reader_ids).init();
    ASSERT_TRUE(datasharing_reader.isInitialized());

    non_datasharing_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_off().init();
    ASSERT_TRUE(non_datasharing_reader.isInitialized());

    auto_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_auto(".", reader_ids).init();
    ASSERT_TRUE(auto_reader.isInitialized());

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", datasharing_reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", non_datasharing_reader.datareader_guid()));
    ASSERT_TRUE(check_shared_file(".", auto_reader.datareader_guid()));
    ASSERT_TRUE(check_shared_file(".", writer.datawriter_guid()));

    writer.wait_discovery(3);
    datasharing_reader.wait_discovery();
    non_datasharing_reader.wait_discovery();
    auto_reader.wait_discovery();

    std::list<FixedSized> data = default_fixed_sized_data_generator(4);
    datasharing_reader.startReception(data);
    non_datasharing_reader.startReception(data);
    auto_reader.startReception(data);

    writer.send(data);
    ASSERT_TRUE(data.empty());

    ASSERT_EQ(non_datasharing_reader.block_for_all(std::chrono::seconds(2)), 0u);
    auto_reader.block_for_all();
    auto_reader.block_for_all();

    // Destroy reader and writer and see if there are dangling files
    datasharing_reader.destroy();
    non_datasharing_reader.destroy();
    auto_reader.destroy();
    writer.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", datasharing_reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", non_datasharing_reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", auto_reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", writer.datawriter_guid()));
}

TEST(DDSDataSharing, DataSharingReader_DifferentDomainWriters)
{
    PubSubWriter<FixedSizedPubSubType> datasharing_writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> non_datasharing_writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> auto_writer(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    std::vector<uint16_t> reader_ids;
    reader_ids.push_back(10);

    std::vector<uint16_t> writer_ids;
    writer_ids.push_back(20);

    datasharing_writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".", writer_ids)
            .resource_limits_extra_samples(5).init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    non_datasharing_writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_off().init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    auto_writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_auto(".", writer_ids)
            .resource_limits_extra_samples(5).init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".", reader_ids).init();
    ASSERT_TRUE(reader.isInitialized());

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", datasharing_writer.datawriter_guid()));
    ASSERT_FALSE(check_shared_file(".", non_datasharing_writer.datawriter_guid()));
    ASSERT_TRUE(check_shared_file(".", auto_writer.datawriter_guid()));
    ASSERT_TRUE(check_shared_file(".", reader.datareader_guid()));

    reader.wait_discovery(std::chrono::seconds::zero(), 3);
    datasharing_writer.wait_discovery();
    non_datasharing_writer.wait_discovery();
    auto_writer.wait_discovery();

    std::list<FixedSized> data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    datasharing_writer.send(data);
    ASSERT_TRUE(data.empty());
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(2)), 0u);

    data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    non_datasharing_writer.send(data);
    ASSERT_TRUE(data.empty());
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(2)), 0u);

    data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    auto_writer.send(data);
    ASSERT_TRUE(data.empty());
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(2)), 0u);

    // Destroy reader and writer and see if there are dangling files
    reader.destroy();
    datasharing_writer.destroy();
    non_datasharing_writer.destroy();
    auto_writer.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", datasharing_writer.datawriter_guid()));
    ASSERT_FALSE(check_shared_file(".", non_datasharing_writer.datawriter_guid()));
    ASSERT_FALSE(check_shared_file(".", auto_writer.datawriter_guid()));
}

TEST(DDSDataSharing, DataSharingReader_CommonDomainWriters)
{
    PubSubWriter<FixedSizedPubSubType> datasharing_writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> non_datasharing_writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> auto_writer(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    std::vector<uint16_t> reader_ids;
    reader_ids.push_back(10);
    reader_ids.push_back(15);

    std::vector<uint16_t> writer_ids;
    writer_ids.push_back(20);
    writer_ids.push_back(15);

    datasharing_writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".", writer_ids)
            .resource_limits_extra_samples(5).init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    non_datasharing_writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_off().init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    auto_writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_auto(".", writer_ids)
            .resource_limits_extra_samples(5).init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on(".", reader_ids).init();
    ASSERT_TRUE(reader.isInitialized());

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", datasharing_writer.datawriter_guid()));
    ASSERT_FALSE(check_shared_file(".", non_datasharing_writer.datawriter_guid()));
    ASSERT_TRUE(check_shared_file(".", auto_writer.datawriter_guid()));
    ASSERT_TRUE(check_shared_file(".", reader.datareader_guid()));

    reader.wait_discovery(std::chrono::seconds::zero(), 3);
    datasharing_writer.wait_discovery();
    non_datasharing_writer.wait_discovery();
    auto_writer.wait_discovery();

    std::list<FixedSized> data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    datasharing_writer.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();

    data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    non_datasharing_writer.send(data);
    ASSERT_TRUE(data.empty());
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(2)), 0u);

    data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    auto_writer.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();

    // Destroy reader and writer and see if there are dangling files
    reader.destroy();
    datasharing_writer.destroy();
    non_datasharing_writer.destroy();
    auto_writer.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", datasharing_writer.datawriter_guid()));
    ASSERT_FALSE(check_shared_file(".", non_datasharing_writer.datawriter_guid()));
    ASSERT_FALSE(check_shared_file(".", auto_writer.datawriter_guid()));
}


TEST(DDSDataSharing, DataSharingPoolError)
{
    PubSubWriter<Data1mbPubSubType> writer_datasharing(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer_auto(TEST_TOPIC_NAME);
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);

    writer_datasharing.resource_limits_max_samples(100000)
            .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .datasharing_on(".").init();
    ASSERT_FALSE(writer_datasharing.isInitialized());

    writer_auto.resource_limits_max_samples(100000)
            .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .datasharing_auto(".").init();
    ASSERT_TRUE(writer_auto.isInitialized());

    reader.datasharing_on(".")
            .history_depth(10)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).init();
    ASSERT_TRUE(reader.isInitialized());

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", writer_datasharing.datawriter_guid()));
    ASSERT_FALSE(check_shared_file(".", writer_auto.datawriter_guid()));
    ASSERT_TRUE(check_shared_file(".", reader.datareader_guid()));

    reader.wait_discovery();
    writer_auto.wait_discovery();

    auto data = default_data300kb_data_generator();
    reader.startReception(data);

    writer_auto.send(data);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();

    // Destroy reader and writer and see if there are dangling files
    reader.destroy();
    writer_datasharing.destroy();
    writer_auto.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", writer_datasharing.datawriter_guid()));
    ASSERT_FALSE(check_shared_file(".", writer_auto.datawriter_guid()));
}


TEST(DDSDataSharing, DataSharingDefaultDirectory)
{
    // Since the default directory heavily depends on the system,
    // we are not checking the creation of the files in this case,
    // only that it is working.
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    reader.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_auto()
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_auto()
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();
}
