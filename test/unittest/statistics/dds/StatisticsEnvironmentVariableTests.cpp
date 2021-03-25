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

#include <string>

#include <gtest/gtest.h>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>

#include <statistics/fastdds/domain/DomainParticipant.cpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

/*
 * This test checks eprosima::fastdds::dds::DomainParticipantFactory::load_statistics_profiles_from_env
 * 1. Call the method with the variable unset: returns empty string
 * 2. Set environment variable
 * 3. Check the data is correctly read
 */
TEST(StatisticsEnvironmentVariableTests, LoadStatisticsProfilesTests)
{
    const char* value = "HISTORY_LATENCY_TOPIC;NETWORK_LATENCY_TOPIC;RTPS_LOST_TOPIC";
    std::string topics;

    // 1. Environment variable yet unset
    load_statistics_profiles_from_env(topics);
    EXPECT_EQ(0, strcmp("", topics.c_str()));

    // 2. Set FASTDDS_STATISTICS environment variable
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s(FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE, value));
#else
    ASSERT_EQ(0, setenv(FASTDDS_STATISTICS_ENVIRONMENT_VARIABLE, value, 1));
#endif // ifdef _WIN32

    // 3. Read environment variable
    load_statistics_profiles_from_env(topics);
    EXPECT_EQ(0, strcmp(value, topics.c_str()));
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
