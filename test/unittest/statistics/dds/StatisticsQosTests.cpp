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

#include <string>

#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/attributes/PropertyPolicy.h>
#include <fastdds/statistics/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/statistics/dds/subscriber/qos/DataReaderQos.hpp>

namespace eprosima {
namespace fastdds {
namespace statistics {
namespace dds {

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
    EXPECT_TRUE(STATISTICS_DATAWRITER_QOS.reliability().kind == eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_TRUE(STATISTICS_DATAWRITER_QOS.durability().kind == eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    const std::string* pushMode_property = eprosima::fastrtps::rtps::PropertyPolicyHelper::find_property(
            STATISTICS_DATAWRITER_QOS.properties(), "pushMode");
    EXPECT_NE(pushMode_property, nullptr);
    EXPECT_EQ(pushMode_property->compare("false"), 0);
    EXPECT_TRUE(STATISTICS_DATAWRITER_QOS.publish_mode().kind == eprosima::fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE);
    EXPECT_TRUE(STATISTICS_DATAWRITER_QOS.history().kind == eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_TRUE(STATISTICS_DATAWRITER_QOS.history().depth == 100);
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
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.reliability().kind == eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.durability().kind == eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.history().kind == eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS);
    EXPECT_TRUE(STATISTICS_DATAREADER_QOS.history().depth == 100);
}

} // namespace dds
} // namespace statistics
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
