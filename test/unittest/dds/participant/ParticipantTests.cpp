// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/core/types.hpp>
#include <dds/domain/find.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

TEST(ParticipantTests, CreateDomainParticipant)
{
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0);

    ASSERT_NE(participant, nullptr);

}

TEST(ParticipantTests, CreatePSMDomainParticipant)
{
    ::dds::domain::DomainParticipant participant = ::dds::core::null;
    participant = ::dds::domain::DomainParticipant(0, PARTICIPANT_QOS_DEFAULT);

    ASSERT_NE(participant, ::dds::core::null);
}

TEST(ParticipantTests, LookupDomainParticipant)
{
    DomainId_t id = 0;

    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(id);

    DomainParticipant* participant2 = DomainParticipantFactory::get_instance()->lookup_participant(id);

    ASSERT_NE(participant2, nullptr);
    ASSERT_EQ(participant, participant2);

}

TEST(ParticipantTests, LookupDomainParticipantNoCreatedParticipant)
{
    DomainParticipant* participant2 = DomainParticipantFactory::get_instance()->lookup_participant(0);

    ASSERT_EQ(participant2, nullptr);
}

TEST(ParticipantTests, FindPSMDomainParticipant)
{
    uint32_t domain_id = 0;

    ::dds::domain::DomainParticipant participant = ::dds::domain::DomainParticipant(domain_id, PARTICIPANT_QOS_DEFAULT);

    ::dds::domain::DomainParticipant participant2 = ::dds::domain::find(domain_id);

    ASSERT_NE(participant2, ::dds::core::null);
    ASSERT_EQ(*participant.delegate().get(), *participant2.delegate().get());

}

TEST(ParticipantTests, FindPSMDomainParticipantNoCreatedParticipant)
{
    uint32_t domain_id = 0;

    ::dds::domain::DomainParticipant participant = ::dds::domain::find(domain_id);

    ASSERT_EQ(participant, ::dds::core::null);

}

} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
