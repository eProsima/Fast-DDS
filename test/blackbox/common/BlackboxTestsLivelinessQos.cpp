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

#include <string>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <gtest/gtest.h>

#include "BlackboxTests.hpp"
#include "PubSubParticipant.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "PubSubWriterReader.hpp"

using namespace eprosima::fastdds;
using namespace eprosima::fastdds::rtps;

enum communication_type
{
    TRANSPORT,
    INTRAPROCESS,
    DATASHARING
};

class LivelinessQos : public testing::TestWithParam<communication_type>
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

    template<typename WriterType, typename ReaderType>
    void config_pdp(
            WriterType& writer,
            ReaderType& reader)
    {
        constexpr unsigned int participant_announcement_period_ms = 50000;

        writer.lease_duration(
            participant_announcement_period_ms * 3e-3, participant_announcement_period_ms * 1e-3);
        reader.lease_duration(
            participant_announcement_period_ms * 3e-3, participant_announcement_period_ms * 1e-3);
    }

};

//! Tests that when kind is automatic liveliness is never lost, even if the writer never sends data
TEST_P(LivelinessQos, Liveliness_Automatic_Reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Liveliness lease duration and announcement period
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    std::this_thread::sleep_for(std::chrono::milliseconds(lease_duration_ms * 2));

    // When using automatic kind, liveliness on both publisher and subscriber should never be lost
    // It would only be lost if the publishing application crashed, which can't be reproduced in this test
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);

    // Remove and re-create publisher, test liveliness on subscriber and the new publisher.
    writer.removePublisher();
    ASSERT_FALSE(writer.isInitialized());
    reader.wait_writer_undiscovery();
    writer.createPublisher();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();
    std::this_thread::sleep_for(std::chrono::milliseconds(lease_duration_ms * 2));

    // Liveliness is recovered as there is a new publisher.
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 2u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
}

//! Same as above using best-effort reliability
TEST_P(LivelinessQos, Liveliness_Automatic_BestEffort)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Liveliness lease duration and announcement period
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    std::this_thread::sleep_for(std::chrono::milliseconds(lease_duration_ms * 2));

    // When using automatic kind, liveliness on both publisher and subscriber should never be lost
    // It would only be lost if the publishing application crashed, which can't be reproduced in this test
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);

    // Remove and re-create publisher, test liveliness on subscriber and the new publisher.
    writer.removePublisher();
    ASSERT_FALSE(writer.isInitialized());
    reader.wait_writer_undiscovery();
    writer.createPublisher();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();
    std::this_thread::sleep_for(std::chrono::milliseconds(lease_duration_ms * 2));

    // Liveliness is recovered as there is a new publisher.
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 2u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
}

//! Tests that liveliness is lost and recovered as expected, with the following paramters
//! Writer is reliable, and MANUAL_BY_PARTICIPANT
//! Reader is reliable, and MANUAL_BY_PARTICIPANT
TEST_P(LivelinessQos, ShortLiveliness_ManualByParticipant_Reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 1500;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    unsigned int count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.wait_liveliness_recovered(count);
        reader.wait_liveliness_lost(count);
        writer.wait_liveliness_lost(count);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        reader.wait_liveliness_recovered(count + num_samples + 1);
        reader.wait_liveliness_lost(count + num_samples + 1);
        writer.wait_liveliness_lost(count + num_samples + 1);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);

    // Remove and re-create publisher, test liveliness on subscriber and the new publisher.
    writer.removePublisher();
    ASSERT_FALSE(writer.isInitialized());
    reader.wait_writer_undiscovery();
    writer.createPublisher();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        reader.wait_liveliness_recovered(count + num_samples * 2 + 1);
        reader.wait_liveliness_lost(count + num_samples * 2 + 1);
        writer.wait_liveliness_lost(count + num_samples * 2 + 1);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 3);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 3);
}

//! Tests that liveliness is lost and recovered as expected, with the following paramters
//! Writer is best-effort, and MANUAL_BY_PARTICIPANT
//! Reader is best-effort, and MANUAL_BY_PARTICIPANT
TEST_P(LivelinessQos, ShortLiveliness_ManualByParticipant_BestEffort)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    unsigned int count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.wait_liveliness_recovered(count);
        reader.wait_liveliness_lost(count);
        writer.wait_liveliness_lost(count);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        reader.wait_liveliness_recovered(count + num_samples + 1);
        reader.wait_liveliness_lost(count + num_samples + 1);
        writer.wait_liveliness_lost(count + num_samples + 1);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);

    // Remove and re-create publisher, test liveliness on subscriber and the new publisher.
    writer.removePublisher();
    ASSERT_FALSE(writer.isInitialized());
    reader.wait_writer_undiscovery();
    writer.createPublisher();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        reader.wait_liveliness_recovered(count + num_samples * 2 + 1);
        reader.wait_liveliness_lost(count + num_samples * 2 + 1);
        writer.wait_liveliness_lost(count + num_samples * 2 + 1);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 3);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 3);
}

//! Tests that liveliness is not lost when lease duration is big, with the following paramters
//! Writer is best-effort, and MANUAL_BY_PARTICIPANT
//! Reader is best-effort, and MANUAL_BY_PARTICIPANT
TEST_P(LivelinessQos, LongLiveliness_ManualByParticipant_Reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Lease duration, announcement period, and sleep time, in milliseconds
    unsigned int sleep_ms = 10;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.block_for_at_least(count);
        reader.wait_liveliness_recovered();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    // Liveliness shouldn't have been lost
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    // Remove and re-create publisher, test liveliness on subscriber and the new publisher.
    writer.removePublisher();
    ASSERT_FALSE(writer.isInitialized());
    reader.wait_writer_undiscovery();
    writer.createPublisher();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    // Liveliness shouldn't have been lost
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 2u);
}

//! Tests that liveliness is not lost when lease duration is big, with the following paramters
//! Writer is best-effort, and MANUAL_BY_PARTICIPANT
//! Reader is best-effort, and MANUAL_BY_PARTICIPANT
TEST_P(LivelinessQos, LongLiveliness_ManualByParticipant_BestEffort)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Lease duration, announcement period, and sleep time, in milliseconds
    unsigned int sleep_ms = 10;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.block_for_at_least(count);
        reader.wait_liveliness_recovered();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    // Liveliness shouldn't have been lost
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    // Remove and re-create publisher, test liveliness on subscriber and the new publisher.
    writer.removePublisher();
    ASSERT_FALSE(writer.isInitialized());
    reader.wait_writer_undiscovery();
    writer.createPublisher();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    // Liveliness shouldn't have been lost
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 2u);
}

//! Tests that liveliness is lost and recovered as expected, with the following paramters
//! Writer is reliable, and MANUAL_BY_TOPIC
//! Reader is reliable, and MANUAL_BY_TOPIC
TEST_P(LivelinessQos, ShortLiveliness_ManualByTopic_Reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    unsigned int count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.wait_liveliness_recovered(count);
        reader.wait_liveliness_lost(count);
        writer.wait_liveliness_lost(count);
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        reader.wait_liveliness_recovered(count + num_samples + 1);
        reader.wait_liveliness_lost(count + num_samples + 1);
        writer.wait_liveliness_lost(count + num_samples + 1);
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);

    // Remove and re-create publisher, test liveliness on subscriber and the new publisher.
    writer.removePublisher();
    ASSERT_FALSE(writer.isInitialized());
    reader.wait_writer_undiscovery();
    writer.createPublisher();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        reader.wait_liveliness_recovered(count + num_samples * 2 + 1);
        reader.wait_liveliness_lost(count + num_samples * 2 + 1);
        writer.wait_liveliness_lost(count + num_samples * 2 + 1);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 3);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 3);
}

//! Tests that liveliness is lost and recovered, with the following paramters
//! Writer is best-effort, and MANUAL_BY_TOPIC
//! Reader is best-effort, and MANUAL_BY_TOPIC
TEST_P(LivelinessQos, ShortLiveliness_ManualByTopic_BestEffort)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    unsigned int count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.wait_liveliness_recovered(count);
        reader.wait_liveliness_lost(count);
        writer.wait_liveliness_lost(count);
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        writer.wait_liveliness_lost(count + num_samples + 1);
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    // Note that in MANUAL_BY_TOPIC liveliness, the assert_liveliness() method relies on sending a heartbeat
    // However best-effort writers don't send heartbeats, so the reader in this case will never get notified
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    // Remove and re-create publisher, test liveliness on subscriber and the new publisher.
    writer.removePublisher();
    ASSERT_FALSE(writer.isInitialized());
    reader.wait_writer_undiscovery();
    writer.createPublisher();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        writer.wait_liveliness_lost(count + num_samples * 2 + 1);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);
}

//! Tests liveliness is not lost when lease duration is big, with the following paramters
//! Writer is reliable, and MANUAL_BY_TOPIC
//! Reader is reliable, and MANUAL_BY_TOPIC
TEST_P(LivelinessQos, LongLiveliness_ManualByTopic_Reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration, announcement period, and sleep time, in milliseconds
    unsigned int sleep_ms = 10;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.block_for_at_least(count);
        reader.wait_liveliness_recovered();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    // Liveliness shouldn't have been lost
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    // Remove and re-create publisher, test liveliness on subscriber and the new publisher.
    writer.removePublisher();
    ASSERT_FALSE(writer.isInitialized());
    reader.wait_writer_undiscovery();
    writer.createPublisher();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    // Liveliness shouldn't have been lost
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 2u);
}

//! Tests liveliness is not lost when lease duration is big, with the following paramters
//! Writer is best-effort, and MANUAL_BY_TOPIC
//! Reader is best-effort, and MANUAL_BY_TOPIC
TEST_P(LivelinessQos, LongLiveliness_ManualByTopic_BestEffort)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration, announcement period, and sleep time, in seconds
    unsigned int sleep_ms = 10;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.block_for_at_least(count);
        reader.wait_liveliness_recovered();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    // Liveliness shouldn't have been lost
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);

    // Remove and re-create publisher, test liveliness on subscriber and the new publisher.
    writer.removePublisher();
    ASSERT_FALSE(writer.isInitialized());
    reader.wait_writer_undiscovery();
    writer.createPublisher();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    // Liveliness shouldn't have been lost
    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Tests liveliness is not lost when lease duration is big, with the following parameters
//! Writer is reliable, liveliness is manual by participant
//! Reader is reliable, liveliness is automatic
TEST_P(LivelinessQos, LongLiveliness_ManualByParticipant_Automatic_Reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Write rate in milliseconds and number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration, announcement period, and sleep time, in millseconds
    unsigned int sleep_ms = 10;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.block_for_at_least(count);
        reader.wait_liveliness_recovered();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Tests liveliness is lost and recovered as expected, with the following parameters
//! Writer is reliable, liveliness is manual by participant
//! Reader is reliable, liveliness is automatic
TEST_P(LivelinessQos, ShortLiveliness_ManualByParticipant_Automatic_Reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    unsigned int count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.wait_liveliness_recovered(count);
        reader.wait_liveliness_lost(count);
        writer.wait_liveliness_lost(count);
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        reader.wait_liveliness_recovered(num_samples + count + 1);
        reader.wait_liveliness_lost(num_samples + count + 1);
        writer.wait_liveliness_lost(num_samples + count + 1);
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);
}

//! Tests liveliness is not lost when lease duration is big, with the following parameters
//! Writer is best-effort, liveliness is manual by participant
//! Reader is best-effort, liveliness is automatic
TEST_P(LivelinessQos, LongLiveliness_ManualByParticipant_Automatic_BestEffort)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration, announcement period, and sleep time, in milliseconds
    unsigned int sleep_ms = 10;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    size_t count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.block_for_at_least(count);
        reader.wait_liveliness_recovered();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
    }

    EXPECT_EQ(writer.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_lost(), 0u);
    EXPECT_EQ(reader.times_liveliness_recovered(), 1u);
}

//! Tests liveliness is lost and recovered as expected, with the following parameters
//! Writer is best-effort, liveliness is manual by participant
//! Reader is best-effort, liveliness is automatic
//! Liveliness is short in comparison to the writer write/assert rate
TEST_P(LivelinessQos, ShortLiveliness_ManualByParticipant_Automatic_BestEffort)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    unsigned int count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.wait_liveliness_recovered(count);
        reader.wait_liveliness_lost(count);
        writer.wait_liveliness_lost(count);
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);

    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        reader.wait_liveliness_recovered(num_samples + count + 1);
        reader.wait_liveliness_lost(num_samples + count + 1);
        writer.wait_liveliness_lost(num_samples + count + 1);
    }
    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);
}

//! Tests liveliness is lost and recovered, with the following parameters
//! Writer is reliable, and uses manual by topic liveliness kind
//! Reader is reliable, and uses automatic liveliness kind
TEST_P(LivelinessQos, ShortLiveliness_ManualByTopic_Automatic_Reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    // Write some samples
    unsigned int count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.wait_liveliness_recovered(count);
        reader.wait_liveliness_lost(count);
        writer.wait_liveliness_lost(count);
    }
    // Now use assert_liveliness() method
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        reader.wait_liveliness_recovered(count + num_samples + 1);
        reader.wait_liveliness_lost(count + num_samples + 1);
        writer.wait_liveliness_lost(count + num_samples + 1);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);
}

//! Tests liveliness is lost and recovered, with the following parameters
//! Writer is best-effort, and uses manual by topic liveliness kind
//! Reader is best-effort, and uses automatic liveliness kind
TEST_P(LivelinessQos, ShortLiveliness_ManualByTopic_Automatic_BestEffort)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of times to assert liveliness
    unsigned int num_samples = 2;

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    // Write some samples
    unsigned int count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.wait_liveliness_recovered(count);
        reader.wait_liveliness_lost(count);
        writer.wait_liveliness_lost(count);
    }
    // Now use assert_liveliness() method
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        writer.wait_liveliness_lost(count + num_samples + 1);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    // As best-effort readers do not process heartbeats the expected number of times liveliness was lost
    // and recovered corresponds to the bit in the test when we sent samples (not when we asserted liveliness)
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);
}

//! Tests liveliness is lost and recovered as expected, with the following parameters
//! Writer is reliable, and uses manual by topic liveliness kind
//! Reader is reliable, and uses manual by participant liveliness kind
TEST_P(LivelinessQos, ShortLiveliness_ManualByTopic_ManualByParticipant_Reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    // Write some samples
    unsigned int count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.wait_liveliness_recovered(count);
        reader.wait_liveliness_lost(count);
        writer.wait_liveliness_lost(count);
    }
    // Now use assert_liveliness() method
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        reader.wait_liveliness_recovered(count + num_samples + 1);
        reader.wait_liveliness_lost(count + num_samples + 1);
        writer.wait_liveliness_lost(count + num_samples + 1);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples * 2);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples * 2);
}

//! Tests liveliness is lost and recovered as expected, with the following parameters
//! Writer is best-effort, and uses manual by topic liveliness kind
//! Reader is best-effort, and uses manual by participant liveliness kind
TEST_P(LivelinessQos, ShortLiveliness_ManualByTopic_ManualByParticipant_BestEffort)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Write rate in milliseconds and number of samples to write
    unsigned int num_samples = 2;

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    reader.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_helloworld_data_generator(num_samples);
    reader.startReception(data);

    // Write some samples
    unsigned int count = 0;
    for (auto data_sample : data)
    {
        ++count;
        writer.send_sample(data_sample);
        reader.wait_liveliness_recovered(count);
        reader.wait_liveliness_lost(count);
        writer.wait_liveliness_lost(count);
    }
    // Now use assert_liveliness() method
    for (count = 0; count < num_samples; count++)
    {
        writer.assert_liveliness();
        writer.wait_liveliness_lost(num_samples + count + 1);
    }

    EXPECT_EQ(writer.times_liveliness_lost(), num_samples * 2);
    // Note that, as best-effor readers do not proccess heartbeats and assert_liveliness() relies on sending a
    // heartbeat to assess liveliness, the expected number of times liveliness was lost and recovered
    // corresponds only to the bit in the test when the writer wrote samples
    EXPECT_EQ(reader.times_liveliness_lost(), num_samples);
    EXPECT_EQ(reader.times_liveliness_recovered(), num_samples);
}

//! Tests liveliness in the following scenario
//! A participant with two publishers (AUTOMATIC and MANUAL_BY_PARTICIPANT) and a single topic
//! A participant with one subscriber (AUTOMATIC)
TEST_P(LivelinessQos, TwoWriters_OneReader_ManualByParticipant)
{
    unsigned int num_pub = 2;
    unsigned int num_sub = 1;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    // Publishers
    PubSubParticipant<HelloWorldPubSubType> publishers(num_pub, 0u, 2u, 0u);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(0u));
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(1u));

    // Subscribers
    PubSubParticipant<HelloWorldPubSubType> subscribers(0u, num_sub, 0u, 2u);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(0u));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();
    subscribers.sub_wait_liveliness_recovered(2u);

    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 0u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 2u);
    // Note that from the subscriber point of view both writers recovered liveliness, even if the
    // MANUAL_BY_PARTICIPANT one didn't assert liveliness explicitly. This is the expected
    // behaviour according to the RTPS standard, section 2.2.3.11 LIVELINESS
}

//! Tests liveliness in the following scenario
//! A participant with two publishers and two topics
//! A participant with two subscribers and two topics
//! Manual by participant liveliness
//! Only one publisher asserts liveliness manually
TEST_P(LivelinessQos, TwoWriters_TwoReaders_ManualByParticipant)
{
    unsigned int num_pub = 2;
    unsigned int num_sub = 2;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    // Publishers
    PubSubParticipant<HelloWorldPubSubType> publishers(num_pub, 0u, num_sub, 0u);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME + "1")
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(0u));
    publishers.pub_topic_name(TEST_TOPIC_NAME + "2")
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(1u));

    // Subscribers
    PubSubParticipant<HelloWorldPubSubType> subscribers(0u, num_sub, 0u, num_pub);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME + "1")
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(0u));
    subscribers.sub_topic_name(TEST_TOPIC_NAME + "2")
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(1u));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();

    // Make the first publisher assert its liveliness, the other should be asserted too by the QoS
    // as liveliness kind is manual by participant
    publishers.assert_liveliness(0u);
    subscribers.sub_wait_liveliness_recovered(num_pub);
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 0u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), num_pub);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), 0u);

    subscribers.sub_wait_liveliness_lost(num_pub);
    publishers.pub_wait_liveliness_lost(num_pub);
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), num_pub);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), num_pub);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), num_pub);
}

//! Tests liveliness in the same scenario as above but using manual by topic liveliness
//! A participant with two publishers and two topics
//! A participant with two subscribers and two topics
//! Manual by topic liveliness
//! Only one publisher asserts liveliness manually
TEST_P(LivelinessQos, TwoWriters_TwoReaders_ManualByTopic)
{
    unsigned int num_pub = 2;
    unsigned int num_sub = 2;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    // Publishers
    PubSubParticipant<HelloWorldPubSubType> publishers(num_pub, 0u, num_sub, 0u);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME + "1")
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(0u));
    publishers.pub_topic_name(TEST_TOPIC_NAME + "2")
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(1u));

    // Subscribers
    PubSubParticipant<HelloWorldPubSubType> subscribers(0u, num_sub, 0u, num_pub);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME + "1")
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(0u));
    subscribers.sub_topic_name(TEST_TOPIC_NAME + "2")
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(1u));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();

    // Make first publisher assert its liveliness and check that only tne
    // first subscriber detected liveliness recovery
    publishers.assert_liveliness(0u);
    subscribers.sub_wait_liveliness_recovered(1u);
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 0u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 1u);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), 0u);

    // Wait until the liveliness is lost and check that:
    // liveliness was recovered only once, i.e. the second subscriber never detects a liveliness recovery
    // liveliness was lost only once, i.e. only the first subscriber detects a liveliness loss
    subscribers.sub_wait_liveliness_lost(1u);
    publishers.pub_wait_liveliness_lost(1u);
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 1u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 1u);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), 1u);
}

//! Tests liveliness in the following scenario
//! A participant with two publishers with different liveliness kinds
//! A participant with two subscribers with different liveliness kinds
TEST_P(LivelinessQos, TwoWriters_TwoReaders)
{
    unsigned int num_pub = 2;
    unsigned int num_sub = 2;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    // Publishers
    PubSubParticipant<HelloWorldPubSubType> publishers(num_pub, 0u, 3u, 0u);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(0u));
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(1u));

    // Subscribers
    PubSubParticipant<HelloWorldPubSubType> subscribers(0u, num_sub, 0u, 3u);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(0u));
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(1u));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();

    publishers.assert_liveliness(1u);
    subscribers.sub_wait_liveliness_recovered(3u);

    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 3u);

    publishers.pub_wait_liveliness_lost(1u);
    subscribers.sub_wait_liveliness_lost(1u);
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 1u);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), 1u);
}

//! Tests liveliness in the same scenario as above but using manual by topic liveliness
//! A participant with three publishers with different liveliness kinds
//! A participant with three subscribers with different liveliness kinds
TEST_P(LivelinessQos, ThreeWriters_ThreeReaders)
{
    unsigned int num_pub = 3;
    unsigned int num_sub = 3;
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    // Publishers
    PubSubParticipant<HelloWorldPubSubType> publishers(num_pub, 0u, 6u, 0u);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(0u));
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(1u));
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(2u));

    // Subscribers
    PubSubParticipant<HelloWorldPubSubType> subscribers(0u, num_sub, 0u, 6u);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(0u));
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(1u));
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(subscribers.init_subscriber(2u));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();

    // From the point of view of the AUTOMATIC reader, the three writers will have recovered liveliness
    subscribers.sub_wait_liveliness_recovered(3u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 3u);

    // The manual by participant writer asserts liveliness
    // The manual by participant reader will consider that both the manual by participant and manual by topic
    // writers have recovered liveliness
    publishers.assert_liveliness(1u);
    subscribers.sub_wait_liveliness_recovered(5u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 5u);

    // The manual by topic publisher asserts liveliness
    // The manual by topic reader will detect that a new writer has recovered liveliness
    publishers.assert_liveliness(2u);
    subscribers.sub_wait_liveliness_recovered(6u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 6u);

    // Wait so that the manual by participant and manual by topic writers lose liveliness
    // The manual by participant subscriber will detect that two writers lost liveliness
    // The manual by topic subscriber will detect that one writer lost liveliness
    // This means that the subscribing participant will see that liveliness was lost three times
    publishers.pub_wait_liveliness_lost(2u);
    subscribers.sub_wait_liveliness_lost(3u);
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 2u);
    EXPECT_EQ(subscribers.sub_times_liveliness_lost(), 3u);
}

//! Tests the case where a writer matched to two readers changes QoS and stays matched to only one reader
TEST_P(LivelinessQos, UnmatchedWriter)
{
    unsigned int num_pub = 1;
    unsigned int num_sub = 2;
    unsigned int lease_duration_ms = 500;
    unsigned int announcement_period_ms = 250;

    // Publishers
    PubSubParticipant<HelloWorldPubSubType> publishers(num_pub, 0u, 2u, 0u);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .pub_deadline_period(0.15);
    ASSERT_TRUE(publishers.init_publisher(0u));

    // Subscribers
    PubSubParticipant<HelloWorldPubSubType> subscribers(0u, num_sub, 0u, 2u);
    ASSERT_TRUE(subscribers.init_participant());
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .sub_deadline_period(0.5);
    ASSERT_TRUE(subscribers.init_subscriber(0u));
    subscribers.sub_topic_name(TEST_TOPIC_NAME)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .sub_deadline_period(1.5);
    ASSERT_TRUE(subscribers.init_subscriber(1u));

    publishers.pub_wait_discovery();
    subscribers.sub_wait_discovery();

    // Change deadline period of the first subscriber so that it no longer matches with the publisher
    subscribers.sub_update_deadline_period(0.10, 0u);

    publishers.assert_liveliness(0u);
    subscribers.sub_wait_liveliness_recovered(1u);
    EXPECT_EQ(subscribers.sub_times_liveliness_recovered(), 1u);
}

//! Tests liveliness structs when a writer changes from being alive to losing liveliness
//! Writer is reliable, and MANUAL_BY_TOPIC
//! Reader is reliable, and MANUAL_BY_TOPIC
TEST_P(LivelinessQos, LivelinessChangedStatus_Alive_NotAlive)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 100;
    unsigned int announcement_period_ms = 10;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    eprosima::fastdds::dds::LivelinessChangedStatus status = reader.liveliness_changed_status();
    EXPECT_EQ(status.alive_count, 0);
    EXPECT_EQ(status.alive_count_change, 0);
    EXPECT_EQ(status.not_alive_count, 0);
    EXPECT_EQ(status.not_alive_count_change, 0);

    // Assert liveliness
    writer.assert_liveliness();
    reader.wait_liveliness_recovered();

    status = reader.liveliness_changed_status();
    EXPECT_EQ(status.alive_count, 1);
    EXPECT_EQ(status.alive_count_change, 1);
    EXPECT_EQ(status.not_alive_count, 0);
    EXPECT_EQ(status.not_alive_count_change, 0);

    // Wait until liveliness is lost
    reader.wait_liveliness_lost();

    status = reader.liveliness_changed_status();
    EXPECT_EQ(status.alive_count, 0);
    EXPECT_EQ(status.alive_count_change, -1);
    EXPECT_EQ(status.not_alive_count, 1);
    EXPECT_EQ(status.not_alive_count_change, 1);
}

//! Tests liveliness structs when an alive writer is unmatched
//! Writer is reliable, and MANUAL_BY_TOPIC
//! Reader is reliable, and MANUAL_BY_TOPIC
TEST_P(LivelinessQos, LivelinessChangedStatus_Alive_Unmatched)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 100;
    unsigned int announcement_period_ms = 10;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .deadline_period(0.15)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .deadline_period(0.15)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Assert liveliness
    writer.assert_liveliness();
    reader.wait_liveliness_recovered();

    eprosima::fastdds::dds::LivelinessChangedStatus status = reader.liveliness_changed_status();
    EXPECT_EQ(status.alive_count, 1);
    EXPECT_EQ(status.alive_count_change, 1);
    EXPECT_EQ(status.not_alive_count, 0);
    EXPECT_EQ(status.not_alive_count_change, 0);

    // Now unmatch by changing the deadline period of the reader
    reader.update_deadline_period(0.10);

    status = reader.liveliness_changed_status();
    EXPECT_EQ(status.alive_count, 0);
    EXPECT_EQ(status.alive_count_change, -1);
    EXPECT_EQ(status.not_alive_count, 0);
    EXPECT_EQ(status.not_alive_count_change, 0);
}

//! Tests liveliness structs when a not alive writer is unmatched
//! Writer is reliable, and MANUAL_BY_TOPIC
//! Reader is reliable, and MANUAL_BY_TOPIC
TEST_P(LivelinessQos, LivelinessChangedStatus_NotAlive_Unmatched)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    config_pdp(writer, reader);

    // Liveliness lease duration and announcement period, in milliseconds
    unsigned int lease_duration_ms = 100;
    unsigned int announcement_period_ms = 10;

    reader.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .deadline_period(0.15)
            .init();
    writer.reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS)
            .liveliness_announcement_period(announcement_period_ms * 1e-3)
            .liveliness_lease_duration(lease_duration_ms * 1e-3)
            .deadline_period(0.15)
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Assert liveliness
    writer.assert_liveliness();
    reader.wait_liveliness_recovered();

    eprosima::fastdds::dds::LivelinessChangedStatus status = reader.liveliness_changed_status();
    EXPECT_EQ(status.alive_count, 1);
    EXPECT_EQ(status.alive_count_change, 1);
    EXPECT_EQ(status.not_alive_count, 0);
    EXPECT_EQ(status.not_alive_count_change, 0);

    // Wait for liveliness lost
    reader.wait_liveliness_lost();

    // Now unmatch by changing the deadline period of the reader
    reader.update_deadline_period(0.10);

    status = reader.liveliness_changed_status();
    EXPECT_EQ(status.alive_count, 0);
    EXPECT_EQ(status.alive_count_change, 0);
    EXPECT_EQ(status.not_alive_count, 0);
    EXPECT_EQ(status.not_alive_count_change, -1);
}


//! Tests the assert_liveliness on the participant
//! A participant with three publishers, two MANUAL_BY_PARTICIPANT liveliness, one MANUAL_BY_TOPIC
TEST_P(LivelinessQos, AssertLivelinessParticipant)
{
    unsigned int num_pub = 3;
    unsigned int lease_duration_ms = 100;
    unsigned int announcement_period_ms = 10;

    // Publishers
    PubSubParticipant<HelloWorldPubSubType> publishers(num_pub, 0u, 0u, 0u);
    ASSERT_TRUE(publishers.init_participant());
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(0u));
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(1u));
    publishers.pub_topic_name(TEST_TOPIC_NAME)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS);
    ASSERT_TRUE(publishers.init_publisher(2u));

    // Assert liveliness
    publishers.assert_liveliness_participant();

    // Wait for alive publishers (only the two MANUAL_BY_PARTICIPANT publishers should be alive) to lose liveliness
    publishers.pub_wait_liveliness_lost(2u);

    // Only the two MANUAL_BY_PARTICIPANT publishers will have lost liveliness, as the
    // MANUAL_BY_TOPIC one was never asserted
    EXPECT_EQ(publishers.pub_times_liveliness_lost(), 2u);
}

//! Tests associated with an ABBA deadlock discovered between PDP mutexes on two different participants.
//! The deadlock scenario involved:
//! + Intraprocess
//! + WLP assertions due to MANUAL_BY_PARTICIPANT set up
//! + Both participants' event thread trigger the liveliness assertion simultaneously
TEST(LivelinessTests, Detect_Deadlock_ManualByParticipant_Intraprocess)
{
    // Set up intraprocess
    eprosima::fastdds::LibrarySettings library_settings;
    library_settings.intraprocess_delivery = eprosima::fastdds::IntraprocessDeliveryType::INTRAPROCESS_FULL;
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_library_settings(library_settings);

    // Create two participants
    PubSubWriterReader<HelloWorldPubSubType> participantA(TEST_TOPIC_NAME), participantB(TEST_TOPIC_NAME);

    // Set up MANUAL_BY_PARTICIPANT liveliness and make it fast
    unsigned int lease_duration_ms = 1000;
    unsigned int announcement_period_ms = 1;

    participantA.pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .sub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .sub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    participantB.pub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .sub_liveliness_kind(eprosima::fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
            .pub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .sub_liveliness_announcement_period(announcement_period_ms * 1e-3)
            .pub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .sub_liveliness_lease_duration(lease_duration_ms * 1e-3)
            .init();

    ASSERT_TRUE(participantA.isInitialized());
    ASSERT_TRUE(participantB.isInitialized());

    // Wait for discovery.
    participantA.wait_discovery();
    participantB.wait_discovery();

    // The DataWriter is alive
    participantA.assert_liveliness();
    participantB.assert_liveliness();

    // Wait a second expecting a deadlock
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Test failure is due to timeout
}

// Regression test of Refs #20584, github issue #4373
TEST(LivelinessTests, Reader_Successfully_Asserts_Liveliness_on_a_Disconnected_Writer)
{
    // Create a test_transport to simulate a network shutdown (Ctrl+C)
    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();

    // Create two writer participants
    PubSubWriter<HelloWorldPubSubType> writer_1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer_2(TEST_TOPIC_NAME + "2");

    // Create a reader participant containing 2 subscribers and readers
    PubSubParticipant<HelloWorldPubSubType> reader(0, 2, 0, 2);

    reader.init_participant();
    // Define the reader's lease duration in 1.6 secs
    reader.sub_liveliness_lease_duration(eprosima::fastdds::dds::Time_t(1, 600000000));

    // Create Subscribers and readers, one for each writer
    reader.sub_topic_name(TEST_TOPIC_NAME);
    reader.init_subscriber(0);
    reader.sub_topic_name(TEST_TOPIC_NAME + "2");
    reader.init_subscriber(1);

    // Create writers
    writer_1.disable_builtin_transport()
            .lease_duration(eprosima::fastdds::dds::c_TimeInfinite, 1)
            .add_user_transport_to_pparams(test_transport)
            .liveliness_lease_duration(eprosima::fastdds::dds::Time_t(1, 0))
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_announcement_period(eprosima::fastdds::dds::Time_t(0, 500000000))
            .init();

    writer_2.lease_duration(eprosima::fastdds::dds::c_TimeInfinite, 1)
            .liveliness_lease_duration(eprosima::fastdds::dds::Time_t(1, 0))
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_announcement_period(eprosima::fastdds::dds::Time_t(0, 500000000))
            .init();

    // Wait for discovery to occur. Liveliness should be recovered twice,
    // one per matched reader.
    reader.sub_wait_liveliness_recovered(2);

    // Simulate a Ctrl+C in one of the writers
    test_transport->test_transport_options->test_UDPv4Transport_ShutdownAllNetwork = true;

    // After 1.6 secs, we should receive a on_liveliness_changed(status lost)
    // in the TEST_TOPIC_NAME reader that was matched with the disconnected writer_1
    ASSERT_EQ(reader.sub_wait_liveliness_lost_for(1, std::chrono::seconds(4)), 1u);
}

// Regression test of Refs #21065, github issue #4610
TEST(LivelinessTests, correct_liveliness_state_one_writer_multiple_readers)
{
    uint8_t num_readers = 2;

    // Create one writer participant
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    // Create a reader participant containing 2 readers
    PubSubParticipant<HelloWorldPubSubType> reader(0, num_readers, 0, num_readers);

    reader.init_participant();
    // Define the reader's lease duration in 1.6 secs
    reader.sub_liveliness_lease_duration(eprosima::fastdds::dds::Time_t(1, 600000000));
    // Both readers on the same topic
    reader.sub_topic_name(TEST_TOPIC_NAME);

    for (size_t i = 0; i < num_readers; i++)
    {
        // Create Subscribers and readers, one for each writer
        reader.init_subscriber(static_cast<unsigned int>(i));
    }

    // Create writers
    writer.lease_duration(eprosima::fastdds::dds::c_TimeInfinite, 1)
            .liveliness_lease_duration(eprosima::fastdds::dds::Time_t(1, 0))
            .liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_announcement_period(eprosima::fastdds::dds::Time_t(0, 500000000))
            .init();

    // Wait for discovery to occur. Liveliness should be recovered twice,
    // one per matched reader.
    reader.sub_wait_liveliness_recovered(2);

    // Destroy the writer
    writer.destroy();

    // After 1.6 secs, we should receive a on_liveliness_changed(status lost) on the two readers
    ASSERT_EQ(reader.sub_wait_liveliness_lost_for(2, std::chrono::seconds(4)), 2u);
}

/**
 * This is a regression test for redmine issue #21189.
 *
 * The test ensures that liveliness changed status is not affected by writers on a topic different from
 * the one of the reader.
 *
 * The test creates two readers and two writers, each reader and writer pair on a different topic.
 * Writing a sample on one writer should not affect the liveliness changed status of the other reader.
 * Destroying the writer should not affect the liveliness changed status of the other reader.
 */
static void test_liveliness_qos_independent_topics(
        const std::string& topic_name,
        eprosima::fastdds::dds::ReliabilityQosPolicyKind reliability_kind)
{
    const auto lease_dutation_time = std::chrono::seconds(1);
    const eprosima::fastdds::dds::Duration_t lease_duration(1, 0);
    const eprosima::fastdds::dds::Duration_t announcement_period(0, 250000000);

    PubSubReader<HelloWorldPubSubType> reader1(topic_name + "1");
    PubSubReader<HelloWorldPubSubType> reader2(topic_name + "2");

    PubSubWriter<HelloWorldPubSubType> writer1(topic_name + "1");
    PubSubWriter<HelloWorldPubSubType> writer2(topic_name + "2");

    // Configure and start the readers
    reader1.liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration)
            .reliability(reliability_kind)
            .init();
    reader2.liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration)
            .reliability(reliability_kind)
            .init();

    // Configure and start the writers
    writer1.liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration)
            .liveliness_announcement_period(announcement_period)
            .reliability(reliability_kind)
            .init();
    writer2.liveliness_kind(eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS)
            .liveliness_lease_duration(lease_duration)
            .liveliness_announcement_period(announcement_period)
            .reliability(reliability_kind)
            .init();

    // Wait for discovery
    reader1.wait_discovery();
    reader2.wait_discovery();
    writer1.wait_discovery();
    writer2.wait_discovery();

    HelloWorldPubSubType::type data;

    // Write a sample on writer1 and wait for reader1 to assert writer1's liveliness
    writer1.send_sample(data);
    reader1.wait_liveliness_recovered();

    // Check liveliness changed status on both readers
    {
        auto liveliness = reader1.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 1);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }

    {
        auto liveliness = reader2.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 0);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }

    // Write a sample on writer2 and wait for reader2 to assert writer2's liveliness
    writer2.send_sample(data);
    reader2.wait_liveliness_recovered();

    // Check liveliness changed status on both readers
    {
        auto liveliness = reader1.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 1);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }

    {
        auto liveliness = reader2.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 1);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }

    // Destroy writer2 and wait twice the lease duration time
    writer2.destroy();
    std::this_thread::sleep_for(lease_dutation_time * 2);

    // Check liveliness changed status on both readers
    {
        auto liveliness = reader1.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 1);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }

    {
        auto liveliness = reader2.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 0);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }

    // Start writer2 again and wait for reader2 to assert writer2's liveliness
    writer2.init();
    reader2.wait_discovery();
    writer2.send_sample(data);
    reader2.wait_liveliness_recovered(2);

    // Check liveliness changed status on both readers
    {
        auto liveliness = reader1.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 1);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }

    {
        auto liveliness = reader2.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 1);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }

    // Destroy writer1 and wait twice the lease duration time
    writer1.destroy();
    std::this_thread::sleep_for(lease_dutation_time * 2);

    // Check liveliness changed status on both readers
    {
        auto liveliness = reader1.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 0);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }

    {
        auto liveliness = reader2.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 1);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }

    // Destroy writer2 and wait twice the lease duration time
    writer2.destroy();
    std::this_thread::sleep_for(lease_dutation_time * 2);

    // Check liveliness changed status on both readers
    {
        auto liveliness = reader1.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 0);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }

    {
        auto liveliness = reader2.get_liveliness_changed_status();
        EXPECT_EQ(liveliness.alive_count, 0);
        EXPECT_EQ(liveliness.not_alive_count, 0);
    }
}

TEST_P(LivelinessQos, IndependentTopics_reliable)
{
    test_liveliness_qos_independent_topics(TEST_TOPIC_NAME, eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
}

TEST_P(LivelinessQos, IndependentTopics_besteffort)
{
    test_liveliness_qos_independent_topics(TEST_TOPIC_NAME, eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS);
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(LivelinessQos,
        LivelinessQos,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<LivelinessQos::ParamType>& info)
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
