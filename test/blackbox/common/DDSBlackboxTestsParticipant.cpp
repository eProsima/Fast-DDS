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

#include <fastdds/dds/core/policy/ParameterTypes.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/rtps/common/CDRMessage_t.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>
#include <fastdds/rtps/transport/test_UDPv4TransportDescriptor.hpp>
#include <gtest/gtest.h>

#include "../utils/filter_helpers.hpp"
#include "BlackboxTests.hpp"
#include "PubSubReader.hpp"
#include "PubSubWriter.hpp"

auto check_qos_in_data_p =
        [](eprosima::fastdds::rtps::CDRMessage_t& msg, std::atomic<uint8_t>& qos_found,
                std::vector<uint16_t>& expected_qos_pids)
        {
            uint32_t qos_size = 0;
            uint32_t original_pos = msg.pos;
            bool is_sentinel = false;

            while (!is_sentinel)
            {
                msg.pos = original_pos + qos_size;

                uint16_t pid = eprosima::fastdds::helpers::cdr_parse_u16(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 2;
                uint16_t plength = eprosima::fastdds::helpers::cdr_parse_u16(
                    (char*)&msg.buffer[msg.pos]);
                msg.pos += 2;
                bool valid = true;

                // If inline_qos submessage is found we will have an additional Sentinel
                if (pid == eprosima::fastdds::dds::PID_SENTINEL)
                {
                    // PID_SENTINEL is always considered of length 0
                    plength = 0;
                    // If the PID is not inline qos, then we need to set the sentinel
                    // to true, as it is the last PID
                    is_sentinel = true;
                }

                qos_size += (4 + plength);

                // Align to 4 byte boundary and prepare for next iteration
                qos_size = (qos_size + 3) & ~3;

                if (!valid || ((msg.pos + plength) > msg.length))
                {
                    return false;
                }
                else if (!is_sentinel)
                {
                    if (pid == eprosima::fastdds::dds::PID_WIREPROTOCOL_CONFIG)
                    {
                        std::cout << "WireProtocolConfig found" << std::endl;
                        qos_found.fetch_add(1u, std::memory_order_seq_cst);
                    }
                    // Delete the PID from the expected list if present
                    expected_qos_pids.erase(
                        std::remove(expected_qos_pids.begin(), expected_qos_pids.end(), pid),
                        expected_qos_pids.end());
                }
            }

            // Do not drop the packet in any case
            return false;
        };

// This tests checks that non-default optional QoS are correctly sent in the Data(p)
// QoS that should be sent:
// - WireProtocolConfigQos
// a) The test is run with the property set to false, so the optional QoS are not serialized.
// b) The test is run with the property set to true, so the optional QoS are serialized.
// Note: In a Participant, if the property 'fastdds.serialize_optional_qos' is set to true,
// the optional QoS are always serialized.
TEST(DDSParticipant, participant_sends_non_default_qos_optional)
{

    std::atomic<uint8_t> qos_found { 0 };
    std::vector<uint16_t> expected_qos_pids = {
        eprosima::fastdds::dds::PID_WIREPROTOCOL_CONFIG
    };
    const uint8_t expected_qos_size = static_cast<uint8_t>(expected_qos_pids.size());

    PubSubWriter<HelloWorldPubSubType> writer(TEST_TOPIC_NAME);
    PubSubReader<HelloWorldPubSubType> reader(TEST_TOPIC_NAME);

    auto test_transport = std::make_shared<eprosima::fastdds::rtps::test_UDPv4TransportDescriptor>();
    test_transport->drop_builtin_data_messages_filter_ = [&](eprosima::fastdds::rtps::CDRMessage_t& msg)
            {
                return check_qos_in_data_p(msg, qos_found, expected_qos_pids);
            };

    // Default participant QoS
    writer.disable_builtin_transport()
            .add_user_transport_to_pparams(test_transport);

    // a) Init both entities without setting the property
    writer.init();
    reader.init();
    ASSERT_TRUE(writer.isInitialized());
    ASSERT_TRUE(reader.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    // No optional QoS should be sent.
    EXPECT_EQ(qos_found.load(), 0u);
    EXPECT_EQ(expected_qos_pids.size(), expected_qos_size);

    // b) Now set the property to serialize optional QoS and re-init the writer
    writer.destroy();
    reader.wait_writer_undiscovery();
    qos_found.store(0);

    eprosima::fastdds::dds::PropertyPolicyQos properties;
    properties.properties().emplace_back("fastdds.serialize_optional_qos", "true");
    writer.property_policy(properties);

    writer.init();
    ASSERT_TRUE(writer.isInitialized());

    writer.wait_discovery();
    reader.wait_discovery();

    // Check that the optional QoS are serialized
    // It may be found more than one time due to initial announcements
    EXPECT_GE(qos_found.load(), expected_qos_size);
    EXPECT_EQ(expected_qos_pids.size(), 0u);
}
