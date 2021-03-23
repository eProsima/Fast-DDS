// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <stdlib.h>

#include <gtest/gtest.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

class StatisticsEnvironmentVariableTests : public eprosima::fastdds::dds::DomainParticipantFactory
{
public:

    const std::string& load_statistics_profiles_from_env()
    {
        return eprosima::fastdds::dds::DomainParticipantFactory::load_statistics_profiles_from_env();
    }
};

/*
 * This test checks eprosima::fastdds::dds::DomainParticipantFactory::load_statistics_profiles_from_env
 * 1. Call the method with the variable unset: returns empty string
 * 2. Set environment variable
 * 3. Check the data is correctly read
 */
TEST(StatisticsEnvironmentVariableTests, LoadStatisticsProfilesTests)
{
    StatisticsEnvironmentVariableTests domain_participant_factory;

    const char* value = "HISTORY_LATENCY_TOPIC;NETWORK_LATENCY_TOPIC;RTPS_LOST_TOPIC";

    // 1. Environment variable yet unset
    EXPECT_EQ(0, strcmp("", domain_participant_factory.load_statistics_profiles_from_env().c_str()));

    // 2. Set FASTDDS_STATISTICS environment variable
#ifdef _WIN32
    ASSERT_EQ(0, _put_env_s(FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE, value));
#else
    ASSERT_EQ(0, setenv(FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE, value, 1));
#endif

    // 3. Read environment variable
    EXPECT_EQ(0, strcmp(value, domain_participant_factory.load_statistics_profiles_from_env().c_str()));
}

} // namespace dds
} // namespace statistics
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

