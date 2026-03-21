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

#include <gtest/gtest.h>

#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>


namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace testing;

#define MAX_MESSAGE_SIZE 300

void cache_change_fragment(
        uint32_t max_message_size,
        uint32_t inline_qos_length,
        bool expected_fragmentation)
{
    uint32_t domain_id = 0;
    uint32_t initial_reserved_caches = 10;
    std::string max_message_size_str = std::to_string(max_message_size);

    RTPSParticipantAttributes p_attr;
    p_attr.properties.properties().emplace_back("fastdds.max_message_size", max_message_size_str);
    RTPSParticipant* participant = RTPSDomain::createParticipant(
        domain_id, true, p_attr);

    ASSERT_NE(participant, nullptr);

    HistoryAttributes h_attr;
    h_attr.memoryPolicy = DYNAMIC_RESERVE_MEMORY_MODE;
    h_attr.initialReservedCaches = initial_reserved_caches;
    h_attr.payloadMaxSize = 250;
    WriterHistory* history = new WriterHistory(h_attr);

    WriterAttributes w_attr;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(participant, w_attr, history);

    ASSERT_NE(writer, nullptr);

    CacheChange_t* change = history->create_change(ALIVE);
    if (expected_fragmentation)
    {
        change->serializedPayload.length = 3 * max_message_size;
    }
    else
    {
        change->serializedPayload.length = max_message_size / 3;
    }
    change->inline_qos.length = inline_qos_length;
    history->add_change(change);

    auto result = change->getFragmentSize();
    std::cout << "Fragment size: " << result << std::endl;
    if (expected_fragmentation)
    {
        ASSERT_NE(result, 0);
    }
    else
    {
        ASSERT_EQ(result, 0);
    }
}

/**
 * This test checks the get_max_allowed_payload_size() method of the BaseWriter class.
 * When setting the RTPS Participant Attribute property fastdds.max_message_size to a value lower than the
 * message overhead, if the method does not overflow the fragment size will be set.
 * If the max_message_size is big enough for the overhead, inline_qos and serializedPayload,
 * then no fragmentation will occur.
 */
TEST(WriterHistoryTests, get_max_allowed_payload_size_overflow)
{
    cache_change_fragment(100, 0, true);
    cache_change_fragment(MAX_MESSAGE_SIZE, 0, false);
}

/**
 * This test checks the fragment size calculation for a cache change depending on the inline qos length.
 * The change.serializedPayload.length is set to 3 times the max_allowed_payload_size, so the fragment size should always be set.
 * In case of an overflow in the attribute high_mark_for_frag_ the fragment size will not be set, which is an error.
 */
TEST(WriterHistoryTests, final_high_mark_for_frag_overflow)
{
    for (uint32_t inline_qos_length = 0; inline_qos_length < MAX_MESSAGE_SIZE; inline_qos_length += 40)
    {
        cache_change_fragment(MAX_MESSAGE_SIZE, inline_qos_length, true);
    }
}

TEST(WriterHistoryTests, add_change_with_undefined_instance_handle_and_no_payload)
{
    uint32_t domain_id = 0;

    RTPSParticipantAttributes p_attr;
    RTPSParticipant* participant = RTPSDomain::createParticipant(
        domain_id, true, p_attr);

    ASSERT_NE(participant, nullptr);

    HistoryAttributes h_attr;
    WriterHistory* history = new WriterHistory(h_attr);

    WriterAttributes w_attr;
    // The topic must be keyed to use instance handles
    w_attr.endpoint.topicKind = WITH_KEY;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(participant, w_attr, history);

    ASSERT_NE(writer, nullptr);

    // Any of these changes have a well defined instance handle
    CacheChange_t* change = history->create_change(ALIVE);
    // Valid because ALIVE changes don't enforce defined instance handles
    ASSERT_TRUE(history->add_change(change));
    // Rest are invalid because instance handle is enforced to be defined
    change = history->create_change(NOT_ALIVE_DISPOSED);
    ASSERT_FALSE(history->add_change(change));
    change = history->create_change(NOT_ALIVE_UNREGISTERED);
    ASSERT_FALSE(history->add_change(change));
    change = history->create_change(NOT_ALIVE_DISPOSED_UNREGISTERED);
    ASSERT_FALSE(history->add_change(change));
}

TEST(WriterHistoryTests, add_change_with_defined_instance_handle_and_no_payload)
{
    uint32_t domain_id = 0;

    RTPSParticipantAttributes p_attr;
    RTPSParticipant* participant = RTPSDomain::createParticipant(
        domain_id, true, p_attr);

    ASSERT_NE(participant, nullptr);

    HistoryAttributes h_attr;
    WriterHistory* history = new WriterHistory(h_attr);

    WriterAttributes w_attr;
    // The topic must be keyed to use instance handles
    w_attr.endpoint.topicKind = WITH_KEY;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(participant, w_attr, history);

    ASSERT_NE(writer, nullptr);

    // All these changes have a well defined instance handle and they must be added successfully
    CacheChange_t* change = history->create_change(ALIVE);
    // Setting any value makes the handle defined because of its = operator
    change->instanceHandle.value[0] = 1;
    ASSERT_TRUE(history->add_change(change));
    change = history->create_change(NOT_ALIVE_DISPOSED);
    change->instanceHandle.value[0] = 1;
    ASSERT_TRUE(history->add_change(change));
    change = history->create_change(NOT_ALIVE_UNREGISTERED);
    change->instanceHandle.value[0] = 1;
    ASSERT_TRUE(history->add_change(change));
    change = history->create_change(NOT_ALIVE_DISPOSED_UNREGISTERED);
    change->instanceHandle.value[0] = 1;
    ASSERT_TRUE(history->add_change(change));
}

TEST(WriterHistoryTests, add_change_with_payload_but_undefined_handle)
{
    uint32_t domain_id = 0;

    RTPSParticipantAttributes p_attr;
    RTPSParticipant* participant = RTPSDomain::createParticipant(
        domain_id, true, p_attr);

    ASSERT_NE(participant, nullptr);

    HistoryAttributes h_attr;
    WriterHistory* history = new WriterHistory(h_attr);

    WriterAttributes w_attr;
    // The topic must be keyed to use instance handles
    w_attr.endpoint.topicKind = WITH_KEY;
    RTPSWriter* writer = RTPSDomain::createRTPSWriter(participant, w_attr, history);

    ASSERT_NE(writer, nullptr);

    CacheChange_t* change = history->create_change(ALIVE);
    // This len simulates a payload
    change->serializedPayload.length = 10;
    ASSERT_TRUE(history->add_change(change));
    change = history->create_change(NOT_ALIVE_DISPOSED);
    change->serializedPayload.length = 10;
    ASSERT_TRUE(history->add_change(change));
    change = history->create_change(NOT_ALIVE_UNREGISTERED);
    change->serializedPayload.length = 10;
    ASSERT_TRUE(history->add_change(change));
    change = history->create_change(NOT_ALIVE_DISPOSED_UNREGISTERED);
    change->serializedPayload.length = 10;
    ASSERT_TRUE(history->add_change(change));
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
