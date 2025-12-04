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

#include <memory>
#include <mutex>

#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>

#include "../types/HelloWorldPubSubTypes.hpp"
#include "../types/KeyedHelloWorldPubSubTypes.hpp"
#include "../utils/filter_helpers.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"
#include "UDPMessageSender.hpp"

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

// Check that compute_key is called with a key-only payload when KEY_HASH is not present
TEST(KeyedTopic, key_only_payload)
{
    using namespace eprosima::fastdds::dds;
    using namespace eprosima::fastdds::rtps;

    struct TestTypeSupport : public KeyedHelloWorldPubSubType
    {
        typedef KeyedHelloWorldPubSubType::type type;

        bool compute_key(
                eprosima::fastdds::rtps::SerializedPayload_t& payload,
                eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
                bool force_md5 = false) override
        {
            if (payload.is_serialized_key)
            {
                // Count the number of times compute_key is called with a key-only payload
                std::lock_guard<std::mutex> lock(mtx_);
                key_only_payload_count_++;
                cv_.notify_all();
            }

            return KeyedHelloWorldPubSubType::compute_key(payload, ihandle, force_md5);
        }

        bool compute_key(
                const void* const data,
                eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
                bool force_md5 = false) override
        {
            return KeyedHelloWorldPubSubType::compute_key(data, ihandle, force_md5);
        }

        uint32_t nb_of_times_called_with_key_only_payload() const
        {
            std::lock_guard<std::mutex> lock(mtx_);
            return key_only_payload_count_;
        }

        void wait_for_key_only_payload(
                std::uint32_t min_value)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this, min_value]()
                    {
                        return key_only_payload_count_ >= min_value;
                    });
        }

    private:

        mutable std::mutex mtx_;
        std::condition_variable cv_;
        std::uint32_t key_only_payload_count_ = 0;
    };

    // Force using UDP transport
    auto udp_transport = std::make_shared<UDPv4TransportDescriptor>();

    PubSubWriter<TestTypeSupport> writer(TEST_TOPIC_NAME);
    PubSubReader<TestTypeSupport> reader(TEST_TOPIC_NAME);

    // Set custom reader locator so we can send hand-crafted data to a known location
    Locator_t reader_locator;
    ASSERT_TRUE(IPLocator::setIPv4(reader_locator, "127.0.0.1"));
    reader_locator.port = 7000;
    reader.add_to_unicast_locator_list("127.0.0.1", 7000);

    reader.disable_builtin_transport().
            add_user_transport_to_pparams(udp_transport).
            init();
    ASSERT_TRUE(reader.isInitialized());

    writer.disable_builtin_transport().
            add_user_transport_to_pparams(udp_transport).
            init();
    ASSERT_TRUE(writer.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    auto data = default_keyedhelloworld_data_generator(2);
    reader.startReception(data);
    // Send data
    writer.send(data);
    EXPECT_TRUE(data.empty());
    reader.block_for_all();

    // Check that compute_key was not called with a key-only payload when KEY_HASH is present
    auto ts = std::dynamic_pointer_cast<TestTypeSupport>(reader.get_type_support());
    ASSERT_TRUE(ts != nullptr);
    EXPECT_EQ(ts->nb_of_times_called_with_key_only_payload(), 0u);

    UDPMessageSender fake_msg_sender;

    // Send hand-crafted data
    {
        auto writer_guid = writer.datawriter_guid();

        struct KeyOnlyPayloadPacket
        {
            std::array<char, 4> rtps_id{ {'R', 'T', 'P', 'S'} };
            std::array<uint8_t, 2> protocol_version{ {2, 3} };
            std::array<uint8_t, 2> vendor_id{ {0x01, 0x0F} };
            GuidPrefix_t sender_prefix{};

            struct DataSubMsg
            {
                struct Header
                {
                    uint8_t submessage_id = 0x15;
    #if FASTDDS_IS_BIG_ENDIAN_TARGET
                    uint8_t flags = 0x08;
    #else
                    uint8_t flags = 0x09;
    #endif  // FASTDDS_IS_BIG_ENDIAN_TARGET
                    uint16_t octets_to_next_header = 28;
                    uint16_t extra_flags = 0;
                    uint16_t octets_to_inline_qos = 16;
                    EntityId_t reader_id{};
                    EntityId_t writer_id{};
                    SequenceNumber_t sn{ 3 };
                };

                struct SerializedData
                {
                    uint8_t encapsulation[2] = {0x00, CDR_LE};
                    uint8_t encapsulation_opts[2] = {0x00, 0x00};
                    uint8_t data[4] = {0x0A, 0x00, 0x00, 0x00};
                };

                Header header;
                SerializedData payload;
            }
            data;
        };

        KeyOnlyPayloadPacket key_only_packet{};
        key_only_packet.sender_prefix = writer_guid.guidPrefix;
        key_only_packet.data.header.writer_id = writer_guid.entityId;
        key_only_packet.data.header.reader_id = reader.datareader_guid().entityId;

        CDRMessage_t msg(0);
        uint32_t msg_len = static_cast<uint32_t>(sizeof(key_only_packet));
        msg.init(reinterpret_cast<octet*>(&key_only_packet), msg_len);
        msg.length = msg_len;
        msg.pos = msg_len;
        fake_msg_sender.send(msg, reader_locator);
    }

    // Wait for key-only compute key to be called
    ts->wait_for_key_only_payload(1);

    // Send hand-crafted data frags
    {
        auto writer_guid = writer.datawriter_guid();

        struct KeyOnlyPayloadTwoFragmentsPacket
        {
            std::array<char, 4> rtps_id{ {'R', 'T', 'P', 'S'} };
            std::array<uint8_t, 2> protocol_version{ {2, 3} };
            std::array<uint8_t, 2> vendor_id{ {0x01, 0x0F} };
            GuidPrefix_t sender_prefix{};

            struct DataFragSubMsg
            {
                struct Header
                {
                    uint8_t submessage_id = 0x16;
    #if FASTDDS_IS_BIG_ENDIAN_TARGET
                    uint8_t flags = 0x04;
    #else
                    uint8_t flags = 0x05;
    #endif  // FASTDDS_IS_BIG_ENDIAN_TARGET
                    uint16_t octets_to_next_header = 36;
                    uint16_t extra_flags = 0;
                    uint16_t octets_to_inline_qos = 28;
                    EntityId_t reader_id{};
                    EntityId_t writer_id{};
                    SequenceNumber_t sn{ 4 };
                    uint32_t fragment_starting_num = 1;
                    uint16_t fragments_in_submessage = 1;
                    uint16_t fragment_size = 4;
                    uint32_t sample_size = 8;
                };

                struct SerializedData
                {
                    uint8_t data[4] = {0x00, 0x00, 0x00, 0x00};
                };

                Header header;
                SerializedData payload;
            }
            data[2];
        };

        KeyOnlyPayloadTwoFragmentsPacket fragments_packet{};
        fragments_packet.sender_prefix = writer_guid.guidPrefix;

        // First fragment with encapsulation header
        fragments_packet.data[0].header.writer_id = writer_guid.entityId;
        fragments_packet.data[0].header.reader_id = reader.datareader_guid().entityId;
        fragments_packet.data[0].header.fragment_starting_num = 1;
        fragments_packet.data[0].payload.data[1] = CDR_LE;

        // Second fragment with serialized key
        fragments_packet.data[1].header.writer_id = writer_guid.entityId;
        fragments_packet.data[1].header.reader_id = reader.datareader_guid().entityId;
        fragments_packet.data[1].header.fragment_starting_num = 2;
        fragments_packet.data[1].payload.data[0] = 0x0B;

        CDRMessage_t msg(0);
        uint32_t msg_len = static_cast<uint32_t>(sizeof(fragments_packet));
        msg.init(reinterpret_cast<octet*>(&fragments_packet), msg_len);
        msg.length = msg_len;
        msg.pos = msg_len;
        fake_msg_sender.send(msg, reader_locator);
    }

    // Wait for key-only compute key to be called
    ts->wait_for_key_only_payload(2);
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
