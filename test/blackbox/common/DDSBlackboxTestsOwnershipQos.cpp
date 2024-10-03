// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/LibrarySettings.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
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

class OwnershipQos : public testing::TestWithParam<communication_type>
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

void exclusive_kind_non_keyed_sample_reception(
        bool reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer1.ownership_strength(1).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer2.ownership_strength(2).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer3.ownership_strength(3).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 3);

    std::function<void(PubSubWriter<HelloWorldPubSubType>&)> wait_functor =
            [](PubSubWriter<HelloWorldPubSubType>&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            };

    if (reliable)
    {
        wait_functor = [](PubSubWriter<HelloWorldPubSubType>& writer)
                {
                    writer.waitForAllAcked(std::chrono::milliseconds(100));
                };
    }

    auto data = default_helloworld_data_generator(13);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW2 changes its strength to 4.
    ASSERT_TRUE(writer2.ownership_strength(4).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW3 changes its strength to 1.
    ASSERT_TRUE(writer3.ownership_strength(1).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer3);

    reader.block_for_at_least(7);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-SAMPLE-01 Tests samples reception works successfully with Non-Keyed types, Reliable,  Ownership QoS
 * EXCLUSIVE and dynamic change of strength.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_reliable_sample_reception)
{
    exclusive_kind_non_keyed_sample_reception(true);
}

void exclusive_kind_keyed_sample_reception(
        bool reliable)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer1.ownership_strength(1).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer2.ownership_strength(2).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer3.ownership_strength(3).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer4.ownership_strength(4).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());
    ASSERT_TRUE(writer4.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    writer4.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 4);

    std::function<void(PubSubWriter<KeyedHelloWorldPubSubType>&)> wait_functor =
            [](PubSubWriter<KeyedHelloWorldPubSubType>&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            };

    if (reliable)
    {
        wait_functor = [](PubSubWriter<KeyedHelloWorldPubSubType>& writer)
                {
                    writer.waitForAllAcked(std::chrono::milliseconds(100));
                };
    }

    auto data = default_keyedhelloworld_data_generator(23);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample of instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW4 sends a sample of instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample of instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW2 changes its strength to 4.
    ASSERT_TRUE(writer2.ownership_strength(4).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW4 sends a sample of instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW3 changes its strength to 1.
    ASSERT_TRUE(writer3.ownership_strength(1).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW4 sends a sample of instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    reader.block_for_at_least(10);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-SAMPLE-02 Tests samples reception works successfully with Keyed types, Reliable,  Ownership QoS
 * EXCLUSIVE and dynamic change of strength.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_reliable_sample_reception)
{
    exclusive_kind_keyed_sample_reception(true);
}

/*!
 * @test DDS-OWN-SAMPLE-03 Tests samples reception works successfully with Non-Keyed types, BestEffort, Ownership QoS
 * EXCLUSIVE and dynamic change of strength.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_besteffort_sample_reception)
{
    exclusive_kind_non_keyed_sample_reception(false);
}

/*!
 * @test DDS-OWN-SAMPLE-04 Tests samples reception works successfully with Keyed types, BestEffort, Ownership QoS
 * EXCLUSIVE and dynamic change of strength.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_besteffort_sample_reception)
{
    exclusive_kind_keyed_sample_reception(false);
}

void exclusive_kind_non_keyed_writers_same_guid(
        bool reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer1.ownership_strength(10).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer2.ownership_strength(10).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer3.ownership_strength(10).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 3);

    std::function<void(PubSubWriter<HelloWorldPubSubType>&)> wait_functor =
            [](PubSubWriter<HelloWorldPubSubType>&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            };

    if (reliable)
    {
        wait_functor = [](PubSubWriter<HelloWorldPubSubType>& writer)
                {
                    writer.waitForAllAcked(std::chrono::milliseconds(100));
                };
    }

    auto data = default_helloworld_data_generator(7);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    reader.block_for_at_least(4);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-SAMPLE-05 Tests samples reception works successfully with Non-Keyed types, Reliable,  Ownership QoS
 * EXCLUSIVE and several writers with same GUID.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_reliable_writers_same_guid)
{
    exclusive_kind_non_keyed_writers_same_guid(true);
}

void exclusive_kind_keyed_writers_same_guid(
        bool reliable)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer1.ownership_strength(10).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer2.ownership_strength(10).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer3.ownership_strength(10).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer4.ownership_strength(10).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());
    ASSERT_TRUE(writer4.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    writer4.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 4);

    std::function<void(PubSubWriter<KeyedHelloWorldPubSubType>&)> wait_functor =
            [](PubSubWriter<KeyedHelloWorldPubSubType>&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            };

    if (reliable)
    {
        wait_functor = [](PubSubWriter<KeyedHelloWorldPubSubType>& writer)
                {
                    writer.waitForAllAcked(std::chrono::milliseconds(100));
                };
    }

    auto data = default_keyedhelloworld_data_generator(10);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW4 sends a sample of instance 1.
    writer4.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW4 sends a sample of instance 2.
    writer4.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample of instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample of instance 2.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    reader.block_for_at_least(6);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-SAMPLE-06 Tests samples reception works successfully with Keyed types, Reliable,  Ownership QoS
 * EXCLUSIVE and several writers with same GUID.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_reliable_writers_same_guid)
{
    exclusive_kind_keyed_writers_same_guid(true);
}

/*!
 * @test DDS-OWN-SAMPLE-07 Tests samples reception works successfully with Keyed types, BestEffort,  Ownership QoS
 * EXCLUSIVE and several writers with same GUID.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_besteffort_writers_same_guid)
{
    exclusive_kind_non_keyed_writers_same_guid(false);
}

/*!
 * @test DDS-OWN-SAMPLE-08 Tests samples reception works successfully with Keyed types, BestEffort,  Ownership QoS
 * EXCLUSIVE and several writers with same GUID.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_besteffort_writers_same_guid)
{
    exclusive_kind_keyed_writers_same_guid(false);
}

/*!
 * @test DDS-OWN-DEADLINE-01 Tests Ownership changes when the current owner doesn't comply with deadline QoS, in a
 * Reliable communication with Non-Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_reliable_deadline)
{

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).deadline_period({0,
                                                                                                                500000000})
            .lease_duration(
        {50, 0}, {3, 0}).init();
    writer1.ownership_strength(1).deadline_period({0, 500000000}).lease_duration({50, 0}, {3, 0}).init();
    writer2.ownership_strength(2).deadline_period({0, 500000000}).lease_duration({50, 0}, {3, 0}).init();


    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 2);

    auto data = default_helloworld_data_generator(10);
    reader.startReception(data);

    decltype(data) denied_samples;

    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    reader.block_for_seq({0, 7});
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-DEADLINE-02 Tests Ownership changes when the current owner doesn't comply with deadline QoS, in a
 * Reliable communication with Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_reliable_deadline)
{

    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).deadline_period({0,
                                                                                                                500000000})
            .lease_duration(
        {50, 0}, {3, 0}).init();
    writer1.ownership_strength(1).deadline_period({0, 500000000}).lease_duration({50, 0}, {3, 0}).init();
    writer2.ownership_strength(2).deadline_period({0, 500000000}).lease_duration({50, 0}, {3, 0}).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 2);

    auto data = default_keyedhelloworld_data_generator(26);
    reader.startReception(data);

    decltype(data) denied_samples;

    writer1.send_sample(data.front());
    data.pop_front();
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer2.send_sample(data.front());
    data.pop_front();
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer2.send_sample(data.front());
    data.pop_front();
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer2.send_sample(data.front());
    data.pop_front();
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    writer1.send_sample(data.front());
    data.pop_front();
    writer2.send_sample(data.front());
    data.pop_front();
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    writer1.send_sample(data.front());
    data.pop_front();
    writer2.send_sample(data.front());
    data.pop_front();
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    writer1.send_sample(data.front());
    data.pop_front();
    writer2.send_sample(data.front());
    data.pop_front();
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    reader.block_for_seq({0, 13});
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

void exclusive_kind_non_keyed_undiscovered_writer(
        bool reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer1.ownership_strength(1).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer2.ownership_strength(2).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer3.ownership_strength(3).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 3);

    std::function<void(PubSubWriter<HelloWorldPubSubType>&)> wait_functor =
            [](PubSubWriter<HelloWorldPubSubType>&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            };

    if (reliable)
    {
        wait_functor = [](PubSubWriter<HelloWorldPubSubType>& writer)
                {
                    writer.waitForAllAcked(std::chrono::milliseconds(100));
                };
    }

    auto data = default_helloworld_data_generator(11);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW3 is detroyed
    writer3.destroy();
    reader.wait_writer_undiscovery(2);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 is detroyed
    writer1.destroy();
    reader.wait_writer_undiscovery(1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    reader.block_for_at_least(7);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-UNDISC-01 Tests Ownership changes when writer removes himself, in a Reliable communication with
 * Non-Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_reliable_undiscovered_writer)
{
    exclusive_kind_non_keyed_undiscovered_writer(true);
}

void exclusive_kind_keyed_undiscovered_writer(
        bool reliable)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer1.ownership_strength(1).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer2.ownership_strength(2).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer3.ownership_strength(3).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer4.ownership_strength(4).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());
    ASSERT_TRUE(writer4.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    writer4.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 4);

    std::function<void(PubSubWriter<KeyedHelloWorldPubSubType>&)> wait_functor =
            [](PubSubWriter<KeyedHelloWorldPubSubType>&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            };

    if (reliable)
    {
        wait_functor = [](PubSubWriter<KeyedHelloWorldPubSubType>& writer)
                {
                    writer.waitForAllAcked(std::chrono::milliseconds(100));
                };
    }

    auto data = default_keyedhelloworld_data_generator(24);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW3 is destroyed.
    writer3.destroy();
    reader.wait_writer_undiscovery(3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 is destroyed.
    writer2.destroy();
    reader.wait_writer_undiscovery(2);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 is destroyed.
    writer4.destroy();
    reader.wait_writer_undiscovery(1);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    reader.block_for_at_least(16);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-UNDISC-02 Tests Ownership changes when writer removes himself, in a Reliable communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_reliable_undiscovered_writer)
{
    exclusive_kind_keyed_undiscovered_writer(true);
}

/*!
 * @test DDS-OWN-UNDISC-03 Tests Ownership changes when writer removes himself, in a BestEffort communication with
 * Non-Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_besteffort_undiscovered_writer)
{
    exclusive_kind_non_keyed_undiscovered_writer(false);
}

/*!
 * @test DDS-OWN-UNDISC-04 Tests Ownership changes when writer removes himself, in a BestEffort communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_besteffort_undiscovered_writer)
{
    exclusive_kind_keyed_undiscovered_writer(false);
}

void exclusive_kind_non_keyed_lost_liveliness(
        bool reliable)
{
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);

    std::atomic<bool> drop_messages1(false);
    auto testTransport1 = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport1->messages_filter_ = [&drop_messages1](eprosima::fastdds::rtps::CDRMessage_t&)
            {
                return drop_messages1.load();
            };
    std::atomic<bool> drop_messages3(false);
    auto testTransport3 = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport3->messages_filter_ = [&drop_messages3](eprosima::fastdds::rtps::CDRMessage_t&)
            {
                return drop_messages3.load();
            };

    reader.ownership_exclusive().reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        )
            .lease_duration({1, 0}, {0, 500000000})
            .init();
    writer1.ownership_strength(1).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        )
            .add_user_transport_to_pparams(testTransport1)
            .disable_builtin_transport()
            .lease_duration({1, 0}, {0, 500000000})
            .init();
    writer2.ownership_strength(2).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        )
            .lease_duration({1, 0}, {0, 500000000})
            .init();
    writer3.ownership_strength(3).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        )
            .add_user_transport_to_pparams(testTransport3)
            .disable_builtin_transport()
            .lease_duration({1, 0}, {0, 500000000})
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 3);

    std::function<void(PubSubWriter<HelloWorldPubSubType>&)> wait_functor =
            [](PubSubWriter<HelloWorldPubSubType>&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            };

    if (reliable)
    {
        wait_functor = [](PubSubWriter<HelloWorldPubSubType>& writer)
                {
                    writer.waitForAllAcked(std::chrono::milliseconds(100));
                };
    }

    auto data = default_helloworld_data_generator(11);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW3 liveliness lost.
    drop_messages3 = true;
    reader.wait_writer_undiscovery(2);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 liveliness lost.
    drop_messages1 = true;
    reader.wait_writer_undiscovery(1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    reader.block_for_at_least(7);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-LIVE-01 Tests Ownership changes when writer removes himself, in a Reliable communication with
 * Non-Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_reliable_lost_liveliness)
{
    exclusive_kind_non_keyed_lost_liveliness(true);
}

void exclusive_kind_keyed_lost_liveliness(
        bool reliable)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    std::atomic<bool> drop_messages2(false);
    auto testTransport2 = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport2->messages_filter_ = [&drop_messages2](eprosima::fastdds::rtps::CDRMessage_t&)
            {
                return drop_messages2.load();
            };
    std::atomic<bool> drop_messages3(false);
    auto testTransport3 = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport3->messages_filter_ = [&drop_messages3](eprosima::fastdds::rtps::CDRMessage_t&)
            {
                return drop_messages3.load();
            };
    std::atomic<bool> drop_messages4(false);
    auto testTransport4 = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    testTransport4->messages_filter_ = [&drop_messages4](eprosima::fastdds::rtps::CDRMessage_t&)
            {
                return drop_messages4.load();
            };

    reader.ownership_exclusive().reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        )
            .lease_duration({1, 0}, {0, 500000000})
            .init();
    writer1.ownership_strength(1).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        )
            .lease_duration({1, 0}, {0, 500000000})
            .init();
    writer2.ownership_strength(2).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        )
            .add_user_transport_to_pparams(testTransport2)
            .disable_builtin_transport()
            .lease_duration({1, 0}, {0, 500000000})
            .init();
    writer3.ownership_strength(3).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        )
            .add_user_transport_to_pparams(testTransport3)
            .disable_builtin_transport()
            .lease_duration({1, 0}, {0, 500000000})
            .init();
    writer4.ownership_strength(4).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        )
            .add_user_transport_to_pparams(testTransport4)
            .disable_builtin_transport()
            .lease_duration({1, 0}, {0, 500000000})
            .init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());
    ASSERT_TRUE(writer4.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    writer4.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 4);

    std::function<void(PubSubWriter<KeyedHelloWorldPubSubType>&)> wait_functor =
            [](PubSubWriter<KeyedHelloWorldPubSubType>&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            };

    if (reliable)
    {
        wait_functor = [](PubSubWriter<KeyedHelloWorldPubSubType>& writer)
                {
                    writer.waitForAllAcked(std::chrono::milliseconds(100));
                };
    }

    auto data = default_keyedhelloworld_data_generator(24);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW3 losses liveliness.
    drop_messages3 = true;
    reader.wait_writer_undiscovery(3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 losses liveliness.
    drop_messages2 = true;
    reader.wait_writer_undiscovery(2);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 losses liveliness.
    drop_messages4 = true;
    reader.wait_writer_undiscovery(1);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    reader.block_for_at_least(16);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-LIVE-02 Tests Ownership changes when writer removes himself, in a Reliable communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_reliable_lost_liveliness)
{
    exclusive_kind_keyed_lost_liveliness(true);
}

/*!
 * @test DDS-OWN-LIVE-03 Tests Ownership changes when writer removes himself, in a BestEffort communication with
 * Non-Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_besteffort_lost_liveliness)
{
    exclusive_kind_non_keyed_lost_liveliness(false);
}

/*!
 * @test DDS-OWN-LIVE-04 Tests Ownership changes when writer removes himself, in a BestEffort communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_besteffort_lost_liveliness)
{
    exclusive_kind_keyed_lost_liveliness(false);
}

void exclusive_kind_keyed_unregistering_instance(
        bool reliable)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer1.ownership_strength(1).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer2.ownership_strength(2).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer3.ownership_strength(3).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer4.ownership_strength(4).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());
    ASSERT_TRUE(writer4.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    writer4.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 4);

    std::function<void(PubSubWriter<KeyedHelloWorldPubSubType>&)> wait_functor =
            [](PubSubWriter<KeyedHelloWorldPubSubType>&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            };

    if (reliable)
    {
        wait_functor = [](PubSubWriter<KeyedHelloWorldPubSubType>& writer)
                {
                    writer.waitForAllAcked(std::chrono::milliseconds(100));
                };
    }

    auto data = default_keyedhelloworld_data_generator(24);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample in instance 1.
    InstanceHandle_t instance_1 = writer1.register_instance(data.front());
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    InstanceHandle_t instance_2 = writer1.register_instance(data.front());
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    wait_functor(writer3);

    // DW3 unregisters instance 2.
    writer3.unregister_instance(data.front(), instance_2);
    data.pop_front();

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 unregisters instance 1.
    writer2.unregister_instance(data.front(), instance_1);
    wait_functor(writer2);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 is destroyed.
    writer4.unregister_instance(data.front(), instance_1);
    wait_functor(writer4);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    reader.block_for_at_least(16);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-UNREG-01 Tests Ownership changes when writer unregisters an instance, in a Reliable communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_reliable_unregistering_instance)
{
    exclusive_kind_keyed_unregistering_instance(true);
}

/*!
 * @test DDS-OWN-UNREG-02 Tests Ownership changes when writer unregisters an instance, in a BestEffort communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_besteffort_unregistering_instance)
{
    exclusive_kind_keyed_unregistering_instance(false);
}

void exclusive_kind_keyed_disposing_instance(
        bool reliable)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer1.ownership_strength(1).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer2.ownership_strength(2).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer3.ownership_strength(3).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();
    writer4.ownership_strength(4).reliability(
        reliable ? eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS : eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS
        ).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());
    ASSERT_TRUE(writer4.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    writer4.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 4);

    std::function<void(PubSubWriter<KeyedHelloWorldPubSubType>&)> wait_functor =
            [](PubSubWriter<KeyedHelloWorldPubSubType>&)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            };

    if (reliable)
    {
        wait_functor = [](PubSubWriter<KeyedHelloWorldPubSubType>& writer)
                {
                    writer.waitForAllAcked(std::chrono::milliseconds(100));
                };
    }

    auto data = default_keyedhelloworld_data_generator(30);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample in instance 1.
    InstanceHandle_t instance_1 = writer1.register_instance(data.front());
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    InstanceHandle_t instance_2 = writer1.register_instance(data.front());
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    wait_functor(writer3);

    // DW3 disposes instance 2.
    writer3.dispose(data.front(), instance_2);
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 disposes instance 1.
    writer2.dispose(data.front(), instance_1);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 is destroyed.
    writer4.dispose(data.front(), instance_1);
    wait_functor(writer4);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW2 changes its strength to 5.
    ASSERT_TRUE(writer2.ownership_strength(5).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update
                                                                 //
    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    wait_functor(writer2);

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    wait_functor(writer3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer1);

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    wait_functor(writer4);

    // DW3 disposes instance 2.
    writer3.dispose(data.front(), instance_2);
    wait_functor(writer3);

    // DW3 is detroyed
    writer3.destroy();
    reader.wait_writer_undiscovery(3);

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    wait_functor(writer1);

    reader.block_for_at_least(12);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-DISP-01 Tests Ownership changes when writer disposes an instance, in a Reliable communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_reliable_disposing_instance)
{
    exclusive_kind_keyed_disposing_instance(true);
}

/*!
 * @test DDS-OWN-DISP-02 Tests Ownership changes when writer disposes an instance, in a BestEffort communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_besteffort_disposing_instance)
{
    exclusive_kind_keyed_disposing_instance(false);
}

/*!
 * This is a regression test for redmine issue 20866.
 *
 * This test checks that a reader keeping a long number of samples and with an exclusive ownership policy only
 * returns the data from the writer with the highest strength.
 *
 * @param use_keep_all_history  Whether to use KEEP_ALL history or KEEP_LAST(20).
 * @param mixed_data            Whether to send data from both writers in an interleaved way.
 */
static void test_exclusive_kind_big_history(
        bool use_keep_all_history,
        bool mixed_data)
{
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> low_strength_writer(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> high_strength_writer(TEST_TOPIC_NAME);

    // Configure history QoS.
    if (use_keep_all_history)
    {
        reader.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
        low_strength_writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
        high_strength_writer.history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS);
    }
    else
    {
        reader.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS).history_depth(20);
        low_strength_writer.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS).history_depth(20);
        high_strength_writer.history_kind(eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS).history_depth(20);
    }

    // Prepare data.
    std::list<KeyedHelloWorld> generated_data = default_keyedhelloworld_data_generator(20);
    auto middle = std::next(generated_data.begin(), 10);
    std::list<KeyedHelloWorld> low_strength_data(generated_data.begin(), middle);
    std::list<KeyedHelloWorld> high_strength_data(middle, generated_data.end());
    auto expected_data = high_strength_data;

    if (mixed_data)
    {
        // Expect reception of the first two samples from the low strength writer (one per instance).
        auto it = low_strength_data.begin();
        expected_data.push_front(*it++);
        expected_data.push_front(*it);
    }

    // Initialize writers.
    low_strength_writer.ownership_strength(3)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();
    ASSERT_TRUE(low_strength_writer.isInitialized());

    // High strength writer will use a custom transport to ensure its data is received after the low strength data.
    auto test_transport = std::make_shared<test_UDPv4TransportDescriptor>();
    std::atomic<bool> drop_messages(false);
    test_transport->messages_filter_ = [&drop_messages](eprosima::fastdds::rtps::CDRMessage_t&)
            {
                return drop_messages.load();
            };
    high_strength_writer.ownership_strength(4)
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .disable_builtin_transport()
            .add_user_transport_to_pparams(test_transport)
            .init();
    ASSERT_TRUE(high_strength_writer.isInitialized());

    // Initialize reader.
    reader.ownership_exclusive()
            .reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS)
            .init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    low_strength_writer.wait_discovery();
    high_strength_writer.wait_discovery();
    reader.wait_discovery(std::chrono::seconds::zero(), 2);

    // Drop the messages from the high strength writer, so they arrive later to the reader.
    drop_messages.store(true);

    if (mixed_data)
    {
        // Send one sample from each writer, with low strength data first.
        while (!low_strength_data.empty() && !high_strength_data.empty())
        {
            EXPECT_TRUE(low_strength_writer.send_sample(low_strength_data.front()));
            EXPECT_TRUE(high_strength_writer.send_sample(high_strength_data.front()));
            low_strength_data.pop_front();
            high_strength_data.pop_front();
        }
    }
    else
    {
        // Send high strength data first, so it has the lowest source timestamps, but drop the messages, so they arrive
        // later to the reader.
        high_strength_writer.send(high_strength_data);
        EXPECT_TRUE(high_strength_data.empty());

        // Send low strength data, so it has the highest source timestamps.
        low_strength_writer.send(low_strength_data);
        EXPECT_TRUE(low_strength_data.empty());
    }

    // Wait for the reader to receive the low strength data.
    EXPECT_TRUE(low_strength_writer.waitForAllAcked(std::chrono::seconds(1)));

    // Let high strength writer send the data, and wait for the reader to receive it.
    drop_messages.store(false);
    EXPECT_TRUE(high_strength_writer.waitForAllAcked(std::chrono::seconds(1)));

    // Make the reader process the data, expecting only the required data.
    // The issue was reproduced by the reader complaining about reception of unexpected data.
    reader.startReception(expected_data);
    reader.block_for_all();
}

TEST(OwnershipQos, exclusive_kind_keep_all_reliable)
{
    test_exclusive_kind_big_history(true, false);
}

TEST(OwnershipQos, exclusive_kind_keep_all_reliable_mixed)
{
    test_exclusive_kind_big_history(true, true);
}

TEST(OwnershipQos, exclusive_kind_keep_last_reliable)
{
    test_exclusive_kind_big_history(false, false);
}

TEST(OwnershipQos, exclusive_kind_keep_last_reliable_mixed)
{
    test_exclusive_kind_big_history(false, true);
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_SUITE_P(x, y, z, w)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z, w) INSTANTIATE_TEST_CASE_P(x, y, z, w)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(OwnershipQos,
        OwnershipQos,
        testing::Values(TRANSPORT, INTRAPROCESS, DATASHARING),
        [](const testing::TestParamInfo<OwnershipQos::ParamType>& info)
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
