// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include "RTPSWithRegistrationReader.hpp"
#include "RTPSWithRegistrationWriter.hpp"
#include "UDPMessageSender.hpp"

/**
 * Checks that an RTPS Writer does not serialize the KEY_HASH inline QoS parameter
 * when the instance handle provided at write time is undefined.
 */
TEST(RTPSKeyedTopic, DataWriterDoesNotSendTheSerializedKeyWhenInstanceUndefined)
{
    using namespace eprosima::fastdds::dds;
    using namespace eprosima::fastdds::rtps;

    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();

    bool writer_sends_inline_qos = true;
    bool writer_sends_pid_key_hash = true;

    // Custom transport layer to inspect outgoing messages
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

    RTPSWithRegistrationWriter<KeyedHelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    RTPSWithRegistrationReader<KeyedHelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    writer.disable_builtin_transport().add_user_transport_to_pparams(test_transport).init();
    ASSERT_TRUE(writer.isInitialized());
    reader.init();
    ASSERT_TRUE(reader.isInitialized());

    // Wait for discovery.
    writer.wait_discovery();
    reader.wait_discovery();

    // Generate a sample
    auto data = default_keyedhelloworld_data_generator(1);
    // Sending a defined handle in KEY_HASH inline qos
    InstanceHandle_t handle;
    handle.value[0] = 1;
    writer.send_sample(data.front(), handle);
    // Message must contain pid KEY_HASH
    EXPECT_TRUE(writer_sends_inline_qos);
    EXPECT_TRUE(writer_sends_pid_key_hash);
    // Now sending an undefined handle
    handle.clear();
    writer.send_sample(data.front(), handle);
    // The writer should not send pid KEY_HASH as it was not defined
    // NOTE: InlineQoS could still contain other parameters it is enough
    // that either inline qos is not sent or pid key hash is not sent
    EXPECT_TRUE(!writer_sends_inline_qos || !writer_sends_pid_key_hash);
}
