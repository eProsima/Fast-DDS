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

#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>

#include "../utils/filter_helpers.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

TEST(KeyedTopic, RegistrationNonKeyedFail)
{
    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_helloworld_data_generator(2);

    for (auto data_sample : data)
    {
        // Register instances
        EXPECT_EQ(writer.register_instance(data_sample), eprosima::fastdds::rtps::c_InstanceHandle_Unknown);
    }
}

TEST(KeyedTopic, RegistrationSuccess)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_keyedhelloworld_data_generator(2);

    for (auto data_sample : data)
    {
        // Register instances
        EXPECT_NE(writer.register_instance(data_sample), eprosima::fastdds::rtps::c_InstanceHandle_Unknown);
    }
}

TEST(KeyedTopic, RegistrationFail)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.
            resource_limits_max_instances(1).
            init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances.
    EXPECT_NE(writer.register_instance(data.front()), eprosima::fastdds::rtps::c_InstanceHandle_Unknown);
    EXPECT_EQ(writer.register_instance(data.back()), eprosima::fastdds::rtps::c_InstanceHandle_Unknown);
}

TEST(KeyedTopic, UnregistrationFail)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.
            resource_limits_max_instances(1).
            init();

    ASSERT_TRUE(writer.isInitialized());

    eprosima::fastdds::rtps::InstanceHandle_t handle;
    handle.value[0] = 1;

    auto data = default_keyedhelloworld_data_generator(1);

    ASSERT_FALSE(writer.unregister_instance(data.front(), handle));
}

TEST(KeyedTopic, DisposeFail)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.
            resource_limits_max_instances(1).
            init();

    ASSERT_TRUE(writer.isInitialized());

    eprosima::fastdds::rtps::InstanceHandle_t handle;
    handle.value[0] = 1;

    auto data = default_keyedhelloworld_data_generator(1);

    ASSERT_FALSE(writer.dispose(data.front(), handle));
}

TEST(KeyedTopic, RegistrationAfterUnregistration)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.
            resource_limits_max_instances(1).
            init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances.
    auto instance_handle_1 = writer.register_instance(data.front());
    EXPECT_NE(instance_handle_1, eprosima::fastdds::rtps::c_InstanceHandle_Unknown);
    EXPECT_EQ(writer.register_instance(data.back()), eprosima::fastdds::rtps::c_InstanceHandle_Unknown);

    ASSERT_TRUE(writer.unregister_instance(data.front(), instance_handle_1));
    ASSERT_FALSE(writer.unregister_instance(data.front(), instance_handle_1));
    EXPECT_NE(writer.register_instance(data.back()), eprosima::fastdds::rtps::c_InstanceHandle_Unknown);
    EXPECT_EQ(writer.register_instance(data.front()), eprosima::fastdds::rtps::c_InstanceHandle_Unknown);

    ASSERT_TRUE(writer.unregister_instance(data.back(), eprosima::fastdds::rtps::c_InstanceHandle_Unknown));
    ASSERT_FALSE(writer.unregister_instance(data.back(), eprosima::fastdds::rtps::c_InstanceHandle_Unknown));
    EXPECT_NE(writer.register_instance(data.front()), eprosima::fastdds::rtps::c_InstanceHandle_Unknown);
}

TEST(KeyedTopic, RegistrationAfterDispose)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.
            resource_limits_max_instances(1).
            init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances.
    auto instance_handle_1 = writer.register_instance(data.front());
    EXPECT_NE(instance_handle_1, eprosima::fastdds::rtps::c_InstanceHandle_Unknown);
    EXPECT_EQ(writer.register_instance(data.back()), eprosima::fastdds::rtps::c_InstanceHandle_Unknown);

    ASSERT_TRUE(writer.dispose(data.front(), instance_handle_1));
    EXPECT_EQ(writer.register_instance(data.back()), eprosima::fastdds::rtps::c_InstanceHandle_Unknown);

    ASSERT_TRUE(writer.unregister_instance(data.front(), instance_handle_1));
    EXPECT_NE(writer.register_instance(data.back()), eprosima::fastdds::rtps::c_InstanceHandle_Unknown);
}

TEST(KeyedTopic, UnregisterWhenHistoryKeepAll)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);

    writer.
            history_kind(eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS).
            init();

    ASSERT_TRUE(writer.isInitialized());

    auto data = default_keyedhelloworld_data_generator();

    // Register instances.
    auto instance_handle_1 = writer.register_instance(data.front());
    auto instance_handle_2 = writer.register_instance(*(++data.begin()));

    writer.send(data);
    // In this test all data should be sent.
    EXPECT_EQ(data.size(), static_cast<size_t>(0));

    data = default_keyedhelloworld_data_generator(2);

    ASSERT_TRUE(writer.unregister_instance(data.front(), instance_handle_1));
    ASSERT_TRUE(writer.unregister_instance(data.back(), instance_handle_2));
}

// Regression test for redmine issue #20239
TEST(KeyedTopic, DataWriterAlwaysSendTheSerializedKeyViaInlineQoS)
{
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();

    bool writer_sends_inline_qos = true;
    bool writer_sends_pid_key_hash = true;

    test_transport->drop_data_messages_filter_ = [&writer_sends_inline_qos,
                    &writer_sends_pid_key_hash](eprosima::fastdds::rtps::CDRMessage_t& msg) -> bool
            {
                // Check for inline_qos
                uint8_t flags = msg.buffer[msg.pos - 3];
                auto old_pos = msg.pos;

                // Skip extraFlags, read octetsToInlineQos, and calculate inline qos position.
                msg.pos += 2;
                uint16_t to_inline_qos = eprosima::fastdds::helpers::cdr_parse_u16(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 2;

                uint32_t inline_qos_pos = msg.pos + to_inline_qos;

                // Filters are only applied to user data
                // no need to check if the packets comer from a builtin

                writer_sends_inline_qos &= static_cast<bool>((flags & (1 << 1)));

                // Stop seeking if inline qos are not present
                // Fail the test afterwards
                if (!writer_sends_inline_qos)
                {
                    return false;
                }
                else
                {
                    // Process inline qos
                    msg.pos = inline_qos_pos;
                    bool key_hash_was_found = false;
                    while (msg.pos < msg.length)
                    {
                        uint16_t pid = eprosima::fastdds::helpers::cdr_parse_u16(
                            (char*)&msg.buffer[msg.pos]);
                        msg.pos += 2;
                        uint16_t plen = eprosima::fastdds::helpers::cdr_parse_u16(
                            (char*)&msg.buffer[msg.pos]);
                        msg.pos += 2;
                        uint32_t next_pos = msg.pos + plen;

                        if (pid == eprosima::fastdds::dds::PID_KEY_HASH)
                        {
                            key_hash_was_found = true;
                        }
                        else if (pid == eprosima::fastdds::dds::PID_SENTINEL)
                        {
                            break;
                        }

                        msg.pos = next_pos;
                    }

                    writer_sends_pid_key_hash &= key_hash_was_found;
                    msg.pos = old_pos;
                }

                // Do not drop the packet in any case
                return false;
            };

    writer.
            disable_builtin_transport().
            add_user_transport_to_pparams(test_transport).
            init();

    ASSERT_TRUE(writer.isInitialized());

    reader.
            expect_inline_qos(false).
            init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator(5);

    reader.startReception(data);
    writer.send(data);

    // In this test all data should be sent.
    EXPECT_TRUE(data.empty());
    reader.block_for_all();

    EXPECT_TRUE(writer_sends_inline_qos);
    EXPECT_TRUE(writer_sends_pid_key_hash);
}

/* Uncomment when DDS API supports NO_WRITERS_ALIVE
   TEST(KeyedTopic, WriteSamplesBestEffort)
   {
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.
    resource_limits_max_instances(1).
    reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
    init();

    ASSERT_TRUE(writer.isInitialized());

    reader.
    resource_limits_max_instances(1).
    reliability(eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS).
    init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances.
    auto instance_handle_1 = writer.register_instance(data.front());
    EXPECT_NE(instance_handle_1, eprosima::fastdds::rtps::c_InstanceHandle_Unknown);

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
    EXPECT_NE(instance_handle_2, eprosima::fastdds::rtps::c_InstanceHandle_Unknown);

    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(1);
   }

   TEST(KeyedTopic, WriteSamplesReliable)
   {
    PubSubWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.
    resource_limits_max_instances(1).
    reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
    init();

    ASSERT_TRUE(writer.isInitialized());

    reader.
    resource_limits_max_instances(1).
    reliability(eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS).
    init();

    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator(2);

    // Register instances.
    auto instance_handle_1 = writer.register_instance(data.front());
    EXPECT_NE(instance_handle_1, eprosima::fastdds::rtps::c_InstanceHandle_Unknown);

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
    EXPECT_EQ(instance_handle_2, eprosima::fastdds::rtps::c_InstanceHandle_Unknown);

    writer.waitForAllAcked(std::chrono::milliseconds(100));

    instance_handle_2 = writer.register_instance(data.back());
    EXPECT_NE(instance_handle_2, eprosima::fastdds::rtps::c_InstanceHandle_Unknown);

    writer.send(data);
    // In this test all data should be sent.
    ASSERT_TRUE(data.empty());
    // Block reader until reception finished or timeout.
    reader.block_for_at_least(1);
   }
 */
