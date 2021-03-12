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

#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/dds/subscriber/qos/DataReaderQos.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace statistics {

/*
 * This test checks that STATISTICS_DATAWRITER_QOS correctly sets the expected QoS.
 * 1. Reliability RELIABLE
 * 2. Durability TRANSIENT LOCAL
 * 3. Pull mode enabled
 * 4. Publication mode ASYNCHRONOUS
 * 5. History kind KEEP LAST
 * 6. History depth 100
 */
TEST(StatisticsQosTests, StatisticsDataWriterQosTest)
{
// TODO(jlbueno) Remove this guards after implementation. Here to prevent failures in current CI.
#ifdef FASTDDS_STATISTICS
    logError(STATISTICS_QOS_TEST, "This test is going to fail because API is not yet implemented.")

    EXPECT_TRUE(STATISTICS_DATAWRITER_QOS.reliability().kind == eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_TRUE(STATISTICS_DATAWRITER_QOS.durability().kind == eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    // TODO(jlbueno) Pull mode is not yet exposed in DDS API
    EXPECT_TRUE(STATISTICS_DATAWRITER_QOS.publish_mode().kind == eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_TRUE(STATISTICS_DATAWRITER_QOS.history().kind == eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_TRUE(STATISTICS_DATAWRITER_QOS.history().depth == 100);
#endif // FASTDDS_STATISTICS
}

/*
 * This test checks that STATISTICS_DATAREADER_QOS correctly sets the expected QoS.
 * 1. Reliability RELIABLE
 * 2. Durability TRANSIENT LOCAL
 * 3. History kind KEEP LAST
 * 4. History depth 100
 */
TEST(StatisticsQosTests, StatisticsDataReaderQosTest)
{
// TODO(jlbueno) Remove this guards after implementation. Here to prevent failures in current CI.
#ifdef FASTDDS_STATISTICS
    logError(STATISTICS_QOS_TEST, "This test is going to fail because API is not yet implemented.")

    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.reliability().kind == eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.durability().kind == eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.history().kind == eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.history().depth == 100);
#endif // FASTDDS_STATISTICS
}

} // namespace statistics
} // namespace dds
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    eprosima::fastdds::dds::Log::SetVerbosity(eprosima::fastdds::dds::Log::Error);

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    eprosima::fastdds::dds::Log::KillThread();
    return ret;
}
