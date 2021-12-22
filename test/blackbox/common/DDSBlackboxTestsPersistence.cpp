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

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "ReqRepAsReliableHelloWorldRequester.hpp"
#include "ReqRepAsReliableHelloWorldReplier.hpp"
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <fastrtps/log/Log.h>

#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class PersistenceLargeData : public testing::TestWithParam<communication_type>
{
public:

    const std::string& db_file_name() const
    {
        return db_file_name_;
    }

protected:

    std::string db_file_name_;

    void SetUp() override
    {
        LibrarySettingsAttributes library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
                xmlparser::XMLProfileManager::library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }

        // Get info about current test
        auto info = ::testing::UnitTest::GetInstance()->current_test_info();

        // Create DB file name from test name and PID
        std::ostringstream ss;
        std::string test_case_name(info->test_case_name());
        std::string test_name(info->name());
        ss <<
            test_case_name.replace(test_case_name.find_first_of('/'), 1, "_") << "_" <<
            test_name.replace(test_name.find_first_of('/'), 1, "_")  << "_" << GET_PID() << ".db";
        db_file_name_ = ss.str();

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
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
        std::remove(db_file_name_.c_str());
    }

    void fragment_data(
            bool large_data)
    {
        PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);
        PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);

        auto testTransport = std::make_shared<UDPv4TransportDescriptor>();
        testTransport->sendBufferSize = 32768;
        testTransport->maxMessageSize = 32768;
        testTransport->receiveBufferSize = 32768;

        writer
                .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
                .resource_limits_max_samples(100)
                .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                .make_persistent(db_file_name(), "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64")
                .disable_builtin_transport()
                .add_user_transport_to_pparams(testTransport)
                .init();

        ASSERT_TRUE(writer.isInitialized());

        auto data = default_data16kb_data_generator();
        if (large_data)
        {
            data = default_data300kb_data_generator();
        }
        auto unreceived_data = data;

        // Send data
        writer.send(data);
        // All data should be sent
        ASSERT_TRUE(data.empty());
        // Destroy the DataWriter
        writer.destroy();
        // Load the persistent DataWriter with the changes saved in the database
        writer.init();

        ASSERT_TRUE(writer.isInitialized());

        reader
                .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
                .history_depth(10)
                .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
                .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
                .socket_buffer_size(1048576)
                .init();

        ASSERT_TRUE(reader.isInitialized());

        // Wait for discovery.
        writer.wait_discovery();
        reader.wait_discovery();

        reader.startReception(unreceived_data);

        // Block reader until reception finished or timeout.
        reader.block_for_all();
    }

};

TEST_P(PersistenceLargeData, PubSubAsReliablePubPersistentWithFrag)
{
    fragment_data(true);
}

TEST_P(PersistenceLargeData, PubSubAsReliablePubPersistentNoFrag)
{
    fragment_data(false);
}

TEST_P(PersistenceLargeData, PubSubAsReliablePubPersistentWithLifespanBefore)
{
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);

    writer
            .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_samples(100)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .make_persistent(db_file_name(), "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64")
            .lifespan_period({1, 0})
            .init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_data16kb_data_generator();
    auto unreceived_data = data;

    // Send data
    writer.send(data);
    // All data should be sent
    ASSERT_TRUE(data.empty());
    // Destroy the DataWriter
    writer.destroy();
    // Load the persistent DataWriter with the changes saved in the database
    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Sleep waiting samples to exceed the lifespan
    std::this_thread::sleep_for(std::chrono::seconds(2));

    reader
            .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
            .history_depth(10)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(unreceived_data);

    // Wait expecting not receiving data.
    ASSERT_EQ(0u, reader.block_for_all(std::chrono::seconds(1)));
}

TEST_P(PersistenceLargeData, PubSubAsReliablePubPersistentWithLifespanSendingBefore)
{
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);

    writer
            .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_samples(100)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .make_persistent(db_file_name(), "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64")
            .lifespan_period({0, 100})
            .init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_data16kb_data_generator();
    auto unreceived_data = data;

    // Send data
    writer.send(data);
    // All data should be sent
    ASSERT_TRUE(data.empty());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    // Destroy the DataWriter
    writer.destroy();
    // Load the persistent DataWriter with the changes saved in the database
    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    data = default_data16kb_data_generator(1);
    unreceived_data.insert(unreceived_data.end(), data.begin(), data.end());
    // Send data
    writer.send(data);
    // All data should be sent
    ASSERT_TRUE(data.empty());

    // Sleep waiting samples to exceed the lifespan
    std::this_thread::sleep_for(std::chrono::seconds(2));

    reader
            .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
            .history_depth(10)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(unreceived_data);

    // Wait expecting not receiving data.
    ASSERT_EQ(0u, reader.block_for_all(std::chrono::seconds(1)));
}

TEST_P(PersistenceLargeData, PubSubAsReliablePubPersistentWithLifespanAfter)
{
    PubSubWriter<Data1mbPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<Data1mbPubSubType> reader(TEST_TOPIC_NAME);

    writer
            .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .resource_limits_max_samples(100)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .make_persistent(db_file_name(), "77.72.69.74.65.72.5f.70.65.72.73.5f|67.75.69.64")
            .lifespan_period({1, 0})
            .init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_data16kb_data_generator();
    auto unreceived_data = data;

    // Send data
    writer.send(data);
    // All data should be sent
    ASSERT_TRUE(data.empty());
    // Destroy the DataWriter
    writer.destroy();

    reader
            .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
            .history_depth(10)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .durability_kind(eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    // Sleep waiting samples to exceed the lifespan
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Load the persistent DataWriter with the changes saved in the database
    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(unreceived_data);

    // Wait expecting not receiving data.
    ASSERT_EQ(0u, reader.block_for_all(std::chrono::seconds(1)));
}

TEST_P(PersistenceLargeData, PubSubAsReliablePubPersistentWithStaticDiscovery)
{
    char* value = nullptr;
    std::string TOPIC_RANDOM_NUMBER;
    std::string W_UNICAST_PORT_RANDOM_NUMBER_STR;
    std::string R_UNICAST_PORT_RANDOM_NUMBER_STR;
    std::string MULTICAST_PORT_RANDOM_NUMBER_STR;
    // Get environment variables.
    value = std::getenv("TOPIC_RANDOM_NUMBER");
    if (value != nullptr)
    {
        TOPIC_RANDOM_NUMBER = value;
    }
    else
    {
        TOPIC_RANDOM_NUMBER = "1";
    }
    value = std::getenv("W_UNICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        W_UNICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        W_UNICAST_PORT_RANDOM_NUMBER_STR = "7411";
    }
    int32_t W_UNICAST_PORT_RANDOM_NUMBER = stoi(W_UNICAST_PORT_RANDOM_NUMBER_STR);
    value = std::getenv("R_UNICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        R_UNICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        R_UNICAST_PORT_RANDOM_NUMBER_STR = "7421";
    }
    int32_t R_UNICAST_PORT_RANDOM_NUMBER = stoi(R_UNICAST_PORT_RANDOM_NUMBER_STR);
    value = std::getenv("MULTICAST_PORT_RANDOM_NUMBER");
    if (value != nullptr)
    {
        MULTICAST_PORT_RANDOM_NUMBER_STR = value;
    }
    else
    {
        MULTICAST_PORT_RANDOM_NUMBER_STR = "7400";
    }
    int32_t MULTICAST_PORT_RANDOM_NUMBER = stoi(MULTICAST_PORT_RANDOM_NUMBER_STR);

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    LocatorList_t WriterUnicastLocators;
    Locator_t LocatorBuffer;

    LocatorBuffer.kind = LOCATOR_KIND_UDPv4;
    LocatorBuffer.port = static_cast<uint16_t>(W_UNICAST_PORT_RANDOM_NUMBER);
    IPLocator::setIPv4(LocatorBuffer, 127, 0, 0, 1);
    WriterUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t WriterMulticastLocators;

    LocatorBuffer.port = static_cast<uint16_t>(MULTICAST_PORT_RANDOM_NUMBER);
    WriterMulticastLocators.push_back(LocatorBuffer);

    writer
            .history_kind(eprosima::fastrtps::KEEP_ALL_HISTORY_QOS)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .make_persistent(db_file_name(), "78.73.69.74.65.72.5f.70.65.72.73.5f|67.75.69.1")
            .static_discovery("file://PubSubWriterPersistence.xml")
            .unicastLocatorList(WriterUnicastLocators)
            .multicastLocatorList(WriterMulticastLocators)
            .setPublisherIDs(1, 2)
            .setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER)
            .userData({'V', 'G', 'W', 0x78, 0x73, 0x69, 0x74, 0x65, 0x72, 0x5f, 0x70, 0x65, 0x72, 0x73, 0x5f, 0x67,
                       0x75, 0x69})
            .init();

    ASSERT_TRUE(writer.isInitialized());

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    LocatorList_t ReaderUnicastLocators;

    LocatorBuffer.port = static_cast<uint16_t>(R_UNICAST_PORT_RANDOM_NUMBER);
    ReaderUnicastLocators.push_back(LocatorBuffer);

    LocatorList_t ReaderMulticastLocators;

    LocatorBuffer.port = static_cast<uint16_t>(MULTICAST_PORT_RANDOM_NUMBER);
    ReaderMulticastLocators.push_back(LocatorBuffer);

    reader
            .history_kind(eprosima::fastrtps::KEEP_LAST_HISTORY_QOS)
            .history_depth(10)
            .reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS)
            .make_persistent(db_file_name(), "78.73.69.74.65.72.5f.70.65.72.73.5f|67.75.69.3")
            .static_discovery("file://PubSubReaderPersistence.xml")
            .unicastLocatorList(ReaderUnicastLocators)
            .multicastLocatorList(ReaderMulticastLocators)
            .setSubscriberIDs(3, 4)
            .setManualTopicName(std::string("BlackBox_StaticDiscovery_") + TOPIC_RANDOM_NUMBER)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator();
    auto unreceived_data = data;

    // Send data
    writer.send(data);
    // All data should be sent
    ASSERT_TRUE(data.empty());

    reader.startReception(unreceived_data);

    // Wait expecting not receiving data.
    ASSERT_EQ(10u, reader.block_for_all(std::chrono::seconds(1)));


    // Destroy the DataWriter
    writer.destroy();
    reader.stopReception();
    // Load the persistent DataWriter with the changes saved in the database
    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    reader.startReception(unreceived_data);

    // Wait expecting not receiving data.
    ASSERT_EQ(0u, reader.block_for_all(std::chrono::seconds(1)));
}


#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(PersistenceLargeData,
        PersistenceLargeData,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<PersistenceLargeData::ParamType>& info)
        {
            switch (info.param)
            {
                case INTRAPROCESS:
                    return "Intraprocess";
                    break;
                case DATASHARING:
                    return "Datasharing";
                    break;
                case TRANSPORT:
                default:
                    return "Transport";
            }

        });
