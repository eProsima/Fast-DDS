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

#include "BlackboxTests.hpp"

#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

TEST(KeyedTopic, RegistrationNonKeyedFail)
{
    PubSubWriter<HelloWorldType> writer(TEST_TOPIC_NAME);

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator(2);

    for (auto data_sample : data)
    {
        // Register instances
        EXPECT_EQ(writer.register_instance(data_sample), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);
    }
}

TEST(KeyedTopic, RegistrationSuccess)
{
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_keyedhelloworld_data_generator(2);

    for (auto data_sample : data)
    {
        // Register instances
        EXPECT_NE(writer.register_instance(data_sample), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);
    }
}

TEST(KeyedTopic, RegistrationFail)
{
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);

    writer.
    resource_limits_max_instances(1).
    init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances.
    EXPECT_NE(writer.register_instance(data.front()), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);
    EXPECT_EQ(writer.register_instance(data.back()), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);
}

TEST(KeyedTopic, UnregistrationFail)
{
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);

    writer.
    resource_limits_max_instances(1).
    init();

    ASSERT_TRUE(writer.isInitialized());

    eprosima::fastrtps::rtps::InstanceHandle_t handle;
    handle.value[0] = 1;

    auto data = default_keyedhelloworld_data_generator(1);

    ASSERT_FALSE(writer.unregister_instance(data.front(), handle));
}

TEST(KeyedTopic, DisposeFail)
{
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);

    writer.
    resource_limits_max_instances(1).
    init();

    ASSERT_TRUE(writer.isInitialized());

    eprosima::fastrtps::rtps::InstanceHandle_t handle;
    handle.value[0] = 1;

    auto data = default_keyedhelloworld_data_generator(1);

    ASSERT_FALSE(writer.dispose(data.front(), handle));
}

TEST(KeyedTopic, RegistrationAfterUnregistration)
{
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);

    writer.
    resource_limits_max_instances(1).
    init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances.
    auto instance_handle_1 = writer.register_instance(data.front());
    EXPECT_NE(instance_handle_1, eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);
    EXPECT_EQ(writer.register_instance(data.back()), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);

    ASSERT_TRUE(writer.unregister_instance(data.front(), instance_handle_1));
    EXPECT_NE(writer.register_instance(data.back()), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);
    EXPECT_EQ(writer.register_instance(data.front()), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);

    ASSERT_TRUE(writer.unregister_instance(data.back(), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown));
    EXPECT_NE(writer.register_instance(data.front()), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);
}

TEST(KeyedTopic, RegistrationAfterDispose)
{
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);

    writer.
    resource_limits_max_instances(1).
    init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances.
    auto instance_handle_1 = writer.register_instance(data.front());
    EXPECT_NE(instance_handle_1, eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);
    EXPECT_EQ(writer.register_instance(data.back()), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);

    ASSERT_TRUE(writer.dispose(data.front(), instance_handle_1));
    EXPECT_EQ(writer.register_instance(data.back()), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);

    ASSERT_TRUE(writer.unregister_instance(data.front(), instance_handle_1));
    EXPECT_NE(writer.register_instance(data.back()), eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);
}

/* Uncomment when DDS API supports NO_WRITERS_ALIVE
   TEST(KeyedTopic, WriteSamplesBestEffort)
   {
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldType> reader(TEST_TOPIC_NAME);

    writer.
    resource_limits_max_instances(1).
    reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
    init();

    ASSERT_TRUE(writer.isInitialized());

    reader.
    resource_limits_max_instances(1).
    reliability(eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS).
    init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances.
    auto instance_handle_1 = writer.register_instance(data.front());
    EXPECT_NE(instance_handle_1, eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    EXPECT_EQ(data.size(), static_cast<size_t>(1));
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(1);

    auto data2 = default_keyedhelloworld_data_generator(2);

    ASSERT_TRUE(writer.unregister_instance(data2.front(), instance_handle_1));

    auto instance_handle_2 = writer.register_instance(data.back());
    EXPECT_NE(instance_handle_2, eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);

    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(1);
   }

   TEST(KeyedTopic, WriteSamplesReliable)
   {
    PubSubWriter<KeyedHelloWorldType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldType> reader(TEST_TOPIC_NAME);

    writer.
    resource_limits_max_instances(1).
    reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
    init();

    ASSERT_TRUE(writer.isInitialized());

    reader.
    resource_limits_max_instances(1).
    reliability(eprosima::fastrtps::RELIABLE_RELIABILITY_QOS).
    init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances.
    auto instance_handle_1 = writer.register_instance(data.front());
    EXPECT_NE(instance_handle_1, eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);

    reader.startReception(data);
    // Send data
    writer.send(data);
    // In this test all data should be sent.
    EXPECT_EQ(data.size(), static_cast<size_t>(1));
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(1);

    auto data2 = default_keyedhelloworld_data_generator(2);
    ASSERT_TRUE(writer.unregister_instance(data2.front(), instance_handle_1));

    auto instance_handle_2 = writer.register_instance(data.back());
    // Is it deterministic?
    EXPECT_EQ(instance_handle_2, eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);

    writer.waitForAllAcked(std::chrono::milliseconds(100));

    instance_handle_2 = writer.register_instance(data.back());
    EXPECT_NE(instance_handle_2, eprosima::fastrtps::rtps::c_InstanceHandle_Unknown);

    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(1);
   }
 */
