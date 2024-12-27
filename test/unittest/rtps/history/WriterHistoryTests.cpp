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

#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/history/WriterHistory.h>


namespace eprosima {
namespace fastrtps {
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

    CacheChange_t* change = writer->new_change(ALIVE);
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
