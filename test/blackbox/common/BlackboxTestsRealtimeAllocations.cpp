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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class RealtimeAllocations : public testing::TestWithParam<communication_type>
{
public:

    void SetUp() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = true;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

    void TearDown() override
    {
        eprosima::fastdds::LibrarySettings library_settings;
        switch (GetParam())
        {
            case INTRAPROCESS:
                library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_OFF;
                eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);
                break;
            case DATASHARING:
                enable_datasharing = false;
                break;
            case TRANSPORT:
            default:
                break;
        }
    }

};

TEST_P(RealtimeAllocations, PubSubReliableWithLimitedSubscribers)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> reader2(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    reader
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .matched_readers_allocation(1u, 1u)
            .expect_no_allocs()
            .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second reader and wait until it discovers the writer
    reader2
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();
    ASSERT_TRUE(reader2.isInitialized());
    reader2.wait_discovery();

    // Start reception on both readers
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    reader2.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Second reader should not receive data
    ASSERT_EQ(reader2.getReceivedCount(), 0u);
}

TEST_P(RealtimeAllocations, AsyncPubSubReliableWithLimitedSubscribers)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> reader2(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    reader
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
            .asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE)
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .matched_readers_allocation(1u, 1u)
            .expect_no_allocs()
            .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second reader and wait until it discovers the writer
    reader2
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();
    ASSERT_TRUE(reader2.isInitialized());
    reader2.wait_discovery();

    // Start reception on both readers
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    reader2.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Second reader should not receive data
    ASSERT_EQ(reader2.getReceivedCount(), 0u);
}

TEST_P(RealtimeAllocations, PubSubBestEffortWithLimitedSubscribers)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> reader2(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    reader
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .matched_readers_allocation(1u, 1u)
            .expect_no_allocs()
            .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second reader and wait until it discovers the writer
    reader2
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();
    ASSERT_TRUE(reader2.isInitialized());
    reader2.wait_discovery();

    // Start reception on both readers
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    reader2.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Second reader should not receive data
    ASSERT_EQ(reader2.getReceivedCount(), 0u);
}

TEST_P(RealtimeAllocations, AsyncPubSubBestEffortWithLimitedSubscribers)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubReader<FixedSizedPubSubType> reader2(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);

    reader
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
            .asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE)
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .matched_readers_allocation(1u, 1u)
            .expect_no_allocs()
            .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second reader and wait until it discovers the writer
    reader2
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();
    ASSERT_TRUE(reader2.isInitialized());
    reader2.wait_discovery();

    // Start reception on both readers
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    reader2.startReception(data);

    // Send data
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Second reader should not receive data
    ASSERT_EQ(reader2.getReceivedCount(), 0u);
}

TEST_P(RealtimeAllocations, PubSubReliableWithLimitedPublishers)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer2(TEST_TOPIC_NAME);

    reader
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .matched_writers_allocation(1u, 1u)
            .expect_no_allocs()
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second writer and wait until it discovers the reader
    writer2
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();
    ASSERT_TRUE(writer2.isInitialized());
    writer2.wait_discovery();

    // Send data from first writer
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Send data from second writer
    data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer2.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    reader.block_for_all(std::chrono::seconds(2));

    // Reader should not receive data
    ASSERT_EQ(reader.getReceivedCount(), 0u);
}

TEST_P(RealtimeAllocations, AsyncPubSubReliableWithLimitedPublishers)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer2(TEST_TOPIC_NAME);

    reader
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .matched_writers_allocation(1u, 1u)
            .expect_no_allocs()
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
            .asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE)
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second writer and wait until it discovers the reader
    writer2
            .asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE)
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();
    ASSERT_TRUE(writer2.isInitialized());
    writer2.wait_discovery();

    // Send data from first writer
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Send data from second writer
    data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer2.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    reader.block_for_all(std::chrono::seconds(2));

    // Reader should not receive data
    ASSERT_EQ(reader.getReceivedCount(), 0u);
}

TEST_P(RealtimeAllocations, PubSubBestEffortWithLimitedPublishers)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer2(TEST_TOPIC_NAME);

    reader
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .matched_writers_allocation(1u, 1u)
            .expect_no_allocs()
            .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second writer and wait until it discovers the reader
    writer2
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();
    ASSERT_TRUE(writer2.isInitialized());
    writer2.wait_discovery();

    // Send data from first writer
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Send data from second writer
    data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer2.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    reader.block_for_all(std::chrono::seconds(2));

    // Reader should not receive data
    ASSERT_EQ(reader.getReceivedCount(), 0u);
}

TEST_P(RealtimeAllocations, AsyncPubSubBestEffortWithLimitedPublishers)
{
    PubSubReader<FixedSizedPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer(TEST_TOPIC_NAME);
    PubSubWriter<FixedSizedPubSubType> writer2(TEST_TOPIC_NAME);

    reader
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .matched_writers_allocation(1u, 1u)
            .expect_no_allocs()
            .init();

    ASSERT_TRUE(reader.isInitialized());

    writer
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE)
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();

    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Initialize second writer and wait until it discovers the reader
    writer2
            .reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .asynchronously(eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE)
            .history_depth(10)
            .resource_limits_max_samples(10).resource_limits_allocated_samples(10)
            .init();
    ASSERT_TRUE(writer2.isInitialized());
    writer2.wait_discovery();

    // Send data from first writer
    auto data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_all();

    // Send data from second writer
    data = default_fixed_sized_data_generator();
    reader.startReception(data);
    writer2.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());

    reader.block_for_all(std::chrono::seconds(2));

    // Reader should not receive data
    ASSERT_EQ(reader.getReceivedCount(), 0u);
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(RealtimeAllocations,
        RealtimeAllocations,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<RealtimeAllocations::ParamType>& info)
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
