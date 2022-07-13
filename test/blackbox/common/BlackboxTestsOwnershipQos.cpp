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

#include "BlackboxTests.hpp"

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

#include <gtest/gtest.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

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
    }

};

/*!
 * @test DDS-OWN-SAMPLE-01 Tests samples reception works successfully with Non-Keyed types, Reliable,  Ownership QoS
 * EXCLUSIVE and dynamic change of strength.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_reliable_sample_reception)
{

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(RELIABLE_RELIABILITY_QOS).init();
    writer1.ownership_strength(1).init();
    writer2.ownership_strength(2).init();
    writer3.ownership_strength(3).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 3);

    auto data = default_helloworld_data_generator(13);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 changes its strength to 4.
    ASSERT_TRUE(writer2.ownership_strength(4).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 changes its strength to 1.
    ASSERT_TRUE(writer3.ownership_strength(1).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    reader.block_for_at_least(7);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-SAMPLE-02 Tests samples reception works successfully with Keyed types, Reliable,  Ownership QoS
 * EXCLUSIVE and dynamic change of strength.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_reliable_sample_reception)
{

    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(RELIABLE_RELIABILITY_QOS).init();
    writer1.ownership_strength(1).init();
    writer2.ownership_strength(2).init();
    writer3.ownership_strength(3).init();
    writer4.ownership_strength(4).init();

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

    auto data = default_keyedhelloworld_data_generator(23);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample of instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW4 sends a sample of instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    writer4.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample of instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 changes its strength to 4.
    ASSERT_TRUE(writer2.ownership_strength(4).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW4 sends a sample of instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    writer4.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 changes its strength to 1.
    ASSERT_TRUE(writer3.ownership_strength(1).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW4 sends a sample of instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    writer4.waitForAllAcked(std::chrono::milliseconds(100));

    reader.block_for_at_least(10);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-SAMPLE-03 Tests samples reception works successfully with Non-Keyed types, BestEffort, Ownership QoS
 * EXCLUSIVE and dynamic change of strength.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_besteffort_sample_reception)
{

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);

    reader.ownership_exclusive().init();
    writer1.ownership_strength(1).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer2.ownership_strength(2).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer3.ownership_strength(3).reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 3);

    auto data = default_helloworld_data_generator(13);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 changes its strength to 4.
    ASSERT_TRUE(writer2.ownership_strength(4).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 changes its strength to 1.
    ASSERT_TRUE(writer3.ownership_strength(1).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    reader.block_for_at_least(7);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-SAMPLE-04 Tests samples reception works successfully with Keyed types, BestEffort, Ownership QoS
 * EXCLUSIVE and dynamic change of strength.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_besteffort_sample_reception)
{

    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    reader.ownership_exclusive().init();
    writer1.ownership_strength(1).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer2.ownership_strength(2).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer3.ownership_strength(3).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer4.ownership_strength(4).reliability(BEST_EFFORT_RELIABILITY_QOS).init();

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

    auto data = default_keyedhelloworld_data_generator(23);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample of instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW4 sends a sample of instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample of instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample of instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 changes its strength to 4.
    ASSERT_TRUE(writer2.ownership_strength(4).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW4 sends a sample of instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 changes its strength to 1.
    ASSERT_TRUE(writer3.ownership_strength(1).set_qos());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait the reader receives the update

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample of instance 2.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample of instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample of instance 2.
    writer3.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW4 sends a sample of instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    reader.block_for_at_least(10);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
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

    reader.ownership_exclusive().reliability(RELIABLE_RELIABILITY_QOS).deadline_period({0, 100000000}).init();
    writer1.ownership_strength(1).deadline_period({0, 100000000}).init();
    writer2.ownership_strength(2).deadline_period({0, 100000000}).init();

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
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

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

    reader.ownership_exclusive().reliability(RELIABLE_RELIABILITY_QOS).deadline_period({0, 100000000}).init();
    writer1.ownership_strength(1).deadline_period({0, 100000000}).init();
    writer2.ownership_strength(2).deadline_period({0, 100000000}).init();

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
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer2.send_sample(data.front());
    data.pop_front();
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer2.send_sample(data.front());
    data.pop_front();
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer2.send_sample(data.front());
    data.pop_front();
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

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

/*!
 * @test DDS-OWN-UNDISC-01 Tests Ownership changes when writer removes himself, in a Reliable communication with
 * Non-Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_reliable_undiscovered_writer)
{

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(RELIABLE_RELIABILITY_QOS).init();
    writer1.ownership_strength(1).init();
    writer2.ownership_strength(2).init();
    writer3.ownership_strength(3).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 3);

    auto data = default_helloworld_data_generator(11);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 is detroyed
    writer3.destroy();
    reader.wait_writer_undiscovery(2);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 is detroyed
    writer1.destroy();
    reader.wait_writer_undiscovery(1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    reader.block_for_at_least(5);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-UNDISC-02 Tests Ownership changes when writer removes himself, in a Reliable communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_reliable_undiscovered_writer)
{

    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(RELIABLE_RELIABILITY_QOS).init();
    writer1.ownership_strength(1).init();
    writer2.ownership_strength(2).init();
    writer3.ownership_strength(3).init();
    writer4.ownership_strength(4).init();

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

    auto data = default_keyedhelloworld_data_generator(24);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    writer4.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 is destroyed.
    writer3.destroy();
    reader.wait_writer_undiscovery(3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    writer4.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 is destroyed.
    writer2.destroy();
    reader.wait_writer_undiscovery(2);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    writer4.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW4 is destroyed.
    writer4.destroy();
    reader.wait_writer_undiscovery(2);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    reader.block_for_at_least(5);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-UNDISC-03 Tests Ownership changes when writer removes himself, in a BestEffort communication with
 * Non-Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_non_keyed_besteffort_undiscovered_writer)
{

    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<HelloWorldPubSubType> writer3(TEST_TOPIC_NAME);

    reader.ownership_exclusive().init();
    writer1.ownership_strength(1).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer2.ownership_strength(2).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer3.ownership_strength(3).reliability(BEST_EFFORT_RELIABILITY_QOS).init();

    ASSERT_TRUE(reader.isInitialized());
    ASSERT_TRUE(writer1.isInitialized());
    ASSERT_TRUE(writer2.isInitialized());
    ASSERT_TRUE(writer3.isInitialized());

    // Wait for discovery.
    writer1.wait_discovery();
    writer2.wait_discovery();
    writer3.wait_discovery();
    reader.wait_discovery(std::chrono::seconds(1), 3);

    auto data = default_helloworld_data_generator(11);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 is detroyed
    writer3.destroy();
    reader.wait_writer_undiscovery(2);

    // DW1 sends a sample.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 is detroyed
    writer1.destroy();
    reader.wait_writer_undiscovery(1);

    // DW2 sends a sample.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    reader.block_for_at_least(5);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-UNDISC-04 Tests Ownership changes when writer removes himself, in a BestEffort communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_besteffort_undiscovered_writer)
{

    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    reader.ownership_exclusive().init();
    writer1.ownership_strength(1).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer2.ownership_strength(2).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer3.ownership_strength(3).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer4.ownership_strength(4).reliability(BEST_EFFORT_RELIABILITY_QOS).init();

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

    auto data = default_keyedhelloworld_data_generator(24);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 is destroyed.
    writer3.destroy();
    reader.wait_writer_undiscovery(3);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 is destroyed.
    writer2.destroy();
    reader.wait_writer_undiscovery(2);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW4 is destroyed.
    writer4.destroy();
    reader.wait_writer_undiscovery(2);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    reader.block_for_at_least(5);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-UNREG-01 Tests Ownership changes when writer unregisters an instance, in a Reliable communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_reliable_unregistering_instance)
{

    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    reader.ownership_exclusive().reliability(RELIABLE_RELIABILITY_QOS).init();
    writer1.ownership_strength(1).init();
    writer2.ownership_strength(2).init();
    writer3.ownership_strength(3).init();
    writer4.ownership_strength(4).init();

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

    auto data = default_keyedhelloworld_data_generator(24);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample in instance 1.
    InstanceHandle_t instance_1 = writer1.register_instance(data.front());
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    InstanceHandle_t instance_2 = writer1.register_instance(data.front());
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    writer4.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    writer3.waitForAllAcked(std::chrono::milliseconds(100));

    // DW3 unregisters instance 2.
    writer3.unregister_instance(data.front(), instance_2);
    data.pop_front();

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer2.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    writer4.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW2 unregisters instance 1.
    writer2.unregister_instance(data.front(), instance_1);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    writer4.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW4 is destroyed.
    writer4.unregister_instance(data.front(), instance_1);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    writer1.waitForAllAcked(std::chrono::milliseconds(100));

    reader.block_for_at_least(5);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
}

/*!
 * @test DDS-OWN-UNREG-02 Tests Ownership changes when writer unregisters an instance, in a BestEffort communication with
 * Keyed types.
 */
TEST_P(OwnershipQos, exclusive_kind_keyed_besteffort_unregistering_instance)
{

    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer1(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer2(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer3(TEST_TOPIC_NAME);
    PubSubWriter<KeyedHelloWorldPubSubType> writer4(TEST_TOPIC_NAME);

    reader.ownership_exclusive().init();
    writer1.ownership_strength(1).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer2.ownership_strength(2).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer3.ownership_strength(3).reliability(BEST_EFFORT_RELIABILITY_QOS).init();
    writer4.ownership_strength(4).reliability(BEST_EFFORT_RELIABILITY_QOS).init();

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

    auto data = default_keyedhelloworld_data_generator(24);
    reader.startReception(data);

    decltype(data) denied_samples;

    // DW1 sends a sample in instance 1.
    InstanceHandle_t instance_1 = writer1.register_instance(data.front());
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    InstanceHandle_t instance_2 = writer1.register_instance(data.front());
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 sends a sample in instance 2.
    writer3.send_sample(data.front());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW3 unregisters instance 2.
    writer3.unregister_instance(data.front(), instance_2);
    data.pop_front();

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 sends a sample in instance 1.
    writer2.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW2 unregisters instance 1.
    writer2.unregister_instance(data.front(), instance_1);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    denied_samples.push_back(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW4 sends a sample in instance 1.
    writer4.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW4 unregisters instance 1.
    writer4.unregister_instance(data.front(), instance_1);

    // DW1 sends a sample in instance 1.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // DW1 sends a sample in instance 2.
    writer1.send_sample(data.front());
    data.pop_front();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    reader.block_for_at_least(5);
    ASSERT_EQ(denied_samples.size(), reader.data_not_received().size());
    ASSERT_EQ(denied_samples, reader.data_not_received());
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
