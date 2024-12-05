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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/rtps/writer/RTPSWriter.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>
#include <fastdds/rtps/history/WriterHistory.hpp>

#include <rtps/writer/BaseWriter.hpp>


namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace testing;

void max_allowed_payload_size(
    uint32_t max_message_size)
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

    BaseWriter* bwriter = BaseWriter::downcast(writer);

    auto result = bwriter->get_max_allowed_payload_size();
    std::cout << "For max_message_size: " << max_message_size << " the max allowed payload size is: " << result << std::endl;

    ASSERT_LE(result, max_message_size);
}

/**
 * This test checks the get_max_allowed_payload_size() method of the BaseWriter.
 * The method is called within a loop with different values of max_message_size.
 * The test checks that the max_payload_size is always less than max_message_size, 
 * in other case it means an overflow has occurred.
 */
TEST(BaseWriterTests, calculate_max_payload_size_overflow)
{
    for (uint32_t max_message_size = 200; max_message_size > 150; max_message_size -= 4)
    {
        max_allowed_payload_size(max_message_size);
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
