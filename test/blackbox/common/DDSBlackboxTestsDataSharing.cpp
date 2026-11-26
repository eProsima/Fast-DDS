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

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <future>
#include <sstream>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "UDPMessageSender.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

bool check_shared_file (
        const char* shared_dir,
        const eprosima::fastdds::rtps::GUID_t& guid)
{
    bool result;
    std::stringstream file_name;
    std::fstream file_stream;
#if ANDROID
    file_name << "/data/local/tmp/" << shared_dir << "/fast_datasharing_" << guid.guidPrefix << "_" << guid.entityId;
#else
    file_name << shared_dir << "/fast_datasharing_" << guid.guidPrefix << "_" << guid.entityId;
#endif // if ANDROID
    file_stream.open(file_name.str(), std::ios::in);
    result = file_stream.is_open();
    file_stream.close();
    return result;
}

class DDSDataSharing : public testing::TestWithParam<bool>
{
public:

    void SetUp() override
    {
        if (GetParam())
        {
            eprosima::fastdds::LibrarySettings library_settings;
            library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
        }
    }

    void TearDown() override
    {
        if (GetParam())
        {
            eprosima::fastdds::LibrarySettings library_settings;
            library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
        }
    }

};

TEST_P(DDSDataSharing, BasicCommunication)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    reader.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".").loan_sample_validation()
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).init();

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
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    constexpr int writer_history_depth = 2;
    constexpr int writer_sent_data = 4;

    writer.history_depth(writer_history_depth)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).init();

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
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS).init();

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


TEST_P(DDSDataSharing, BestEffortDirtyPayloads)
{
    // The writer's pool is smaller than the reader history.
    // The number of samples is larger than the pool size, so some payloads get reused
    // leaving dirty payloads in the reader
    PubSubReader<FixedSizedPubSubType> read_reader(TEST_TOPIC_NAME, false);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    constexpr int writer_history_depth = 2;
    constexpr int writer_sent_data = 5;

    writer.history_depth(writer_history_depth)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .resource_limits_extra_samples(1).init();

    ASSERT_TRUE(writer.isInitialized());

    read_reader.history_depth(writer_sent_data)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).init();

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

TEST_P(DDSDataSharing, ReliableDirtyPayloads)
{
    // The writer's pool is smaller than the reader history.
    // The number of samples is larger than the pool size, so some payloads get rused
    // leaving dirty payloads in the reader
    PubSubReader<FixedSizedPubSubType> read_reader(TEST_TOPIC_NAME, false);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    constexpr int writer_history_depth = 2;
    constexpr int writer_sent_data = 5;

    writer.history_depth(writer_history_depth)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .resource_limits_extra_samples(1).init();

    ASSERT_TRUE(writer.isInitialized());

    read_reader.history_depth(writer_sent_data)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();

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

    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
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

    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
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

    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
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

    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
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


TEST_P(DDSDataSharing, DataSharingPoolError)
{
    PubSubWriter<Data1mbPubSubType> writer_datasharing(TEST_TOPIC_NAME);
    PubSubWriter<Data1mbPubSubType> writer_auto(TEST_TOPIC_NAME);
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);

    writer_datasharing.resource_limits_max_samples(100000)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .datasharing_on(".").init();
    ASSERT_FALSE(writer_datasharing.isInitialized());

    writer_auto.resource_limits_max_samples(100000)
            .history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .datasharing_auto(".").init();
    ASSERT_TRUE(writer_auto.isInitialized());

    reader.datasharing_on(".")
            .history_depth(10)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).init();
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


TEST_P(DDSDataSharing, DataSharingDefaultDirectory)
{
    // Since the default directory heavily depends on the system,
    // we are not checking the creation of the files in this case,
    // only that it is working.
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    reader.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_auto()
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_auto()
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).init();

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

/*!
 * @test Regression test for bug #15176.
 */
TEST(DDSDataSharing, acknack_reception_when_change_removed_by_history)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    writer.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .lifespan_period({5, 0})
            .init();

    ASSERT_TRUE(writer.isInitialized());

    reader.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .lifespan_period({0, 10})
            .init();

    ASSERT_TRUE(reader.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_TRUE(check_shared_file(".", writer.datawriter_guid()));

    auto data = default_fixed_sized_data_generator(10);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    auto now = std::chrono::steady_clock::now();
    writer.waitForAllAcked(std::chrono::milliseconds(10000));
    ASSERT_LT(std::chrono::steady_clock::now() - now, std::chrono::milliseconds(5000));

    // Destroy reader and writer and see if there are dangling files
    reader.destroy();
    writer.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", writer.datawriter_guid()));
}

/*!
 * @test Regression test for bug #15176. Corner case using `get_unread_count(true)`.
 */
TEST(DDSDataSharing, acknack_reception_when_get_unread_count_and_change_removed_by_history)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    writer.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(writer.isInitialized());

    reader.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .loan_sample_validation()
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_TRUE(check_shared_file(".", writer.datawriter_guid()));

    auto data = default_fixed_sized_data_generator(2);
    auto data_recv = data;

    // Send data
    writer.send_sample(data.front());
    data.pop_front();

    // Wait to arrive data.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto& native_reader = reader.get_native_reader();
    ASSERT_EQ(1u, native_reader.get_unread_count(true));

    writer.send_sample(data.front());

    // Wait to arrive data.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    reader.startReception(data_recv);
    reader.block_for_all();
    auto now = std::chrono::steady_clock::now();
    writer.waitForAllAcked(std::chrono::milliseconds(10000));
    ASSERT_LT(std::chrono::steady_clock::now() - now, std::chrono::milliseconds(5000));

    // Destroy reader and writer and see if there are dangling files
    reader.destroy();
    writer.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", writer.datawriter_guid()));
}

/*!
 * @test Regression test for bug #15176. Corner case using `get_unread_count(true)`.
 */
TEST(DDSDataSharing, acknack_reception_when_get_unread_count)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    writer.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(writer.isInitialized());

    reader.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .loan_sample_validation()
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    // Because its volatile the durability
    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Check that the shared files are created on the correct directory
    ASSERT_TRUE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_TRUE(check_shared_file(".", writer.datawriter_guid()));

    auto data = default_fixed_sized_data_generator(10);
    auto data_recv = data;

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    // Wait to arrive data.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto& native_reader = reader.get_native_reader();
    ASSERT_EQ(10u, native_reader.get_unread_count(true));

    auto now = std::chrono::steady_clock::now();
    writer.waitForAllAcked(std::chrono::milliseconds(2000));
    ASSERT_GE(std::chrono::steady_clock::now() - now, std::chrono::milliseconds(1900));

    reader.startReception(data_recv);
    reader.block_for_all();
    now = std::chrono::steady_clock::now();
    writer.waitForAllAcked(std::chrono::milliseconds(2000));
    ASSERT_LT(std::chrono::steady_clock::now() - now, std::chrono::milliseconds(1900));

    // Destroy reader and writer and see if there are dangling files
    reader.destroy();
    writer.destroy();

    // Check that the shared files are created on the correct directory
    ASSERT_FALSE(check_shared_file(".", reader.datareader_guid()));
    ASSERT_FALSE(check_shared_file(".", writer.datawriter_guid()));
}

/*!
 * Regression test: deleting a data-sharing reader must not hang after one of its samples was
 * rejected by the reader's own history (resource limits exhausted, nobody drained it).
 *
 * DataSharingListener::process_new_data() calls ReaderPool::get_next_unread_payload(), which
 * returns a valid sample and immediately sets ReaderPool::last_sn_ to its sequence number --
 * *before* the caller hands the sample to the reader. If DataReader::process_data_msg() then
 * rejects it (history full), ReaderPool::advance_to_next_payload() is never called, so the very
 * same slot is re-read on the next loop pass, with last_sn_ already equal to its sequence number.
 * get_next_unread_payload() treats that as "this payload was overwritten by the writer, discard
 * it" -- which used to discard without advancing, so it kept re-reading that same, unchanged slot
 * forever. That spin never returns from get_next_unread_payload(), so DataSharingListener::stop()
 * -- called from the reader destructor -- hangs forever on listening_thread_.join().
 *
 * We give the listener thread a brief moment to process the (accepted then rejected) samples, then destroy the
 * reader under a watchdog so a hang fails the test instead of blocking the whole suite.
 */
TEST(DDSDataSharing, ReaderDeletionWithRejectedSampleDoesNotHang)
{
    // Force pure data-sharing (drop all network data messages).
    auto testTransport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // KEEP_ALL with room for a single sample, and the reader never calls take()/read(), so the
    // second sample the listener thread tries to hand over is guaranteed to be rejected.
    reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .resource_limits_allocated_samples(1)
            .resource_limits_max_samples(1)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).init();
    ASSERT_TRUE(reader.isInitialized());

    writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS)
            .add_user_transport_to_pparams(testTransport)
            .disable_builtin_transport()
            .datasharing_on(".")
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).init();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    // Send more samples than the reader can ever hold; at least one is guaranteed to be rejected.
    auto data = default_fixed_sized_data_generator(4);
    writer.send(data);

    // Give the listener thread time to process the accepted sample and then trip on the rejected
    // one.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Destroying the reader must not hang. Run it on a watchdog thread so a hang fails the test
    // instead of blocking the whole suite forever.
    std::promise<void> destroyed_promise;
    std::future<void> destroyed_future = destroyed_promise.get_future();
    std::thread destroy_thread([&reader, &destroyed_promise]()
            {
                reader.destroy();
                destroyed_promise.set_value();
            });

    const bool destroyed_in_time =
            destroyed_future.wait_for(std::chrono::seconds(10)) == std::future_status::ready;

    EXPECT_TRUE(destroyed_in_time)
        << "Data-sharing reader destruction hung after a sample was rejected by the reader's own "
        << "history (DataSharingListener::stop() starved on listening_thread_.join()).";

    if (!destroyed_in_time)
    {
        // reader is wedged with participant_ still non-null: letting this function return would
        // run ~PubSubReader() on the main thread, which calls destroy() again on that same
        // participant and hangs a second time, unbounded. Bail out of the whole process now that
        // the failure is already recorded, instead of risking a real, watchdog-less hang.
        std::_Exit(1);
    }

    destroy_thread.join();
    writer.destroy();
}

namespace {

// Serialized FixedSized sample: encapsulation header + FixedSized::index = 0x1337
struct SpoofedSerializedData
{
    uint8_t encapsulation[2] = {0x00, CDR_LE};
    uint8_t encapsulation_opts[2] = {0x00, 0x00};
    uint8_t data[4] = {0x37, 0x13, 0x00, 0x00};
};

struct SpoofedDataSubMsg
{
    struct Header
    {
        uint8_t submessage_id = 0x15;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
        uint8_t flags = 0x04;        // Data present, big endian
#else
        uint8_t flags = 0x05;        // Data present, little endian
#endif  // FASTDDS_IS_BIG_ENDIAN_TARGET
        uint16_t octets_to_next_header = 28;
        uint16_t extra_flags = 0;
        uint16_t octets_to_inline_qos = 16;
        EntityId_t reader_id{};
        EntityId_t writer_id{};
        SequenceNumber_t sn{ 1 };
    };

    Header header;
    SpoofedSerializedData payload;
};

struct SpoofedDataFragSubMsg
{
    struct Header
    {
        uint8_t submessage_id = 0x16;
#if FASTDDS_IS_BIG_ENDIAN_TARGET
        uint8_t flags = 0x00;        // big endian, no key
#else
        uint8_t flags = 0x01;        // little endian, no key
#endif  // FASTDDS_IS_BIG_ENDIAN_TARGET
        uint16_t octets_to_next_header = 40;
        uint16_t extra_flags = 0;
        uint16_t octets_to_inline_qos = 28;
        EntityId_t reader_id{};
        EntityId_t writer_id{};
        SequenceNumber_t sn{ 1 };
        // Whole sample delivered as two fragments in a single submessage
        uint32_t fragment_starting_num = 1;
        uint16_t fragments_in_submessage = 2;
        uint16_t fragment_size = 4;
        uint32_t sample_size = 8;
    };

    Header header;
    SpoofedSerializedData payload;
};

template<typename SubMsg>
struct SpoofedPacket
{
    std::array<char, 4> rtps_id{ {'R', 'T', 'P', 'S'} };
    std::array<uint8_t, 2> protocol_version{ {2, 3} };
    std::array<uint8_t, 2> vendor_id{ {0x01, 0x0F} };
    GuidPrefix_t sender_prefix{};
    SubMsg data;
};

/**
 * Matches a DataSharing reader and writer, sends a spoofed network packet carrying
 * the writer GUID and checks it is rejected while regular samples still arrive.
 */
template<typename SubMsg>
void check_spoofed_network_data_is_not_treated_as_datasharing(
        eprosima::fastdds::dds::ReliabilityQosPolicyKind reliability)
{
    SCOPED_TRACE(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS == reliability ? "RELIABLE" : "BEST_EFFORT");

    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    // Force a UDP transport and a known reader locator so the spoofed datagram
    // reaches the reader's receiver through the production RTPS path.
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    Locator_t reader_locator;
    ASSERT_TRUE(IPLocator::setIPv4(reader_locator, "127.0.0.1"));
    reader_locator.port = 7400;

    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(udp_transport)
            .add_to_unicast_locator_list("127.0.0.1", 7400)
            .datasharing_auto(".")
            .reliability(reliability)
            .init();
    ASSERT_TRUE(reader.isInitialized());

    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(udp_transport)
            .datasharing_auto(".")
            .reliability(reliability)
            .init();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    // The reader must have matched the writer through DataSharing
    ASSERT_TRUE(check_shared_file(".", writer.datawriter_guid()));

    // Expect to receive nothing: the forged sample must be rejected.
    reader.startReception(1);

    UDPMessageSender fake_msg_sender;
    {
        auto writer_guid = writer.datawriter_guid();

        SpoofedPacket<SubMsg> packet{};
        packet.sender_prefix = writer_guid.guidPrefix;
        packet.data.header.writer_id = writer_guid.entityId;
        packet.data.header.reader_id = reader.datareader_guid().entityId;

        CDRMessage_t msg(0);
        uint32_t msg_len = static_cast<uint32_t>(sizeof(packet));
        msg.init(reinterpret_cast<octet*>(&packet), msg_len);
        msg.length = msg_len;
        msg.pos = msg_len;
        fake_msg_sender.send(msg, reader_locator);
    }

    // Give the reception thread time to process the datagram
    EXPECT_FALSE(reader.wait_for_all_received(std::chrono::seconds(2), 1));

    auto data = default_fixed_sized_data_generator(1);
    writer.send(data);
    ASSERT_TRUE(data.empty());
    EXPECT_TRUE(reader.wait_for_all_received(std::chrono::seconds(2), 1));

    reader.destroy();
    writer.destroy();
}

} // namespace

#if defined(__SANITIZE_ADDRESS__)
/**
 * @test Regression for a DataSharing payload-provenance spoofing issue.
 *
 * A reader that is matched with a DataSharing writer classifies an incoming
 * sample as DataSharing solely from the writer GUID. An attacker could use a
 * fake message with the same GUID to cause an OOB by treating as a DataSharing
 * payload a non-compatible payload.
 * Run this test under AddressSanitizer to detect the resulting invalid memory access.
 */
TEST(DDSDataSharing, SpoofedNetworkDataIsNotTreatedAsDataSharing)
{
    ASSERT_NO_FATAL_FAILURE(check_spoofed_network_data_is_not_treated_as_datasharing<SpoofedDataSubMsg>(
                eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS));
    ASSERT_NO_FATAL_FAILURE(check_spoofed_network_data_is_not_treated_as_datasharing<SpoofedDataSubMsg>(
                eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS));
}
#endif // if defined(__SANITIZE_ADDRESS__)

/**
 * @test Same as SpoofedNetworkDataIsNotTreatedAsDataSharing, but the spoofed sample is sent
 * as a DATA_FRAG submessage. DataSharing writers never send fragments, so the forged sample
 * must not be delivered nor consume the sequence number of the real one.
 * Covers both the StatelessReader (best effort) and StatefulReader (reliable).
 */
TEST(DDSDataSharing, SpoofedNetworkDataFragIsNotTreatedAsDataSharing)
{
    ASSERT_NO_FATAL_FAILURE(check_spoofed_network_data_is_not_treated_as_datasharing<SpoofedDataFragSubMsg>(
                eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS));
    ASSERT_NO_FATAL_FAILURE(check_spoofed_network_data_is_not_treated_as_datasharing<SpoofedDataFragSubMsg>(
                eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS));
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(DDSDataSharing,
        DDSDataSharing,
        testing::Values(false, true),
        [](const testing::TestParamInfo<DDSDataSharing::ParamType>& info)
        {
            bool intraprocess = info.param;
            return intraprocess ? "Intraprocess_and_datasharing" : "Datasharing_only";
        });
