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
#include <fastrtps/transport/test_UDPv4Transport.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;


TEST(DDSDataSharing, BasicCommunication)
{
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    reader.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready")
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.history_depth(100)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready")
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


TEST(DDSDataSharing, TransientReader)
{
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    constexpr int writer_history_depth = 2;
    constexpr int writer_sent_data = 4;

    writer.history_depth(writer_history_depth)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready")
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

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
            .datasharing_on("Unused. change when ready")
            .reliability(BEST_EFFORT_RELIABILITY_QOS)
            .durability_kind(TRANSIENT_LOCAL_DURABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(received_data);
    reader.block_for_all();
}


TEST(DDSDataSharing, BestEffortDirtyPayloads)
{
    PubSubReader<FixedSizedType> take_reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> read_reader(TEST_TOPIC_NAME, false);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    constexpr int writer_history_depth = 2;
    constexpr int writer_sent_data = 4;

    writer.history_depth(writer_history_depth)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready")
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    take_reader.history_depth(writer_sent_data)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready")
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(take_reader.isInitialized());

    read_reader.history_depth(writer_sent_data)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready")
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(read_reader.isInitialized());

    writer.wait_discovery(2);
    take_reader.wait_discovery();
    read_reader.wait_discovery();

    // Send the data to fill the history and overwrite old changes
    // The reader will receive all changes but the application will see only the last ones
    std::list<FixedSized> data = default_fixed_sized_data_generator(writer_sent_data);
    std::list<FixedSized> received_data;
    auto data_it = data.begin();
    std::advance(data_it, writer_sent_data - writer_history_depth);
    std::copy(data_it, data.end(), std::back_inserter(received_data));

    // Until the ACK system is in place,
    // we have to give the listener time to get the payload before the read/write pointers get messed
    // Remove the sleep time once the ACK are in place.
    writer.send(data, 100);
    ASSERT_TRUE(data.empty());

    take_reader.startReception(received_data);
    take_reader.block_for_all();

    read_reader.startReception(received_data);
    read_reader.block_for_all();
}

TEST(DDSDataSharing, ReliableDirtyPayloads)
{
    PubSubReader<FixedSizedType> take_reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> read_reader(TEST_TOPIC_NAME, false);
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);

    // Disable transports to ensure we are using datasharing
    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    constexpr int writer_history_depth = 2;
    constexpr int writer_sent_data = 4;

    writer.history_depth(writer_history_depth)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready")
            .reliability(RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(writer.isInitialized());

    take_reader.history_depth(writer_sent_data)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready")
            .reliability(RELIABLE_RELIABILITY_QOS).init();

    ASSERT_TRUE(take_reader.isInitialized());

    read_reader.history_depth(writer_sent_data)
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready")
            .reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(read_reader.isInitialized());

    writer.wait_discovery(2);
    take_reader.wait_discovery();
    read_reader.wait_discovery();

    // Send the data to fill the history and overwrite old changes
    // The reader will receive all changes but the application will see only the last ones
    std::list<FixedSized> data = default_fixed_sized_data_generator(writer_sent_data);
    std::list<FixedSized> received_data;
    auto data_it = data.begin();
    std::advance(data_it, writer_sent_data - writer_history_depth);
    std::copy(data_it, data.end(), std::back_inserter(received_data));

    // Until the ACK system is in place,
    // we have to give the listener time to get the payload before the read/write pointers get messed
    // Remove the sleep time once the ACK are in place.
    writer.send(data, 100);
    ASSERT_TRUE(data.empty());

    take_reader.startReception(received_data);
    take_reader.block_for_all();

    read_reader.startReception(received_data);
    read_reader.block_for_all();
}

TEST(DDSDataSharing, DataSharingWriter_DifferentDomainReaders)
{
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> datasharing_reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> non_datasharing_reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> auto_reader(TEST_TOPIC_NAME);

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    std::vector<uint16_t> reader_ids;
    reader_ids.push_back(10);

    std::vector<uint16_t> writer_ids;
    writer_ids.push_back(20);

    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready", writer_ids).init();
    ASSERT_TRUE(writer.isInitialized());

    datasharing_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready", reader_ids).init();
    ASSERT_TRUE(datasharing_reader.isInitialized());

    non_datasharing_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_off().init();
    ASSERT_TRUE(non_datasharing_reader.isInitialized());

    auto_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_auto(reader_ids).init();
    ASSERT_TRUE(auto_reader.isInitialized());

    writer.wait_discovery(3);
    datasharing_reader.wait_discovery();
    non_datasharing_reader.wait_discovery();
    auto_reader.wait_discovery();

    std::list<FixedSized> data = default_fixed_sized_data_generator(4);
    datasharing_reader.startReception(data);
    non_datasharing_reader.startReception(data);
    auto_reader.startReception(data);

    // Until the ACK system is in place,
    // we have to give the listener time to get the payload before the read/write pointers get messed
    // Remove the sleep time once the ACK are in place.
    writer.send(data, 100);
    ASSERT_TRUE(data.empty());

    ASSERT_EQ(datasharing_reader.block_for_all(std::chrono::seconds(2)), 0u);
    ASSERT_EQ(non_datasharing_reader.block_for_all(std::chrono::seconds(2)), 0u);
    ASSERT_EQ(auto_reader.block_for_all(std::chrono::seconds(2)), 0u);
}

TEST(DDSDataSharing, DataSharingWriter_CommonDomainReaders)
{
    PubSubWriter<FixedSizedType> writer(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> datasharing_reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> non_datasharing_reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> auto_reader(TEST_TOPIC_NAME);

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
            .datasharing_on("Unused. change when ready", writer_ids).init();
    ASSERT_TRUE(writer.isInitialized());

    datasharing_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready", reader_ids).init();
    ASSERT_TRUE(datasharing_reader.isInitialized());

    non_datasharing_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_off().init();
    ASSERT_TRUE(non_datasharing_reader.isInitialized());

    auto_reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_auto(reader_ids).init();
    ASSERT_TRUE(auto_reader.isInitialized());

    writer.wait_discovery(3);
    datasharing_reader.wait_discovery();
    non_datasharing_reader.wait_discovery();
    auto_reader.wait_discovery();

    std::list<FixedSized> data = default_fixed_sized_data_generator(4);
    datasharing_reader.startReception(data);
    non_datasharing_reader.startReception(data);
    auto_reader.startReception(data);

    // Until the ACK system is in place,
    // we have to give the listener time to get the payload before the read/write pointers get messed
    // Remove the sleep time once the ACK are in place.
    writer.send(data, 100);
    ASSERT_TRUE(data.empty());

    ASSERT_EQ(non_datasharing_reader.block_for_all(std::chrono::seconds(2)), 0u);
    auto_reader.block_for_all();
    auto_reader.block_for_all();
}

TEST(DDSDataSharing, DataSharingReader_DifferentDomainWriters)
{
    PubSubWriter<FixedSizedType> datasharing_writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> non_datasharing_writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> auto_writer(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);

    auto testTransport = std::make_shared<test_UDPv4TransportDescriptor>();
    testTransport->dropDataMessagesPercentage = 100;

    std::vector<uint16_t> reader_ids;
    reader_ids.push_back(10);

    std::vector<uint16_t> writer_ids;
    writer_ids.push_back(20);

    datasharing_writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready", writer_ids).init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    non_datasharing_writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_off().init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    auto_writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_auto(writer_ids).init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready", reader_ids).init();
    ASSERT_TRUE(reader.isInitialized());

    reader.wait_discovery(std::chrono::seconds::zero(), 3);
    datasharing_writer.wait_discovery();
    non_datasharing_writer.wait_discovery();
    auto_writer.wait_discovery();

    std::list<FixedSized> data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    // Until the ACK system is in place,
    // we have to give the listener time to get the payload before the read/write pointers get messed
    // Remove the sleep time once the ACK are in place.
    datasharing_writer.send(data, 100);
    ASSERT_TRUE(data.empty());
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(2)), 0u);

    data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    // Until the ACK system is in place,
    // we have to give the listener time to get the payload before the read/write pointers get messed
    // Remove the sleep time once the ACK are in place.
    non_datasharing_writer.send(data, 100);
    ASSERT_TRUE(data.empty());
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(2)), 0u);

    data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    // Until the ACK system is in place,
    // we have to give the listener time to get the payload before the read/write pointers get messed
    // Remove the sleep time once the ACK are in place.
    auto_writer.send(data, 100);
    ASSERT_TRUE(data.empty());
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(2)), 0u);
}

TEST(DDSDataSharing, DataSharingReader_CommonDomainWriters)
{
    PubSubWriter<FixedSizedType> datasharing_writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> non_datasharing_writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedType> auto_writer(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedType> reader(TEST_TOPIC_NAME);

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
            .datasharing_on("Unused. change when ready", writer_ids).init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    non_datasharing_writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_off().init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    auto_writer.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_auto(writer_ids).init();
    ASSERT_TRUE(datasharing_writer.isInitialized());

    reader.disable_builtin_transport()
            .add_user_transport_to_pparams(testTransport)
            .datasharing_on("Unused. change when ready", reader_ids).init();
    ASSERT_TRUE(reader.isInitialized());

    reader.wait_discovery(std::chrono::seconds::zero(), 3);
    datasharing_writer.wait_discovery();
    non_datasharing_writer.wait_discovery();
    auto_writer.wait_discovery();

    std::list<FixedSized> data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    // Until the ACK system is in place,
    // we have to give the listener time to get the payload before the read/write pointers get messed
    // Remove the sleep time once the ACK are in place.
    datasharing_writer.send(data, 100);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();

    data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    // Until the ACK system is in place,
    // we have to give the listener time to get the payload before the read/write pointers get messed
    // Remove the sleep time once the ACK are in place.
    non_datasharing_writer.send(data, 100);
    ASSERT_TRUE(data.empty());
    ASSERT_EQ(reader.block_for_all(std::chrono::seconds(2)), 0u);

    data = default_fixed_sized_data_generator(4);
    reader.startReception(data);

    // Until the ACK system is in place,
    // we have to give the listener time to get the payload before the read/write pointers get messed
    // Remove the sleep time once the ACK are in place.
    auto_writer.send(data, 100);
    ASSERT_TRUE(data.empty());
    reader.block_for_all();
}