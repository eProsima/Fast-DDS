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

#include <gtest/gtest.h>

#include <fastdds/rtps/RTPSDomain.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace testing;

RTPSParticipant* transport_size_participant_init(
        uint32_t max_message_size)
{
    uint32_t domain_id = 0;
    std::string max_message_size_str = std::to_string(max_message_size);

    RTPSParticipantAttributes p_attr;
    BuiltinTransportsOptions options;
    options.maxMessageSize = max_message_size;
    p_attr.setup_transports(BuiltinTransports::SHM, options);
    RTPSParticipant* participant = RTPSDomain::createParticipant(
        domain_id, true, p_attr);

    return participant;
}

/**
 * This test checks that the participant is not created when the max message size is smaller than the PDP package size
 * but it is properly created when the max message size is bigger than the PDP package size.
 */
TEST(RTPSParticipantTests, participant_creation_message_size)
{
    ASSERT_EQ(transport_size_participant_init(100), nullptr);
    ASSERT_NE(transport_size_participant_init(1000), nullptr);
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
